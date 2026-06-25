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

namespace {

bool parse_size_value(const std::string& text, std::size_t& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

bool parse_int_value(const std::string& text, int& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

bool parse_double_value(const std::string& text, double& value) {
    if (text.empty() || std::isspace(static_cast<unsigned char>(text.front()))) {
        return false;
    }
    char* end = nullptr;
    errno = 0;
    const double parsed = std::strtod(text.c_str(), &end);
    if (errno != 0 || end != text.c_str() + text.size()) {
        return false;
    }
    value = parsed;
    return true;
}

std::string lowercase_copy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::optional<bool> parse_bool_value(std::string text) {
    text = lowercase_copy(std::move(text));
    if (text == "true" || text == ".t." || text == "t" || text == "1" || text == "yes" || text == "on") {
        return true;
    }
    if (text == "false" || text == ".f." || text == "f" || text == "0" || text == "no" || text == "off") {
        return false;
    }
    return std::nullopt;
}

std::optional<StudioEditorSelectionContext> parse_selection_context_token(std::string token) {
    token = lowercase_copy(std::move(token));
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::visual_object)) {
        return StudioEditorSelectionContext::visual_object;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::visual_method)) {
        return StudioEditorSelectionContext::visual_method;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::container_object)) {
        return StudioEditorSelectionContext::container_object;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::class_designer)) {
        return StudioEditorSelectionContext::class_designer;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::report_expression)) {
        return StudioEditorSelectionContext::report_expression;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::label_expression)) {
        return StudioEditorSelectionContext::label_expression;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::menu_item)) {
        return StudioEditorSelectionContext::menu_item;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::project_item)) {
        return StudioEditorSelectionContext::project_item;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::data_environment)) {
        return StudioEditorSelectionContext::data_environment;
    }
    return std::nullopt;
}

const localization::LocalizedCatalog& default_launch_catalog() {
    static const localization::LocalizedCatalog catalog = localization::load_catalogs(
        localization::resolve_catalog_root(),
        localization::default_locale);
    return catalog;
}

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
    return catalog.translate(
        "StudioHost.LaunchParse.Error.SelectionContextValueRequired",
        {
            {"option", "--selection-context"},
            {"allowedValues",
                "visual_object, visual_method, container_object, class_designer, report_expression, "
                "label_expression, menu_item, project_item, or data_environment"}
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
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.RequestItemRequiresOptionAfterTargetSelector",
        {
            {"itemName", std::string(item_name)},
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

bool parse_form_set_class_argument(const std::string& argument,
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
            error = "Missing value after --form-set-class.";
            return true;
        }
        result.request.form_set_class = args[++index];
        result.request.form_set_class_available = true;
        return true;
    }

    if (argument == "--form-set-class-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --form-set-class-target-object-name.";
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
            error = "Missing value after --form-set-class-target-unique-id.";
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
            error = "Missing value after --default-file-path.";
            return true;
        }
        result.request.default_file_path = args[++index];
        result.request.default_file_path_available = true;
        return true;
    }

    if (argument == "--default-file-path-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --default-file-path-target-object-name.";
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
            error = "Missing value after --default-file-path-target-unique-id.";
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
            error = "Missing value after --initial-selected-alias.";
            return true;
        }
        result.request.initial_selected_alias = args[++index];
        result.request.initial_selected_alias_available = true;
        return true;
    }

    if (argument == "--initial-selected-alias-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --initial-selected-alias-target-object-name.";
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
            error = "Missing value after --initial-selected-alias-target-unique-id.";
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
            error = "Missing value after --tab-orientation.";
            return true;
        }
        int tab_orientation = 0;
        if (!parse_int_value(args[++index], tab_orientation)) {
            error = "The --tab-orientation value must be an integer.";
            return true;
        }
        if (tab_orientation < 0) {
            error = "The --tab-orientation value must not be negative.";
            return true;
        }
        result.request.tab_orientation = tab_orientation;
        result.request.tab_orientation_available = true;
        return true;
    }

    if (argument == "--tab-orientation-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --tab-orientation-target-object-name.";
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
            error = "Missing value after --tab-orientation-target-unique-id.";
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
            error = "Missing value after --display-orientation.";
            return true;
        }
        int display_orientation = 0;
        if (!parse_int_value(args[++index], display_orientation)) {
            error = "The --display-orientation value must be an integer.";
            return true;
        }
        if (display_orientation < 0) {
            error = "The --display-orientation value must not be negative.";
            return true;
        }
        result.request.display_orientation = display_orientation;
        result.request.display_orientation_available = true;
        return true;
    }

    if (argument == "--display-orientation-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --display-orientation-target-object-name.";
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
            error = "Missing value after --display-orientation-target-unique-id.";
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
            error = "Missing value after --help-context-id.";
            return true;
        }
        int help_context_id = 0;
        if (!parse_int_value(args[++index], help_context_id)) {
            error = "The --help-context-id value must be an integer.";
            return true;
        }
        if (help_context_id < 0) {
            error = "The --help-context-id value must not be negative.";
            return true;
        }
        result.request.help_context_id = help_context_id;
        result.request.help_context_id_available = true;
        return true;
    }

    if (argument == "--help-context-id-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --help-context-id-target-object-name.";
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
            error = "Missing value after --help-context-id-target-unique-id.";
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
            error = "Missing value after --whats-this-help-id.";
            return true;
        }
        int whats_this_help_id = 0;
        if (!parse_int_value(args[++index], whats_this_help_id)) {
            error = "The --whats-this-help-id value must be an integer.";
            return true;
        }
        if (whats_this_help_id < 0) {
            error = "The --whats-this-help-id value must not be negative.";
            return true;
        }
        result.request.whats_this_help_id = whats_this_help_id;
        result.request.whats_this_help_id_available = true;
        return true;
    }

    if (argument == "--whats-this-help-id-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --whats-this-help-id-target-object-name.";
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
            error = "Missing value after --whats-this-help-id-target-unique-id.";
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
            error = "Missing value after --whats-this-help.";
            return true;
        }
        const auto value = parse_bool_value(args[++index]);
        if (!value.has_value()) {
            error = "The --whats-this-help value must be a logical value.";
            return true;
        }
        result.request.whats_this_help = *value;
        result.request.whats_this_help_available = true;
        return true;
    }

    if (argument == "--whats-this-help-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --whats-this-help-target-object-name.";
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
            error = "Missing value after --whats-this-help-target-unique-id.";
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
            error = "Missing value after --whats-this-button.";
            return true;
        }
        const auto value = parse_bool_value(args[++index]);
        if (!value.has_value()) {
            error = "The --whats-this-button value must be a logical value.";
            return true;
        }
        result.request.whats_this_button = *value;
        result.request.whats_this_button_available = true;
        return true;
    }

    if (argument == "--whats-this-button-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = "Missing value after --whats-this-button-target-object-name.";
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
            error = "Missing value after --whats-this-button-target-unique-id.";
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

std::optional<std::string> validate_form_set_class_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.form_set_class_object && !request.form_set_class_available) {
        return localized_object_assignment_requires_option(catalog, "form set class", "--form-set-class");
    }
    if (request.form_set_class_object && request.form_set_class_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "form set class");
    }
    if (!request.form_set_class_object &&
        (request.form_set_class_available ||
         !request.form_set_class_objects.empty())) {
        return localized_object_arguments_require_mode(catalog, "Form-set-class", "--form-set-class-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_default_file_path_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.default_file_path_object && !request.default_file_path_available) {
        return localized_object_assignment_requires_option(catalog, "default file path", "--default-file-path");
    }
    if (request.default_file_path_object && request.default_file_path_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "default file path");
    }
    if (!request.default_file_path_object &&
        (request.default_file_path_available ||
         !request.default_file_path_objects.empty())) {
        return localized_object_arguments_require_mode(catalog, "Default-file-path", "--default-file-path-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_initial_selected_alias_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.initial_selected_alias_object && !request.initial_selected_alias_available) {
        return localized_object_assignment_requires_option(
            catalog,
            "initial selected alias",
            "--initial-selected-alias");
    }
    if (request.initial_selected_alias_object && request.initial_selected_alias_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "initial selected alias");
    }
    if (!request.initial_selected_alias_object &&
        (request.initial_selected_alias_available ||
         !request.initial_selected_alias_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            "Initial-selected-alias",
            "--initial-selected-alias-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_tab_orientation_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.tab_orientation_object && !request.tab_orientation_available) {
        return localized_object_assignment_requires_option(catalog, "tab orientation", "--tab-orientation");
    }
    if (request.tab_orientation_object && request.tab_orientation_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "tab orientation");
    }
    if (!request.tab_orientation_object &&
        (request.tab_orientation_available ||
         !request.tab_orientation_objects.empty())) {
        return localized_object_arguments_require_mode(catalog, "Tab-orientation", "--tab-orientation-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_display_orientation_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.display_orientation_object && !request.display_orientation_available) {
        return localized_object_assignment_requires_option(catalog, "display orientation", "--display-orientation");
    }
    if (request.display_orientation_object && request.display_orientation_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "display orientation");
    }
    if (!request.display_orientation_object &&
        (request.display_orientation_available ||
         !request.display_orientation_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            "Display-orientation",
            "--display-orientation-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_help_context_id_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.help_context_id_object && !request.help_context_id_available) {
        return localized_object_assignment_requires_option(catalog, "help context ID", "--help-context-id");
    }
    if (request.help_context_id_object && request.help_context_id_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "help context ID");
    }
    if (!request.help_context_id_object &&
        (request.help_context_id_available ||
         !request.help_context_id_objects.empty())) {
        return localized_object_arguments_require_mode(catalog, "Help-context-id", "--help-context-id-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_whats_this_help_id_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.whats_this_help_id_object && !request.whats_this_help_id_available) {
        return localized_object_assignment_requires_option(catalog, "WhatsThis help ID", "--whats-this-help-id");
    }
    if (request.whats_this_help_id_object && request.whats_this_help_id_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "WhatsThis help ID");
    }
    if (!request.whats_this_help_id_object &&
        (request.whats_this_help_id_available ||
         !request.whats_this_help_id_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            "Whats-this-help-id",
            "--whats-this-help-id-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_whats_this_help_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.whats_this_help_object && !request.whats_this_help_available) {
        return localized_object_assignment_requires_option(catalog, "WhatsThis help", "--whats-this-help");
    }
    if (request.whats_this_help_object && request.whats_this_help_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "WhatsThis help");
    }
    if (!request.whats_this_help_object &&
        (request.whats_this_help_available ||
         !request.whats_this_help_objects.empty())) {
        return localized_object_arguments_require_mode(catalog, "Whats-this-help", "--whats-this-help-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_whats_this_button_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.whats_this_button_object && !request.whats_this_button_available) {
        return localized_object_assignment_requires_option(catalog, "WhatsThis button", "--whats-this-button");
    }
    if (request.whats_this_button_object && request.whats_this_button_objects.empty()) {
        return localized_object_assignment_requires_target(catalog, "WhatsThis button");
    }
    if (!request.whats_this_button_object &&
        (request.whats_this_button_available ||
         !request.whats_this_button_objects.empty())) {
        return localized_object_arguments_require_mode(catalog, "Whats-this-button", "--whats-this-button-object");
    }
    return std::nullopt;
}

}  // namespace

LaunchParseResult parse_launch_arguments(
    const std::vector<std::string>& args,
    const localization::LocalizedCatalog& catalog) {
    LaunchParseResult result;

    for (std::size_t index = 0; index < args.size(); ++index) {
        const std::string& argument = args[index];
        std::string parsed_argument_error;

        if (parse_form_set_class_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }
        if (parse_default_file_path_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }
        if (parse_initial_selected_alias_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }
        if (parse_tab_orientation_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }
        if (parse_display_orientation_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }
        if (parse_help_context_id_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }
        if (parse_whats_this_help_id_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }
        if (parse_whats_this_help_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }
        if (parse_whats_this_button_argument(argument, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                return {.ok = false, .error = parsed_argument_error};
            }
            continue;
        }

        if (argument == "--help" || argument == "-h" || argument == "/?") {
            result.ok = true;
            result.show_help = true;
            return result;
        }

        if (argument == "--from-vs") {
            result.request.launched_from_visual_studio = true;
            continue;
        }

        if (argument == "--read-only") {
            result.request.read_only = true;
            continue;
        }

        if (argument == "--set-property") {
            result.request.apply_property_update = true;
            continue;
        }

        if (argument == "--clear-property") {
            result.request.clear_property = true;
            continue;
        }

        if (argument == "--rename-property") {
            result.request.rename_property = true;
            continue;
        }

        if (argument == "--delete-object") {
            result.request.delete_object = true;
            continue;
        }

        if (argument == "--restore-object") {
            result.request.restore_object = true;
            continue;
        }

        if (argument == "--deleted-states") {
            result.request.deleted_states = true;
            continue;
        }

        if (argument == "--subtree-deleted-state") {
            result.request.subtree_deleted_state = true;
            continue;
        }

        if (argument == "--duplicate-object") {
            result.request.duplicate_object = true;
            continue;
        }

        if (argument == "--rename-object") {
            result.request.rename_object = true;
            continue;
        }

        if (argument == "--reparent-object") {
            result.request.reparent_object = true;
            continue;
        }

        if (argument == "--reorder-object") {
            result.request.reorder_object = true;
            continue;
        }

        if (argument == "--group-object") {
            result.request.group_object = true;
            continue;
        }

        if (argument == "--align-object") {
            result.request.align_object = true;
            continue;
        }

        if (argument == "--resize-object") {
            result.request.resize_object = true;
            continue;
        }

        if (argument == "--distribute-object") {
            result.request.distribute_object = true;
            continue;
        }

        if (argument == "--snap-object") {
            result.request.snap_object = true;
            continue;
        }

        if (argument == "--nudge-object") {
            result.request.nudge_object = true;
            continue;
        }

        if (argument == "--tab-order-object") {
            result.request.tab_order_object = true;
            continue;
        }

        if (argument == "--tab-stop-object") {
            result.request.tab_stop_object = true;
            continue;
        }

        if (argument == "--visibility-object") {
            result.request.visibility_object = true;
            continue;
        }

        if (argument == "--enabled-object") {
            result.request.enabled_object = true;
            continue;
        }

        if (argument == "--read-only-object") {
            result.request.read_only_object = true;
            continue;
        }

        if (argument == "--locked-object") {
            result.request.locked_object = true;
            continue;
        }

        if (argument == "--caption-object") {
            result.request.caption_object = true;
            continue;
        }

        if (argument == "--record-source-object") {
            result.request.record_source_object = true;
            continue;
        }

        if (argument == "--tooltip-text-object") {
            result.request.tooltip_text_object = true;
            continue;
        }

        if (argument == "--status-bar-text-object") {
            result.request.status_bar_text_object = true;
            continue;
        }

        if (argument == "--link-master-object") {
            result.request.link_master_object = true;
            continue;
        }

        if (argument == "--control-source-object") {
            result.request.control_source_object = true;
            continue;
        }

        if (argument == "--current-control-object") {
            result.request.current_control_object = true;
            continue;
        }

        if (argument == "--input-mask-object") {
            result.request.input_mask_object = true;
            continue;
        }

        if (argument == "--format-object") {
            result.request.format_object = true;
            continue;
        }

        if (argument == "--row-source-object") {
            result.request.row_source_object = true;
            continue;
        }

        if (argument == "--column-widths-object") {
            result.request.column_widths_object = true;
            continue;
        }

        if (argument == "--column-lines-object") {
            result.request.column_lines_object = true;
            continue;
        }

        if (argument == "--integral-height-object") {
            result.request.integral_height_object = true;
            continue;
        }

        if (argument == "--incremental-search-object") {
            result.request.incremental_search_object = true;
            continue;
        }

        if (argument == "--multi-select-object") {
            result.request.multi_select_object = true;
            continue;
        }

        if (argument == "--row-source-type-object") {
            result.request.row_source_type_object = true;
            continue;
        }

        if (argument == "--bound-column-object") {
            result.request.bound_column_object = true;
            continue;
        }

        if (argument == "--column-count-object") {
            result.request.column_count_object = true;
            continue;
        }

        if (argument == "--style-object") {
            result.request.style_object = true;
            continue;
        }

        if (argument == "--list-index-object") {
            result.request.list_index_object = true;
            continue;
        }

        if (argument == "--left-column-object") {
            result.request.left_column_object = true;
            continue;
        }

        if (argument == "--display-value-object") {
            result.request.display_value_object = true;
            continue;
        }

        if (argument == "--selected-back-color-object") {
            result.request.selected_back_color_object = true;
            continue;
        }

        if (argument == "--selected-fore-color-object") {
            result.request.selected_fore_color_object = true;
            continue;
        }

        if (argument == "--selected-item-back-color-object") {
            result.request.selected_item_back_color_object = true;
            continue;
        }

        if (argument == "--selected-item-fore-color-object") {
            result.request.selected_item_fore_color_object = true;
            continue;
        }

        if (argument == "--disabled-item-back-color-object") {
            result.request.disabled_item_back_color_object = true;
            continue;
        }

        if (argument == "--disabled-item-fore-color-object") {
            result.request.disabled_item_fore_color_object = true;
            continue;
        }

        if (argument == "--item-back-color-object") {
            result.request.item_back_color_object = true;
            continue;
        }

        if (argument == "--item-fore-color-object") {
            result.request.item_fore_color_object = true;
            continue;
        }

        if (argument == "--highlight-back-color-object") {
            result.request.highlight_back_color_object = true;
            continue;
        }

        if (argument == "--highlight-fore-color-object") {
            result.request.highlight_fore_color_object = true;
            continue;
        }

        if (argument == "--back-color-object") {
            result.request.back_color_object = true;
            continue;
        }

        if (argument == "--fore-color-object") {
            result.request.fore_color_object = true;
            continue;
        }

        if (argument == "--disabled-back-color-object") {
            result.request.disabled_back_color_object = true;
            continue;
        }

        if (argument == "--disabled-fore-color-object") {
            result.request.disabled_fore_color_object = true;
            continue;
        }

        if (argument == "--dynamic-back-color-object") {
            result.request.dynamic_back_color_object = true;
            continue;
        }

        if (argument == "--dynamic-fore-color-object") {
            result.request.dynamic_fore_color_object = true;
            continue;
        }

        if (argument == "--closable-object") {
            result.request.closable_object = true;
            continue;
        }

        if (argument == "--control-box-object") {
            result.request.control_box_object = true;
            continue;
        }

        if (argument == "--allow-output-object") {
            result.request.allow_output_object = true;
            continue;
        }

        if (argument == "--bind-controls-object") {
            result.request.bind_controls_object = true;
            continue;
        }

        if (argument == "--auto-verb-menu-object") {
            result.request.auto_verb_menu_object = true;
            continue;
        }

        if (argument == "--desktop-object") {
            result.request.desktop_object = true;
            continue;
        }

        if (argument == "--key-preview-object") {
            result.request.key_preview_object = true;
            continue;
        }

        if (argument == "--mac-desktop-object") {
            result.request.mac_desktop_object = true;
            continue;
        }

        if (argument == "--max-button-object") {
            result.request.max_button_object = true;
            continue;
        }

        if (argument == "--min-button-object") {
            result.request.min_button_object = true;
            continue;
        }

        if (argument == "--min-height-object") {
            result.request.min_height_object = true;
            continue;
        }

        if (argument == "--min-width-object") {
            result.request.min_width_object = true;
            continue;
        }

        if (argument == "--max-height-object") {
            result.request.max_height_object = true;
            continue;
        }

        if (argument == "--movable-object") {
            result.request.movable_object = true;
            continue;
        }

        if (argument == "--half-height-caption-object") {
            result.request.half_height_caption_object = true;
            continue;
        }

        if (argument == "--mdi-form-object") {
            result.request.mdi_form_object = true;
            continue;
        }

        if (argument == "--back-style-object") {
            result.request.back_style_object = true;
            continue;
        }

        if (argument == "--border-style-object") {
            result.request.border_style_object = true;
            continue;
        }

        if (argument == "--border-width-object") {
            result.request.border_width_object = true;
            continue;
        }

        if (argument == "--border-color-object") {
            result.request.border_color_object = true;
            continue;
        }

        if (argument == "--special-effect-object") {
            result.request.special_effect_object = true;
            continue;
        }

        if (argument == "--scroll-bars-object") {
            result.request.scroll_bars_object = true;
            continue;
        }

        if (argument == "--window-state-object") {
            result.request.window_state_object = true;
            continue;
        }

        if (argument == "--show-window-object") {
            result.request.show_window_object = true;
            continue;
        }

        if (argument == "--title-bar-object") {
            result.request.title_bar_object = true;
            continue;
        }

        if (argument == "--mouse-pointer-object") {
            result.request.mouse_pointer_object = true;
            continue;
        }

        if (argument == "--picture-margin-object") {
            result.request.picture_margin_object = true;
            continue;
        }

        if (argument == "--picture-position-object") {
            result.request.picture_position_object = true;
            continue;
        }

        if (argument == "--picture-spacing-object") {
            result.request.picture_spacing_object = true;
            continue;
        }

        if (argument == "--picture-selection-display-object") {
            result.request.picture_selection_display_object = true;
            continue;
        }

        if (argument == "--dynamic-input-mask-object") {
            result.request.dynamic_input_mask_object = true;
            continue;
        }

        if (argument == "--dynamic-line-height-object") {
            result.request.dynamic_line_height_object = true;
            continue;
        }

        if (argument == "--dynamic-alignment-object") {
            result.request.dynamic_alignment_object = true;
            continue;
        }

        if (argument == "--dynamic-current-control-object") {
            result.request.dynamic_current_control_object = true;
            continue;
        }

        if (argument == "--dynamic-font-name-object") {
            result.request.dynamic_font_name_object = true;
            continue;
        }

        if (argument == "--dynamic-font-size-object") {
            result.request.dynamic_font_size_object = true;
            continue;
        }

        if (argument == "--dynamic-font-bold-object") {
            result.request.dynamic_font_bold_object = true;
            continue;
        }

        if (argument == "--dynamic-font-italic-object") {
            result.request.dynamic_font_italic_object = true;
            continue;
        }

        if (argument == "--dynamic-font-underline-object") {
            result.request.dynamic_font_underline_object = true;
            continue;
        }

        if (argument == "--dynamic-font-strikethru-object") {
            result.request.dynamic_font_strikethru_object = true;
            continue;
        }

        if (argument == "--dynamic-font-outline-object") {
            result.request.dynamic_font_outline_object = true;
            continue;
        }

        if (argument == "--dynamic-font-shadow-object") {
            result.request.dynamic_font_shadow_object = true;
            continue;
        }

        if (argument == "--font-name-object") {
            result.request.font_name_object = true;
            continue;
        }

        if (argument == "--font-size-object") {
            result.request.font_size_object = true;
            continue;
        }

        if (argument == "--font-bold-object") {
            result.request.font_bold_object = true;
            continue;
        }

        if (argument == "--font-italic-object") {
            result.request.font_italic_object = true;
            continue;
        }

        if (argument == "--font-underline-object") {
            result.request.font_underline_object = true;
            continue;
        }

        if (argument == "--font-strikethru-object") {
            result.request.font_strikethru_object = true;
            continue;
        }

        if (argument == "--font-outline-object") {
            result.request.font_outline_object = true;
            continue;
        }

        if (argument == "--font-shadow-object") {
            result.request.font_shadow_object = true;
            continue;
        }

        if (argument == "--max-width-object") {
            result.request.max_width_object = true;
            continue;
        }

        if (argument == "--max-left-object") {
            result.request.max_left_object = true;
            continue;
        }

        if (argument == "--max-top-object") {
            result.request.max_top_object = true;
            continue;
        }

        if (argument == "--auto-center-object") {
            result.request.auto_center_object = true;
            continue;
        }

        if (argument == "--auto-size-object") {
            result.request.auto_size_object = true;
            continue;
        }

        if (argument == "--auto-release-object") {
            result.request.auto_release_object = true;
            continue;
        }

        if (argument == "--continuous-scroll-object") {
            result.request.continuous_scroll_object = true;
            continue;
        }

        if (argument == "--dockable-object") {
            result.request.dockable_object = true;
            continue;
        }

        if (argument == "--clip-controls-object") {
            result.request.clip_controls_object = true;
            continue;
        }

        if (argument == "--sparse-object") {
            result.request.sparse_object = true;
            continue;
        }

        if (argument == "--lock-screen-object") {
            result.request.lock_screen_object = true;
            continue;
        }

        if (argument == "--hide-selection-object") {
            result.request.hide_selection_object = true;
            continue;
        }

        if (argument == "--allow-cell-selection-object") {
            result.request.allow_cell_selection_object = true;
            continue;
        }

        if (argument == "--delete-mark-object") {
            result.request.delete_mark_object = true;
            continue;
        }

        if (argument == "--record-mark-object") {
            result.request.record_mark_object = true;
            continue;
        }

        if (argument == "--split-bar-object") {
            result.request.split_bar_object = true;
            continue;
        }

        if (argument == "--highlight-row-object") {
            result.request.highlight_row_object = true;
            continue;
        }

        if (argument == "--panel-link-object") {
            result.request.panel_link_object = true;
            continue;
        }

        if (argument == "--allow-header-sizing-object") {
            result.request.allow_header_sizing_object = true;
            continue;
        }

        if (argument == "--allow-row-sizing-object") {
            result.request.allow_row_sizing_object = true;
            continue;
        }

        if (argument == "--resizable-object") {
            result.request.resizable_object = true;
            continue;
        }

        if (argument == "--add-line-feeds-object") {
            result.request.add_line_feeds_object = true;
            continue;
        }

        if (argument == "--always-on-top-object") {
            result.request.always_on_top_object = true;
            continue;
        }

        if (argument == "--always-on-bottom-object") {
            result.request.always_on_bottom_object = true;
            continue;
        }

        if (argument == "--picture-object") {
            result.request.picture_object = true;
            continue;
        }

        if (argument == "--down-picture-object") {
            result.request.down_picture_object = true;
            continue;
        }

        if (argument == "--disabled-picture-object") {
            result.request.disabled_picture_object = true;
            continue;
        }

        if (argument == "--ole-drag-picture-object") {
            result.request.ole_drag_picture_object = true;
            continue;
        }

        if (argument == "--mouse-icon-object") {
            result.request.mouse_icon_object = true;
            continue;
        }

        if (argument == "--drag-icon-object") {
            result.request.drag_icon_object = true;
            continue;
        }

        if (argument == "--drag-mode-object") {
            result.request.drag_mode_object = true;
            continue;
        }

        if (argument == "--ole-drag-mode-object") {
            result.request.ole_drag_mode_object = true;
            continue;
        }

        if (argument == "--ole-drop-mode-object") {
            result.request.ole_drop_mode_object = true;
            continue;
        }

        if (argument == "--ole-drop-effects-object") {
            result.request.ole_drop_effects_object = true;
            continue;
        }

        if (argument == "--ole-drop-text-insertion-object") {
            result.request.ole_drop_text_insertion_object = true;
            continue;
        }

        if (argument == "--button-count-object") {
            result.request.button_count_object = true;
            continue;
        }

        if (argument == "--curvature-object") {
            result.request.curvature_object = true;
            continue;
        }

        if (argument == "--draw-mode-object") {
            result.request.draw_mode_object = true;
            continue;
        }

        if (argument == "--draw-style-object") {
            result.request.draw_style_object = true;
            continue;
        }

        if (argument == "--draw-width-object") {
            result.request.draw_width_object = true;
            continue;
        }

        if (argument == "--fill-style-object") {
            result.request.fill_style_object = true;
            continue;
        }

        if (argument == "--scale-mode-object") {
            result.request.scale_mode_object = true;
            continue;
        }

        if (argument == "--buffer-mode-object") {
            result.request.buffer_mode_object = true;
            continue;
        }

        if (argument == "--buffer-mode-override-object") {
            result.request.buffer_mode_override_object = true;
            continue;
        }

        if (argument == "--data-session-object") {
            result.request.data_session_object = true;
            continue;
        }

        if (argument == "--grid-line-color-object") {
            result.request.grid_line_color_object = true;
            continue;
        }

        if (argument == "--header-height-object") {
            result.request.header_height_object = true;
            continue;
        }

        if (argument == "--row-height-object") {
            result.request.row_height_object = true;
            continue;
        }

        if (argument == "--lock-columns-object") {
            result.request.lock_columns_object = true;
            continue;
        }

        if (argument == "--lock-columns-left-object") {
            result.request.lock_columns_left_object = true;
            continue;
        }

        if (argument == "--grid-line-width-object") {
            result.request.grid_line_width_object = true;
            continue;
        }

        if (argument == "--grid-lines-object") {
            result.request.grid_lines_object = true;
            continue;
        }

        if (argument == "--highlight-row-line-width-object") {
            result.request.highlight_row_line_width_object = true;
            continue;
        }

        if (argument == "--partition-object") {
            result.request.partition_object = true;
            continue;
        }

        if (argument == "--record-source-type-object") {
            result.request.record_source_type_object = true;
            continue;
        }

        if (argument == "--column-order-object") {
            result.request.column_order_object = true;
            continue;
        }

        if (argument == "--highlight-style-object") {
            result.request.highlight_style_object = true;
            continue;
        }

        if (argument == "--child-order-object") {
            result.request.child_order_object = true;
            continue;
        }

        if (argument == "--fill-color-object") {
            result.request.fill_color_object = true;
            continue;
        }

        if (argument == "--list-item-id-object") {
            result.request.list_item_id_object = true;
            continue;
        }

        if (argument == "--ungroup-object") {
            result.request.ungroup_object = true;
            continue;
        }

        if (argument == "--clear-parent") {
            result.request.clear_parent = true;
            continue;
        }

        if (argument == "--json") {
            result.output_json = true;
            continue;
        }

        if (argument == "--path") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--path")};
            }
            result.request.path = args[++index];
            continue;
        }

        if (argument == "--symbol") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--symbol")};
            }
            result.request.symbol = args[++index];
            continue;
        }

        if (argument == "--selection-context") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--selection-context")};
            }
            const auto selection_context = parse_selection_context_token(args[++index]);
            if (!selection_context.has_value()) {
                return {.ok = false, .error = localized_selection_context_error(catalog)};
            }
            result.request.designer_selection_contexts.push_back(*selection_context);
            continue;
        }

        if (argument == "--record") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--record")};
            }
            std::size_t record_index = 0;
            if (!parse_size_value(args[++index], record_index)) {
                return {.ok = false, .error = localized_unsigned_integer_value_required(catalog, "--record")};
            }
            result.request.record_index = record_index;
            result.request.selection_record_available = true;
            continue;
        }

        if (argument == "--property-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--property-name")};
            }
            result.request.property_name = args[++index];
            continue;
        }

        if (argument == "--property-value") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--property-value")};
            }
            result.request.property_value = args[++index];
            continue;
        }

        if (argument == "--new-property-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--new-property-name")};
            }
            result.request.new_property_name = args[++index];
            continue;
        }

        if (argument == "--new-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--new-object-name")};
            }
            result.request.new_object_name = args[++index];
            continue;
        }

        if (argument == "--new-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--new-name")};
            }
            result.request.new_name = args[++index];
            continue;
        }

        if (argument == "--new-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--new-unique-id")};
            }
            result.request.new_unique_id = args[++index];
            continue;
        }

        if (argument == "--parent-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--parent-name")};
            }
            result.request.parent_name = args[++index];
            continue;
        }

        if (argument == "--parent-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--parent-unique-id")};
            }
            result.request.parent_unique_id = args[++index];
            continue;
        }

        if (argument == "--placement") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--placement")};
            }
            result.request.placement = args[++index];
            continue;
        }

        if (argument == "--target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--target-object-name")};
            }
            result.request.target_object_name = args[++index];
            continue;
        }

        if (argument == "--target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--target-unique-id")};
            }
            result.request.target_unique_id = args[++index];
            continue;
        }

        if (argument == "--group-child-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--group-child-object-name")};
            }
            result.request.group_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--group-child-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--group-child-unique-id")};
            }
            result.request.group_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--field-value") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--field-value")};
            }
            const std::string assignment = args[++index];
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                return {.ok = false, .error = localized_field_value_name_value_syntax_required(catalog)};
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
            continue;
        }

        if (argument == "--alignment-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--alignment-mode")};
            }
            result.request.alignment_mode = args[++index];
            continue;
        }

        if (argument == "--resize-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--resize-mode")};
            }
            result.request.resize_mode = args[++index];
            continue;
        }

        if (argument == "--distribution-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--distribution-mode")};
            }
            result.request.distribution_mode = args[++index];
            continue;
        }

        if (argument == "--snap-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--snap-mode")};
            }
            result.request.snap_mode = args[++index];
            continue;
        }

        if (argument == "--nudge-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--nudge-mode")};
            }
            result.request.nudge_mode = args[++index];
            continue;
        }

        if (argument == "--grid-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-width")};
            }
            double grid_width = 0.0;
            if (!parse_double_value(args[++index], grid_width)) {
                return {.ok = false, .error = localized_numeric_value_required(catalog, "--grid-width")};
            }
            result.request.grid_width = grid_width;
            continue;
        }

        if (argument == "--grid-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-height")};
            }
            double grid_height = 0.0;
            if (!parse_double_value(args[++index], grid_height)) {
                return {.ok = false, .error = localized_numeric_value_required(catalog, "--grid-height")};
            }
            result.request.grid_height = grid_height;
            continue;
        }

        if (argument == "--delta-hpos") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--delta-hpos")};
            }
            double delta_hpos = 0.0;
            if (!parse_double_value(args[++index], delta_hpos)) {
                return {.ok = false, .error = localized_numeric_value_required(catalog, "--delta-hpos")};
            }
            result.request.delta_hpos = delta_hpos;
            continue;
        }

        if (argument == "--delta-vpos") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--delta-vpos")};
            }
            double delta_vpos = 0.0;
            if (!parse_double_value(args[++index], delta_vpos)) {
                return {.ok = false, .error = localized_numeric_value_required(catalog, "--delta-vpos")};
            }
            result.request.delta_vpos = delta_vpos;
            continue;
        }

        if (argument == "--starting-tab-index") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--starting-tab-index")};
            }
            int starting_tab_index = 0;
            if (!parse_int_value(args[++index], starting_tab_index)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--starting-tab-index")};
            }
            result.request.starting_tab_index = starting_tab_index;
            result.request.starting_tab_index_available = true;
            continue;
        }

        if (argument == "--tab-stop") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-stop")};
            }
            const auto tab_stop = parse_bool_value(args[++index]);
            if (!tab_stop.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--tab-stop")};
            }
            result.request.tab_stop = *tab_stop;
            result.request.tab_stop_available = true;
            continue;
        }

        if (argument == "--visible") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--visible")};
            }
            const auto visible = parse_bool_value(args[++index]);
            if (!visible.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--visible")};
            }
            result.request.visible = *visible;
            result.request.visible_available = true;
            continue;
        }

        if (argument == "--enabled") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--enabled")};
            }
            const auto enabled = parse_bool_value(args[++index]);
            if (!enabled.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--enabled")};
            }
            result.request.enabled = *enabled;
            result.request.enabled_available = true;
            continue;
        }

        if (argument == "--object-read-only") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--object-read-only")};
            }
            const auto object_read_only = parse_bool_value(args[++index]);
            if (!object_read_only.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--object-read-only")};
            }
            result.request.object_read_only = *object_read_only;
            result.request.object_read_only_available = true;
            continue;
        }

        if (argument == "--locked") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--locked")};
            }
            const auto locked = parse_bool_value(args[++index]);
            if (!locked.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--locked")};
            }
            result.request.locked = *locked;
            result.request.locked_available = true;
            continue;
        }

        if (argument == "--caption") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--caption")};
            }
            result.request.caption = args[++index];
            result.request.caption_available = true;
            continue;
        }

        if (argument == "--picture") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture")};
            }
            result.request.picture = args[++index];
            result.request.picture_available = true;
            continue;
        }

        if (argument == "--down-picture") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--down-picture")};
            }
            result.request.down_picture = args[++index];
            result.request.down_picture_available = true;
            continue;
        }

        if (argument == "--disabled-picture") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-picture")};
            }
            result.request.disabled_picture = args[++index];
            result.request.disabled_picture_available = true;
            continue;
        }

        if (argument == "--ole-drag-picture") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-picture")};
            }
            result.request.ole_drag_picture = args[++index];
            result.request.ole_drag_picture_available = true;
            continue;
        }

        if (argument == "--mouse-icon") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-icon")};
            }
            result.request.mouse_icon = args[++index];
            result.request.mouse_icon_available = true;
            continue;
        }

        if (argument == "--drag-icon") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-icon")};
            }
            result.request.drag_icon = args[++index];
            result.request.drag_icon_available = true;
            continue;
        }

        if (argument == "--record-source") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-source")};
            }
            result.request.record_source = args[++index];
            result.request.record_source_available = true;
            continue;
        }

        if (argument == "--tooltip-text") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--tooltip-text")};
            }
            result.request.tooltip_text = args[++index];
            result.request.tooltip_text_available = true;
            continue;
        }

        if (argument == "--status-bar-text") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--status-bar-text")};
            }
            result.request.status_bar_text = args[++index];
            result.request.status_bar_text_available = true;
            continue;
        }

        if (argument == "--link-master") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--link-master")};
            }
            result.request.link_master = args[++index];
            result.request.link_master_available = true;
            continue;
        }

        if (argument == "--control-source") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--control-source")};
            }
            result.request.control_source = args[++index];
            result.request.control_source_available = true;
            continue;
        }

        if (argument == "--current-control") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--current-control")};
            }
            result.request.current_control = args[++index];
            result.request.current_control_available = true;
            continue;
        }

        if (argument == "--input-mask") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--input-mask")};
            }
            result.request.input_mask = args[++index];
            result.request.input_mask_available = true;
            continue;
        }

        if (argument == "--format") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--format")};
            }
            result.request.format = args[++index];
            result.request.format_available = true;
            continue;
        }

        if (argument == "--row-source") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-source")};
            }
            result.request.row_source = args[++index];
            result.request.row_source_available = true;
            continue;
        }

        if (argument == "--column-widths") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-widths")};
            }
            result.request.column_widths = args[++index];
            result.request.column_widths_available = true;
            continue;
        }

        if (argument == "--column-lines") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-lines")};
            }
            const auto column_lines = parse_bool_value(args[++index]);
            if (!column_lines.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--column-lines")};
            }
            result.request.column_lines = *column_lines;
            result.request.column_lines_available = true;
            continue;
        }

        if (argument == "--integral-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--integral-height")};
            }
            const auto integral_height = parse_bool_value(args[++index]);
            if (!integral_height.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--integral-height")};
            }
            result.request.integral_height = *integral_height;
            result.request.integral_height_available = true;
            continue;
        }

        if (argument == "--incremental-search") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--incremental-search")};
            }
            const auto incremental_search = parse_bool_value(args[++index]);
            if (!incremental_search.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--incremental-search")};
            }
            result.request.incremental_search = *incremental_search;
            result.request.incremental_search_available = true;
            continue;
        }

        if (argument == "--multi-select") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--multi-select")};
            }
            const auto multi_select = parse_bool_value(args[++index]);
            if (!multi_select.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--multi-select")};
            }
            result.request.multi_select = *multi_select;
            result.request.multi_select_available = true;
            continue;
        }

        if (argument == "--row-source-type") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-source-type")};
            }
            int row_source_type = 0;
            if (!parse_int_value(args[++index], row_source_type)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--row-source-type")};
            }
            result.request.row_source_type = row_source_type;
            result.request.row_source_type_available = true;
            continue;
        }

        if (argument == "--bound-column") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--bound-column")};
            }
            int bound_column = 0;
            if (!parse_int_value(args[++index], bound_column)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--bound-column")};
            }
            result.request.bound_column = bound_column;
            result.request.bound_column_available = true;
            continue;
        }

        if (argument == "--column-count") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-count")};
            }
            int column_count = 0;
            if (!parse_int_value(args[++index], column_count)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--column-count")};
            }
            result.request.column_count = column_count;
            result.request.column_count_available = true;
            continue;
        }

        if (argument == "--style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--style")};
            }
            int style = 0;
            if (!parse_int_value(args[++index], style)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--style")};
            }
            result.request.style = style;
            result.request.style_available = true;
            continue;
        }

        if (argument == "--list-index") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--list-index")};
            }
            int list_index = 0;
            if (!parse_int_value(args[++index], list_index)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--list-index")};
            }
            result.request.list_index = list_index;
            result.request.list_index_available = true;
            continue;
        }

        if (argument == "--left-column") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--left-column")};
            }
            int left_column = 0;
            if (!parse_int_value(args[++index], left_column)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--left-column")};
            }
            result.request.left_column = left_column;
            result.request.left_column_available = true;
            continue;
        }

        if (argument == "--drag-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-mode")};
            }
            int drag_mode = 0;
            if (!parse_int_value(args[++index], drag_mode)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--drag-mode")};
            }
            if (drag_mode < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--drag-mode")};
            }
            result.request.drag_mode = drag_mode;
            result.request.drag_mode_available = true;
            continue;
        }

        if (argument == "--ole-drag-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-mode")};
            }
            int ole_drag_mode = 0;
            if (!parse_int_value(args[++index], ole_drag_mode)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--ole-drag-mode")};
            }
            if (ole_drag_mode < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--ole-drag-mode")};
            }
            result.request.ole_drag_mode = ole_drag_mode;
            result.request.ole_drag_mode_available = true;
            continue;
        }

        if (argument == "--ole-drop-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-mode")};
            }
            int ole_drop_mode = 0;
            if (!parse_int_value(args[++index], ole_drop_mode)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--ole-drop-mode")};
            }
            if (ole_drop_mode < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--ole-drop-mode")};
            }
            result.request.ole_drop_mode = ole_drop_mode;
            result.request.ole_drop_mode_available = true;
            continue;
        }

        if (argument == "--ole-drop-effects") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-effects")};
            }
            int ole_drop_effects = 0;
            if (!parse_int_value(args[++index], ole_drop_effects)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--ole-drop-effects")};
            }
            if (ole_drop_effects < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--ole-drop-effects")};
            }
            result.request.ole_drop_effects = ole_drop_effects;
            result.request.ole_drop_effects_available = true;
            continue;
        }

        if (argument == "--ole-drop-text-insertion") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-text-insertion")};
            }
            int ole_drop_text_insertion = 0;
            if (!parse_int_value(args[++index], ole_drop_text_insertion)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--ole-drop-text-insertion")};
            }
            if (ole_drop_text_insertion < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--ole-drop-text-insertion")};
            }
            result.request.ole_drop_text_insertion = ole_drop_text_insertion;
            result.request.ole_drop_text_insertion_available = true;
            continue;
        }

        if (argument == "--button-count") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--button-count")};
            }
            int button_count = 0;
            if (!parse_int_value(args[++index], button_count)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--button-count")};
            }
            if (button_count < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--button-count")};
            }
            result.request.button_count = button_count;
            result.request.button_count_available = true;
            continue;
        }

        if (argument == "--curvature") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--curvature")};
            }
            int curvature = 0;
            if (!parse_int_value(args[++index], curvature)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--curvature")};
            }
            if (curvature < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--curvature")};
            }
            result.request.curvature = curvature;
            result.request.curvature_available = true;
            continue;
        }

        if (argument == "--draw-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-mode")};
            }
            int draw_mode = 0;
            if (!parse_int_value(args[++index], draw_mode)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--draw-mode")};
            }
            if (draw_mode < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--draw-mode")};
            }
            result.request.draw_mode = draw_mode;
            result.request.draw_mode_available = true;
            continue;
        }

        if (argument == "--draw-style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-style")};
            }
            int draw_style = 0;
            if (!parse_int_value(args[++index], draw_style)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--draw-style")};
            }
            if (draw_style < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--draw-style")};
            }
            result.request.draw_style = draw_style;
            result.request.draw_style_available = true;
            continue;
        }

        if (argument == "--draw-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-width")};
            }
            int draw_width = 0;
            if (!parse_int_value(args[++index], draw_width)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--draw-width")};
            }
            if (draw_width < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--draw-width")};
            }
            result.request.draw_width = draw_width;
            result.request.draw_width_available = true;
            continue;
        }

        if (argument == "--fill-style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-style")};
            }
            int fill_style = 0;
            if (!parse_int_value(args[++index], fill_style)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--fill-style")};
            }
            if (fill_style < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--fill-style")};
            }
            result.request.fill_style = fill_style;
            result.request.fill_style_available = true;
            continue;
        }

        if (argument == "--scale-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--scale-mode")};
            }
            int scale_mode = 0;
            if (!parse_int_value(args[++index], scale_mode)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--scale-mode")};
            }
            if (scale_mode < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--scale-mode")};
            }
            result.request.scale_mode = scale_mode;
            result.request.scale_mode_available = true;
            continue;
        }

        if (argument == "--buffer-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode")};
            }
            int buffer_mode = 0;
            if (!parse_int_value(args[++index], buffer_mode)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--buffer-mode")};
            }
            if (buffer_mode < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--buffer-mode")};
            }
            result.request.buffer_mode = buffer_mode;
            result.request.buffer_mode_available = true;
            continue;
        }

        if (argument == "--buffer-mode-override") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-override")};
            }
            int buffer_mode_override = 0;
            if (!parse_int_value(args[++index], buffer_mode_override)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--buffer-mode-override")};
            }
            if (buffer_mode_override < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--buffer-mode-override")};
            }
            result.request.buffer_mode_override = buffer_mode_override;
            result.request.buffer_mode_override_available = true;
            continue;
        }

        if (argument == "--data-session") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--data-session")};
            }
            int data_session = 0;
            if (!parse_int_value(args[++index], data_session)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--data-session")};
            }
            if (data_session < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--data-session")};
            }
            result.request.data_session = data_session;
            result.request.data_session_available = true;
            continue;
        }

        if (argument == "--grid-line-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-color")};
            }
            int grid_line_color = 0;
            if (!parse_int_value(args[++index], grid_line_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--grid-line-color")};
            }
            if (grid_line_color < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--grid-line-color")};
            }
            result.request.grid_line_color = grid_line_color;
            result.request.grid_line_color_available = true;
            continue;
        }

        if (argument == "--header-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--header-height")};
            }
            int header_height = 0;
            if (!parse_int_value(args[++index], header_height)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--header-height")};
            }
            if (header_height < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--header-height")};
            }
            result.request.header_height = header_height;
            result.request.header_height_available = true;
            continue;
        }

        if (argument == "--row-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-height")};
            }
            int row_height = 0;
            if (!parse_int_value(args[++index], row_height)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--row-height")};
            }
            if (row_height < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--row-height")};
            }
            result.request.row_height = row_height;
            result.request.row_height_available = true;
            continue;
        }

        if (argument == "--lock-columns") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns")};
            }
            int lock_columns = 0;
            if (!parse_int_value(args[++index], lock_columns)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--lock-columns")};
            }
            if (lock_columns < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--lock-columns")};
            }
            result.request.lock_columns = lock_columns;
            result.request.lock_columns_available = true;
            continue;
        }

        if (argument == "--lock-columns-left") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-left")};
            }
            int lock_columns_left = 0;
            if (!parse_int_value(args[++index], lock_columns_left)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--lock-columns-left")};
            }
            if (lock_columns_left < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--lock-columns-left")};
            }
            result.request.lock_columns_left = lock_columns_left;
            result.request.lock_columns_left_available = true;
            continue;
        }

        if (argument == "--grid-line-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-width")};
            }
            int grid_line_width = 0;
            if (!parse_int_value(args[++index], grid_line_width)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--grid-line-width")};
            }
            if (grid_line_width < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--grid-line-width")};
            }
            result.request.grid_line_width = grid_line_width;
            result.request.grid_line_width_available = true;
            continue;
        }

        if (argument == "--grid-lines") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-lines")};
            }
            int grid_lines = 0;
            if (!parse_int_value(args[++index], grid_lines)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--grid-lines")};
            }
            if (grid_lines < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--grid-lines")};
            }
            result.request.grid_lines = grid_lines;
            result.request.grid_lines_available = true;
            continue;
        }

        if (argument == "--highlight-row-line-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row-line-width")};
            }
            int highlight_row_line_width = 0;
            if (!parse_int_value(args[++index], highlight_row_line_width)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--highlight-row-line-width")};
            }
            if (highlight_row_line_width < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--highlight-row-line-width")};
            }
            result.request.highlight_row_line_width = highlight_row_line_width;
            result.request.highlight_row_line_width_available = true;
            continue;
        }

        if (argument == "--partition") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--partition")};
            }
            int partition = 0;
            if (!parse_int_value(args[++index], partition)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--partition")};
            }
            if (partition < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--partition")};
            }
            result.request.partition = partition;
            result.request.partition_available = true;
            continue;
        }

        if (argument == "--record-source-type") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-source-type")};
            }
            int record_source_type = 0;
            if (!parse_int_value(args[++index], record_source_type)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--record-source-type")};
            }
            if (record_source_type < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--record-source-type")};
            }
            result.request.record_source_type = record_source_type;
            result.request.record_source_type_available = true;
            continue;
        }

        if (argument == "--column-order") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-order")};
            }
            int column_order = 0;
            if (!parse_int_value(args[++index], column_order)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--column-order")};
            }
            if (column_order < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--column-order")};
            }
            result.request.column_order = column_order;
            result.request.column_order_available = true;
            continue;
        }

        if (argument == "--highlight-style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-style")};
            }
            int highlight_style = 0;
            if (!parse_int_value(args[++index], highlight_style)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--highlight-style")};
            }
            if (highlight_style < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--highlight-style")};
            }
            result.request.highlight_style = highlight_style;
            result.request.highlight_style_available = true;
            continue;
        }

        if (argument == "--child-order") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--child-order")};
            }
            int child_order = 0;
            if (!parse_int_value(args[++index], child_order)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--child-order")};
            }
            if (child_order < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--child-order")};
            }
            result.request.child_order = child_order;
            result.request.child_order_available = true;
            continue;
        }

        if (argument == "--fill-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-color")};
            }
            int fill_color = 0;
            if (!parse_int_value(args[++index], fill_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--fill-color")};
            }
            if (fill_color < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--fill-color")};
            }
            result.request.fill_color = fill_color;
            result.request.fill_color_available = true;
            continue;
        }

        if (argument == "--list-item-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--list-item-id")};
            }
            int list_item_id = 0;
            if (!parse_int_value(args[++index], list_item_id)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--list-item-id")};
            }
            if (list_item_id < 0) {
                return {.ok = false, .error = localized_non_negative_value_required(catalog, "--list-item-id")};
            }
            result.request.list_item_id = list_item_id;
            result.request.list_item_id_available = true;
            continue;
        }

        if (argument == "--display-value") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--display-value")};
            }
            result.request.display_value = args[++index];
            result.request.display_value_available = true;
            continue;
        }

        if (argument == "--dynamic-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-back-color")};
            }
            result.request.dynamic_back_color = args[++index];
            result.request.dynamic_back_color_available = true;
            continue;
        }

        if (argument == "--dynamic-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-fore-color")};
            }
            result.request.dynamic_fore_color = args[++index];
            result.request.dynamic_fore_color_available = true;
            continue;
        }

        if (argument == "--closable") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--closable")};
            }
            const auto closable = parse_bool_value(args[++index]);
            if (!closable.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--closable")};
            }
            result.request.closable = *closable;
            result.request.closable_available = true;
            continue;
        }

        if (argument == "--control-box") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--control-box")};
            }
            const auto control_box = parse_bool_value(args[++index]);
            if (!control_box.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--control-box")};
            }
            result.request.control_box = *control_box;
            result.request.control_box_available = true;
            continue;
        }

        if (argument == "--allow-output") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-output")};
            }
            const auto allow_output = parse_bool_value(args[++index]);
            if (!allow_output.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--allow-output")};
            }
            result.request.allow_output = *allow_output;
            result.request.allow_output_available = true;
            continue;
        }

        if (argument == "--auto-center") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-center")};
            }
            const auto auto_center = parse_bool_value(args[++index]);
            if (!auto_center.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--auto-center")};
            }
            result.request.auto_center = *auto_center;
            result.request.auto_center_available = true;
            continue;
        }

        if (argument == "--auto-verb-menu") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-verb-menu")};
            }
            const auto auto_verb_menu = parse_bool_value(args[++index]);
            if (!auto_verb_menu.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--auto-verb-menu")};
            }
            result.request.auto_verb_menu = *auto_verb_menu;
            result.request.auto_verb_menu_available = true;
            continue;
        }

        if (argument == "--bind-controls") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--bind-controls")};
            }
            const auto bind_controls = parse_bool_value(args[++index]);
            if (!bind_controls.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--bind-controls")};
            }
            result.request.bind_controls = *bind_controls;
            result.request.bind_controls_available = true;
            continue;
        }

        if (argument == "--desktop") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--desktop")};
            }
            const auto desktop = parse_bool_value(args[++index]);
            if (!desktop.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--desktop")};
            }
            result.request.desktop = *desktop;
            result.request.desktop_available = true;
            continue;
        }

        if (argument == "--key-preview") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--key-preview")};
            }
            const auto key_preview = parse_bool_value(args[++index]);
            if (!key_preview.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--key-preview")};
            }
            result.request.key_preview = *key_preview;
            result.request.key_preview_available = true;
            continue;
        }

        if (argument == "--mac-desktop") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--mac-desktop")};
            }
            const auto mac_desktop = parse_bool_value(args[++index]);
            if (!mac_desktop.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--mac-desktop")};
            }
            result.request.mac_desktop = *mac_desktop;
            result.request.mac_desktop_available = true;
            continue;
        }

        if (argument == "--max-button") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-button")};
            }
            const auto max_button = parse_bool_value(args[++index]);
            if (!max_button.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--max-button")};
            }
            result.request.max_button = *max_button;
            result.request.max_button_available = true;
            continue;
        }

        if (argument == "--min-button") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-button")};
            }
            const auto min_button = parse_bool_value(args[++index]);
            if (!min_button.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--min-button")};
            }
            result.request.min_button = *min_button;
            result.request.min_button_available = true;
            continue;
        }

        if (argument == "--min-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-height")};
            }
            int min_height = 0;
            if (!parse_int_value(args[++index], min_height)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--min-height")};
            }
            if (min_height < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--min-height")};
            }
            result.request.min_height = min_height;
            result.request.min_height_available = true;
            continue;
        }

        if (argument == "--min-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-width")};
            }
            int min_width = 0;
            if (!parse_int_value(args[++index], min_width)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--min-width")};
            }
            if (min_width < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--min-width")};
            }
            result.request.min_width = min_width;
            result.request.min_width_available = true;
            continue;
        }

        if (argument == "--max-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-height")};
            }
            int max_height = 0;
            if (!parse_int_value(args[++index], max_height)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--max-height")};
            }
            if (max_height < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--max-height")};
            }
            result.request.max_height = max_height;
            result.request.max_height_available = true;
            continue;
        }

        if (argument == "--movable") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--movable")};
            }
            const auto movable = parse_bool_value(args[++index]);
            if (!movable.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--movable")};
            }
            result.request.movable = *movable;
            result.request.movable_available = true;
            continue;
        }

        if (argument == "--half-height-caption") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--half-height-caption")};
            }
            const auto half_height_caption = parse_bool_value(args[++index]);
            if (!half_height_caption.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--half-height-caption")};
            }
            result.request.half_height_caption = *half_height_caption;
            result.request.half_height_caption_available = true;
            continue;
        }

        if (argument == "--mdi-form") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--mdi-form")};
            }
            const auto mdi_form = parse_bool_value(args[++index]);
            if (!mdi_form.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--mdi-form")};
            }
            result.request.mdi_form = *mdi_form;
            result.request.mdi_form_available = true;
            continue;
        }

        if (argument == "--back-style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--back-style")};
            }
            int back_style = 0;
            if (!parse_int_value(args[++index], back_style)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--back-style")};
            }
            if (back_style < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--back-style")};
            }
            result.request.back_style = back_style;
            result.request.back_style_available = true;
            continue;
        }

        if (argument == "--border-style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-style")};
            }
            int border_style = 0;
            if (!parse_int_value(args[++index], border_style)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--border-style")};
            }
            if (border_style < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--border-style")};
            }
            result.request.border_style = border_style;
            result.request.border_style_available = true;
            continue;
        }

        if (argument == "--border-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-width")};
            }
            int border_width = 0;
            if (!parse_int_value(args[++index], border_width)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--border-width")};
            }
            if (border_width < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--border-width")};
            }
            result.request.border_width = border_width;
            result.request.border_width_available = true;
            continue;
        }

        if (argument == "--border-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-color")};
            }
            int border_color = 0;
            if (!parse_int_value(args[++index], border_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--border-color")};
            }
            if (border_color < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--border-color")};
            }
            result.request.border_color = border_color;
            result.request.border_color_available = true;
            continue;
        }

        if (argument == "--special-effect") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--special-effect")};
            }
            int special_effect = 0;
            if (!parse_int_value(args[++index], special_effect)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--special-effect")};
            }
            if (special_effect < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--special-effect")};
            }
            result.request.special_effect = special_effect;
            result.request.special_effect_available = true;
            continue;
        }

        if (argument == "--scroll-bars") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--scroll-bars")};
            }
            int scroll_bars = 0;
            if (!parse_int_value(args[++index], scroll_bars)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--scroll-bars")};
            }
            if (scroll_bars < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--scroll-bars")};
            }
            result.request.scroll_bars = scroll_bars;
            result.request.scroll_bars_available = true;
            continue;
        }

        if (argument == "--window-state") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--window-state")};
            }
            int window_state = 0;
            if (!parse_int_value(args[++index], window_state)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--window-state")};
            }
            if (window_state < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--window-state")};
            }
            result.request.window_state = window_state;
            result.request.window_state_available = true;
            continue;
        }

        if (argument == "--show-window") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--show-window")};
            }
            int show_window = 0;
            if (!parse_int_value(args[++index], show_window)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--show-window")};
            }
            if (show_window < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--show-window")};
            }
            result.request.show_window = show_window;
            result.request.show_window_available = true;
            continue;
        }

        if (argument == "--title-bar") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--title-bar")};
            }
            int title_bar = 0;
            if (!parse_int_value(args[++index], title_bar)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--title-bar")};
            }
            if (title_bar < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--title-bar")};
            }
            result.request.title_bar = title_bar;
            result.request.title_bar_available = true;
            continue;
        }

        if (argument == "--mouse-pointer") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-pointer")};
            }
            int mouse_pointer = 0;
            if (!parse_int_value(args[++index], mouse_pointer)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--mouse-pointer")};
            }
            if (mouse_pointer < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--mouse-pointer")};
            }
            result.request.mouse_pointer = mouse_pointer;
            result.request.mouse_pointer_available = true;
            continue;
        }

        if (argument == "--picture-margin") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-margin")};
            }
            int picture_margin = 0;
            if (!parse_int_value(args[++index], picture_margin)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--picture-margin")};
            }
            if (picture_margin < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--picture-margin")};
            }
            result.request.picture_margin = picture_margin;
            result.request.picture_margin_available = true;
            continue;
        }

        if (argument == "--picture-position") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-position")};
            }
            int picture_position = 0;
            if (!parse_int_value(args[++index], picture_position)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--picture-position")};
            }
            if (picture_position < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--picture-position")};
            }
            result.request.picture_position = picture_position;
            result.request.picture_position_available = true;
            continue;
        }

        if (argument == "--picture-spacing") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-spacing")};
            }
            int picture_spacing = 0;
            if (!parse_int_value(args[++index], picture_spacing)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--picture-spacing")};
            }
            if (picture_spacing < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--picture-spacing")};
            }
            result.request.picture_spacing = picture_spacing;
            result.request.picture_spacing_available = true;
            continue;
        }

        if (argument == "--picture-selection-display") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-selection-display")};
            }
            int picture_selection_display = 0;
            if (!parse_int_value(args[++index], picture_selection_display)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--picture-selection-display")};
            }
            if (picture_selection_display < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--picture-selection-display")};
            }
            result.request.picture_selection_display = picture_selection_display;
            result.request.picture_selection_display_available = true;
            continue;
        }

        if (argument == "--dynamic-input-mask") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-input-mask")};
            }
            result.request.dynamic_input_mask = args[++index];
            result.request.dynamic_input_mask_available = true;
            continue;
        }

        if (argument == "--dynamic-line-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-line-height")};
            }
            result.request.dynamic_line_height = args[++index];
            result.request.dynamic_line_height_available = true;
            continue;
        }

        if (argument == "--dynamic-alignment") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-alignment")};
            }
            result.request.dynamic_alignment = args[++index];
            result.request.dynamic_alignment_available = true;
            continue;
        }

        if (argument == "--dynamic-current-control") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-current-control")};
            }
            result.request.dynamic_current_control = args[++index];
            result.request.dynamic_current_control_available = true;
            continue;
        }

        if (argument == "--dynamic-font-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-name")};
            }
            result.request.dynamic_font_name = args[++index];
            result.request.dynamic_font_name_available = true;
            continue;
        }

        if (argument == "--dynamic-font-size") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-size")};
            }
            result.request.dynamic_font_size = args[++index];
            result.request.dynamic_font_size_available = true;
            continue;
        }

        if (argument == "--dynamic-font-bold") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-bold")};
            }
            result.request.dynamic_font_bold = args[++index];
            result.request.dynamic_font_bold_available = true;
            continue;
        }

        if (argument == "--dynamic-font-italic") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-italic")};
            }
            result.request.dynamic_font_italic = args[++index];
            result.request.dynamic_font_italic_available = true;
            continue;
        }

        if (argument == "--dynamic-font-underline") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-underline")};
            }
            result.request.dynamic_font_underline = args[++index];
            result.request.dynamic_font_underline_available = true;
            continue;
        }

        if (argument == "--dynamic-font-strikethru") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-strikethru")};
            }
            result.request.dynamic_font_strikethru = args[++index];
            result.request.dynamic_font_strikethru_available = true;
            continue;
        }

        if (argument == "--dynamic-font-outline") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-outline")};
            }
            result.request.dynamic_font_outline = args[++index];
            result.request.dynamic_font_outline_available = true;
            continue;
        }

        if (argument == "--dynamic-font-shadow") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-shadow")};
            }
            result.request.dynamic_font_shadow = args[++index];
            result.request.dynamic_font_shadow_available = true;
            continue;
        }

        if (argument == "--font-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-name")};
            }
            result.request.font_name = args[++index];
            result.request.font_name_available = true;
            continue;
        }

        if (argument == "--font-size") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-size")};
            }
            double font_size = 0.0;
            if (!parse_double_value(args[++index], font_size) || !std::isfinite(font_size)) {
                return {.ok = false, .error = localized_numeric_value_required(catalog, "--font-size")};
            }
            if (font_size < 0.0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--font-size")};
            }
            result.request.font_size = font_size;
            result.request.font_size_available = true;
            continue;
        }

        if (argument == "--font-bold") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-bold")};
            }
            const auto font_bold = parse_bool_value(args[++index]);
            if (!font_bold.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--font-bold")};
            }
            result.request.font_bold = *font_bold;
            result.request.font_bold_available = true;
            continue;
        }

        if (argument == "--font-italic") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-italic")};
            }
            const auto font_italic = parse_bool_value(args[++index]);
            if (!font_italic.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--font-italic")};
            }
            result.request.font_italic = *font_italic;
            result.request.font_italic_available = true;
            continue;
        }

        if (argument == "--font-underline") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-underline")};
            }
            const auto font_underline = parse_bool_value(args[++index]);
            if (!font_underline.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--font-underline")};
            }
            result.request.font_underline = *font_underline;
            result.request.font_underline_available = true;
            continue;
        }

        if (argument == "--font-strikethru") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-strikethru")};
            }
            const auto font_strikethru = parse_bool_value(args[++index]);
            if (!font_strikethru.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--font-strikethru")};
            }
            result.request.font_strikethru = *font_strikethru;
            result.request.font_strikethru_available = true;
            continue;
        }

        if (argument == "--font-outline") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-outline")};
            }
            const auto font_outline = parse_bool_value(args[++index]);
            if (!font_outline.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--font-outline")};
            }
            result.request.font_outline = *font_outline;
            result.request.font_outline_available = true;
            continue;
        }

        if (argument == "--font-shadow") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-shadow")};
            }
            const auto font_shadow = parse_bool_value(args[++index]);
            if (!font_shadow.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--font-shadow")};
            }
            result.request.font_shadow = *font_shadow;
            result.request.font_shadow_available = true;
            continue;
        }

        if (argument == "--max-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-width")};
            }
            int max_width = 0;
            if (!parse_int_value(args[++index], max_width)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--max-width")};
            }
            if (max_width < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--max-width")};
            }
            result.request.max_width = max_width;
            result.request.max_width_available = true;
            continue;
        }

        if (argument == "--max-left") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-left")};
            }
            int max_left = 0;
            if (!parse_int_value(args[++index], max_left)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--max-left")};
            }
            if (max_left < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--max-left")};
            }
            result.request.max_left = max_left;
            result.request.max_left_available = true;
            continue;
        }

        if (argument == "--max-top") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-top")};
            }
            int max_top = 0;
            if (!parse_int_value(args[++index], max_top)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--max-top")};
            }
            if (max_top < 0) {
                return {.ok = false, .error = localized_not_negative_value_required(catalog, "--max-top")};
            }
            result.request.max_top = max_top;
            result.request.max_top_available = true;
            continue;
        }

        if (argument == "--auto-size") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-size")};
            }
            const auto auto_size = parse_bool_value(args[++index]);
            if (!auto_size.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--auto-size")};
            }
            result.request.auto_size = *auto_size;
            result.request.auto_size_available = true;
            continue;
        }

        if (argument == "--auto-release") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-release")};
            }
            const auto auto_release = parse_bool_value(args[++index]);
            if (!auto_release.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--auto-release")};
            }
            result.request.auto_release = *auto_release;
            result.request.auto_release_available = true;
            continue;
        }

        if (argument == "--continuous-scroll") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--continuous-scroll")};
            }
            const auto continuous_scroll = parse_bool_value(args[++index]);
            if (!continuous_scroll.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--continuous-scroll")};
            }
            result.request.continuous_scroll = *continuous_scroll;
            result.request.continuous_scroll_available = true;
            continue;
        }

        if (argument == "--dockable") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--dockable")};
            }
            const auto dockable = parse_bool_value(args[++index]);
            if (!dockable.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--dockable")};
            }
            result.request.dockable = *dockable;
            result.request.dockable_available = true;
            continue;
        }

        if (argument == "--clip-controls") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--clip-controls")};
            }
            const auto clip_controls = parse_bool_value(args[++index]);
            if (!clip_controls.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--clip-controls")};
            }
            result.request.clip_controls = *clip_controls;
            result.request.clip_controls_available = true;
            continue;
        }

        if (argument == "--sparse") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--sparse")};
            }
            const auto sparse = parse_bool_value(args[++index]);
            if (!sparse.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--sparse")};
            }
            result.request.sparse = *sparse;
            result.request.sparse_available = true;
            continue;
        }

        if (argument == "--lock-screen") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-screen")};
            }
            const auto lock_screen = parse_bool_value(args[++index]);
            if (!lock_screen.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--lock-screen")};
            }
            result.request.lock_screen = *lock_screen;
            result.request.lock_screen_available = true;
            continue;
        }

        if (argument == "--allow-cell-selection") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-cell-selection")};
            }
            const auto allow_cell_selection = parse_bool_value(args[++index]);
            if (!allow_cell_selection.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--allow-cell-selection")};
            }
            result.request.allow_cell_selection = *allow_cell_selection;
            result.request.allow_cell_selection_available = true;
            continue;
        }

        if (argument == "--hide-selection") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--hide-selection")};
            }
            const auto hide_selection = parse_bool_value(args[++index]);
            if (!hide_selection.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--hide-selection")};
            }
            result.request.hide_selection = *hide_selection;
            result.request.hide_selection_available = true;
            continue;
        }

        if (argument == "--delete-mark") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--delete-mark")};
            }
            const auto delete_mark = parse_bool_value(args[++index]);
            if (!delete_mark.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--delete-mark")};
            }
            result.request.delete_mark = *delete_mark;
            result.request.delete_mark_available = true;
            continue;
        }

        if (argument == "--record-mark") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-mark")};
            }
            const auto record_mark = parse_bool_value(args[++index]);
            if (!record_mark.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--record-mark")};
            }
            result.request.record_mark = *record_mark;
            result.request.record_mark_available = true;
            continue;
        }

        if (argument == "--split-bar") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--split-bar")};
            }
            const auto split_bar = parse_bool_value(args[++index]);
            if (!split_bar.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--split-bar")};
            }
            result.request.split_bar = *split_bar;
            result.request.split_bar_available = true;
            continue;
        }

        if (argument == "--highlight-row") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row")};
            }
            const auto highlight_row = parse_bool_value(args[++index]);
            if (!highlight_row.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--highlight-row")};
            }
            result.request.highlight_row = *highlight_row;
            result.request.highlight_row_available = true;
            continue;
        }

        if (argument == "--panel-link") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--panel-link")};
            }
            const auto panel_link = parse_bool_value(args[++index]);
            if (!panel_link.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--panel-link")};
            }
            result.request.panel_link = *panel_link;
            result.request.panel_link_available = true;
            continue;
        }

        if (argument == "--allow-header-sizing") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-header-sizing")};
            }
            const auto allow_header_sizing = parse_bool_value(args[++index]);
            if (!allow_header_sizing.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--allow-header-sizing")};
            }
            result.request.allow_header_sizing = *allow_header_sizing;
            result.request.allow_header_sizing_available = true;
            continue;
        }

        if (argument == "--allow-row-sizing") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-row-sizing")};
            }
            const auto allow_row_sizing = parse_bool_value(args[++index]);
            if (!allow_row_sizing.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--allow-row-sizing")};
            }
            result.request.allow_row_sizing = *allow_row_sizing;
            result.request.allow_row_sizing_available = true;
            continue;
        }

        if (argument == "--resizable") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--resizable")};
            }
            const auto resizable = parse_bool_value(args[++index]);
            if (!resizable.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--resizable")};
            }
            result.request.resizable = *resizable;
            result.request.resizable_available = true;
            continue;
        }

        if (argument == "--add-line-feeds") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--add-line-feeds")};
            }
            const auto add_line_feeds = parse_bool_value(args[++index]);
            if (!add_line_feeds.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--add-line-feeds")};
            }
            result.request.add_line_feeds = *add_line_feeds;
            result.request.add_line_feeds_available = true;
            continue;
        }

        if (argument == "--always-on-top") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--always-on-top")};
            }
            const auto always_on_top = parse_bool_value(args[++index]);
            if (!always_on_top.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--always-on-top")};
            }
            result.request.always_on_top = *always_on_top;
            result.request.always_on_top_available = true;
            continue;
        }

        if (argument == "--always-on-bottom") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--always-on-bottom")};
            }
            const auto always_on_bottom = parse_bool_value(args[++index]);
            if (!always_on_bottom.has_value()) {
                return {.ok = false, .error = localized_true_false_value_required(catalog, "--always-on-bottom")};
            }
            result.request.always_on_bottom = *always_on_bottom;
            result.request.always_on_bottom_available = true;
            continue;
        }

        if (argument == "--selected-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-back-color")};
            }
            int selected_back_color = 0;
            if (!parse_int_value(args[++index], selected_back_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--selected-back-color")};
            }
            result.request.selected_back_color = selected_back_color;
            result.request.selected_back_color_available = true;
            continue;
        }

        if (argument == "--selected-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-fore-color")};
            }
            int selected_fore_color = 0;
            if (!parse_int_value(args[++index], selected_fore_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--selected-fore-color")};
            }
            result.request.selected_fore_color = selected_fore_color;
            result.request.selected_fore_color_available = true;
            continue;
        }

        if (argument == "--selected-item-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-item-back-color")};
            }
            int selected_item_back_color = 0;
            if (!parse_int_value(args[++index], selected_item_back_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--selected-item-back-color")};
            }
            result.request.selected_item_back_color = selected_item_back_color;
            result.request.selected_item_back_color_available = true;
            continue;
        }

        if (argument == "--selected-item-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-item-fore-color")};
            }
            int selected_item_fore_color = 0;
            if (!parse_int_value(args[++index], selected_item_fore_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--selected-item-fore-color")};
            }
            result.request.selected_item_fore_color = selected_item_fore_color;
            result.request.selected_item_fore_color_available = true;
            continue;
        }

        if (argument == "--disabled-item-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-item-back-color")};
            }
            int disabled_item_back_color = 0;
            if (!parse_int_value(args[++index], disabled_item_back_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--disabled-item-back-color")};
            }
            result.request.disabled_item_back_color = disabled_item_back_color;
            result.request.disabled_item_back_color_available = true;
            continue;
        }

        if (argument == "--disabled-item-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-item-fore-color")};
            }
            int disabled_item_fore_color = 0;
            if (!parse_int_value(args[++index], disabled_item_fore_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--disabled-item-fore-color")};
            }
            result.request.disabled_item_fore_color = disabled_item_fore_color;
            result.request.disabled_item_fore_color_available = true;
            continue;
        }

        if (argument == "--item-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--item-back-color")};
            }
            int item_back_color = 0;
            if (!parse_int_value(args[++index], item_back_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--item-back-color")};
            }
            result.request.item_back_color = item_back_color;
            result.request.item_back_color_available = true;
            continue;
        }

        if (argument == "--item-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--item-fore-color")};
            }
            int item_fore_color = 0;
            if (!parse_int_value(args[++index], item_fore_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--item-fore-color")};
            }
            result.request.item_fore_color = item_fore_color;
            result.request.item_fore_color_available = true;
            continue;
        }

        if (argument == "--highlight-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-back-color")};
            }
            int highlight_back_color = 0;
            if (!parse_int_value(args[++index], highlight_back_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--highlight-back-color")};
            }
            result.request.highlight_back_color = highlight_back_color;
            result.request.highlight_back_color_available = true;
            continue;
        }

        if (argument == "--highlight-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-fore-color")};
            }
            int highlight_fore_color = 0;
            if (!parse_int_value(args[++index], highlight_fore_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--highlight-fore-color")};
            }
            result.request.highlight_fore_color = highlight_fore_color;
            result.request.highlight_fore_color_available = true;
            continue;
        }

        if (argument == "--back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--back-color")};
            }
            int back_color = 0;
            if (!parse_int_value(args[++index], back_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--back-color")};
            }
            result.request.back_color = back_color;
            result.request.back_color_available = true;
            continue;
        }

        if (argument == "--fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--fore-color")};
            }
            int fore_color = 0;
            if (!parse_int_value(args[++index], fore_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--fore-color")};
            }
            result.request.fore_color = fore_color;
            result.request.fore_color_available = true;
            continue;
        }

        if (argument == "--disabled-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-back-color")};
            }
            int disabled_back_color = 0;
            if (!parse_int_value(args[++index], disabled_back_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--disabled-back-color")};
            }
            result.request.disabled_back_color = disabled_back_color;
            result.request.disabled_back_color_available = true;
            continue;
        }

        if (argument == "--disabled-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-fore-color")};
            }
            int disabled_fore_color = 0;
            if (!parse_int_value(args[++index], disabled_fore_color)) {
                return {.ok = false, .error = localized_integer_value_required(catalog, "--disabled-fore-color")};
            }
            result.request.disabled_fore_color = disabled_fore_color;
            result.request.disabled_fore_color_available = true;
            continue;
        }

        if (argument == "--anchor-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--anchor-object-name")};
            }
            result.request.anchor_object_name = args[++index];
            continue;
        }

        if (argument == "--anchor-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--anchor-unique-id")};
            }
            result.request.anchor_unique_id = args[++index];
            continue;
        }

        if (argument == "--align-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--align-target-object-name")};
            }
            result.request.align_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--align-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--align-target-unique-id")};
            }
            result.request.align_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--resize-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--resize-target-object-name")};
            }
            result.request.resize_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--resize-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--resize-target-unique-id")};
            }
            result.request.resize_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--distribute-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--distribute-target-object-name")};
            }
            result.request.distribute_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--distribute-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--distribute-target-unique-id")};
            }
            result.request.distribute_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--snap-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--snap-target-object-name")};
            }
            result.request.snap_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--snap-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--snap-target-unique-id")};
            }
            result.request.snap_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--nudge-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--nudge-target-object-name")};
            }
            result.request.nudge_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--nudge-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--nudge-target-unique-id")};
            }
            result.request.nudge_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--tab-order-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-order-target-object-name")};
            }
            result.request.tab_order_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--tab-order-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-order-target-unique-id")};
            }
            result.request.tab_order_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--tab-stop-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-stop-target-object-name")};
            }
            result.request.tab_stop_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--tab-stop-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-stop-target-unique-id")};
            }
            result.request.tab_stop_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--visibility-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--visibility-target-object-name")};
            }
            result.request.visibility_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--visibility-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--visibility-target-unique-id")};
            }
            result.request.visibility_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--enabled-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--enabled-target-object-name")};
            }
            result.request.enabled_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--enabled-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--enabled-target-unique-id")};
            }
            result.request.enabled_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--read-only-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--read-only-target-object-name")};
            }
            result.request.read_only_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--read-only-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--read-only-target-unique-id")};
            }
            result.request.read_only_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--locked-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--locked-target-object-name")};
            }
            result.request.locked_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--locked-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--locked-target-unique-id")};
            }
            result.request.locked_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--caption-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--caption-target-object-name")};
            }
            result.request.caption_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--caption-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--caption-target-unique-id")};
            }
            result.request.caption_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-target-object-name")};
            }
            result.request.picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-target-unique-id")};
            }
            result.request.picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--down-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--down-picture-target-object-name")};
            }
            result.request.down_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--down-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--down-picture-target-unique-id")};
            }
            result.request.down_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-picture-target-object-name")};
            }
            result.request.disabled_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-picture-target-unique-id")};
            }
            result.request.disabled_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drag-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-picture-target-object-name")};
            }
            result.request.ole_drag_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drag-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-picture-target-unique-id")};
            }
            result.request.ole_drag_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--mouse-icon-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-icon-target-object-name")};
            }
            result.request.mouse_icon_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--mouse-icon-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-icon-target-unique-id")};
            }
            result.request.mouse_icon_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--drag-icon-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-icon-target-object-name")};
            }
            result.request.drag_icon_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--drag-icon-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-icon-target-unique-id")};
            }
            result.request.drag_icon_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--drag-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-mode-target-object-name")};
            }
            result.request.drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--drag-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-mode-target-unique-id")};
            }
            result.request.drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drag-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-mode-target-object-name")};
            }
            result.request.ole_drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drag-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-mode-target-unique-id")};
            }
            result.request.ole_drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drop-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-mode-target-object-name")};
            }
            result.request.ole_drop_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drop-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-mode-target-unique-id")};
            }
            result.request.ole_drop_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drop-effects-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-effects-target-object-name")};
            }
            result.request.ole_drop_effects_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drop-effects-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-effects-target-unique-id")};
            }
            result.request.ole_drop_effects_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drop-text-insertion-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-text-insertion-target-object-name")};
            }
            result.request.ole_drop_text_insertion_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drop-text-insertion-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-text-insertion-target-unique-id")};
            }
            result.request.ole_drop_text_insertion_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--button-count-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--button-count-target-object-name")};
            }
            result.request.button_count_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--button-count-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--button-count-target-unique-id")};
            }
            result.request.button_count_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--curvature-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--curvature-target-object-name")};
            }
            result.request.curvature_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--curvature-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--curvature-target-unique-id")};
            }
            result.request.curvature_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--draw-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-mode-target-object-name")};
            }
            result.request.draw_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--draw-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-mode-target-unique-id")};
            }
            result.request.draw_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--draw-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-style-target-object-name")};
            }
            result.request.draw_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--draw-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-style-target-unique-id")};
            }
            result.request.draw_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--draw-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-width-target-object-name")};
            }
            result.request.draw_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--draw-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-width-target-unique-id")};
            }
            result.request.draw_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--fill-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-style-target-object-name")};
            }
            result.request.fill_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--fill-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-style-target-unique-id")};
            }
            result.request.fill_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--scale-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--scale-mode-target-object-name")};
            }
            result.request.scale_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--scale-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--scale-mode-target-unique-id")};
            }
            result.request.scale_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--buffer-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-target-object-name")};
            }
            result.request.buffer_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--buffer-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-target-unique-id")};
            }
            result.request.buffer_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--buffer-mode-override-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-override-target-object-name")};
            }
            result.request.buffer_mode_override_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--buffer-mode-override-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-override-target-unique-id")};
            }
            result.request.buffer_mode_override_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--data-session-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--data-session-target-object-name")};
            }
            result.request.data_session_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--data-session-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--data-session-target-unique-id")};
            }
            result.request.data_session_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--grid-line-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-color-target-object-name")};
            }
            result.request.grid_line_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--grid-line-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-color-target-unique-id")};
            }
            result.request.grid_line_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--header-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--header-height-target-object-name")};
            }
            result.request.header_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--header-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--header-height-target-unique-id")};
            }
            result.request.header_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--row-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-height-target-object-name")};
            }
            result.request.row_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--row-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-height-target-unique-id")};
            }
            result.request.row_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--lock-columns-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-target-object-name")};
            }
            result.request.lock_columns_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--lock-columns-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-target-unique-id")};
            }
            result.request.lock_columns_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--lock-columns-left-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-left-target-object-name")};
            }
            result.request.lock_columns_left_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--lock-columns-left-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-left-target-unique-id")};
            }
            result.request.lock_columns_left_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--grid-line-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-width-target-object-name")};
            }
            result.request.grid_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--grid-line-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-width-target-unique-id")};
            }
            result.request.grid_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--grid-lines-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-lines-target-object-name")};
            }
            result.request.grid_lines_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--grid-lines-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-lines-target-unique-id")};
            }
            result.request.grid_lines_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-row-line-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row-line-width-target-object-name")};
            }
            result.request.highlight_row_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-row-line-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row-line-width-target-unique-id")};
            }
            result.request.highlight_row_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--partition-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --partition-target-object-name."};
            }
            result.request.partition_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--partition-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --partition-target-unique-id."};
            }
            result.request.partition_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--record-source-type-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-source-type-target-object-name."};
            }
            result.request.record_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--record-source-type-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-source-type-target-unique-id."};
            }
            result.request.record_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--column-order-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-order-target-object-name."};
            }
            result.request.column_order_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--column-order-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-order-target-unique-id."};
            }
            result.request.column_order_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-style-target-object-name."};
            }
            result.request.highlight_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-style-target-unique-id."};
            }
            result.request.highlight_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--child-order-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --child-order-target-object-name."};
            }
            result.request.child_order_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--child-order-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --child-order-target-unique-id."};
            }
            result.request.child_order_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--fill-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fill-color-target-object-name."};
            }
            result.request.fill_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--fill-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fill-color-target-unique-id."};
            }
            result.request.fill_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--list-item-id-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --list-item-id-target-object-name."};
            }
            result.request.list_item_id_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--list-item-id-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --list-item-id-target-unique-id."};
            }
            result.request.list_item_id_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--record-source-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-source-target-object-name."};
            }
            result.request.record_source_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--record-source-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-source-target-unique-id."};
            }
            result.request.record_source_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--tooltip-text-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tooltip-text-target-object-name."};
            }
            result.request.tooltip_text_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--tooltip-text-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tooltip-text-target-unique-id."};
            }
            result.request.tooltip_text_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--status-bar-text-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --status-bar-text-target-object-name."};
            }
            result.request.status_bar_text_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--status-bar-text-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --status-bar-text-target-unique-id."};
            }
            result.request.status_bar_text_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--link-master-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --link-master-target-object-name."};
            }
            result.request.link_master_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--link-master-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --link-master-target-unique-id."};
            }
            result.request.link_master_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--control-source-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-source-target-object-name."};
            }
            result.request.control_source_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--control-source-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-source-target-unique-id."};
            }
            result.request.control_source_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--current-control-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --current-control-target-object-name."};
            }
            result.request.current_control_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--current-control-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --current-control-target-unique-id."};
            }
            result.request.current_control_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--input-mask-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --input-mask-target-object-name."};
            }
            result.request.input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--input-mask-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --input-mask-target-unique-id."};
            }
            result.request.input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--format-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --format-target-object-name."};
            }
            result.request.format_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--format-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --format-target-unique-id."};
            }
            result.request.format_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--row-source-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-target-object-name."};
            }
            result.request.row_source_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--row-source-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-target-unique-id."};
            }
            result.request.row_source_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--column-widths-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-widths-target-object-name."};
            }
            result.request.column_widths_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--column-widths-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-widths-target-unique-id."};
            }
            result.request.column_widths_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--column-lines-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-lines-target-object-name."};
            }
            result.request.column_lines_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--column-lines-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-lines-target-unique-id."};
            }
            result.request.column_lines_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--integral-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --integral-height-target-object-name."};
            }
            result.request.integral_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--integral-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --integral-height-target-unique-id."};
            }
            result.request.integral_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--incremental-search-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --incremental-search-target-object-name."};
            }
            result.request.incremental_search_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--incremental-search-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --incremental-search-target-unique-id."};
            }
            result.request.incremental_search_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--multi-select-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --multi-select-target-object-name."};
            }
            result.request.multi_select_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--multi-select-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --multi-select-target-unique-id."};
            }
            result.request.multi_select_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--deleted-state-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --deleted-state-target-object-name."};
            }
            result.request.deleted_state_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {},
                .deleted = false,
                .deleted_available = false
            });
            continue;
        }

        if (argument == "--deleted-state-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --deleted-state-target-unique-id."};
            }
            result.request.deleted_state_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index],
                .deleted = false,
                .deleted_available = false
            });
            continue;
        }

        if (argument == "--deleted-state") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --deleted-state."};
            }
            const auto deleted_state = parse_bool_value(args[++index]);
            if (!deleted_state.has_value()) {
                return {.ok = false, .error = "The --deleted-state value must be true or false."};
            }
            auto pending = std::find_if(
                result.request.deleted_state_objects.rbegin(),
                result.request.deleted_state_objects.rend(),
                [](const StudioDeletedStateSelector& object) {
                    return !object.deleted_available;
                });
            if (pending == result.request.deleted_state_objects.rend()) {
                return {.ok = false, .error = "A deleted-state value requires a preceding deleted-state target selector."};
            }
            pending->deleted = *deleted_state;
            pending->deleted_available = true;
            continue;
        }

        if (argument == "--subtree-deleted") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --subtree-deleted."};
            }
            const auto subtree_deleted = parse_bool_value(args[++index]);
            if (!subtree_deleted.has_value()) {
                return {.ok = false, .error = "The --subtree-deleted value must be true or false."};
            }
            result.request.subtree_deleted = *subtree_deleted;
            result.request.subtree_deleted_available = true;
            continue;
        }

        if (argument == "--row-source-type-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-type-target-object-name."};
            }
            result.request.row_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--row-source-type-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-type-target-unique-id."};
            }
            result.request.row_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--bound-column-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --bound-column-target-object-name."};
            }
            result.request.bound_column_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--bound-column-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --bound-column-target-unique-id."};
            }
            result.request.bound_column_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--column-count-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-count-target-object-name."};
            }
            result.request.column_count_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--column-count-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-count-target-unique-id."};
            }
            result.request.column_count_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --style-target-object-name."};
            }
            result.request.style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --style-target-unique-id."};
            }
            result.request.style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--list-index-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --list-index-target-object-name."};
            }
            result.request.list_index_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--list-index-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --list-index-target-unique-id."};
            }
            result.request.list_index_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--left-column-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --left-column-target-object-name."};
            }
            result.request.left_column_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--left-column-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --left-column-target-unique-id."};
            }
            result.request.left_column_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--display-value-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --display-value-target-object-name."};
            }
            result.request.display_value_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--display-value-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --display-value-target-unique-id."};
            }
            result.request.display_value_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--selected-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-back-color-target-object-name."};
            }
            result.request.selected_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--selected-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-back-color-target-unique-id."};
            }
            result.request.selected_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--selected-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-fore-color-target-object-name."};
            }
            result.request.selected_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--selected-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-fore-color-target-unique-id."};
            }
            result.request.selected_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--selected-item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-back-color-target-object-name."};
            }
            result.request.selected_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--selected-item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-back-color-target-unique-id."};
            }
            result.request.selected_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--selected-item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-fore-color-target-object-name."};
            }
            result.request.selected_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--selected-item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-fore-color-target-unique-id."};
            }
            result.request.selected_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-back-color-target-object-name."};
            }
            result.request.disabled_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-back-color-target-unique-id."};
            }
            result.request.disabled_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-fore-color-target-object-name."};
            }
            result.request.disabled_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-fore-color-target-unique-id."};
            }
            result.request.disabled_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-back-color-target-object-name."};
            }
            result.request.item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-back-color-target-unique-id."};
            }
            result.request.item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-fore-color-target-object-name."};
            }
            result.request.item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-fore-color-target-unique-id."};
            }
            result.request.item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-back-color-target-object-name."};
            }
            result.request.highlight_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-back-color-target-unique-id."};
            }
            result.request.highlight_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-fore-color-target-object-name."};
            }
            result.request.highlight_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-fore-color-target-unique-id."};
            }
            result.request.highlight_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --back-color-target-object-name."};
            }
            result.request.back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --back-color-target-unique-id."};
            }
            result.request.back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fore-color-target-object-name."};
            }
            result.request.fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fore-color-target-unique-id."};
            }
            result.request.fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-back-color-target-object-name."};
            }
            result.request.disabled_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-back-color-target-unique-id."};
            }
            result.request.disabled_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-fore-color-target-object-name."};
            }
            result.request.disabled_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-fore-color-target-unique-id."};
            }
            result.request.disabled_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-back-color-target-object-name."};
            }
            result.request.dynamic_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-back-color-target-unique-id."};
            }
            result.request.dynamic_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-fore-color-target-object-name."};
            }
            result.request.dynamic_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-fore-color-target-unique-id."};
            }
            result.request.dynamic_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--closable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --closable-target-object-name."};
            }
            result.request.closable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--closable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --closable-target-unique-id."};
            }
            result.request.closable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--control-box-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-box-target-object-name."};
            }
            result.request.control_box_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--control-box-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-box-target-unique-id."};
            }
            result.request.control_box_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--allow-output-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-output-target-object-name."};
            }
            result.request.allow_output_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--allow-output-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-output-target-unique-id."};
            }
            result.request.allow_output_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--auto-center-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-center-target-object-name."};
            }
            result.request.auto_center_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--auto-center-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-center-target-unique-id."};
            }
            result.request.auto_center_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--auto-verb-menu-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-verb-menu-target-object-name."};
            }
            result.request.auto_verb_menu_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--auto-verb-menu-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-verb-menu-target-unique-id."};
            }
            result.request.auto_verb_menu_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--bind-controls-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --bind-controls-target-object-name."};
            }
            result.request.bind_controls_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--bind-controls-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --bind-controls-target-unique-id."};
            }
            result.request.bind_controls_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--desktop-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --desktop-target-object-name."};
            }
            result.request.desktop_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--desktop-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --desktop-target-unique-id."};
            }
            result.request.desktop_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--key-preview-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --key-preview-target-object-name."};
            }
            result.request.key_preview_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--key-preview-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --key-preview-target-unique-id."};
            }
            result.request.key_preview_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--mac-desktop-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mac-desktop-target-object-name."};
            }
            result.request.mac_desktop_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--mac-desktop-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mac-desktop-target-unique-id."};
            }
            result.request.mac_desktop_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--max-button-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-button-target-object-name."};
            }
            result.request.max_button_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--max-button-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-button-target-unique-id."};
            }
            result.request.max_button_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--min-button-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --min-button-target-object-name."};
            }
            result.request.min_button_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--min-button-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --min-button-target-unique-id."};
            }
            result.request.min_button_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--min-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --min-height-target-object-name."};
            }
            result.request.min_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--min-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --min-height-target-unique-id."};
            }
            result.request.min_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--min-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --min-width-target-object-name."};
            }
            result.request.min_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--min-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --min-width-target-unique-id."};
            }
            result.request.min_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--max-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-height-target-object-name."};
            }
            result.request.max_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--max-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-height-target-unique-id."};
            }
            result.request.max_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--movable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --movable-target-object-name."};
            }
            result.request.movable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--movable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --movable-target-unique-id."};
            }
            result.request.movable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--half-height-caption-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --half-height-caption-target-object-name."};
            }
            result.request.half_height_caption_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--half-height-caption-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --half-height-caption-target-unique-id."};
            }
            result.request.half_height_caption_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--mdi-form-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mdi-form-target-object-name."};
            }
            result.request.mdi_form_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--mdi-form-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mdi-form-target-unique-id."};
            }
            result.request.mdi_form_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--back-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --back-style-target-object-name."};
            }
            result.request.back_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--back-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --back-style-target-unique-id."};
            }
            result.request.back_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--border-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --border-style-target-object-name."};
            }
            result.request.border_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--border-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --border-style-target-unique-id."};
            }
            result.request.border_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--border-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --border-width-target-object-name."};
            }
            result.request.border_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--border-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --border-width-target-unique-id."};
            }
            result.request.border_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--border-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --border-color-target-object-name."};
            }
            result.request.border_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--border-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --border-color-target-unique-id."};
            }
            result.request.border_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--special-effect-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --special-effect-target-object-name."};
            }
            result.request.special_effect_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--special-effect-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --special-effect-target-unique-id."};
            }
            result.request.special_effect_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--scroll-bars-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --scroll-bars-target-object-name."};
            }
            result.request.scroll_bars_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--scroll-bars-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --scroll-bars-target-unique-id."};
            }
            result.request.scroll_bars_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--window-state-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --window-state-target-object-name."};
            }
            result.request.window_state_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--window-state-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --window-state-target-unique-id."};
            }
            result.request.window_state_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--show-window-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --show-window-target-object-name."};
            }
            result.request.show_window_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--show-window-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --show-window-target-unique-id."};
            }
            result.request.show_window_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--title-bar-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --title-bar-target-object-name."};
            }
            result.request.title_bar_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--title-bar-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --title-bar-target-unique-id."};
            }
            result.request.title_bar_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--mouse-pointer-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mouse-pointer-target-object-name."};
            }
            result.request.mouse_pointer_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--mouse-pointer-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mouse-pointer-target-unique-id."};
            }
            result.request.mouse_pointer_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--picture-margin-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-margin-target-object-name."};
            }
            result.request.picture_margin_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--picture-margin-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-margin-target-unique-id."};
            }
            result.request.picture_margin_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--picture-position-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-position-target-object-name."};
            }
            result.request.picture_position_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--picture-position-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-position-target-unique-id."};
            }
            result.request.picture_position_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--picture-spacing-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-spacing-target-object-name."};
            }
            result.request.picture_spacing_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--picture-spacing-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-spacing-target-unique-id."};
            }
            result.request.picture_spacing_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--picture-selection-display-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-selection-display-target-object-name."};
            }
            result.request.picture_selection_display_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--picture-selection-display-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-selection-display-target-unique-id."};
            }
            result.request.picture_selection_display_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-input-mask-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-input-mask-target-object-name."};
            }
            result.request.dynamic_input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-input-mask-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-input-mask-target-unique-id."};
            }
            result.request.dynamic_input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-line-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-line-height-target-object-name."};
            }
            result.request.dynamic_line_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-line-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-line-height-target-unique-id."};
            }
            result.request.dynamic_line_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-alignment-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-alignment-target-object-name."};
            }
            result.request.dynamic_alignment_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-alignment-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-alignment-target-unique-id."};
            }
            result.request.dynamic_alignment_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-current-control-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-current-control-target-object-name."};
            }
            result.request.dynamic_current_control_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-current-control-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-current-control-target-unique-id."};
            }
            result.request.dynamic_current_control_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-font-name-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-name-target-object-name."};
            }
            result.request.dynamic_font_name_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-font-name-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-name-target-unique-id."};
            }
            result.request.dynamic_font_name_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-font-size-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-size-target-object-name."};
            }
            result.request.dynamic_font_size_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-font-size-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-size-target-unique-id."};
            }
            result.request.dynamic_font_size_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-font-bold-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-bold-target-object-name."};
            }
            result.request.dynamic_font_bold_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-font-bold-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-bold-target-unique-id."};
            }
            result.request.dynamic_font_bold_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-font-italic-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-italic-target-object-name."};
            }
            result.request.dynamic_font_italic_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-font-italic-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-italic-target-unique-id."};
            }
            result.request.dynamic_font_italic_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-font-underline-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-underline-target-object-name."};
            }
            result.request.dynamic_font_underline_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-font-underline-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-underline-target-unique-id."};
            }
            result.request.dynamic_font_underline_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-font-strikethru-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-strikethru-target-object-name."};
            }
            result.request.dynamic_font_strikethru_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-font-strikethru-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-strikethru-target-unique-id."};
            }
            result.request.dynamic_font_strikethru_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-font-outline-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-outline-target-object-name."};
            }
            result.request.dynamic_font_outline_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-font-outline-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-outline-target-unique-id."};
            }
            result.request.dynamic_font_outline_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-font-shadow-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-shadow-target-object-name."};
            }
            result.request.dynamic_font_shadow_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-font-shadow-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-font-shadow-target-unique-id."};
            }
            result.request.dynamic_font_shadow_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--font-name-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-name-target-object-name."};
            }
            result.request.font_name_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--font-name-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-name-target-unique-id."};
            }
            result.request.font_name_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--font-size-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-size-target-object-name."};
            }
            result.request.font_size_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--font-size-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-size-target-unique-id."};
            }
            result.request.font_size_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--font-bold-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-bold-target-object-name."};
            }
            result.request.font_bold_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--font-bold-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-bold-target-unique-id."};
            }
            result.request.font_bold_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--font-italic-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-italic-target-object-name."};
            }
            result.request.font_italic_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--font-italic-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-italic-target-unique-id."};
            }
            result.request.font_italic_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--font-underline-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-underline-target-object-name."};
            }
            result.request.font_underline_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--font-underline-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-underline-target-unique-id."};
            }
            result.request.font_underline_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--font-strikethru-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-strikethru-target-object-name."};
            }
            result.request.font_strikethru_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--font-strikethru-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-strikethru-target-unique-id."};
            }
            result.request.font_strikethru_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--font-outline-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-outline-target-object-name."};
            }
            result.request.font_outline_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--font-outline-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-outline-target-unique-id."};
            }
            result.request.font_outline_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--font-shadow-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-shadow-target-object-name."};
            }
            result.request.font_shadow_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--font-shadow-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --font-shadow-target-unique-id."};
            }
            result.request.font_shadow_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--max-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-width-target-object-name."};
            }
            result.request.max_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--max-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-width-target-unique-id."};
            }
            result.request.max_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--max-left-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-left-target-object-name."};
            }
            result.request.max_left_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--max-left-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-left-target-unique-id."};
            }
            result.request.max_left_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--max-top-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-top-target-object-name."};
            }
            result.request.max_top_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--max-top-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --max-top-target-unique-id."};
            }
            result.request.max_top_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--auto-size-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-size-target-object-name."};
            }
            result.request.auto_size_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--auto-size-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-size-target-unique-id."};
            }
            result.request.auto_size_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--auto-release-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-release-target-object-name."};
            }
            result.request.auto_release_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--auto-release-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-release-target-unique-id."};
            }
            result.request.auto_release_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--continuous-scroll-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --continuous-scroll-target-object-name."};
            }
            result.request.continuous_scroll_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--continuous-scroll-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --continuous-scroll-target-unique-id."};
            }
            result.request.continuous_scroll_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dockable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dockable-target-object-name."};
            }
            result.request.dockable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dockable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dockable-target-unique-id."};
            }
            result.request.dockable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--clip-controls-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --clip-controls-target-object-name."};
            }
            result.request.clip_controls_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--clip-controls-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --clip-controls-target-unique-id."};
            }
            result.request.clip_controls_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--sparse-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --sparse-target-object-name."};
            }
            result.request.sparse_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--sparse-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --sparse-target-unique-id."};
            }
            result.request.sparse_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--lock-screen-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-screen-target-object-name."};
            }
            result.request.lock_screen_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--lock-screen-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-screen-target-unique-id."};
            }
            result.request.lock_screen_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--allow-cell-selection-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-cell-selection-target-object-name."};
            }
            result.request.allow_cell_selection_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--allow-cell-selection-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-cell-selection-target-unique-id."};
            }
            result.request.allow_cell_selection_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--hide-selection-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --hide-selection-target-object-name."};
            }
            result.request.hide_selection_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--hide-selection-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --hide-selection-target-unique-id."};
            }
            result.request.hide_selection_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--delete-mark-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --delete-mark-target-object-name."};
            }
            result.request.delete_mark_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--delete-mark-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --delete-mark-target-unique-id."};
            }
            result.request.delete_mark_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--record-mark-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-mark-target-object-name."};
            }
            result.request.record_mark_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--record-mark-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-mark-target-unique-id."};
            }
            result.request.record_mark_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--split-bar-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --split-bar-target-object-name."};
            }
            result.request.split_bar_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--split-bar-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --split-bar-target-unique-id."};
            }
            result.request.split_bar_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-row-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-row-target-object-name."};
            }
            result.request.highlight_row_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-row-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-row-target-unique-id."};
            }
            result.request.highlight_row_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--panel-link-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --panel-link-target-object-name."};
            }
            result.request.panel_link_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--panel-link-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --panel-link-target-unique-id."};
            }
            result.request.panel_link_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--allow-header-sizing-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-header-sizing-target-object-name."};
            }
            result.request.allow_header_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--allow-header-sizing-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-header-sizing-target-unique-id."};
            }
            result.request.allow_header_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--allow-row-sizing-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-row-sizing-target-object-name."};
            }
            result.request.allow_row_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--allow-row-sizing-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-row-sizing-target-unique-id."};
            }
            result.request.allow_row_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--resizable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --resizable-target-object-name."};
            }
            result.request.resizable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--resizable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --resizable-target-unique-id."};
            }
            result.request.resizable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--add-line-feeds-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --add-line-feeds-target-object-name."};
            }
            result.request.add_line_feeds_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--add-line-feeds-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --add-line-feeds-target-unique-id."};
            }
            result.request.add_line_feeds_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--always-on-top-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-top-target-object-name."};
            }
            result.request.always_on_top_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--always-on-top-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-top-target-unique-id."};
            }
            result.request.always_on_top_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--always-on-bottom-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-bottom-target-object-name."};
            }
            result.request.always_on_bottom_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--always-on-bottom-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-bottom-target-unique-id."};
            }
            result.request.always_on_bottom_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --object-name."};
            }
            result.request.object_name = args[++index];
            continue;
        }

        if (argument == "--unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --unique-id."};
            }
            result.request.unique_id = args[++index];
            continue;
        }

        if (argument == "--line") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --line."};
            }
            std::size_t line = 0;
            if (!parse_size_value(args[++index], line)) {
                return {.ok = false, .error = "The --line value must be an unsigned integer."};
            }
            result.request.line = line;
            continue;
        }

        if (argument == "--column") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column."};
            }
            std::size_t column = 0;
            if (!parse_size_value(args[++index], column)) {
                return {.ok = false, .error = "The --column value must be an unsigned integer."};
            }
            result.request.column = column;
            continue;
        }

        if (argument == "--undo-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --undo-mode."};
            }
            std::string mode = lowercase_copy(args[++index]);
            if (mode == "edit") {
                result.request.undo_mode = StudioUndoMode::edit;
                continue;
            }
            if (mode == "command") {
                result.request.undo_mode = StudioUndoMode::command;
                continue;
            }
            return {.ok = false, .error = "The --undo-mode value must be edit or command."};
        }

        if (argument == "--undo-label") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --undo-label."};
            }
            result.request.undo_label = args[++index];
            continue;
        }

        if (!argument.empty() && argument[0] == '-') {
            return {.ok = false, .error = "Unknown argument: " + argument};
        }

        if (result.request.path.empty()) {
            result.request.path = argument;
            continue;
        }

        return {.ok = false, .error = "Unexpected extra positional argument: " + argument};
    }

    if (result.request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }

    if (result.request.apply_property_update && result.request.property_name.empty()) {
        return {.ok = false, .error = "A property update requires --property-name."};
    }
    if (result.request.clear_property && result.request.property_name.empty()) {
        return {.ok = false, .error = "A property clear requires --property-name."};
    }
    if (result.request.rename_property && result.request.property_name.empty()) {
        return {.ok = false, .error = "A property rename requires --property-name."};
    }
    if (result.request.rename_property && result.request.new_property_name.empty()) {
        return {.ok = false, .error = "A property rename requires --new-property-name."};
    }
    if (result.request.rename_object &&
        result.request.new_object_name.empty() &&
        result.request.new_name.empty() &&
        result.request.new_unique_id.empty()) {
        return {.ok = false, .error = "An object rename requires --new-object-name, --new-name, or --new-unique-id."};
    }
    if (result.request.reparent_object &&
        !result.request.clear_parent &&
        result.request.parent_name.empty() &&
        result.request.parent_unique_id.empty()) {
        return {.ok = false, .error = "An object reparent requires --parent-name, --parent-unique-id, or --clear-parent."};
    }
    if (result.request.reorder_object && result.request.placement.empty()) {
        return {.ok = false, .error = "An object reorder requires --placement."};
    }
    if (result.request.group_object && result.request.field_values.empty()) {
        return {.ok = false, .error = "An object group requires at least one --field-value."};
    }
    if (result.request.group_object && result.request.group_objects.empty()) {
        return {.ok = false, .error = "An object group requires at least one grouped child selector."};
    }
    if (!result.request.group_object && !result.request.field_values.empty()) {
        return {.ok = false, .error = "--field-value can only be used with --group-object."};
    }
    if (!result.request.group_object && !result.request.group_objects.empty()) {
        return {.ok = false, .error = "Grouped child selectors can only be used with --group-object."};
    }
    if (result.request.align_object && result.request.alignment_mode.empty()) {
        return {.ok = false, .error = localized_object_action_requires_option(
            catalog,
            "alignment",
            "--alignment-mode")};
    }
    if (result.request.align_object &&
        result.request.anchor_object_name.empty() &&
        result.request.anchor_unique_id.empty()) {
        return {.ok = false, .error = localized_object_action_requires_either_option(
            catalog,
            "alignment",
            "--anchor-object-name",
            "--anchor-unique-id")};
    }
    if (result.request.align_object && result.request.align_objects.empty()) {
        return {.ok = false, .error = localized_object_action_requires_target(catalog, "alignment")};
    }
    if (!result.request.align_object &&
        (!result.request.alignment_mode.empty() ||
         !result.request.align_objects.empty())) {
        return {.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            "Alignment",
            "--align-object")};
    }
    if (result.request.resize_object && result.request.resize_mode.empty()) {
        return {.ok = false, .error = localized_object_action_requires_option(
            catalog,
            "resize",
            "--resize-mode")};
    }
    if (result.request.resize_object &&
        result.request.anchor_object_name.empty() &&
        result.request.anchor_unique_id.empty()) {
        return {.ok = false, .error = localized_object_action_requires_either_option(
            catalog,
            "resize",
            "--anchor-object-name",
            "--anchor-unique-id")};
    }
    if (result.request.resize_object && result.request.resize_objects.empty()) {
        return {.ok = false, .error = localized_object_action_requires_target(catalog, "resize")};
    }
    if (!result.request.resize_object &&
        (!result.request.resize_mode.empty() ||
         !result.request.resize_objects.empty())) {
        return {.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            "Resize",
            "--resize-object")};
    }
    if (result.request.distribute_object && result.request.distribution_mode.empty()) {
        return {.ok = false, .error = localized_object_action_requires_option(
            catalog,
            "distribution",
            "--distribution-mode")};
    }
    if (result.request.distribute_object && result.request.distribute_objects.empty()) {
        return {.ok = false, .error = localized_object_action_requires_target(catalog, "distribution")};
    }
    if (!result.request.distribute_object &&
        (!result.request.distribution_mode.empty() ||
         !result.request.distribute_objects.empty())) {
        return {.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            "Distribution",
            "--distribute-object")};
    }
    if (result.request.snap_object && result.request.snap_mode.empty()) {
        return {.ok = false, .error = localized_object_action_requires_option(
            catalog,
            "snap",
            "--snap-mode")};
    }
    if (result.request.snap_object && result.request.snap_objects.empty()) {
        return {.ok = false, .error = localized_object_action_requires_target(catalog, "snap")};
    }
    if (!result.request.snap_object &&
        (!result.request.snap_mode.empty() ||
         result.request.grid_width != 0.0 ||
         result.request.grid_height != 0.0 ||
         !result.request.snap_objects.empty())) {
        return {.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            "Snap",
            "--snap-object")};
    }
    if (result.request.nudge_object && result.request.nudge_mode.empty()) {
        return {.ok = false, .error = localized_object_action_requires_option(
            catalog,
            "nudge",
            "--nudge-mode")};
    }
    if (result.request.nudge_object && result.request.nudge_objects.empty()) {
        return {.ok = false, .error = localized_object_action_requires_target(catalog, "nudge")};
    }
    if (!result.request.nudge_object &&
        (!result.request.nudge_mode.empty() ||
         result.request.delta_hpos != 0.0 ||
         result.request.delta_vpos != 0.0 ||
         !result.request.nudge_objects.empty())) {
        return {.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            "Nudge",
            "--nudge-object")};
    }
    if (result.request.tab_order_object && result.request.tab_order_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "tab-order")};
    }
    if (result.request.tab_order_object && result.request.starting_tab_index < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "tab-order",
            "starting tab index")};
    }
    if (!result.request.tab_order_object &&
        (result.request.starting_tab_index_available ||
         !result.request.tab_order_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Tab-order",
            "--tab-order-object")};
    }
    if (result.request.tab_stop_object && !result.request.tab_stop_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "tab-stop",
            "--tab-stop")};
    }
    if (result.request.tab_stop_object && result.request.tab_stop_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "tab-stop")};
    }
    if (!result.request.tab_stop_object &&
        (result.request.tab_stop_available ||
         !result.request.tab_stop_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Tab-stop",
            "--tab-stop-object")};
    }
    if (result.request.visibility_object && !result.request.visible_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "visibility",
            "--visible")};
    }
    if (result.request.visibility_object && result.request.visibility_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "visibility")};
    }
    if (!result.request.visibility_object &&
        (result.request.visible_available ||
         !result.request.visibility_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Visibility",
            "--visibility-object")};
    }
    if (result.request.enabled_object && !result.request.enabled_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "enabled",
            "--enabled")};
    }
    if (result.request.enabled_object && result.request.enabled_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "enabled")};
    }
    if (!result.request.enabled_object &&
        (result.request.enabled_available ||
         !result.request.enabled_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Enabled",
            "--enabled-object")};
    }
    if (result.request.read_only_object && !result.request.object_read_only_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "read-only",
            "--object-read-only")};
    }
    if (result.request.read_only_object && result.request.read_only_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "read-only")};
    }
    if (!result.request.read_only_object &&
        (result.request.object_read_only_available ||
         !result.request.read_only_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Read-only",
            "--read-only-object")};
    }
    if (result.request.locked_object && !result.request.locked_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "locked",
            "--locked")};
    }
    if (result.request.locked_object && result.request.locked_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "locked")};
    }
    if (!result.request.locked_object &&
        (result.request.locked_available ||
         !result.request.locked_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Locked",
            "--locked-object")};
    }
    if (result.request.caption_object && !result.request.caption_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "caption",
            "--caption")};
    }
    if (result.request.caption_object && result.request.caption_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "caption")};
    }
    if (!result.request.caption_object &&
        (result.request.caption_available ||
         !result.request.caption_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Caption",
            "--caption-object")};
    }
    if (result.request.picture_object && !result.request.picture_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "picture",
            "--picture")};
    }
    if (result.request.picture_object && result.request.picture_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "picture")};
    }
    if (!result.request.picture_object &&
        (result.request.picture_available ||
         !result.request.picture_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Picture",
            "--picture-object")};
    }
    if (result.request.down_picture_object && !result.request.down_picture_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "down-picture",
            "--down-picture")};
    }
    if (result.request.down_picture_object && result.request.down_picture_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "down-picture")};
    }
    if (!result.request.down_picture_object &&
        (result.request.down_picture_available ||
         !result.request.down_picture_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Down-picture",
            "--down-picture-object")};
    }
    if (result.request.disabled_picture_object && !result.request.disabled_picture_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "disabled-picture",
            "--disabled-picture")};
    }
    if (result.request.disabled_picture_object && result.request.disabled_picture_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "disabled-picture")};
    }
    if (!result.request.disabled_picture_object &&
        (result.request.disabled_picture_available ||
         !result.request.disabled_picture_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Disabled-picture",
            "--disabled-picture-object")};
    }
    if (result.request.ole_drag_picture_object && !result.request.ole_drag_picture_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "OLE drag-picture",
            "--ole-drag-picture")};
    }
    if (result.request.ole_drag_picture_object && result.request.ole_drag_picture_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "OLE drag-picture")};
    }
    if (!result.request.ole_drag_picture_object &&
        (result.request.ole_drag_picture_available ||
         !result.request.ole_drag_picture_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "OLE drag-picture",
            "--ole-drag-picture-object")};
    }
    if (result.request.mouse_icon_object && !result.request.mouse_icon_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "mouse-icon",
            "--mouse-icon")};
    }
    if (result.request.mouse_icon_object && result.request.mouse_icon_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "mouse-icon")};
    }
    if (!result.request.mouse_icon_object &&
        (result.request.mouse_icon_available ||
         !result.request.mouse_icon_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Mouse-icon",
            "--mouse-icon-object")};
    }
    if (result.request.drag_icon_object && !result.request.drag_icon_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "drag-icon",
            "--drag-icon")};
    }
    if (result.request.drag_icon_object && result.request.drag_icon_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "drag-icon")};
    }
    if (!result.request.drag_icon_object &&
        (result.request.drag_icon_available ||
         !result.request.drag_icon_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Drag-icon",
            "--drag-icon-object")};
    }
    if (result.request.drag_mode_object && !result.request.drag_mode_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "drag-mode",
            "--drag-mode")};
    }
    if (result.request.drag_mode_object && result.request.drag_mode_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "drag-mode")};
    }
    if (!result.request.drag_mode_object &&
        (result.request.drag_mode_available ||
         !result.request.drag_mode_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Drag-mode",
            "--drag-mode-object")};
    }
    if (result.request.ole_drag_mode_object && !result.request.ole_drag_mode_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "OLE drag-mode",
            "--ole-drag-mode")};
    }
    if (result.request.ole_drag_mode_object && result.request.ole_drag_mode_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "OLE drag-mode")};
    }
    if (!result.request.ole_drag_mode_object &&
        (result.request.ole_drag_mode_available ||
         !result.request.ole_drag_mode_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "OLE drag-mode",
            "--ole-drag-mode-object")};
    }
    if (result.request.ole_drop_mode_object && !result.request.ole_drop_mode_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "OLE drop-mode",
            "--ole-drop-mode")};
    }
    if (result.request.ole_drop_mode_object && result.request.ole_drop_mode_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "OLE drop-mode")};
    }
    if (!result.request.ole_drop_mode_object &&
        (result.request.ole_drop_mode_available ||
         !result.request.ole_drop_mode_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "OLE drop-mode",
            "--ole-drop-mode-object")};
    }
    if (result.request.ole_drop_effects_object && !result.request.ole_drop_effects_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "OLE drop-effects",
            "--ole-drop-effects")};
    }
    if (result.request.ole_drop_effects_object && result.request.ole_drop_effects_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "OLE drop-effects")};
    }
    if (!result.request.ole_drop_effects_object &&
        (result.request.ole_drop_effects_available ||
         !result.request.ole_drop_effects_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "OLE drop-effects",
            "--ole-drop-effects-object")};
    }
    if (result.request.ole_drop_text_insertion_object && !result.request.ole_drop_text_insertion_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "OLE drop text-insertion",
            "--ole-drop-text-insertion")};
    }
    if (result.request.ole_drop_text_insertion_object && result.request.ole_drop_text_insertion_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "OLE drop text-insertion")};
    }
    if (!result.request.ole_drop_text_insertion_object &&
        (result.request.ole_drop_text_insertion_available ||
         !result.request.ole_drop_text_insertion_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "OLE drop text-insertion",
            "--ole-drop-text-insertion-object")};
    }
    if (result.request.button_count_object && !result.request.button_count_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "button-count",
            "--button-count")};
    }
    if (result.request.button_count_object && result.request.button_count_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "button-count")};
    }
    if (!result.request.button_count_object &&
        (result.request.button_count_available ||
         !result.request.button_count_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Button-count",
            "--button-count-object")};
    }
    if (result.request.curvature_object && !result.request.curvature_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "curvature",
            "--curvature")};
    }
    if (result.request.curvature_object && result.request.curvature_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "curvature")};
    }
    if (!result.request.curvature_object &&
        (result.request.curvature_available ||
         !result.request.curvature_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Curvature",
            "--curvature-object")};
    }
    if (result.request.draw_mode_object && !result.request.draw_mode_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "draw-mode",
            "--draw-mode")};
    }
    if (result.request.draw_mode_object && result.request.draw_mode_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "draw-mode")};
    }
    if (!result.request.draw_mode_object &&
        (result.request.draw_mode_available ||
         !result.request.draw_mode_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Draw-mode",
            "--draw-mode-object")};
    }
    if (result.request.draw_style_object && !result.request.draw_style_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "draw-style",
            "--draw-style")};
    }
    if (result.request.draw_style_object && result.request.draw_style_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "draw-style")};
    }
    if (!result.request.draw_style_object &&
        (result.request.draw_style_available ||
         !result.request.draw_style_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Draw-style",
            "--draw-style-object")};
    }
    if (result.request.draw_width_object && !result.request.draw_width_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "draw-width",
            "--draw-width")};
    }
    if (result.request.draw_width_object && result.request.draw_width_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "draw-width")};
    }
    if (!result.request.draw_width_object &&
        (result.request.draw_width_available ||
         !result.request.draw_width_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Draw-width",
            "--draw-width-object")};
    }
    if (result.request.fill_style_object && !result.request.fill_style_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "fill-style",
            "--fill-style")};
    }
    if (result.request.fill_style_object && result.request.fill_style_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "fill-style")};
    }
    if (!result.request.fill_style_object &&
        (result.request.fill_style_available ||
         !result.request.fill_style_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Fill-style",
            "--fill-style-object")};
    }
    if (result.request.scale_mode_object && !result.request.scale_mode_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "scale-mode",
            "--scale-mode")};
    }
    if (result.request.scale_mode_object && result.request.scale_mode_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "scale-mode")};
    }
    if (!result.request.scale_mode_object &&
        (result.request.scale_mode_available ||
         !result.request.scale_mode_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Scale-mode",
            "--scale-mode-object")};
    }
    if (result.request.buffer_mode_object && !result.request.buffer_mode_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "buffer-mode",
            "--buffer-mode")};
    }
    if (result.request.buffer_mode_object && result.request.buffer_mode_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "buffer-mode")};
    }
    if (!result.request.buffer_mode_object &&
        (result.request.buffer_mode_available ||
         !result.request.buffer_mode_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Buffer-mode",
            "--buffer-mode-object")};
    }
    if (result.request.buffer_mode_override_object && !result.request.buffer_mode_override_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "buffer-mode-override",
            "--buffer-mode-override")};
    }
    if (result.request.buffer_mode_override_object && result.request.buffer_mode_override_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "buffer-mode-override")};
    }
    if (!result.request.buffer_mode_override_object &&
        (result.request.buffer_mode_override_available ||
         !result.request.buffer_mode_override_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Buffer-mode-override",
            "--buffer-mode-override-object")};
    }
    if (result.request.data_session_object && !result.request.data_session_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "data-session",
            "--data-session")};
    }
    if (result.request.data_session_object && result.request.data_session_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "data-session")};
    }
    if (!result.request.data_session_object &&
        (result.request.data_session_available ||
         !result.request.data_session_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Data-session",
            "--data-session-object")};
    }
    if (result.request.grid_line_color_object && !result.request.grid_line_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "grid-line-color",
            "--grid-line-color")};
    }
    if (result.request.grid_line_color_object && result.request.grid_line_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "grid-line-color")};
    }
    if (!result.request.grid_line_color_object &&
        (result.request.grid_line_color_available ||
         !result.request.grid_line_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Grid-line-color",
            "--grid-line-color-object")};
    }
    if (result.request.header_height_object && !result.request.header_height_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "header-height",
            "--header-height")};
    }
    if (result.request.header_height_object && result.request.header_height_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "header-height")};
    }
    if (!result.request.header_height_object &&
        (result.request.header_height_available ||
         !result.request.header_height_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Header-height",
            "--header-height-object")};
    }
    if (result.request.row_height_object && !result.request.row_height_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "row-height",
            "--row-height")};
    }
    if (result.request.row_height_object && result.request.row_height_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "row-height")};
    }
    if (!result.request.row_height_object &&
        (result.request.row_height_available ||
         !result.request.row_height_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Row-height",
            "--row-height-object")};
    }
    if (result.request.lock_columns_object && !result.request.lock_columns_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "lock-columns",
            "--lock-columns")};
    }
    if (result.request.lock_columns_object && result.request.lock_columns_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "lock-columns")};
    }
    if (!result.request.lock_columns_object &&
        (result.request.lock_columns_available ||
         !result.request.lock_columns_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Lock-columns",
            "--lock-columns-object")};
    }
    if (result.request.lock_columns_left_object && !result.request.lock_columns_left_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "lock-columns-left",
            "--lock-columns-left")};
    }
    if (result.request.lock_columns_left_object && result.request.lock_columns_left_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "lock-columns-left")};
    }
    if (!result.request.lock_columns_left_object &&
        (result.request.lock_columns_left_available ||
         !result.request.lock_columns_left_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Lock-columns-left",
            "--lock-columns-left-object")};
    }
    if (result.request.grid_line_width_object && !result.request.grid_line_width_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "grid-line-width",
            "--grid-line-width")};
    }
    if (result.request.grid_line_width_object && result.request.grid_line_width_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "grid-line-width")};
    }
    if (!result.request.grid_line_width_object &&
        (result.request.grid_line_width_available ||
         !result.request.grid_line_width_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Grid-line-width",
            "--grid-line-width-object")};
    }
    if (result.request.grid_lines_object && !result.request.grid_lines_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "grid-lines",
            "--grid-lines")};
    }
    if (result.request.grid_lines_object && result.request.grid_lines_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "grid-lines")};
    }
    if (!result.request.grid_lines_object &&
        (result.request.grid_lines_available ||
         !result.request.grid_lines_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Grid-lines",
            "--grid-lines-object")};
    }
    if (result.request.highlight_row_line_width_object && !result.request.highlight_row_line_width_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "highlight-row-line-width",
            "--highlight-row-line-width")};
    }
    if (result.request.highlight_row_line_width_object && result.request.highlight_row_line_width_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "highlight-row-line-width")};
    }
    if (!result.request.highlight_row_line_width_object &&
        (result.request.highlight_row_line_width_available ||
         !result.request.highlight_row_line_width_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Highlight-row-line-width",
            "--highlight-row-line-width-object")};
    }
    if (result.request.partition_object && !result.request.partition_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "partition",
            "--partition")};
    }
    if (result.request.partition_object && result.request.partition_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "partition")};
    }
    if (!result.request.partition_object &&
        (result.request.partition_available ||
         !result.request.partition_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Partition",
            "--partition-object")};
    }
    if (result.request.record_source_type_object && !result.request.record_source_type_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "record-source-type",
            "--record-source-type")};
    }
    if (result.request.record_source_type_object && result.request.record_source_type_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "record-source-type")};
    }
    if (!result.request.record_source_type_object &&
        (result.request.record_source_type_available ||
         !result.request.record_source_type_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "RecordSourceType",
            "--record-source-type-object")};
    }
    if (result.request.column_order_object && !result.request.column_order_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "column-order",
            "--column-order")};
    }
    if (result.request.column_order_object && result.request.column_order_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "column-order")};
    }
    if (!result.request.column_order_object &&
        (result.request.column_order_available ||
         !result.request.column_order_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "ColumnOrder",
            "--column-order-object")};
    }
    if (result.request.highlight_style_object && !result.request.highlight_style_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "highlight-style",
            "--highlight-style")};
    }
    if (result.request.highlight_style_object && result.request.highlight_style_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "highlight-style")};
    }
    if (!result.request.highlight_style_object &&
        (result.request.highlight_style_available ||
         !result.request.highlight_style_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "HighlightStyle",
            "--highlight-style-object")};
    }
    if (result.request.child_order_object && !result.request.child_order_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "child-order",
            "--child-order")};
    }
    if (result.request.child_order_object && result.request.child_order_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "child-order")};
    }
    if (!result.request.child_order_object &&
        (result.request.child_order_available ||
         !result.request.child_order_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "ChildOrder",
            "--child-order-object")};
    }
    if (result.request.fill_color_object && !result.request.fill_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "fill-color",
            "--fill-color")};
    }
    if (result.request.fill_color_object && result.request.fill_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "fill-color")};
    }
    if (!result.request.fill_color_object &&
        (result.request.fill_color_available ||
         !result.request.fill_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "FillColor",
            "--fill-color-object")};
    }
    if (result.request.list_item_id_object && !result.request.list_item_id_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "list-item-id",
            "--list-item-id")};
    }
    if (result.request.list_item_id_object && result.request.list_item_id_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "list-item-id")};
    }
    if (!result.request.list_item_id_object &&
        (result.request.list_item_id_available ||
         !result.request.list_item_id_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "ListItemId",
            "--list-item-id-object")};
    }
    if (result.request.record_source_object && !result.request.record_source_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "record source",
            "--record-source")};
    }
    if (result.request.record_source_object && result.request.record_source_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "record source")};
    }
    if (!result.request.record_source_object &&
        (result.request.record_source_available ||
         !result.request.record_source_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Record source",
            "--record-source-object")};
    }
    if (const auto form_set_class_error = validate_form_set_class_request(result.request, catalog)) {
        return {.ok = false, .error = *form_set_class_error};
    }
    if (const auto default_file_path_error = validate_default_file_path_request(result.request, catalog)) {
        return {.ok = false, .error = *default_file_path_error};
    }
    if (const auto initial_selected_alias_error = validate_initial_selected_alias_request(result.request, catalog)) {
        return {.ok = false, .error = *initial_selected_alias_error};
    }
    if (const auto tab_orientation_error = validate_tab_orientation_request(result.request, catalog)) {
        return {.ok = false, .error = *tab_orientation_error};
    }
    if (const auto display_orientation_error = validate_display_orientation_request(result.request, catalog)) {
        return {.ok = false, .error = *display_orientation_error};
    }
    if (const auto help_context_id_error = validate_help_context_id_request(result.request, catalog)) {
        return {.ok = false, .error = *help_context_id_error};
    }
    if (const auto whats_this_help_id_error = validate_whats_this_help_id_request(result.request, catalog)) {
        return {.ok = false, .error = *whats_this_help_id_error};
    }
    if (const auto whats_this_help_error = validate_whats_this_help_request(result.request, catalog)) {
        return {.ok = false, .error = *whats_this_help_error};
    }
    if (const auto whats_this_button_error = validate_whats_this_button_request(result.request, catalog)) {
        return {.ok = false, .error = *whats_this_button_error};
    }
    if (result.request.tooltip_text_object && !result.request.tooltip_text_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "tooltip text",
            "--tooltip-text")};
    }
    if (result.request.tooltip_text_object && result.request.tooltip_text_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "tooltip text")};
    }
    if (!result.request.tooltip_text_object &&
        (result.request.tooltip_text_available ||
         !result.request.tooltip_text_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Tooltip text",
            "--tooltip-text-object")};
    }
    if (result.request.status_bar_text_object && !result.request.status_bar_text_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "status-bar text",
            "--status-bar-text")};
    }
    if (result.request.status_bar_text_object && result.request.status_bar_text_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "status-bar text")};
    }
    if (!result.request.status_bar_text_object &&
        (result.request.status_bar_text_available ||
         !result.request.status_bar_text_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Status-bar text",
            "--status-bar-text-object")};
    }
    if (result.request.link_master_object && !result.request.link_master_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "link-master",
            "--link-master")};
    }
    if (result.request.link_master_object && result.request.link_master_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "link-master")};
    }
    if (!result.request.link_master_object &&
        (result.request.link_master_available ||
         !result.request.link_master_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Link-master",
            "--link-master-object")};
    }
    if (result.request.control_source_object && !result.request.control_source_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "control-source",
            "--control-source")};
    }
    if (result.request.control_source_object && result.request.control_source_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "control-source")};
    }
    if (!result.request.control_source_object &&
        (result.request.control_source_available ||
         !result.request.control_source_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Control-source",
            "--control-source-object")};
    }
    if (result.request.current_control_object && !result.request.current_control_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "current-control",
            "--current-control")};
    }
    if (result.request.current_control_object && result.request.current_control_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "current-control")};
    }
    if (!result.request.current_control_object &&
        (result.request.current_control_available ||
         !result.request.current_control_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Current-control",
            "--current-control-object")};
    }
    if (result.request.input_mask_object && !result.request.input_mask_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "input-mask",
            "--input-mask")};
    }
    if (result.request.input_mask_object && result.request.input_mask_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "input-mask")};
    }
    if (!result.request.input_mask_object &&
        (result.request.input_mask_available ||
         !result.request.input_mask_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Input-mask",
            "--input-mask-object")};
    }
    if (result.request.format_object && !result.request.format_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "format",
            "--format")};
    }
    if (result.request.format_object && result.request.format_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "format")};
    }
    if (!result.request.format_object &&
        (result.request.format_available ||
         !result.request.format_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Format",
            "--format-object")};
    }
    if (result.request.row_source_object && !result.request.row_source_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "row-source",
            "--row-source")};
    }
    if (result.request.row_source_object && result.request.row_source_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "row-source")};
    }
    if (!result.request.row_source_object &&
        (result.request.row_source_available ||
         !result.request.row_source_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Row-source",
            "--row-source-object")};
    }
    if (result.request.column_widths_object && !result.request.column_widths_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "column-widths",
            "--column-widths")};
    }
    if (result.request.column_widths_object && result.request.column_widths_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "column-widths")};
    }
    if (!result.request.column_widths_object &&
        (result.request.column_widths_available ||
         !result.request.column_widths_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Column-widths",
            "--column-widths-object")};
    }
    if (result.request.column_lines_object && !result.request.column_lines_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "column-lines",
            "--column-lines")};
    }
    if (result.request.column_lines_object && result.request.column_lines_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "column-lines")};
    }
    if (!result.request.column_lines_object &&
        (result.request.column_lines_available ||
         !result.request.column_lines_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Column-lines",
            "--column-lines-object")};
    }
    if (result.request.integral_height_object && !result.request.integral_height_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "integral-height",
            "--integral-height")};
    }
    if (result.request.integral_height_object && result.request.integral_height_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "integral-height")};
    }
    if (!result.request.integral_height_object &&
        (result.request.integral_height_available ||
         !result.request.integral_height_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Integral-height",
            "--integral-height-object")};
    }
    if (result.request.incremental_search_object && !result.request.incremental_search_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "incremental-search",
            "--incremental-search")};
    }
    if (result.request.incremental_search_object && result.request.incremental_search_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "incremental-search")};
    }
    if (!result.request.incremental_search_object &&
        (result.request.incremental_search_available ||
         !result.request.incremental_search_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Incremental-search",
            "--incremental-search-object")};
    }
    if (result.request.multi_select_object && !result.request.multi_select_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "multi-select",
            "--multi-select")};
    }
    if (result.request.multi_select_object && result.request.multi_select_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "multi-select")};
    }
    if (!result.request.multi_select_object &&
        (result.request.multi_select_available ||
         !result.request.multi_select_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Multi-select",
            "--multi-select-object")};
    }
    if (result.request.deleted_states && result.request.deleted_state_objects.empty()) {
        return {.ok = false, .error = localized_request_requires_selector(
            catalog,
            "deleted-states",
            "target")};
    }
    if (result.request.deleted_states) {
        for (const auto& object : result.request.deleted_state_objects) {
            if (!object.deleted_available) {
                return {.ok = false, .error = localized_request_item_requires_option_after_target(
                    catalog,
                    "deleted-states",
                    "--deleted-state")};
            }
        }
    }
    if (!result.request.deleted_states && !result.request.deleted_state_objects.empty()) {
        return {.ok = false, .error = localized_request_arguments_require_mode(
            catalog,
            "Deleted-state target",
            "--deleted-states")};
    }
    if (result.request.subtree_deleted_state && !result.request.subtree_deleted_available) {
        return {.ok = false, .error = localized_request_requires_option(
            catalog,
            "subtree deleted-state",
            "--subtree-deleted")};
    }
    if (result.request.subtree_deleted_state &&
        result.request.object_name.empty() &&
        result.request.unique_id.empty()) {
        return {.ok = false, .error = localized_request_requires_selector(
            catalog,
            "subtree deleted-state",
            "root")};
    }
    if (!result.request.subtree_deleted_state && result.request.subtree_deleted_available) {
        return {.ok = false, .error = localized_request_arguments_require_mode(
            catalog,
            "Subtree deleted-state",
            "--subtree-deleted-state")};
    }
    if (result.request.row_source_type_object && !result.request.row_source_type_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "row-source-type",
            "--row-source-type")};
    }
    if (result.request.row_source_type_object && result.request.row_source_type < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "row-source-type",
            "value")};
    }
    if (result.request.row_source_type_object && result.request.row_source_type_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "row-source-type")};
    }
    if (!result.request.row_source_type_object &&
        (result.request.row_source_type_available ||
         !result.request.row_source_type_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Row-source-type",
            "--row-source-type-object")};
    }
    if (result.request.bound_column_object && !result.request.bound_column_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "bound-column",
            "--bound-column")};
    }
    if (result.request.bound_column_object && result.request.bound_column < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "bound-column",
            "value")};
    }
    if (result.request.bound_column_object && result.request.bound_column_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "bound-column")};
    }
    if (!result.request.bound_column_object &&
        (result.request.bound_column_available ||
         !result.request.bound_column_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Bound-column",
            "--bound-column-object")};
    }
    if (result.request.column_count_object && !result.request.column_count_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "column-count",
            "--column-count")};
    }
    if (result.request.column_count_object && result.request.column_count < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "column-count",
            "value")};
    }
    if (result.request.column_count_object && result.request.column_count_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "column-count")};
    }
    if (!result.request.column_count_object &&
        (result.request.column_count_available ||
         !result.request.column_count_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Column-count",
            "--column-count-object")};
    }
    if (result.request.style_object && !result.request.style_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "style",
            "--style")};
    }
    if (result.request.style_object && result.request.style < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "style",
            "value")};
    }
    if (result.request.style_object && result.request.style_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "style")};
    }
    if (!result.request.style_object &&
        (result.request.style_available ||
         !result.request.style_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Style",
            "--style-object")};
    }
    if (result.request.list_index_object && !result.request.list_index_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "list-index",
            "--list-index")};
    }
    if (result.request.list_index_object && result.request.list_index < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "list-index",
            "value")};
    }
    if (result.request.list_index_object && result.request.list_index_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "list-index")};
    }
    if (!result.request.list_index_object &&
        (result.request.list_index_available ||
         !result.request.list_index_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "List-index",
            "--list-index-object")};
    }
    if (result.request.left_column_object && !result.request.left_column_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "left-column",
            "--left-column")};
    }
    if (result.request.left_column_object && result.request.left_column < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "left-column",
            "value")};
    }
    if (result.request.left_column_object && result.request.left_column_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "left-column")};
    }
    if (!result.request.left_column_object &&
        (result.request.left_column_available ||
         !result.request.left_column_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Left-column",
            "--left-column-object")};
    }
    if (result.request.display_value_object && !result.request.display_value_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "display-value",
            "--display-value")};
    }
    if (result.request.display_value_object && result.request.display_value_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "display-value")};
    }
    if (!result.request.display_value_object &&
        (result.request.display_value_available ||
         !result.request.display_value_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Display-value",
            "--display-value-object")};
    }
    if (result.request.selected_back_color_object && !result.request.selected_back_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "selected-back-color",
            "--selected-back-color")};
    }
    if (result.request.selected_back_color_object && result.request.selected_back_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "selected-back-color",
            "value")};
    }
    if (result.request.selected_back_color_object && result.request.selected_back_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "selected-back-color")};
    }
    if (!result.request.selected_back_color_object &&
        (result.request.selected_back_color_available ||
         !result.request.selected_back_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Selected-back-color",
            "--selected-back-color-object")};
    }
    if (result.request.selected_fore_color_object && !result.request.selected_fore_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "selected-fore-color",
            "--selected-fore-color")};
    }
    if (result.request.selected_fore_color_object && result.request.selected_fore_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "selected-fore-color",
            "value")};
    }
    if (result.request.selected_fore_color_object && result.request.selected_fore_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "selected-fore-color")};
    }
    if (!result.request.selected_fore_color_object &&
        (result.request.selected_fore_color_available ||
         !result.request.selected_fore_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Selected-fore-color",
            "--selected-fore-color-object")};
    }
    if (result.request.selected_item_back_color_object && !result.request.selected_item_back_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "selected-item-back-color",
            "--selected-item-back-color")};
    }
    if (result.request.selected_item_back_color_object && result.request.selected_item_back_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "selected-item-back-color",
            "value")};
    }
    if (result.request.selected_item_back_color_object && result.request.selected_item_back_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "selected-item-back-color")};
    }
    if (!result.request.selected_item_back_color_object &&
        (result.request.selected_item_back_color_available ||
         !result.request.selected_item_back_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Selected-item-back-color",
            "--selected-item-back-color-object")};
    }
    if (result.request.selected_item_fore_color_object && !result.request.selected_item_fore_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "selected-item-fore-color",
            "--selected-item-fore-color")};
    }
    if (result.request.selected_item_fore_color_object && result.request.selected_item_fore_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "selected-item-fore-color",
            "value")};
    }
    if (result.request.selected_item_fore_color_object && result.request.selected_item_fore_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "selected-item-fore-color")};
    }
    if (!result.request.selected_item_fore_color_object &&
        (result.request.selected_item_fore_color_available ||
         !result.request.selected_item_fore_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Selected-item-fore-color",
            "--selected-item-fore-color-object")};
    }
    if (result.request.disabled_item_back_color_object && !result.request.disabled_item_back_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "disabled-item-back-color",
            "--disabled-item-back-color")};
    }
    if (result.request.disabled_item_back_color_object && result.request.disabled_item_back_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "disabled-item-back-color",
            "value")};
    }
    if (result.request.disabled_item_back_color_object && result.request.disabled_item_back_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "disabled-item-back-color")};
    }
    if (!result.request.disabled_item_back_color_object &&
        (result.request.disabled_item_back_color_available ||
         !result.request.disabled_item_back_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Disabled-item-back-color",
            "--disabled-item-back-color-object")};
    }
    if (result.request.disabled_item_fore_color_object && !result.request.disabled_item_fore_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "disabled-item-fore-color",
            "--disabled-item-fore-color")};
    }
    if (result.request.disabled_item_fore_color_object && result.request.disabled_item_fore_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "disabled-item-fore-color",
            "value")};
    }
    if (result.request.disabled_item_fore_color_object && result.request.disabled_item_fore_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            "disabled-item-fore-color")};
    }
    if (!result.request.disabled_item_fore_color_object &&
        (result.request.disabled_item_fore_color_available ||
         !result.request.disabled_item_fore_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Disabled-item-fore-color",
            "--disabled-item-fore-color-object")};
    }
    if (result.request.item_back_color_object && !result.request.item_back_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "item-back-color",
            "--item-back-color")};
    }
    if (result.request.item_back_color_object && result.request.item_back_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "item-back-color",
            "value")};
    }
    if (result.request.item_back_color_object && result.request.item_back_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "item-back-color")};
    }
    if (!result.request.item_back_color_object &&
        (result.request.item_back_color_available ||
         !result.request.item_back_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Item-back-color",
            "--item-back-color-object")};
    }
    if (result.request.item_fore_color_object && !result.request.item_fore_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "item-fore-color",
            "--item-fore-color")};
    }
    if (result.request.item_fore_color_object && result.request.item_fore_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "item-fore-color",
            "value")};
    }
    if (result.request.item_fore_color_object && result.request.item_fore_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "item-fore-color")};
    }
    if (!result.request.item_fore_color_object &&
        (result.request.item_fore_color_available ||
         !result.request.item_fore_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Item-fore-color",
            "--item-fore-color-object")};
    }
    if (result.request.highlight_back_color_object && !result.request.highlight_back_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "highlight-back-color",
            "--highlight-back-color")};
    }
    if (result.request.highlight_back_color_object && result.request.highlight_back_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "highlight-back-color",
            "value")};
    }
    if (result.request.highlight_back_color_object && result.request.highlight_back_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "highlight-back-color")};
    }
    if (!result.request.highlight_back_color_object &&
        (result.request.highlight_back_color_available ||
         !result.request.highlight_back_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Highlight-back-color",
            "--highlight-back-color-object")};
    }
    if (result.request.highlight_fore_color_object && !result.request.highlight_fore_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "highlight-fore-color",
            "--highlight-fore-color")};
    }
    if (result.request.highlight_fore_color_object && result.request.highlight_fore_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "highlight-fore-color",
            "value")};
    }
    if (result.request.highlight_fore_color_object && result.request.highlight_fore_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "highlight-fore-color")};
    }
    if (!result.request.highlight_fore_color_object &&
        (result.request.highlight_fore_color_available ||
         !result.request.highlight_fore_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Highlight-fore-color",
            "--highlight-fore-color-object")};
    }
    if (result.request.back_color_object && !result.request.back_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "back-color",
            "--back-color")};
    }
    if (result.request.back_color_object && result.request.back_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "back-color",
            "value")};
    }
    if (result.request.back_color_object && result.request.back_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "back-color")};
    }
    if (!result.request.back_color_object &&
        (result.request.back_color_available ||
         !result.request.back_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Back-color",
            "--back-color-object")};
    }
    if (result.request.fore_color_object && !result.request.fore_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "fore-color",
            "--fore-color")};
    }
    if (result.request.fore_color_object && result.request.fore_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "fore-color",
            "value")};
    }
    if (result.request.fore_color_object && result.request.fore_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "fore-color")};
    }
    if (!result.request.fore_color_object &&
        (result.request.fore_color_available ||
         !result.request.fore_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Fore-color",
            "--fore-color-object")};
    }
    if (result.request.disabled_back_color_object && !result.request.disabled_back_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "disabled-back-color",
            "--disabled-back-color")};
    }
    if (result.request.disabled_back_color_object && result.request.disabled_back_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "disabled-back-color",
            "value")};
    }
    if (result.request.disabled_back_color_object && result.request.disabled_back_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "disabled-back-color")};
    }
    if (!result.request.disabled_back_color_object &&
        (result.request.disabled_back_color_available ||
         !result.request.disabled_back_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Disabled-back-color",
            "--disabled-back-color-object")};
    }
    if (result.request.disabled_fore_color_object && !result.request.disabled_fore_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "disabled-fore-color",
            "--disabled-fore-color")};
    }
    if (result.request.disabled_fore_color_object && result.request.disabled_fore_color < 0) {
        return {.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            "disabled-fore-color",
            "value")};
    }
    if (result.request.disabled_fore_color_object && result.request.disabled_fore_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "disabled-fore-color")};
    }
    if (!result.request.disabled_fore_color_object &&
        (result.request.disabled_fore_color_available ||
         !result.request.disabled_fore_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Disabled-fore-color",
            "--disabled-fore-color-object")};
    }
    if (result.request.dynamic_back_color_object && !result.request.dynamic_back_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-back-color",
            "--dynamic-back-color")};
    }
    if (result.request.dynamic_back_color_object && result.request.dynamic_back_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-back-color")};
    }
    if (!result.request.dynamic_back_color_object &&
        (result.request.dynamic_back_color_available ||
         !result.request.dynamic_back_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-back-color",
            "--dynamic-back-color-object")};
    }
    if (result.request.dynamic_fore_color_object && !result.request.dynamic_fore_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-fore-color",
            "--dynamic-fore-color")};
    }
    if (result.request.dynamic_fore_color_object && result.request.dynamic_fore_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-fore-color")};
    }
    if (!result.request.dynamic_fore_color_object &&
        (result.request.dynamic_fore_color_available ||
         !result.request.dynamic_fore_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-fore-color",
            "--dynamic-fore-color-object")};
    }
    if (result.request.closable_object && !result.request.closable_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "closable", "--closable")};
    }
    if (result.request.closable_object && result.request.closable_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "closable")};
    }
    if (!result.request.closable_object &&
        (result.request.closable_available ||
         !result.request.closable_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Closable",
            "--closable-object")};
    }
    if (result.request.control_box_object && !result.request.control_box_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "control-box", "--control-box")};
    }
    if (result.request.control_box_object && result.request.control_box_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "control-box")};
    }
    if (!result.request.control_box_object &&
        (result.request.control_box_available ||
         !result.request.control_box_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Control-box",
            "--control-box-object")};
    }
    if (result.request.allow_output_object && !result.request.allow_output_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "allow-output", "--allow-output")};
    }
    if (result.request.allow_output_object && result.request.allow_output_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "allow-output")};
    }
    if (!result.request.allow_output_object &&
        (result.request.allow_output_available ||
         !result.request.allow_output_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Allow-output",
            "--allow-output-object")};
    }
    if (result.request.bind_controls_object && !result.request.bind_controls_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "bind-controls", "--bind-controls")};
    }
    if (result.request.bind_controls_object && result.request.bind_controls_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "bind-controls")};
    }
    if (!result.request.bind_controls_object &&
        (result.request.bind_controls_available ||
         !result.request.bind_controls_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Bind-controls",
            "--bind-controls-object")};
    }
    if (result.request.auto_verb_menu_object && !result.request.auto_verb_menu_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "auto-verb-menu",
            "--auto-verb-menu")};
    }
    if (result.request.auto_verb_menu_object && result.request.auto_verb_menu_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "auto-verb-menu")};
    }
    if (!result.request.auto_verb_menu_object &&
        (result.request.auto_verb_menu_available ||
         !result.request.auto_verb_menu_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Auto-verb-menu",
            "--auto-verb-menu-object")};
    }
    if (result.request.desktop_object && !result.request.desktop_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "desktop", "--desktop")};
    }
    if (result.request.desktop_object && result.request.desktop_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "desktop")};
    }
    if (!result.request.desktop_object &&
        (result.request.desktop_available ||
         !result.request.desktop_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Desktop",
            "--desktop-object")};
    }
    if (result.request.key_preview_object && !result.request.key_preview_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "key-preview", "--key-preview")};
    }
    if (result.request.key_preview_object && result.request.key_preview_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "key-preview")};
    }
    if (!result.request.key_preview_object &&
        (result.request.key_preview_available ||
         !result.request.key_preview_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Key-preview",
            "--key-preview-object")};
    }
    if (result.request.mac_desktop_object && !result.request.mac_desktop_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "mac-desktop", "--mac-desktop")};
    }
    if (result.request.mac_desktop_object && result.request.mac_desktop_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "mac-desktop")};
    }
    if (!result.request.mac_desktop_object &&
        (result.request.mac_desktop_available ||
         !result.request.mac_desktop_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Mac-desktop",
            "--mac-desktop-object")};
    }
    if (result.request.max_button_object && !result.request.max_button_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "max-button", "--max-button")};
    }
    if (result.request.max_button_object && result.request.max_button_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "max-button")};
    }
    if (!result.request.max_button_object &&
        (result.request.max_button_available ||
         !result.request.max_button_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Max-button",
            "--max-button-object")};
    }
    if (result.request.min_button_object && !result.request.min_button_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "min-button", "--min-button")};
    }
    if (result.request.min_button_object && result.request.min_button_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "min-button")};
    }
    if (!result.request.min_button_object &&
        (result.request.min_button_available ||
         !result.request.min_button_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Min-button",
            "--min-button-object")};
    }
    if (result.request.min_height_object && !result.request.min_height_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "min-height", "--min-height")};
    }
    if (result.request.min_height_object && result.request.min_height_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "min-height")};
    }
    if (!result.request.min_height_object &&
        (result.request.min_height_available ||
         !result.request.min_height_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Min-height",
            "--min-height-object")};
    }
    if (result.request.min_width_object && !result.request.min_width_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "min-width", "--min-width")};
    }
    if (result.request.min_width_object && result.request.min_width_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "min-width")};
    }
    if (!result.request.min_width_object &&
        (result.request.min_width_available ||
         !result.request.min_width_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Min-width",
            "--min-width-object")};
    }
    if (result.request.max_height_object && !result.request.max_height_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "max-height", "--max-height")};
    }
    if (result.request.max_height_object && result.request.max_height_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "max-height")};
    }
    if (!result.request.max_height_object &&
        (result.request.max_height_available ||
         !result.request.max_height_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Max-height",
            "--max-height-object")};
    }
    if (result.request.movable_object && !result.request.movable_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "movable", "--movable")};
    }
    if (result.request.movable_object && result.request.movable_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "movable")};
    }
    if (!result.request.movable_object &&
        (result.request.movable_available ||
         !result.request.movable_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Movable",
            "--movable-object")};
    }
    if (result.request.half_height_caption_object && !result.request.half_height_caption_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "half-height-caption",
            "--half-height-caption")};
    }
    if (result.request.half_height_caption_object && result.request.half_height_caption_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "half-height-caption")};
    }
    if (!result.request.half_height_caption_object &&
        (result.request.half_height_caption_available ||
         !result.request.half_height_caption_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Half-height-caption",
            "--half-height-caption-object")};
    }
    if (result.request.mdi_form_object && !result.request.mdi_form_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "MDI-form", "--mdi-form")};
    }
    if (result.request.mdi_form_object && result.request.mdi_form_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "MDI-form")};
    }
    if (!result.request.mdi_form_object &&
        (result.request.mdi_form_available ||
         !result.request.mdi_form_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "MDI-form",
            "--mdi-form-object")};
    }
    if (result.request.back_style_object && !result.request.back_style_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "back-style", "--back-style")};
    }
    if (result.request.back_style_object && result.request.back_style_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "back-style")};
    }
    if (!result.request.back_style_object &&
        (result.request.back_style_available ||
         !result.request.back_style_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Back-style",
            "--back-style-object")};
    }
    if (result.request.border_style_object && !result.request.border_style_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "border-style", "--border-style")};
    }
    if (result.request.border_style_object && result.request.border_style_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "border-style")};
    }
    if (!result.request.border_style_object &&
        (result.request.border_style_available ||
         !result.request.border_style_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Border-style",
            "--border-style-object")};
    }
    if (result.request.border_width_object && !result.request.border_width_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "border-width", "--border-width")};
    }
    if (result.request.border_width_object && result.request.border_width_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "border-width")};
    }
    if (!result.request.border_width_object &&
        (result.request.border_width_available ||
         !result.request.border_width_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Border-width",
            "--border-width-object")};
    }
    if (result.request.border_color_object && !result.request.border_color_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "border-color", "--border-color")};
    }
    if (result.request.border_color_object && result.request.border_color_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "border-color")};
    }
    if (!result.request.border_color_object &&
        (result.request.border_color_available ||
         !result.request.border_color_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Border-color",
            "--border-color-object")};
    }
    if (result.request.special_effect_object && !result.request.special_effect_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "special-effect", "--special-effect")};
    }
    if (result.request.special_effect_object && result.request.special_effect_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "special-effect")};
    }
    if (!result.request.special_effect_object &&
        (result.request.special_effect_available ||
         !result.request.special_effect_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Special-effect",
            "--special-effect-object")};
    }
    if (result.request.scroll_bars_object && !result.request.scroll_bars_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "scroll-bars", "--scroll-bars")};
    }
    if (result.request.scroll_bars_object && result.request.scroll_bars_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "scroll-bars")};
    }
    if (!result.request.scroll_bars_object &&
        (result.request.scroll_bars_available ||
         !result.request.scroll_bars_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Scroll-bars",
            "--scroll-bars-object")};
    }
    if (result.request.window_state_object && !result.request.window_state_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "window-state", "--window-state")};
    }
    if (result.request.window_state_object && result.request.window_state_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "window-state")};
    }
    if (!result.request.window_state_object &&
        (result.request.window_state_available ||
         !result.request.window_state_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Window-state",
            "--window-state-object")};
    }
    if (result.request.show_window_object && !result.request.show_window_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "show-window", "--show-window")};
    }
    if (result.request.show_window_object && result.request.show_window_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "show-window")};
    }
    if (!result.request.show_window_object &&
        (result.request.show_window_available ||
         !result.request.show_window_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Show-window",
            "--show-window-object")};
    }
    if (result.request.title_bar_object && !result.request.title_bar_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "title-bar", "--title-bar")};
    }
    if (result.request.title_bar_object && result.request.title_bar_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "title-bar")};
    }
    if (!result.request.title_bar_object &&
        (result.request.title_bar_available ||
         !result.request.title_bar_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Title-bar",
            "--title-bar-object")};
    }
    if (result.request.mouse_pointer_object && !result.request.mouse_pointer_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "mouse-pointer", "--mouse-pointer")};
    }
    if (result.request.mouse_pointer_object && result.request.mouse_pointer_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "mouse-pointer")};
    }
    if (!result.request.mouse_pointer_object &&
        (result.request.mouse_pointer_available ||
         !result.request.mouse_pointer_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Mouse-pointer",
            "--mouse-pointer-object")};
    }
    if (result.request.picture_margin_object && !result.request.picture_margin_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "picture-margin",
            "--picture-margin")};
    }
    if (result.request.picture_margin_object && result.request.picture_margin_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "picture-margin")};
    }
    if (!result.request.picture_margin_object &&
        (result.request.picture_margin_available ||
         !result.request.picture_margin_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Picture-margin",
            "--picture-margin-object")};
    }
    if (result.request.picture_position_object && !result.request.picture_position_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "picture-position",
            "--picture-position")};
    }
    if (result.request.picture_position_object && result.request.picture_position_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "picture-position")};
    }
    if (!result.request.picture_position_object &&
        (result.request.picture_position_available ||
         !result.request.picture_position_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Picture-position",
            "--picture-position-object")};
    }
    if (result.request.picture_spacing_object && !result.request.picture_spacing_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "picture-spacing",
            "--picture-spacing")};
    }
    if (result.request.picture_spacing_object && result.request.picture_spacing_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "picture-spacing")};
    }
    if (!result.request.picture_spacing_object &&
        (result.request.picture_spacing_available ||
         !result.request.picture_spacing_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Picture-spacing",
            "--picture-spacing-object")};
    }
    if (result.request.picture_selection_display_object && !result.request.picture_selection_display_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "picture-selection-display",
            "--picture-selection-display")};
    }
    if (result.request.picture_selection_display_object && result.request.picture_selection_display_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "picture-selection-display")};
    }
    if (!result.request.picture_selection_display_object &&
        (result.request.picture_selection_display_available ||
         !result.request.picture_selection_display_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Picture-selection-display",
            "--picture-selection-display-object")};
    }
    if (result.request.dynamic_input_mask_object && !result.request.dynamic_input_mask_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-input-mask",
            "--dynamic-input-mask")};
    }
    if (result.request.dynamic_input_mask_object && result.request.dynamic_input_mask_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-input-mask")};
    }
    if (!result.request.dynamic_input_mask_object &&
        (result.request.dynamic_input_mask_available ||
         !result.request.dynamic_input_mask_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-input-mask",
            "--dynamic-input-mask-object")};
    }
    if (result.request.dynamic_line_height_object && !result.request.dynamic_line_height_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-line-height",
            "--dynamic-line-height")};
    }
    if (result.request.dynamic_line_height_object && result.request.dynamic_line_height_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-line-height")};
    }
    if (!result.request.dynamic_line_height_object &&
        (result.request.dynamic_line_height_available ||
         !result.request.dynamic_line_height_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-line-height",
            "--dynamic-line-height-object")};
    }
    if (result.request.dynamic_alignment_object && !result.request.dynamic_alignment_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-alignment",
            "--dynamic-alignment")};
    }
    if (result.request.dynamic_alignment_object && result.request.dynamic_alignment_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-alignment")};
    }
    if (!result.request.dynamic_alignment_object &&
        (result.request.dynamic_alignment_available ||
         !result.request.dynamic_alignment_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-alignment",
            "--dynamic-alignment-object")};
    }
    if (result.request.dynamic_current_control_object && !result.request.dynamic_current_control_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-current-control",
            "--dynamic-current-control")};
    }
    if (result.request.dynamic_current_control_object && result.request.dynamic_current_control_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-current-control")};
    }
    if (!result.request.dynamic_current_control_object &&
        (result.request.dynamic_current_control_available ||
         !result.request.dynamic_current_control_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-current-control",
            "--dynamic-current-control-object")};
    }
    if (result.request.dynamic_font_name_object && !result.request.dynamic_font_name_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-font-name",
            "--dynamic-font-name")};
    }
    if (result.request.dynamic_font_name_object && result.request.dynamic_font_name_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-font-name")};
    }
    if (!result.request.dynamic_font_name_object &&
        (result.request.dynamic_font_name_available ||
         !result.request.dynamic_font_name_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-font-name",
            "--dynamic-font-name-object")};
    }
    if (result.request.dynamic_font_size_object && !result.request.dynamic_font_size_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-font-size",
            "--dynamic-font-size")};
    }
    if (result.request.dynamic_font_size_object && result.request.dynamic_font_size_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-font-size")};
    }
    if (!result.request.dynamic_font_size_object &&
        (result.request.dynamic_font_size_available ||
         !result.request.dynamic_font_size_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-font-size",
            "--dynamic-font-size-object")};
    }
    if (result.request.dynamic_font_bold_object && !result.request.dynamic_font_bold_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-font-bold",
            "--dynamic-font-bold")};
    }
    if (result.request.dynamic_font_bold_object && result.request.dynamic_font_bold_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-font-bold")};
    }
    if (!result.request.dynamic_font_bold_object &&
        (result.request.dynamic_font_bold_available ||
         !result.request.dynamic_font_bold_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-font-bold",
            "--dynamic-font-bold-object")};
    }
    if (result.request.dynamic_font_italic_object && !result.request.dynamic_font_italic_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-font-italic",
            "--dynamic-font-italic")};
    }
    if (result.request.dynamic_font_italic_object && result.request.dynamic_font_italic_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-font-italic")};
    }
    if (!result.request.dynamic_font_italic_object &&
        (result.request.dynamic_font_italic_available ||
         !result.request.dynamic_font_italic_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-font-italic",
            "--dynamic-font-italic-object")};
    }
    if (result.request.dynamic_font_underline_object && !result.request.dynamic_font_underline_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-font-underline",
            "--dynamic-font-underline")};
    }
    if (result.request.dynamic_font_underline_object && result.request.dynamic_font_underline_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-font-underline")};
    }
    if (!result.request.dynamic_font_underline_object &&
        (result.request.dynamic_font_underline_available ||
         !result.request.dynamic_font_underline_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-font-underline",
            "--dynamic-font-underline-object")};
    }
    if (result.request.dynamic_font_strikethru_object && !result.request.dynamic_font_strikethru_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-font-strikethru",
            "--dynamic-font-strikethru")};
    }
    if (result.request.dynamic_font_strikethru_object && result.request.dynamic_font_strikethru_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-font-strikethru")};
    }
    if (!result.request.dynamic_font_strikethru_object &&
        (result.request.dynamic_font_strikethru_available ||
         !result.request.dynamic_font_strikethru_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-font-strikethru",
            "--dynamic-font-strikethru-object")};
    }
    if (result.request.dynamic_font_outline_object && !result.request.dynamic_font_outline_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-font-outline",
            "--dynamic-font-outline")};
    }
    if (result.request.dynamic_font_outline_object && result.request.dynamic_font_outline_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-font-outline")};
    }
    if (!result.request.dynamic_font_outline_object &&
        (result.request.dynamic_font_outline_available ||
         !result.request.dynamic_font_outline_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-font-outline",
            "--dynamic-font-outline-object")};
    }
    if (result.request.dynamic_font_shadow_object && !result.request.dynamic_font_shadow_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "dynamic-font-shadow",
            "--dynamic-font-shadow")};
    }
    if (result.request.dynamic_font_shadow_object && result.request.dynamic_font_shadow_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dynamic-font-shadow")};
    }
    if (!result.request.dynamic_font_shadow_object &&
        (result.request.dynamic_font_shadow_available ||
         !result.request.dynamic_font_shadow_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dynamic-font-shadow",
            "--dynamic-font-shadow-object")};
    }
    if (result.request.font_name_object && !result.request.font_name_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "font-name", "--font-name")};
    }
    if (result.request.font_name_object && result.request.font_name_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "font-name")};
    }
    if (!result.request.font_name_object &&
        (result.request.font_name_available ||
         !result.request.font_name_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Font-name",
            "--font-name-object")};
    }
    if (result.request.font_size_object && !result.request.font_size_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "font-size", "--font-size")};
    }
    if (result.request.font_size_object && result.request.font_size_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "font-size")};
    }
    if (!result.request.font_size_object &&
        (result.request.font_size_available ||
         !result.request.font_size_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Font-size",
            "--font-size-object")};
    }
    if (result.request.font_bold_object && !result.request.font_bold_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "font-bold", "--font-bold")};
    }
    if (result.request.font_bold_object && result.request.font_bold_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "font-bold")};
    }
    if (!result.request.font_bold_object &&
        (result.request.font_bold_available ||
         !result.request.font_bold_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Font-bold",
            "--font-bold-object")};
    }
    if (result.request.font_italic_object && !result.request.font_italic_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "font-italic", "--font-italic")};
    }
    if (result.request.font_italic_object && result.request.font_italic_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "font-italic")};
    }
    if (!result.request.font_italic_object &&
        (result.request.font_italic_available ||
         !result.request.font_italic_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Font-italic",
            "--font-italic-object")};
    }
    if (result.request.font_underline_object && !result.request.font_underline_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "font-underline", "--font-underline")};
    }
    if (result.request.font_underline_object && result.request.font_underline_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "font-underline")};
    }
    if (!result.request.font_underline_object &&
        (result.request.font_underline_available ||
         !result.request.font_underline_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Font-underline",
            "--font-underline-object")};
    }
    if (result.request.font_strikethru_object && !result.request.font_strikethru_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "font-strikethru", "--font-strikethru")};
    }
    if (result.request.font_strikethru_object && result.request.font_strikethru_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "font-strikethru")};
    }
    if (!result.request.font_strikethru_object &&
        (result.request.font_strikethru_available ||
         !result.request.font_strikethru_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Font-strikethru",
            "--font-strikethru-object")};
    }
    if (result.request.font_outline_object && !result.request.font_outline_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "font-outline", "--font-outline")};
    }
    if (result.request.font_outline_object && result.request.font_outline_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "font-outline")};
    }
    if (!result.request.font_outline_object &&
        (result.request.font_outline_available ||
         !result.request.font_outline_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Font-outline",
            "--font-outline-object")};
    }
    if (result.request.font_shadow_object && !result.request.font_shadow_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "font-shadow", "--font-shadow")};
    }
    if (result.request.font_shadow_object && result.request.font_shadow_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "font-shadow")};
    }
    if (!result.request.font_shadow_object &&
        (result.request.font_shadow_available ||
         !result.request.font_shadow_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Font-shadow",
            "--font-shadow-object")};
    }
    if (result.request.max_width_object && !result.request.max_width_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "max-width", "--max-width")};
    }
    if (result.request.max_width_object && result.request.max_width_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "max-width")};
    }
    if (!result.request.max_width_object &&
        (result.request.max_width_available ||
         !result.request.max_width_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Max-width",
            "--max-width-object")};
    }
    if (result.request.max_left_object && !result.request.max_left_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "max-left", "--max-left")};
    }
    if (result.request.max_left_object && result.request.max_left_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "max-left")};
    }
    if (!result.request.max_left_object &&
        (result.request.max_left_available ||
         !result.request.max_left_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Max-left",
            "--max-left-object")};
    }
    if (result.request.max_top_object && !result.request.max_top_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "max-top", "--max-top")};
    }
    if (result.request.max_top_object && result.request.max_top_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "max-top")};
    }
    if (!result.request.max_top_object &&
        (result.request.max_top_available ||
         !result.request.max_top_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Max-top",
            "--max-top-object")};
    }
    if (result.request.auto_center_object && !result.request.auto_center_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "auto-center", "--auto-center")};
    }
    if (result.request.auto_center_object && result.request.auto_center_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "auto-center")};
    }
    if (!result.request.auto_center_object &&
        (result.request.auto_center_available ||
         !result.request.auto_center_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Auto-center",
            "--auto-center-object")};
    }
    if (result.request.auto_size_object && !result.request.auto_size_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "auto-size", "--auto-size")};
    }
    if (result.request.auto_size_object && result.request.auto_size_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "auto-size")};
    }
    if (!result.request.auto_size_object &&
        (result.request.auto_size_available ||
         !result.request.auto_size_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Auto-size",
            "--auto-size-object")};
    }
    if (result.request.auto_release_object && !result.request.auto_release_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "auto-release", "--auto-release")};
    }
    if (result.request.auto_release_object && result.request.auto_release_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "auto-release")};
    }
    if (!result.request.auto_release_object &&
        (result.request.auto_release_available ||
         !result.request.auto_release_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Auto-release",
            "--auto-release-object")};
    }
    if (result.request.continuous_scroll_object && !result.request.continuous_scroll_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "continuous-scroll",
            "--continuous-scroll")};
    }
    if (result.request.continuous_scroll_object && result.request.continuous_scroll_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "continuous-scroll")};
    }
    if (!result.request.continuous_scroll_object &&
        (result.request.continuous_scroll_available ||
         !result.request.continuous_scroll_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Continuous-scroll",
            "--continuous-scroll-object")};
    }
    if (result.request.dockable_object && !result.request.dockable_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "dockable", "--dockable")};
    }
    if (result.request.dockable_object && result.request.dockable_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "dockable")};
    }
    if (!result.request.dockable_object &&
        (result.request.dockable_available ||
         !result.request.dockable_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Dockable",
            "--dockable-object")};
    }
    if (result.request.clip_controls_object && !result.request.clip_controls_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "clip-controls", "--clip-controls")};
    }
    if (result.request.clip_controls_object && result.request.clip_controls_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "clip-controls")};
    }
    if (!result.request.clip_controls_object &&
        (result.request.clip_controls_available ||
         !result.request.clip_controls_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Clip-controls",
            "--clip-controls-object")};
    }
    if (result.request.sparse_object && !result.request.sparse_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "sparse", "--sparse")};
    }
    if (result.request.sparse_object && result.request.sparse_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "sparse")};
    }
    if (!result.request.sparse_object &&
        (result.request.sparse_available ||
         !result.request.sparse_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Sparse",
            "--sparse-object")};
    }
    if (result.request.lock_screen_object && !result.request.lock_screen_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "lock-screen", "--lock-screen")};
    }
    if (result.request.lock_screen_object && result.request.lock_screen_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "lock-screen")};
    }
    if (!result.request.lock_screen_object &&
        (result.request.lock_screen_available ||
         !result.request.lock_screen_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Lock-screen",
            "--lock-screen-object")};
    }
    if (result.request.allow_cell_selection_object && !result.request.allow_cell_selection_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "allow-cell-selection",
            "--allow-cell-selection")};
    }
    if (result.request.allow_cell_selection_object && result.request.allow_cell_selection_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "allow-cell-selection")};
    }
    if (!result.request.allow_cell_selection_object &&
        (result.request.allow_cell_selection_available ||
         !result.request.allow_cell_selection_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Allow-cell-selection",
            "--allow-cell-selection-object")};
    }
    if (result.request.hide_selection_object && !result.request.hide_selection_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "hide-selection", "--hide-selection")};
    }
    if (result.request.hide_selection_object && result.request.hide_selection_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "hide-selection")};
    }
    if (!result.request.hide_selection_object &&
        (result.request.hide_selection_available ||
         !result.request.hide_selection_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Hide-selection",
            "--hide-selection-object")};
    }
    if (result.request.delete_mark_object && !result.request.delete_mark_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "delete-mark", "--delete-mark")};
    }
    if (result.request.delete_mark_object && result.request.delete_mark_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "delete-mark")};
    }
    if (!result.request.delete_mark_object &&
        (result.request.delete_mark_available ||
         !result.request.delete_mark_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Delete-mark",
            "--delete-mark-object")};
    }
    if (result.request.record_mark_object && !result.request.record_mark_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "record-mark", "--record-mark")};
    }
    if (result.request.record_mark_object && result.request.record_mark_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "record-mark")};
    }
    if (!result.request.record_mark_object &&
        (result.request.record_mark_available ||
         !result.request.record_mark_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Record-mark",
            "--record-mark-object")};
    }
    if (result.request.split_bar_object && !result.request.split_bar_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "split-bar", "--split-bar")};
    }
    if (result.request.split_bar_object && result.request.split_bar_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "split-bar")};
    }
    if (!result.request.split_bar_object &&
        (result.request.split_bar_available ||
         !result.request.split_bar_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Split-bar",
            "--split-bar-object")};
    }
    if (result.request.highlight_row_object && !result.request.highlight_row_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "highlight-row", "--highlight-row")};
    }
    if (result.request.highlight_row_object && result.request.highlight_row_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "highlight-row")};
    }
    if (!result.request.highlight_row_object &&
        (result.request.highlight_row_available ||
         !result.request.highlight_row_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Highlight-row",
            "--highlight-row-object")};
    }
    if (result.request.panel_link_object && !result.request.panel_link_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "panel-link", "--panel-link")};
    }
    if (result.request.panel_link_object && result.request.panel_link_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "panel-link")};
    }
    if (!result.request.panel_link_object &&
        (result.request.panel_link_available ||
         !result.request.panel_link_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Panel-link",
            "--panel-link-object")};
    }
    if (result.request.allow_header_sizing_object && !result.request.allow_header_sizing_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "allow-header-sizing",
            "--allow-header-sizing")};
    }
    if (result.request.allow_header_sizing_object && result.request.allow_header_sizing_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "allow-header-sizing")};
    }
    if (!result.request.allow_header_sizing_object &&
        (result.request.allow_header_sizing_available ||
         !result.request.allow_header_sizing_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Allow-header-sizing",
            "--allow-header-sizing-object")};
    }
    if (result.request.allow_row_sizing_object && !result.request.allow_row_sizing_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "allow-row-sizing",
            "--allow-row-sizing")};
    }
    if (result.request.allow_row_sizing_object && result.request.allow_row_sizing_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "allow-row-sizing")};
    }
    if (!result.request.allow_row_sizing_object &&
        (result.request.allow_row_sizing_available ||
         !result.request.allow_row_sizing_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Allow-row-sizing",
            "--allow-row-sizing-object")};
    }
    if (result.request.resizable_object && !result.request.resizable_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "resizable", "--resizable")};
    }
    if (result.request.resizable_object && result.request.resizable_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "resizable")};
    }
    if (!result.request.resizable_object &&
        (result.request.resizable_available ||
         !result.request.resizable_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Resizable",
            "--resizable-object")};
    }
    if (result.request.add_line_feeds_object && !result.request.add_line_feeds_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "add-line-feeds",
            "--add-line-feeds")};
    }
    if (result.request.add_line_feeds_object && result.request.add_line_feeds_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "add-line-feeds")};
    }
    if (!result.request.add_line_feeds_object &&
        (result.request.add_line_feeds_available ||
         !result.request.add_line_feeds_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Add-line-feeds",
            "--add-line-feeds-object")};
    }
    if (result.request.always_on_top_object && !result.request.always_on_top_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(catalog, "always-on-top", "--always-on-top")};
    }
    if (result.request.always_on_top_object && result.request.always_on_top_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "always-on-top")};
    }
    if (!result.request.always_on_top_object &&
        (result.request.always_on_top_available ||
         !result.request.always_on_top_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Always-on-top",
            "--always-on-top-object")};
    }
    if (result.request.always_on_bottom_object && !result.request.always_on_bottom_available) {
        return {.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            "always-on-bottom",
            "--always-on-bottom")};
    }
    if (result.request.always_on_bottom_object && result.request.always_on_bottom_objects.empty()) {
        return {.ok = false, .error = localized_object_assignment_requires_target(catalog, "always-on-bottom")};
    }
    if (!result.request.always_on_bottom_object &&
        (result.request.always_on_bottom_available ||
         !result.request.always_on_bottom_objects.empty())) {
        return {.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            "Always-on-bottom",
            "--always-on-bottom-object")};
    }
    if (!result.request.align_object && !result.request.resize_object &&
        (!result.request.anchor_object_name.empty() || !result.request.anchor_unique_id.empty())) {
        return {.ok = false, .error = catalog.translate(
            "StudioHost.LaunchParse.Error.AnchorSelectorsRequireAlignOrResize",
            {
                {"alignOption", "--align-object"},
                {"resizeOption", "--resize-object"}
            })};
    }
    const int property_command_count =
        (result.request.apply_property_update ? 1 : 0) +
        (result.request.clear_property ? 1 : 0) +
        (result.request.rename_property ? 1 : 0);
    const int object_command_count =
        (result.request.delete_object ? 1 : 0) +
        (result.request.restore_object ? 1 : 0) +
        (result.request.deleted_states ? 1 : 0) +
        (result.request.subtree_deleted_state ? 1 : 0) +
        (result.request.duplicate_object ? 1 : 0) +
        (result.request.rename_object ? 1 : 0) +
        (result.request.reparent_object ? 1 : 0) +
        (result.request.reorder_object ? 1 : 0) +
        (result.request.group_object ? 1 : 0) +
        (result.request.align_object ? 1 : 0) +
        (result.request.resize_object ? 1 : 0) +
        (result.request.distribute_object ? 1 : 0) +
        (result.request.snap_object ? 1 : 0) +
        (result.request.nudge_object ? 1 : 0) +
        (result.request.tab_order_object ? 1 : 0) +
        (result.request.tab_stop_object ? 1 : 0) +
        (result.request.visibility_object ? 1 : 0) +
        (result.request.enabled_object ? 1 : 0) +
        (result.request.read_only_object ? 1 : 0) +
        (result.request.locked_object ? 1 : 0) +
        (result.request.caption_object ? 1 : 0) +
        (result.request.picture_object ? 1 : 0) +
        (result.request.down_picture_object ? 1 : 0) +
        (result.request.disabled_picture_object ? 1 : 0) +
        (result.request.ole_drag_picture_object ? 1 : 0) +
        (result.request.mouse_icon_object ? 1 : 0) +
        (result.request.drag_icon_object ? 1 : 0) +
        (result.request.drag_mode_object ? 1 : 0) +
        (result.request.ole_drag_mode_object ? 1 : 0) +
        (result.request.ole_drop_mode_object ? 1 : 0) +
        (result.request.ole_drop_effects_object ? 1 : 0) +
        (result.request.ole_drop_text_insertion_object ? 1 : 0) +
        (result.request.tooltip_text_object ? 1 : 0) +
        (result.request.status_bar_text_object ? 1 : 0) +
        (result.request.link_master_object ? 1 : 0) +
        (result.request.control_source_object ? 1 : 0) +
        (result.request.current_control_object ? 1 : 0) +
        (result.request.input_mask_object ? 1 : 0) +
        (result.request.format_object ? 1 : 0) +
        (result.request.row_source_object ? 1 : 0) +
        (result.request.column_widths_object ? 1 : 0) +
        (result.request.column_lines_object ? 1 : 0) +
        (result.request.integral_height_object ? 1 : 0) +
        (result.request.incremental_search_object ? 1 : 0) +
        (result.request.multi_select_object ? 1 : 0) +
        (result.request.row_source_type_object ? 1 : 0) +
        (result.request.bound_column_object ? 1 : 0) +
        (result.request.column_count_object ? 1 : 0) +
        (result.request.style_object ? 1 : 0) +
        (result.request.list_index_object ? 1 : 0) +
        (result.request.left_column_object ? 1 : 0) +
        (result.request.display_value_object ? 1 : 0) +
        (result.request.selected_back_color_object ? 1 : 0) +
        (result.request.selected_fore_color_object ? 1 : 0) +
        (result.request.selected_item_back_color_object ? 1 : 0) +
        (result.request.selected_item_fore_color_object ? 1 : 0) +
        (result.request.disabled_item_back_color_object ? 1 : 0) +
        (result.request.disabled_item_fore_color_object ? 1 : 0) +
        (result.request.item_back_color_object ? 1 : 0) +
        (result.request.item_fore_color_object ? 1 : 0) +
        (result.request.highlight_back_color_object ? 1 : 0) +
        (result.request.highlight_fore_color_object ? 1 : 0) +
        (result.request.back_color_object ? 1 : 0) +
        (result.request.fore_color_object ? 1 : 0) +
        (result.request.disabled_back_color_object ? 1 : 0) +
        (result.request.disabled_fore_color_object ? 1 : 0) +
        (result.request.dynamic_back_color_object ? 1 : 0) +
        (result.request.dynamic_fore_color_object ? 1 : 0) +
        (result.request.closable_object ? 1 : 0) +
        (result.request.control_box_object ? 1 : 0) +
        (result.request.button_count_object ? 1 : 0) +
        (result.request.curvature_object ? 1 : 0) +
        (result.request.draw_mode_object ? 1 : 0) +
        (result.request.draw_style_object ? 1 : 0) +
        (result.request.draw_width_object ? 1 : 0) +
        (result.request.fill_style_object ? 1 : 0) +
        (result.request.scale_mode_object ? 1 : 0) +
        (result.request.buffer_mode_object ? 1 : 0) +
        (result.request.buffer_mode_override_object ? 1 : 0) +
        (result.request.data_session_object ? 1 : 0) +
        (result.request.grid_line_color_object ? 1 : 0) +
        (result.request.header_height_object ? 1 : 0) +
        (result.request.row_height_object ? 1 : 0) +
        (result.request.lock_columns_object ? 1 : 0) +
        (result.request.lock_columns_left_object ? 1 : 0) +
        (result.request.grid_line_width_object ? 1 : 0) +
        (result.request.grid_lines_object ? 1 : 0) +
        (result.request.highlight_row_line_width_object ? 1 : 0) +
        (result.request.partition_object ? 1 : 0) +
        (result.request.record_source_type_object ? 1 : 0) +
        (result.request.column_order_object ? 1 : 0) +
        (result.request.highlight_style_object ? 1 : 0) +
        (result.request.child_order_object ? 1 : 0) +
        (result.request.fill_color_object ? 1 : 0) +
        (result.request.list_item_id_object ? 1 : 0) +
        (result.request.tab_orientation_object ? 1 : 0) +
        (result.request.display_orientation_object ? 1 : 0) +
        (result.request.help_context_id_object ? 1 : 0) +
        (result.request.whats_this_help_id_object ? 1 : 0) +
        (result.request.whats_this_help_object ? 1 : 0) +
        (result.request.whats_this_button_object ? 1 : 0) +
        (result.request.record_source_object ? 1 : 0) +
        (result.request.form_set_class_object ? 1 : 0) +
        (result.request.default_file_path_object ? 1 : 0) +
        (result.request.initial_selected_alias_object ? 1 : 0) +
        (result.request.allow_output_object ? 1 : 0) +
        (result.request.bind_controls_object ? 1 : 0) +
        (result.request.auto_verb_menu_object ? 1 : 0) +
        (result.request.desktop_object ? 1 : 0) +
        (result.request.key_preview_object ? 1 : 0) +
        (result.request.mac_desktop_object ? 1 : 0) +
        (result.request.max_button_object ? 1 : 0) +
        (result.request.min_button_object ? 1 : 0) +
        (result.request.min_height_object ? 1 : 0) +
        (result.request.min_width_object ? 1 : 0) +
        (result.request.max_height_object ? 1 : 0) +
        (result.request.movable_object ? 1 : 0) +
        (result.request.half_height_caption_object ? 1 : 0) +
        (result.request.mdi_form_object ? 1 : 0) +
        (result.request.back_style_object ? 1 : 0) +
        (result.request.border_style_object ? 1 : 0) +
        (result.request.border_width_object ? 1 : 0) +
        (result.request.border_color_object ? 1 : 0) +
        (result.request.special_effect_object ? 1 : 0) +
        (result.request.scroll_bars_object ? 1 : 0) +
        (result.request.window_state_object ? 1 : 0) +
        (result.request.show_window_object ? 1 : 0) +
        (result.request.title_bar_object ? 1 : 0) +
        (result.request.mouse_pointer_object ? 1 : 0) +
        (result.request.picture_margin_object ? 1 : 0) +
        (result.request.picture_position_object ? 1 : 0) +
        (result.request.picture_spacing_object ? 1 : 0) +
        (result.request.picture_selection_display_object ? 1 : 0) +
        (result.request.dynamic_input_mask_object ? 1 : 0) +
        (result.request.dynamic_line_height_object ? 1 : 0) +
        (result.request.dynamic_alignment_object ? 1 : 0) +
        (result.request.dynamic_current_control_object ? 1 : 0) +
        (result.request.dynamic_font_name_object ? 1 : 0) +
        (result.request.dynamic_font_size_object ? 1 : 0) +
        (result.request.dynamic_font_bold_object ? 1 : 0) +
        (result.request.dynamic_font_italic_object ? 1 : 0) +
        (result.request.dynamic_font_underline_object ? 1 : 0) +
        (result.request.dynamic_font_strikethru_object ? 1 : 0) +
        (result.request.dynamic_font_outline_object ? 1 : 0) +
        (result.request.dynamic_font_shadow_object ? 1 : 0) +
        (result.request.font_name_object ? 1 : 0) +
        (result.request.font_size_object ? 1 : 0) +
        (result.request.font_bold_object ? 1 : 0) +
        (result.request.font_italic_object ? 1 : 0) +
        (result.request.font_underline_object ? 1 : 0) +
        (result.request.font_strikethru_object ? 1 : 0) +
        (result.request.font_outline_object ? 1 : 0) +
        (result.request.font_shadow_object ? 1 : 0) +
        (result.request.max_width_object ? 1 : 0) +
        (result.request.max_left_object ? 1 : 0) +
        (result.request.max_top_object ? 1 : 0) +
        (result.request.auto_center_object ? 1 : 0) +
        (result.request.auto_size_object ? 1 : 0) +
        (result.request.auto_release_object ? 1 : 0) +
        (result.request.continuous_scroll_object ? 1 : 0) +
        (result.request.dockable_object ? 1 : 0) +
        (result.request.clip_controls_object ? 1 : 0) +
        (result.request.sparse_object ? 1 : 0) +
        (result.request.lock_screen_object ? 1 : 0) +
        (result.request.allow_cell_selection_object ? 1 : 0) +
        (result.request.hide_selection_object ? 1 : 0) +
        (result.request.delete_mark_object ? 1 : 0) +
        (result.request.record_mark_object ? 1 : 0) +
        (result.request.split_bar_object ? 1 : 0) +
        (result.request.highlight_row_object ? 1 : 0) +
        (result.request.panel_link_object ? 1 : 0) +
        (result.request.allow_header_sizing_object ? 1 : 0) +
        (result.request.allow_row_sizing_object ? 1 : 0) +
        (result.request.resizable_object ? 1 : 0) +
        (result.request.add_line_feeds_object ? 1 : 0) +
        (result.request.always_on_top_object ? 1 : 0) +
        (result.request.always_on_bottom_object ? 1 : 0) +
        (result.request.ungroup_object ? 1 : 0);
    if (property_command_count > 1) {
        return {.ok = false, .error = catalog.translate("StudioHost.LaunchParse.Error.SinglePropertyCommand")};
    }
    if (object_command_count > 1) {
        return {.ok = false, .error = catalog.translate("StudioHost.LaunchParse.Error.SingleObjectCommand")};
    }
    if (object_command_count > 0 && property_command_count > 0) {
        return {.ok = false, .error = catalog.translate("StudioHost.LaunchParse.Error.MixedObjectPropertyCommands")};
    }

    result.ok = true;
    return result;
}

LaunchParseResult parse_launch_arguments(const std::vector<std::string>& args) {
    return parse_launch_arguments(args, default_launch_catalog());
}

}  // namespace copperfin::studio
