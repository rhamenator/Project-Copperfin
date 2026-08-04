// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string toolbox_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.MissingValue",
        {{"option", option}});
}

std::string toolbox_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.UnknownSelectionContextToken",
        {{"token", token}});
}

std::string toolbox_parse_unknown_toolbox_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.UnknownToolboxContextToken",
        {{"token", token}});
}

std::string toolbox_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.NonNegativeInteger",
        {{"option", option}});
}

std::string toolbox_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.BooleanValueRequired",
        {
            {"option", option},
            {"trueValue", "true"},
            {"falseValue", "false"}
        });
}

std::string toolbox_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.UnknownOption",
        {
            {"commandName", command_name},
            {"argument", argument}
        });
}

std::string toolbox_parse_batch_item_requires_toolbox_item(
    const copperfin::localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.BatchItemRequiresToolboxItem",
        {{"toolboxItemOption", "--toolbox-item"}});
}

std::string toolbox_parse_selection_batch_item_requires_toolbox_item(
    const copperfin::localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.SelectionBatchItemRequiresToolboxItem",
        {{"toolboxItemOption", "--toolbox-item"}});
}

std::string toolbox_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

bool parse_toolbox_context_token(
    const std::string& token,
    copperfin::studio::StudioToolboxContext& context) {
    for (const auto candidate : {
             copperfin::studio::StudioToolboxContext::form,
             copperfin::studio::StudioToolboxContext::class_designer,
             copperfin::studio::StudioToolboxContext::container,
             copperfin::studio::StudioToolboxContext::report
         }) {
        if (token == copperfin::studio::studio_toolbox_context_name(candidate)) {
            context = candidate;
            return true;
        }
    }
    return false;
}

void print_json_toolbox_contexts(const std::vector<copperfin::studio::StudioToolboxContext>& contexts) {
    std::cout << "[";
    for (std::size_t index = 0; index < contexts.size(); ++index) {
        print_json_string(copperfin::studio::studio_toolbox_context_name(contexts[index]));
        if ((index + 1U) != contexts.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

void print_json_toolbox_item_descriptor(
    const copperfin::studio::StudioToolboxItemDescriptor& item,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"id\": ";
    print_json_string_view(item.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(item.title);
    std::cout << ",\n";
    std::cout << indent << "  \"category\": ";
    print_json_string_view(item.category);
    std::cout << ",\n";
    std::cout << indent << "  \"vfpClass\": ";
    print_json_string_view(item.vfp_class);
    std::cout << ",\n";
    std::cout << indent << "  \"baseClass\": ";
    print_json_string_view(item.base_class);
    std::cout << ",\n";
    std::cout << indent << "  \"defaultNamePrefix\": ";
    print_json_string_view(item.default_name_prefix);
    std::cout << ",\n";
    std::cout << indent << "  \"contexts\": ";
    print_json_toolbox_contexts(item.contexts);
    std::cout << ",\n";
    std::cout << indent << "  \"container\": " << (item.container ? "true" : "false") << ",\n";
    std::cout << indent << "  \"description\": ";
    print_json_string_view(item.description);
    std::cout << "\n";
    std::cout << indent << "}";
}

}  // namespace cf_studio_host_main_detail
