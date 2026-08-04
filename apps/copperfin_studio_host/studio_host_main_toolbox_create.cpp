// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string toolbox_parse_selection_batch_create_item_requires_toolbox_item(
    const copperfin::localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.SelectionBatchCreateItemRequiresToolboxItem",
        {{"toolboxItemOption", "--toolbox-item"}});
}

ToolboxCreatePlanParseResult parse_toolbox_create_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreatePlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--toolbox-create-plan") {
            result.request.toolbox_item_id = require_value(argument);
        } else if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.request.toolbox_context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-plan", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    return result;
}

ToolboxCreateFromDispatchPlanParseResult parse_toolbox_create_from_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateFromDispatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-from-dispatch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.launch_request.asset_path = require_value(argument);
        } else if (argument == "--toolbox-create-from-dispatch-plan") {
            result.create_request.toolbox_item_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.launch_request.selection_context = parsed_context;
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.launch_request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.launch_request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.launch_request.unique_id = require_value(argument);
        } else if (argument == "--create-object-name") {
            result.create_request.object_name = require_value(argument);
        } else if (argument == "--create-unique-id") {
            result.create_request.unique_id = require_value(argument);
        } else if (argument == "--create-parent-name") {
            result.create_request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.create_request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.admit_palette_invocation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-from-dispatch-plan", argument));
        }
    }

    if (result.ok && result.launch_request.asset_path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.create_request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxCreateFromDispatchParseResult parse_toolbox_create_from_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateFromDispatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-from-dispatch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.launch_request.asset_path = require_value(argument);
        } else if (argument == "--toolbox-create-from-dispatch") {
            result.create_request.toolbox_item_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.launch_request.selection_context = parsed_context;
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.launch_request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.launch_request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.launch_request.unique_id = require_value(argument);
        } else if (argument == "--create-object-name") {
            result.create_request.object_name = require_value(argument);
        } else if (argument == "--create-unique-id") {
            result.create_request.unique_id = require_value(argument);
        } else if (argument == "--create-parent-name") {
            result.create_request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.create_request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.admit_palette_invocation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-from-dispatch", argument));
        }
    }

    if (result.ok && result.launch_request.asset_path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.create_request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxCreateDispatchFromDispatchPlanParseResult parse_toolbox_create_dispatch_from_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateDispatchFromDispatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--toolbox-create-dispatch-from-dispatch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.launch_request.asset_path = require_value(argument);
        } else if (argument == "--toolbox-create-dispatch-from-dispatch-plan") {
            result.dispatch_request.create_request.toolbox_item_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.launch_request.selection_context = parsed_context;
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.launch_request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.launch_request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.launch_request.unique_id = require_value(argument);
        } else if (argument == "--create-object-name") {
            result.dispatch_request.create_request.object_name = require_value(argument);
        } else if (argument == "--create-unique-id") {
            result.dispatch_request.create_request.unique_id = require_value(argument);
        } else if (argument == "--create-parent-name") {
            result.dispatch_request.create_request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.dispatch_request.create_request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.admit_palette_invocation = admitted;
        } else if (argument == "--admit-create-operation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-create-operation"));
                continue;
            }
            result.dispatch_request.admit_create_operation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-dispatch-from-dispatch-plan", argument));
        }
    }

    if (result.ok && result.launch_request.asset_path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.dispatch_request.create_request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

SelectionToolboxCreatePlanParseResult parse_selection_toolbox_create_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreatePlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-toolbox-create-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--selection-toolbox-create-plan") {
            result.request.toolbox_item_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-plan", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

SelectionToolboxCreateDispatchPlanParseResult parse_selection_toolbox_create_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreateDispatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-toolbox-create-dispatch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.request.create_request.path = require_value(argument);
        } else if (argument == "--selection-toolbox-create-dispatch-plan") {
            result.request.create_request.toolbox_item_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.create_request.selection_context = parsed_context;
        } else if (argument == "--object-name") {
            result.request.create_request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.create_request.unique_id = require_value(argument);
        } else if (argument == "--parent-name") {
            result.request.create_request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.create_request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else if (argument == "--admit-create-operation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-create-operation"));
                continue;
            }
            result.request.admit_create_operation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-dispatch-plan", argument));
        }
    }

    if (result.ok && result.request.create_request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.create_request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxCreateDispatchPlanParseResult parse_toolbox_create_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateDispatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-dispatch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--toolbox-create-dispatch-plan") {
            result.request.toolbox_item_id = require_value(argument);
        } else if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.request.toolbox_context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else if (argument == "--admit-create-operation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-create-operation"));
                continue;
            }
            result.admit_create_operation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-dispatch-plan", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    return result;
}

ToolboxCreatePlanCatalogParseResult parse_toolbox_create_plan_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreatePlanCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-plan-catalog") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-create-plan-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-plan-catalog", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxContext"));
    }
    return result;
}

SelectionToolboxCreatePlanCatalogParseResult parse_selection_toolbox_create_plan_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreatePlanCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-toolbox-create-plan-catalog") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-toolbox-create-plan-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-plan-catalog", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxCreateDispatchCatalogParseResult parse_toolbox_create_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-dispatch-catalog") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-create-dispatch-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else if (argument == "--admit-create-operation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-create-operation"));
                continue;
            }
            result.request.admit_create_operation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-dispatch-catalog", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxContext"));
    }
    return result;
}

SelectionToolboxCreateDispatchCatalogParseResult parse_selection_toolbox_create_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreateDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--selection-toolbox-create-dispatch-catalog") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-toolbox-create-dispatch-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else if (argument == "--admit-create-operation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-create-operation"));
                continue;
            }
            result.request.admit_create_operation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-dispatch-catalog", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

SelectionToolboxCreateParseResult parse_selection_toolbox_create_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreateParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-toolbox-create") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--selection-toolbox-create") {
            result.request.toolbox_item_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxCreateParseResult parse_toolbox_create_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--toolbox-create") {
            result.request.toolbox_item_id = require_value(argument);
        } else if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.request.toolbox_context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.toolbox_item_id.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemId"));
    }
    return result;
}

void print_json_toolbox_create_result(const copperfin::vfp::VisualObjectCreateResult& result) {
    std::vector<std::string> created_object_names;
    std::vector<std::string> created_unique_ids;
    std::vector<std::string> create_errors;
    if (result.ok) {
        if (!result.object_name.empty()) {
            created_object_names.push_back(result.object_name);
        }
        if (!result.unique_id.empty()) {
            created_unique_ids.push_back(result.unique_id);
        }
    } else if (!result.error.empty()) {
        create_errors.push_back(result.error);
    }

    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreate\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(result.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(result.unique_id);
    std::cout << ",\n";
    std::cout << "    \"parentName\": ";
    print_json_string(result.parent_name);
    std::cout << ",\n";
    std::cout << "    \"createdObjectNames\": ";
    print_json_string_array(created_object_names);
    std::cout << ",\n";
    std::cout << "    \"createdUniqueIds\": ";
    print_json_string_array(created_unique_ids);
    std::cout << ",\n";
    std::cout << "    \"createErrors\": ";
    print_json_string_array(create_errors);
    std::cout << "\n";
    std::cout << "  }\n";
    std::cout << "}\n";
}

void print_json_toolbox_create_from_dispatch_result(
    const copperfin::studio::StudioToolboxObjectCreateFromDispatchResult& result) {
    std::vector<std::string> created_object_names;
    std::vector<std::string> created_unique_ids;
    std::vector<std::string> create_errors;
    if (result.create_result.ok) {
        if (!result.create_result.object_name.empty()) {
            created_object_names.push_back(result.create_result.object_name);
        }
        if (!result.create_result.unique_id.empty()) {
            created_unique_ids.push_back(result.create_result.unique_id);
        }
    } else {
        const std::string& error =
            !result.create_result.error.empty() ? result.create_result.error : result.error;
        if (!error.empty()) {
            create_errors.push_back(error);
        }
    }

    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreateFromDispatch\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"createPlanOk\": " << (result.create_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"createPlanError\": ";
    print_json_string(result.create_plan.error);
    std::cout << ",\n";
    std::cout << "    \"createPlan\": ";
    if (!result.create_plan.ok) {
        std::cout << "null,\n";
    } else {
        const auto& plan = result.create_plan.plan;
        std::cout << "{\n";
        std::cout << "      \"toolboxItemId\": ";
        print_json_string_view(plan.toolbox_item.id);
        std::cout << ",\n";
        std::cout << "      \"title\": ";
        print_json_string_view(plan.toolbox_item.title);
        std::cout << ",\n";
        std::cout << "      \"className\": ";
        print_json_string_view(plan.toolbox_item.vfp_class);
        std::cout << ",\n";
        std::cout << "      \"baseClassName\": ";
        print_json_string_view(plan.toolbox_item.base_class);
        std::cout << ",\n";
        std::cout << "      \"toolboxContextProvided\": "
                  << (plan.toolbox_context_provided ? "true" : "false") << ",\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"targetRecordIndex\": " << plan.target_record_index << ",\n";
        std::cout << "      \"objectName\": ";
        print_json_string(plan.object_name);
        std::cout << ",\n";
        std::cout << "      \"uniqueId\": ";
        print_json_string(plan.unique_id);
        std::cout << ",\n";
        std::cout << "      \"parentName\": ";
        print_json_string(plan.parent_name);
        std::cout << ",\n";
        std::cout << "      \"fieldValues\": [\n";
        for (std::size_t index = 0U; index < plan.field_values.size(); ++index) {
            const auto& field_value = plan.field_values[index];
            std::cout << "        {\"propertyName\": ";
            print_json_string(field_value.property_name);
            std::cout << ", \"propertyValue\": ";
            print_json_string(field_value.property_value);
            std::cout << "}";
            if ((index + 1U) != plan.field_values.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
        std::cout << "    },\n";
    }
    std::cout << "    \"createResult\": {\n";
    std::cout << "      \"ok\": " << (result.create_result.ok ? "true" : "false") << ",\n";
    std::cout << "      \"error\": ";
    print_json_string(result.create_result.error);
    std::cout << ",\n";
    std::cout << "      \"recordIndex\": " << result.create_result.record_index << ",\n";
    std::cout << "      \"objectName\": ";
    print_json_string(result.create_result.object_name);
    std::cout << ",\n";
    std::cout << "      \"uniqueId\": ";
    print_json_string(result.create_result.unique_id);
    std::cout << ",\n";
    std::cout << "      \"parentName\": ";
    print_json_string(result.create_result.parent_name);
    std::cout << "\n";
    std::cout << "    },\n";
    std::cout << "    \"createdObjectNames\": ";
    print_json_string_array(created_object_names);
    std::cout << ",\n";
    std::cout << "    \"createdUniqueIds\": ";
    print_json_string_array(created_unique_ids);
    std::cout << ",\n";
    std::cout << "    \"createErrors\": ";
    print_json_string_array(create_errors);
    std::cout << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  }\n";
    std::cout << "}\n";
}

void print_json_toolbox_create_plan_result(
    const copperfin::studio::StudioToolboxObjectCreatePlanResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreatePlan\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    const std::vector<std::string> plan_ready_item_ids{std::string(plan.toolbox_item.id)};
    const std::vector<std::string> plan_blocked_item_ids;
    const std::vector<std::string> plan_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"toolboxItemId\": ";
    print_json_string_view(plan.toolbox_item.id);
    std::cout << ",\n";
    std::cout << "    \"title\": ";
    print_json_string_view(plan.toolbox_item.title);
    std::cout << ",\n";
    std::cout << "    \"className\": ";
    print_json_string_view(plan.toolbox_item.vfp_class);
    std::cout << ",\n";
    std::cout << "    \"baseClassName\": ";
    print_json_string_view(plan.toolbox_item.base_class);
    std::cout << ",\n";
    std::cout << "    \"toolboxContextProvided\": "
              << (plan.toolbox_context_provided ? "true" : "false") << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"planReadyItemIds\": ";
    print_json_string_array(plan_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"planBlockedItemIds\": ";
    print_json_string_array(plan_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"planBlockedErrors\": ";
    print_json_string_array(plan_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"targetRecordIndex\": " << plan.target_record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << "    \"parentName\": ";
    print_json_string(plan.parent_name);
    std::cout << ",\n";
    std::cout << "    \"fieldValues\": [\n";
    for (std::size_t index = 0U; index < plan.field_values.size(); ++index) {
        const auto& field_value = plan.field_values[index];
        std::cout << "      {\"propertyName\": ";
        print_json_string(field_value.property_name);
        std::cout << ", \"propertyValue\": ";
        print_json_string(field_value.property_value);
        std::cout << "}";
        if ((index + 1U) != plan.field_values.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_create_dispatch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateDispatchResult& result) {
    std::vector<std::string> dispatch_ready_item_ids;
    std::vector<std::string> dispatch_blocked_item_ids;
    std::vector<std::string> dispatch_blocked_errors;
    if (result.ok) {
        dispatch_ready_item_ids.push_back(std::string(result.plan.toolbox_item.id));
    } else if (!result.error.empty()) {
        dispatch_blocked_errors.push_back(result.error);
    }

    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreateDispatchPlan\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"dispatchReadyItemIds\": ";
        print_json_string_array(dispatch_ready_item_ids);
        std::cout << ",\n";
        std::cout << "  \"dispatchBlockedItemIds\": ";
        print_json_string_array(dispatch_blocked_item_ids);
        std::cout << ",\n";
        std::cout << "  \"dispatchBlockedErrors\": ";
        print_json_string_array(dispatch_blocked_errors);
        std::cout << ",\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"toolboxItemId\": ";
    print_json_string_view(plan.toolbox_item.id);
    std::cout << ",\n";
    std::cout << "    \"title\": ";
    print_json_string_view(plan.toolbox_item.title);
    std::cout << ",\n";
    std::cout << "    \"className\": ";
    print_json_string_view(plan.toolbox_item.vfp_class);
    std::cout << ",\n";
    std::cout << "    \"baseClassName\": ";
    print_json_string_view(plan.toolbox_item.base_class);
    std::cout << ",\n";
    std::cout << "    \"toolboxContextProvided\": "
              << (plan.toolbox_context_provided ? "true" : "false") << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"targetRecordIndex\": " << plan.target_record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << "    \"parentName\": ";
    print_json_string(plan.parent_name);
    std::cout << ",\n";
    std::cout << "    \"fieldValues\": [\n";
    for (std::size_t index = 0U; index < plan.field_values.size(); ++index) {
        const auto& field_value = plan.field_values[index];
        std::cout << "      {\"propertyName\": ";
        print_json_string(field_value.property_name);
        std::cout << ", \"propertyValue\": ";
        print_json_string(field_value.property_value);
        std::cout << "}";
        if ((index + 1U) != plan.field_values.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"dispatchArguments\": [";
    for (std::size_t index = 0U; index < plan.dispatch_arguments.size(); ++index) {
        print_json_string(plan.dispatch_arguments[index]);
        if ((index + 1U) != plan.dispatch_arguments.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "],\n";
    std::cout << "    \"dispatchReadyItemIds\": ";
    print_json_string_array(dispatch_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedItemIds\": ";
    print_json_string_array(dispatch_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_create_plan_catalog_entry(
    const copperfin::studio::StudioToolboxObjectCreatePlanCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"toolboxItemId\": ";
    print_json_string_view(entry.toolbox_item.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(entry.toolbox_item.title);
    std::cout << ",\n";
    std::cout << indent << "  \"className\": ";
    print_json_string_view(entry.toolbox_item.vfp_class);
    std::cout << ",\n";
    std::cout << indent << "  \"baseClassName\": ";
    print_json_string_view(entry.toolbox_item.base_class);
    std::cout << ",\n";
    std::cout << indent << "  \"planOk\": " << (entry.create_plan.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(entry.create_plan.error);
    if (!entry.create_plan.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }

    const auto& plan = entry.create_plan.plan;
    std::cout << ",\n";
    std::cout << indent << "  \"targetRecordIndex\": " << plan.target_record_index << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"parentName\": ";
    print_json_string(plan.parent_name);
    std::cout << ",\n";
    std::cout << indent << "  \"fieldValues\": [\n";
    for (std::size_t index = 0U; index < plan.field_values.size(); ++index) {
        const auto& field_value = plan.field_values[index];
        std::cout << indent << "    {\"propertyName\": ";
        print_json_string(field_value.property_name);
        std::cout << ", \"propertyValue\": ";
        print_json_string(field_value.property_value);
        std::cout << "}";
        if ((index + 1U) != plan.field_values.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << indent << "  ],\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_toolbox_create_plan_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreatePlanCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreatePlanCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> plan_ready_item_ids;
    std::vector<std::string> plan_blocked_item_ids;
    std::vector<std::string> plan_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.create_plan.ok) {
            plan_ready_item_ids.push_back(std::string(entry.toolbox_item.id));
        } else {
            plan_blocked_item_ids.push_back(std::string(entry.toolbox_item.id));
            plan_blocked_errors.push_back(entry.create_plan.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"planCount\": " << result.plan_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"planReadyItemIds\": ";
    print_json_string_array(plan_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"planBlockedItemIds\": ";
    print_json_string_array(plan_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"planBlockedErrors\": ";
    print_json_string_array(plan_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_toolbox_create_plan_catalog_entry(result.entries[index], "      ");
        if ((index + 1U) != result.entries.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_create_dispatch_catalog_entry(
    const copperfin::studio::StudioToolboxObjectCreateDispatchCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"toolboxItemId\": ";
    print_json_string_view(entry.toolbox_item.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(entry.toolbox_item.title);
    std::cout << ",\n";
    std::cout << indent << "  \"className\": ";
    print_json_string_view(entry.toolbox_item.vfp_class);
    std::cout << ",\n";
    std::cout << indent << "  \"baseClassName\": ";
    print_json_string_view(entry.toolbox_item.base_class);
    std::cout << ",\n";
    std::cout << indent << "  \"createPlanOk\": " << (entry.create_plan.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"createPlanError\": ";
    print_json_string(entry.create_plan.error);
    std::cout << ",\n";
    if (entry.create_plan.ok) {
        const auto& create_plan = entry.create_plan.plan;
        std::cout << indent << "  \"targetRecordIndex\": " << create_plan.target_record_index << ",\n";
        std::cout << indent << "  \"objectName\": ";
        print_json_string(create_plan.object_name);
        std::cout << ",\n";
        std::cout << indent << "  \"parentName\": ";
        print_json_string(create_plan.parent_name);
        std::cout << ",\n";
        std::cout << indent << "  \"fieldValues\": [\n";
        for (std::size_t index = 0U; index < create_plan.field_values.size(); ++index) {
            const auto& field_value = create_plan.field_values[index];
            std::cout << indent << "    {\"propertyName\": ";
            print_json_string(field_value.property_name);
            std::cout << ", \"propertyValue\": ";
            print_json_string(field_value.property_value);
            std::cout << "}";
            if ((index + 1U) != create_plan.field_values.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << indent << "  ],\n";
    } else {
        std::cout << indent << "  \"targetRecordIndex\": 0,\n";
        std::cout << indent << "  \"objectName\": \"\",\n";
        std::cout << indent << "  \"parentName\": \"\",\n";
        std::cout << indent << "  \"fieldValues\": [],\n";
    }
    std::cout << indent << "  \"dispatchOk\": " << (entry.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dispatchError\": ";
    print_json_string(entry.dispatch.error);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchArguments\": [";
    if (entry.dispatch.ok) {
        for (std::size_t index = 0U; index < entry.dispatch.plan.dispatch_arguments.size(); ++index) {
            print_json_string(entry.dispatch.plan.dispatch_arguments[index]);
            if ((index + 1U) != entry.dispatch.plan.dispatch_arguments.size()) {
                std::cout << ", ";
            }
        }
    }
    std::cout << "],\n";
    std::cout << indent << "  \"dispatchAdmitted\": "
              << (entry.dispatch.ok && entry.dispatch.plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": "
              << (entry.dispatch.ok && !entry.dispatch.plan.dry_run ? "false" : "true") << ",\n";
    std::cout << indent << "  \"executed\": "
              << (entry.dispatch.ok && entry.dispatch.plan.executed ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": "
              << (entry.dispatch.ok && entry.dispatch.plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_toolbox_create_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreateDispatchCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> dispatch_ready_item_ids;
    std::vector<std::string> dispatch_blocked_item_ids;
    std::vector<std::string> dispatch_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.dispatch.ok) {
            dispatch_ready_item_ids.push_back(std::string(entry.toolbox_item.id));
        } else {
            dispatch_blocked_item_ids.push_back(std::string(entry.toolbox_item.id));
            dispatch_blocked_errors.push_back(entry.dispatch.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"dispatchCount\": " << result.dispatch_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchReadyItemIds\": ";
    print_json_string_array(dispatch_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedItemIds\": ";
    print_json_string_array(dispatch_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_toolbox_create_dispatch_catalog_entry(result.entries[index], "      ");
        if ((index + 1U) != result.entries.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_text_toolbox_create_result(const copperfin::vfp::VisualObjectCreateResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "object_name: " << result.object_name << "\n";
    std::cout << "unique_id: " << result.unique_id << "\n";
    std::cout << "parent_name: " << result.parent_name << "\n";
}

void print_text_toolbox_create_plan_result(
    const copperfin::studio::StudioToolboxObjectCreatePlanResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "toolbox_item_id: " << plan.toolbox_item.id << "\n";
    std::cout << "title: " << plan.toolbox_item.title << "\n";
    std::cout << "class_name: " << plan.toolbox_item.vfp_class << "\n";
    std::cout << "baseclass_name: " << plan.toolbox_item.base_class << "\n";
    std::cout << "toolbox_context_provided: " << (plan.toolbox_context_provided ? "true" : "false") << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(plan.toolbox_context)
              << "\n";
    std::cout << "target_record_index: " << plan.target_record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "parent_name: " << plan.parent_name << "\n";
    for (const auto& field_value : plan.field_values) {
        std::cout << "field_value: " << field_value.property_name << "=" << field_value.property_value << "\n";
    }
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_toolbox_create_from_dispatch_result(
    const copperfin::studio::StudioToolboxObjectCreateFromDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "create_plan_ok: " << (result.create_plan.ok ? "true" : "false") << "\n";
    if (!result.create_plan.error.empty()) {
        std::cout << "create_plan_error: " << result.create_plan.error << "\n";
    }
    if (result.create_plan.ok) {
        const auto& plan = result.create_plan.plan;
        std::cout << "toolbox_item_id: " << plan.toolbox_item.id << "\n";
        std::cout << "target_record_index: " << plan.target_record_index << "\n";
        std::cout << "planned_object_name: " << plan.object_name << "\n";
        std::cout << "planned_unique_id: " << plan.unique_id << "\n";
        std::cout << "planned_parent_name: " << plan.parent_name << "\n";
    }
    std::cout << "create_result_ok: " << (result.create_result.ok ? "true" : "false") << "\n";
    if (!result.create_result.error.empty()) {
        std::cout << "create_result_error: " << result.create_result.error << "\n";
    }
    std::cout << "record_index: " << result.create_result.record_index << "\n";
    std::cout << "object_name: " << result.create_result.object_name << "\n";
    std::cout << "unique_id: " << result.create_result.unique_id << "\n";
    std::cout << "parent_name: " << result.create_result.parent_name << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
}

void print_text_toolbox_create_dispatch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "toolbox_item_id: " << plan.toolbox_item.id << "\n";
    std::cout << "title: " << plan.toolbox_item.title << "\n";
    std::cout << "class_name: " << plan.toolbox_item.vfp_class << "\n";
    std::cout << "baseclass_name: " << plan.toolbox_item.base_class << "\n";
    std::cout << "toolbox_context_provided: " << (plan.toolbox_context_provided ? "true" : "false") << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(plan.toolbox_context)
              << "\n";
    std::cout << "target_record_index: " << plan.target_record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "parent_name: " << plan.parent_name << "\n";
    for (const auto& field_value : plan.field_values) {
        std::cout << "field_value: " << field_value.property_name << "=" << field_value.property_value << "\n";
    }
    for (const auto& argument : plan.dispatch_arguments) {
        std::cout << "dispatch_argument: " << argument << "\n";
    }
    std::cout << "dispatch_admitted: " << (plan.dispatch_admitted ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "executed: " << (plan.executed ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_toolbox_create_plan_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreatePlanCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "plan_count: " << result.plan_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_toolbox_item_id: " << entry.toolbox_item.id << "\n";
        std::cout << "entry_plan_ok: " << (entry.create_plan.ok ? "true" : "false") << "\n";
        if (entry.create_plan.ok) {
            std::cout << "entry_object_name: " << entry.create_plan.plan.object_name << "\n";
        } else if (!entry.create_plan.error.empty()) {
            std::cout << "entry_error: " << entry.create_plan.error << "\n";
        }
    }
}

void print_text_toolbox_create_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "dispatch_count: " << result.dispatch_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_toolbox_item_id: " << entry.toolbox_item.id << "\n";
        std::cout << "entry_create_plan_ok: " << (entry.create_plan.ok ? "true" : "false") << "\n";
        std::cout << "entry_dispatch_ok: " << (entry.dispatch.ok ? "true" : "false") << "\n";
        if (!entry.dispatch.error.empty()) {
            std::cout << "entry_dispatch_error: " << entry.dispatch.error << "\n";
        }
    }
}

std::optional<int> try_handle_toolbox_create_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_dispatch_catalog_parse = parse_toolbox_create_dispatch_catalog_arguments(catalog, args);
    if (!(toolbox_create_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateDispatchCatalogResult{
                .ok = false,
                .error = toolbox_create_dispatch_catalog_parse.error,
                .toolbox_context = {},
                .item_count = 0U,
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (toolbox_create_dispatch_catalog_parse.output_json) {
                print_json_toolbox_create_dispatch_catalog_result(result);
            } else {
                print_text_toolbox_create_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto catalog_result = copperfin::studio::plan_visual_object_create_dispatch_catalog(
            toolbox_create_dispatch_catalog_parse.request);
        if (toolbox_create_dispatch_catalog_parse.output_json) {
            print_json_toolbox_create_dispatch_catalog_result(catalog_result);
        } else {
            print_text_toolbox_create_dispatch_catalog_result(catalog_result);
        }
        return catalog_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_dispatch_catalog_parse =
        parse_selection_toolbox_create_dispatch_catalog_arguments(catalog, args);
    if (!(selection_toolbox_create_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreateDispatchCatalogResult{
                .ok = false,
                .error = selection_toolbox_create_dispatch_catalog_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .item_count = 0U,
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (selection_toolbox_create_dispatch_catalog_parse.output_json) {
                print_json_selection_toolbox_create_dispatch_catalog_result(result);
            } else {
                print_text_selection_toolbox_create_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto catalog_result =
            copperfin::studio::plan_visual_object_create_dispatch_catalog_from_toolbox_selection(
                selection_toolbox_create_dispatch_catalog_parse.request);
        if (selection_toolbox_create_dispatch_catalog_parse.output_json) {
            print_json_selection_toolbox_create_dispatch_catalog_result(catalog_result);
        } else {
            print_text_selection_toolbox_create_dispatch_catalog_result(catalog_result);
        }
        return catalog_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_plan_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_plan_catalog_parse = parse_toolbox_create_plan_catalog_arguments(catalog, args);
    if (!(toolbox_create_plan_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_plan_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreatePlanCatalogResult{
                .ok = false,
                .error = toolbox_create_plan_catalog_parse.error,
                .toolbox_context = {},
                .item_count = 0U,
                .plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (toolbox_create_plan_catalog_parse.output_json) {
                print_json_toolbox_create_plan_catalog_result(result);
            } else {
                print_text_toolbox_create_plan_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto catalog_result = copperfin::studio::plan_visual_object_catalog_from_toolbox_context(
            toolbox_create_plan_catalog_parse.request);
        if (toolbox_create_plan_catalog_parse.output_json) {
            print_json_toolbox_create_plan_catalog_result(catalog_result);
        } else {
            print_text_toolbox_create_plan_catalog_result(catalog_result);
        }
        return catalog_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_plan_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_plan_catalog_parse =
        parse_selection_toolbox_create_plan_catalog_arguments(catalog, args);
    if (!(selection_toolbox_create_plan_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_plan_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreatePlanCatalogResult{
                .ok = false,
                .error = selection_toolbox_create_plan_catalog_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .item_count = 0U,
                .plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (selection_toolbox_create_plan_catalog_parse.output_json) {
                print_json_selection_toolbox_create_plan_catalog_result(result);
            } else {
                print_text_selection_toolbox_create_plan_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto catalog_result = copperfin::studio::plan_visual_object_catalog_from_toolbox_selection(
            selection_toolbox_create_plan_catalog_parse.request);
        if (selection_toolbox_create_plan_catalog_parse.output_json) {
            print_json_selection_toolbox_create_plan_catalog_result(catalog_result);
        } else {
            print_text_selection_toolbox_create_plan_catalog_result(catalog_result);
        }
        return catalog_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_from_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_from_dispatch_parse = parse_toolbox_create_from_dispatch_arguments(catalog, args);
    if (!(toolbox_create_from_dispatch_parse.requested)) {
        return std::nullopt;
    }

        auto print_create_error = [&](const std::string& error, const int exit_code) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateFromDispatchResult{
                .ok = false,
                .error = error,
                .create_plan = {
                    .ok = false,
                    .error = error,
                    .plan = {}
                },
                .create_result = {
                    .ok = false,
                    .error = error,
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = {},
                    .parent_name = {}
                },
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_create_from_dispatch_parse.output_json) {
                print_json_toolbox_create_from_dispatch_result(result);
            } else {
                print_text_toolbox_create_from_dispatch_result(result);
                if (exit_code == 2) {
                    print_usage(catalog);
                }
            }
            return exit_code;
        };

        if (!toolbox_create_from_dispatch_parse.ok) {
            return print_create_error(toolbox_create_from_dispatch_parse.error, 2);
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_create_from_dispatch_parse.launch_request);
        if (!launch_result.ok) {
            return print_create_error(launch_result.error, 4);
        }

        const auto admission_result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation = toolbox_create_from_dispatch_parse.admit_palette_invocation
        });
        if (!admission_result.ok) {
            return print_create_error(admission_result.error, 4);
        }

        const auto dispatch_result = copperfin::studio::plan_studio_toolbox_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            return print_create_error(dispatch_result.error, 4);
        }

        auto create_request = toolbox_create_from_dispatch_parse.create_request;
        create_request.dispatch_plan = dispatch_result.plan;
        const auto create_result = copperfin::studio::create_visual_object_from_toolbox_dispatch(create_request);
        if (toolbox_create_from_dispatch_parse.output_json) {
            print_json_toolbox_create_from_dispatch_result(create_result);
        } else {
            print_text_toolbox_create_from_dispatch_result(create_result);
        }
        return create_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_from_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_from_dispatch_plan_parse =
        parse_toolbox_create_from_dispatch_plan_arguments(catalog, args);
    if (!(toolbox_create_from_dispatch_plan_parse.requested)) {
        return std::nullopt;
    }

        auto print_create_plan_error = [&](const std::string& error, const int exit_code) {
            const auto result = copperfin::studio::StudioToolboxObjectCreatePlanResult{
                .ok = false,
                .error = error,
                .plan = {}
            };
            if (toolbox_create_from_dispatch_plan_parse.output_json) {
                print_json_toolbox_create_plan_result(result);
            } else {
                print_text_toolbox_create_plan_result(result);
                if (exit_code == 2) {
                    print_usage(catalog);
                }
            }
            return exit_code;
        };

        if (!toolbox_create_from_dispatch_plan_parse.ok) {
            return print_create_plan_error(toolbox_create_from_dispatch_plan_parse.error, 2);
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_create_from_dispatch_plan_parse.launch_request);
        if (!launch_result.ok) {
            return print_create_plan_error(launch_result.error, 4);
        }

        const auto admission_result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation = toolbox_create_from_dispatch_plan_parse.admit_palette_invocation
        });
        if (!admission_result.ok) {
            return print_create_plan_error(admission_result.error, 4);
        }

        const auto dispatch_result = copperfin::studio::plan_studio_toolbox_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            return print_create_plan_error(dispatch_result.error, 4);
        }

        auto create_request = toolbox_create_from_dispatch_plan_parse.create_request;
        create_request.dispatch_plan = dispatch_result.plan;
        const auto create_plan_result = copperfin::studio::plan_visual_object_from_toolbox_dispatch(create_request);
        if (toolbox_create_from_dispatch_plan_parse.output_json) {
            print_json_toolbox_create_plan_result(create_plan_result);
        } else {
            print_text_toolbox_create_plan_result(create_plan_result);
        }
        return create_plan_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_dispatch_from_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_dispatch_from_dispatch_plan_parse =
        parse_toolbox_create_dispatch_from_dispatch_plan_arguments(catalog, args);
    if (!(toolbox_create_dispatch_from_dispatch_plan_parse.requested)) {
        return std::nullopt;
    }

        auto print_create_dispatch_error = [&](const std::string& error, const int exit_code) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateDispatchResult{
                .ok = false,
                .error = error,
                .plan = {}
            };
            if (toolbox_create_dispatch_from_dispatch_plan_parse.output_json) {
                print_json_toolbox_create_dispatch_plan_result(result);
            } else {
                print_text_toolbox_create_dispatch_plan_result(result);
                if (exit_code == 2) {
                    print_usage(catalog);
                }
            }
            return exit_code;
        };

        if (!toolbox_create_dispatch_from_dispatch_plan_parse.ok) {
            return print_create_dispatch_error(toolbox_create_dispatch_from_dispatch_plan_parse.error, 2);
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_create_dispatch_from_dispatch_plan_parse.launch_request);
        if (!launch_result.ok) {
            return print_create_dispatch_error(launch_result.error, 4);
        }

        const auto admission_result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation =
                toolbox_create_dispatch_from_dispatch_plan_parse.admit_palette_invocation
        });
        if (!admission_result.ok) {
            return print_create_dispatch_error(admission_result.error, 4);
        }

        const auto dispatch_result = copperfin::studio::plan_studio_toolbox_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            return print_create_dispatch_error(dispatch_result.error, 4);
        }

        auto create_dispatch_request = toolbox_create_dispatch_from_dispatch_plan_parse.dispatch_request;
        create_dispatch_request.create_request.dispatch_plan = dispatch_result.plan;
        const auto create_dispatch_result =
            copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_dispatch(create_dispatch_request);
        if (toolbox_create_dispatch_from_dispatch_plan_parse.output_json) {
            print_json_toolbox_create_dispatch_plan_result(create_dispatch_result);
        } else {
            print_text_toolbox_create_dispatch_plan_result(create_dispatch_result);
        }
        return create_dispatch_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_dispatch_plan_parse = parse_toolbox_create_dispatch_plan_arguments(catalog, args);
    if (!(toolbox_create_dispatch_plan_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_dispatch_plan_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateDispatchResult{
                .ok = false,
                .error = toolbox_create_dispatch_plan_parse.error,
                .plan = {}
            };
            if (toolbox_create_dispatch_plan_parse.output_json) {
                print_json_toolbox_create_dispatch_plan_result(result);
            } else {
                print_text_toolbox_create_dispatch_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto create_plan_result = copperfin::studio::plan_visual_object_from_toolbox_item(
            toolbox_create_dispatch_plan_parse.request);
        if (!create_plan_result.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateDispatchResult{
                .ok = false,
                .error = create_plan_result.error,
                .plan = {}
            };
            if (toolbox_create_dispatch_plan_parse.output_json) {
                print_json_toolbox_create_dispatch_plan_result(result);
            } else {
                print_text_toolbox_create_dispatch_plan_result(result);
            }
            return 4;
        }

        const auto dispatch_result = copperfin::studio::plan_visual_object_create_dispatch({
            .create_plan = create_plan_result.plan,
            .admit_create_operation = toolbox_create_dispatch_plan_parse.admit_create_operation
        });
        if (toolbox_create_dispatch_plan_parse.output_json) {
            print_json_toolbox_create_dispatch_plan_result(dispatch_result);
        } else {
            print_text_toolbox_create_dispatch_plan_result(dispatch_result);
        }
        return dispatch_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_dispatch_plan_parse =
        parse_selection_toolbox_create_dispatch_plan_arguments(catalog, args);
    if (!(selection_toolbox_create_dispatch_plan_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_dispatch_plan_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreateDispatchResult{
                .ok = false,
                .error = selection_toolbox_create_dispatch_plan_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .create_plan = {},
                .dispatch = {},
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false
            };
            if (selection_toolbox_create_dispatch_plan_parse.output_json) {
                print_json_selection_toolbox_create_dispatch_plan_result(result);
            } else {
                print_text_selection_toolbox_create_dispatch_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto dispatch_result =
            copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_selection(
                selection_toolbox_create_dispatch_plan_parse.request);
        if (selection_toolbox_create_dispatch_plan_parse.output_json) {
            print_json_selection_toolbox_create_dispatch_plan_result(dispatch_result);
        } else {
            print_text_selection_toolbox_create_dispatch_plan_result(dispatch_result);
        }
        return dispatch_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_plan_parse = parse_selection_toolbox_create_plan_arguments(catalog, args);
    if (!(selection_toolbox_create_plan_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_plan_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreatePlanResult{
                .ok = false,
                .error = selection_toolbox_create_plan_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .create_plan = {},
                .dry_run = true,
                .mutates_asset = false
            };
            if (selection_toolbox_create_plan_parse.output_json) {
                print_json_selection_toolbox_create_plan_result(result);
            } else {
                print_text_selection_toolbox_create_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto plan_result = copperfin::studio::plan_visual_object_from_toolbox_selection(
            selection_toolbox_create_plan_parse.request);
        if (selection_toolbox_create_plan_parse.output_json) {
            print_json_selection_toolbox_create_plan_result(plan_result);
        } else {
            print_text_selection_toolbox_create_plan_result(plan_result);
        }
        return plan_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_plan_parse = parse_toolbox_create_plan_arguments(catalog, args);
    if (!(toolbox_create_plan_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_plan_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreatePlanResult{
                .ok = false,
                .error = toolbox_create_plan_parse.error,
                .plan = {}
            };
            if (toolbox_create_plan_parse.output_json) {
                print_json_toolbox_create_plan_result(result);
            } else {
                print_text_toolbox_create_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto plan_result = copperfin::studio::plan_visual_object_from_toolbox_item(
            toolbox_create_plan_parse.request);
        if (toolbox_create_plan_parse.output_json) {
            print_json_toolbox_create_plan_result(plan_result);
        } else {
            print_text_toolbox_create_plan_result(plan_result);
        }
        return plan_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_parse = parse_selection_toolbox_create_arguments(catalog, args);
    if (!(selection_toolbox_create_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreateResult{
                .ok = false,
                .error = selection_toolbox_create_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .create_plan = {},
                .create_result = {},
                .dry_run = true,
                .mutates_asset = false
            };
            if (selection_toolbox_create_parse.output_json) {
                print_json_selection_toolbox_create_result(result);
            } else {
                print_text_selection_toolbox_create_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto create_result = copperfin::studio::create_visual_object_from_toolbox_selection(
            selection_toolbox_create_parse.request);
        if (selection_toolbox_create_parse.output_json) {
            print_json_selection_toolbox_create_result(create_result);
        } else {
            print_text_selection_toolbox_create_result(create_result);
        }
        return create_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_parse = parse_toolbox_create_arguments(catalog, args);
    if (!(toolbox_create_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectCreateResult{
                .ok = false,
                .error = toolbox_create_parse.error,
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .parent_name = {}
            };
            if (toolbox_create_parse.output_json) {
                print_json_toolbox_create_result(result);
            } else {
                print_text_toolbox_create_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item(
            toolbox_create_parse.request);
        if (toolbox_create_parse.output_json) {
            print_json_toolbox_create_result(create_result);
        } else {
            print_text_toolbox_create_result(create_result);
        }
        return create_result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
