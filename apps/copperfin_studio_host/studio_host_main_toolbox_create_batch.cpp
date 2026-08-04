// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
ToolboxCreateBatchFromDispatchPlanParseResult parse_toolbox_create_batch_from_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateBatchFromDispatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--toolbox-create-batch-from-dispatch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.create_request.items.empty()) {
            fail(toolbox_parse_batch_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.create_request.items.back();
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

        if (argument == "--json" || argument == "--toolbox-create-batch-from-dispatch-plan") {
            continue;
        }
        if (argument == "--path") {
            result.launch_request.asset_path = require_value(argument);
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
        } else if (argument == "--toolbox-item") {
            const std::string toolbox_item_id = require_value(argument);
            result.create_request.items.push_back({
                .toolbox_item_id = toolbox_item_id,
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--create-object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--create-unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--create-parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
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
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-batch-from-dispatch-plan", argument));
        }
    }

    if (result.ok && result.launch_request.asset_path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.create_request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

ToolboxCreateBatchFromDispatchParseResult parse_toolbox_create_batch_from_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateBatchFromDispatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--toolbox-create-batch-from-dispatch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.create_request.items.empty()) {
            fail(toolbox_parse_batch_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.create_request.items.back();
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

        if (argument == "--json" || argument == "--toolbox-create-batch-from-dispatch") {
            continue;
        }
        if (argument == "--path") {
            result.launch_request.asset_path = require_value(argument);
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
        } else if (argument == "--toolbox-item") {
            const std::string toolbox_item_id = require_value(argument);
            result.create_request.items.push_back({
                .toolbox_item_id = toolbox_item_id,
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--create-object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--create-unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--create-parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
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
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-batch-from-dispatch", argument));
        }
    }

    if (result.ok && result.launch_request.asset_path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.create_request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

ToolboxCreateBatchDispatchFromDispatchPlanParseResult
parse_toolbox_create_batch_dispatch_from_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateBatchDispatchFromDispatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--toolbox-create-batch-dispatch-from-dispatch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.dispatch_request.create_request.items.empty()) {
            fail(toolbox_parse_batch_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.dispatch_request.create_request.items.back();
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

        if (argument == "--json" || argument == "--toolbox-create-batch-dispatch-from-dispatch-plan") {
            continue;
        }
        if (argument == "--path") {
            result.launch_request.asset_path = require_value(argument);
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
        } else if (argument == "--toolbox-item") {
            const std::string toolbox_item_id = require_value(argument);
            result.dispatch_request.create_request.items.push_back({
                .toolbox_item_id = toolbox_item_id,
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--create-object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--create-unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--create-parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
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
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-batch-dispatch-from-dispatch-plan", argument));
        }
    }

    if (result.ok && result.launch_request.asset_path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.dispatch_request.create_request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

ToolboxCreateBatchPlanParseResult parse_toolbox_create_batch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateBatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-batch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.request.items.empty()) {
            fail(toolbox_parse_batch_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.request.items.back();
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

        if (argument == "--json" || argument == "--toolbox-create-batch-plan") {
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
            result.request.toolbox_context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--toolbox-item") {
            const std::string toolbox_item_id = require_value(argument);
            result.request.items.push_back({
                .toolbox_item_id = toolbox_item_id,
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-batch-plan", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

SelectionToolboxCreateBatchPlanParseResult parse_selection_toolbox_create_batch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreateBatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-toolbox-create-batch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.request.items.empty()) {
            fail(toolbox_parse_selection_batch_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.request.items.back();
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

        if (argument == "--json" || argument == "--selection-toolbox-create-batch-plan") {
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
        } else if (argument == "--toolbox-item") {
            result.request.items.push_back({
                .toolbox_item_id = require_value(argument),
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-batch-plan", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

SelectionToolboxCreateBatchParseResult parse_selection_toolbox_create_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreateBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-toolbox-create-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.request.items.empty()) {
            fail(toolbox_parse_selection_batch_create_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.request.items.back();
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

        if (argument == "--json" || argument == "--selection-toolbox-create-batch") {
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
        } else if (argument == "--toolbox-item") {
            result.request.items.push_back({
                .toolbox_item_id = require_value(argument),
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-batch", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

SelectionToolboxCreateBatchDispatchPlanParseResult parse_selection_toolbox_create_batch_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreateBatchDispatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--selection-toolbox-create-batch-dispatch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.request.batch_request.items.empty()) {
            fail(toolbox_parse_selection_batch_dispatch_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.request.batch_request.items.back();
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

        if (argument == "--json" || argument == "--selection-toolbox-create-batch-dispatch-plan") {
            continue;
        }
        if (argument == "--path") {
            result.request.batch_request.path = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.batch_request.selection_context = parsed_context;
        } else if (argument == "--toolbox-item") {
            result.request.batch_request.items.push_back({
                .toolbox_item_id = require_value(argument),
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
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
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-batch-dispatch-plan", argument));
        }
    }

    if (result.ok && result.request.batch_request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.request.batch_request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

ToolboxCreateBatchDispatchPlanParseResult parse_toolbox_create_batch_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateBatchDispatchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-batch-dispatch-plan") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.request.items.empty()) {
            fail(toolbox_parse_batch_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.request.items.back();
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

        if (argument == "--json" || argument == "--toolbox-create-batch-dispatch-plan") {
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
            result.request.toolbox_context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--toolbox-item") {
            const std::string toolbox_item_id = require_value(argument);
            result.request.items.push_back({
                .toolbox_item_id = toolbox_item_id,
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
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
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-batch-dispatch-plan", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

ToolboxCreateBatchParseResult parse_toolbox_create_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto require_current_item = [&]() -> copperfin::studio::StudioToolboxObjectCreateBatchItem* {
        if (result.request.items.empty()) {
            fail(toolbox_parse_batch_item_requires_toolbox_item(catalog));
            return nullptr;
        }
        return &result.request.items.back();
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

        if (argument == "--json" || argument == "--toolbox-create-batch") {
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
            result.request.toolbox_context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--toolbox-item") {
            const std::string toolbox_item_id = require_value(argument);
            result.request.items.push_back({
                .toolbox_item_id = toolbox_item_id,
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            });
        } else if (argument == "--object-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->unique_id = require_value(argument);
            }
        } else if (argument == "--parent-name") {
            auto* item = require_current_item();
            if (item != nullptr) {
                item->parent_name = require_value(argument);
            }
        } else if (argument == "--field-value") {
            auto* item = require_current_item();
            if (item == nullptr) {
                continue;
            }
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail(catalog.translate(
                    "StudioHost.ToolboxParse.Error.FieldValueSyntax",
                    {{"assignmentSyntax", "name=value"}}));
                continue;
            }
            item->field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-batch", argument));
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.items.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxItemIds"));
    }
    return result;
}

ToolboxCreateBatchPlanCatalogParseResult parse_toolbox_create_batch_plan_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateBatchPlanCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-batch-plan-catalog") != args.end();
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

        if (argument == "--json" || argument == "--toolbox-create-batch-plan-catalog") {
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
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-batch-plan-catalog", argument));
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

SelectionToolboxCreateBatchPlanCatalogParseResult parse_selection_toolbox_create_batch_plan_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreateBatchPlanCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--selection-toolbox-create-batch-plan-catalog") != args.end();
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

        if (argument == "--json" || argument == "--selection-toolbox-create-batch-plan-catalog") {
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
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-batch-plan-catalog", argument));
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

ToolboxCreateBatchDispatchCatalogParseResult parse_toolbox_create_batch_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxCreateBatchDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create-batch-dispatch-catalog") != args.end();
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

        if (argument == "--json" || argument == "--toolbox-create-batch-dispatch-catalog") {
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
            fail(toolbox_parse_unknown_option(catalog, "toolbox-create-batch-dispatch-catalog", argument));
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

SelectionToolboxCreateBatchDispatchCatalogParseResult
parse_selection_toolbox_create_batch_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxCreateBatchDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--selection-toolbox-create-batch-dispatch-catalog") != args.end();
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

        if (argument == "--json" || argument == "--selection-toolbox-create-batch-dispatch-catalog") {
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
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-create-batch-dispatch-catalog", argument));
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

void print_json_toolbox_create_batch_result(const copperfin::vfp::VisualObjectCreateBatchResult& result) {
    std::vector<std::string> created_object_names;
    std::vector<std::string> created_unique_ids;
    std::vector<std::string> create_errors;
    if (result.ok) {
        for (const auto& object : result.created_objects) {
            if (!object.object_name.empty()) {
                created_object_names.push_back(object.object_name);
            }
            if (!object.unique_id.empty()) {
                created_unique_ids.push_back(object.unique_id);
            }
        }
    } else if (!result.error.empty()) {
        create_errors.push_back(result.error);
    }

    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreateBatch\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"recordIndexes\": [";
    for (std::size_t index = 0U; index < result.record_indexes.size(); ++index) {
        std::cout << result.record_indexes[index];
        if ((index + 1U) != result.record_indexes.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "],\n";
    std::cout << "    \"createdObjects\": [\n";
    for (std::size_t index = 0U; index < result.created_objects.size(); ++index) {
        print_json_created_visual_object(result.created_objects[index], "      ");
        if ((index + 1U) != result.created_objects.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
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

void print_json_toolbox_create_batch_plan_entry(
    const copperfin::studio::StudioToolboxObjectCreatePlan& plan,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"toolboxItemId\": ";
    print_json_string_view(plan.toolbox_item.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(plan.toolbox_item.title);
    std::cout << ",\n";
    std::cout << indent << "  \"className\": ";
    print_json_string_view(plan.toolbox_item.vfp_class);
    std::cout << ",\n";
    std::cout << indent << "  \"baseClassName\": ";
    print_json_string_view(plan.toolbox_item.base_class);
    std::cout << ",\n";
    std::cout << indent << "  \"toolboxContextProvided\": "
              << (plan.toolbox_context_provided ? "true" : "false") << ",\n";
    std::cout << indent << "  \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << indent << "  \"targetRecordIndex\": " << plan.target_record_index << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"uniqueId\": ";
    print_json_string(plan.unique_id);
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

void print_json_toolbox_create_batch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchPlanResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreateBatchPlan\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    std::vector<std::string> plan_ready_item_ids;
    plan_ready_item_ids.reserve(plan.plans.size());
    for (const auto& create_plan : plan.plans) {
        plan_ready_item_ids.push_back(std::string(create_plan.toolbox_item.id));
    }
    const std::vector<std::string> plan_blocked_item_ids;
    const std::vector<std::string> plan_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"path\": ";
    print_json_string(plan.path);
    std::cout << ",\n";
    std::cout << "    \"toolboxContextProvided\": "
              << (plan.toolbox_context_provided ? "true" : "false") << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << plan.item_count << ",\n";
    std::cout << "    \"planReadyItemIds\": ";
    print_json_string_array(plan_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"planBlockedItemIds\": ";
    print_json_string_array(plan_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"planBlockedErrors\": ";
    print_json_string_array(plan_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"plans\": [\n";
    for (std::size_t index = 0U; index < plan.plans.size(); ++index) {
        print_json_toolbox_create_batch_plan_entry(plan.plans[index], "      ");
        if ((index + 1U) != plan.plans.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_create_batch_from_dispatch_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchFromDispatchResult& result) {
    std::vector<std::string> created_object_names;
    std::vector<std::string> created_unique_ids;
    std::vector<std::string> create_errors;
    if (result.create_result.ok) {
        for (const auto& object : result.create_result.created_objects) {
            if (!object.object_name.empty()) {
                created_object_names.push_back(object.object_name);
            }
            if (!object.unique_id.empty()) {
                created_unique_ids.push_back(object.unique_id);
            }
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
    std::cout << "  \"toolboxCreateBatchFromDispatch\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"batchPlanOk\": " << (result.batch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"batchPlanError\": ";
    print_json_string(result.batch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"batchPlan\": ";
    if (!result.batch_plan.ok) {
        std::cout << "null,\n";
    } else {
        const auto& plan = result.batch_plan.plan;
        std::cout << "{\n";
        std::cout << "      \"path\": ";
        print_json_string(plan.path);
        std::cout << ",\n";
        std::cout << "      \"toolboxContextProvided\": "
                  << (plan.toolbox_context_provided ? "true" : "false") << ",\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"itemCount\": " << plan.item_count << ",\n";
        std::cout << "      \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << ",\n";
        std::cout << "      \"plans\": [\n";
        for (std::size_t index = 0U; index < plan.plans.size(); ++index) {
            print_json_toolbox_create_batch_plan_entry(plan.plans[index], "        ");
            if ((index + 1U) != plan.plans.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ]\n";
        std::cout << "    },\n";
    }
    std::cout << "    \"createResult\": {\n";
    std::cout << "      \"ok\": " << (result.create_result.ok ? "true" : "false") << ",\n";
    std::cout << "      \"error\": ";
    print_json_string(result.create_result.error);
    std::cout << ",\n";
    std::cout << "      \"recordIndexes\": [";
    for (std::size_t index = 0U; index < result.create_result.record_indexes.size(); ++index) {
        std::cout << result.create_result.record_indexes[index];
        if ((index + 1U) != result.create_result.record_indexes.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "],\n";
    std::cout << "      \"createdObjects\": [\n";
    for (std::size_t index = 0U; index < result.create_result.created_objects.size(); ++index) {
        print_json_created_visual_object(result.create_result.created_objects[index], "        ");
        if ((index + 1U) != result.create_result.created_objects.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "      ]\n";
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

void print_json_toolbox_create_batch_dispatch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchDispatchResult& result) {
    std::vector<std::string> dispatch_ready_item_ids;
    std::vector<std::string> dispatch_blocked_item_ids;
    std::vector<std::string> dispatch_blocked_errors;
    if (result.ok) {
        for (const auto& plan : result.plan.plans) {
            dispatch_ready_item_ids.push_back(std::string(plan.toolbox_item.id));
        }
    } else if (!result.error.empty()) {
        dispatch_blocked_errors.push_back(result.error);
    }

    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreateBatchDispatchPlan\": ";
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
    std::cout << "    \"path\": ";
    print_json_string(plan.path);
    std::cout << ",\n";
    std::cout << "    \"toolboxContextProvided\": "
              << (plan.toolbox_context_provided ? "true" : "false") << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << plan.item_count << ",\n";
    std::cout << "    \"plans\": [\n";
    for (std::size_t index = 0U; index < plan.plans.size(); ++index) {
        print_json_toolbox_create_batch_plan_entry(plan.plans[index], "      ");
        if ((index + 1U) != plan.plans.size()) {
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

void print_json_toolbox_create_batch_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreateBatchDispatchCatalog\": ";
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
    if (result.dispatch.ok) {
        for (const auto& plan : result.dispatch.plan.plans) {
            dispatch_ready_item_ids.push_back(std::string(plan.toolbox_item.id));
        }
    } else if (!result.dispatch.error.empty()) {
        dispatch_blocked_errors.push_back(result.dispatch.error);
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
    std::cout << "    \"batchPlanOk\": " << (result.batch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"batchPlanError\": ";
    print_json_string(result.batch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"batchPlan\": ";
    if (!result.batch_plan.ok) {
        std::cout << "null,\n";
    } else {
        const auto& batch_plan = result.batch_plan.plan;
        std::cout << "{\n";
        std::cout << "      \"path\": ";
        print_json_string(batch_plan.path);
        std::cout << ",\n";
        std::cout << "      \"toolboxContextProvided\": "
                  << (batch_plan.toolbox_context_provided ? "true" : "false") << ",\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(batch_plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"itemCount\": " << batch_plan.item_count << ",\n";
        std::cout << "      \"plans\": [\n";
        for (std::size_t index = 0U; index < batch_plan.plans.size(); ++index) {
            print_json_toolbox_create_batch_plan_entry(batch_plan.plans[index], "        ");
            if ((index + 1U) != batch_plan.plans.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"dryRun\": " << (batch_plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (batch_plan.mutates_asset ? "true" : "false") << "\n";
        std::cout << "    },\n";
    }
    std::cout << "    \"dispatchOk\": " << (result.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchError\": ";
    print_json_string(result.dispatch.error);
    std::cout << ",\n";
    std::cout << "    \"dispatch\": ";
    if (!result.dispatch.ok) {
        std::cout << "null\n";
    } else {
        const auto& dispatch_plan = result.dispatch.plan;
        std::cout << "{\n";
        std::cout << "      \"path\": ";
        print_json_string(dispatch_plan.path);
        std::cout << ",\n";
        std::cout << "      \"toolboxContextProvided\": "
                  << (dispatch_plan.toolbox_context_provided ? "true" : "false") << ",\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(dispatch_plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"itemCount\": " << dispatch_plan.item_count << ",\n";
        std::cout << "      \"plans\": [\n";
        for (std::size_t index = 0U; index < dispatch_plan.plans.size(); ++index) {
            print_json_toolbox_create_batch_plan_entry(dispatch_plan.plans[index], "        ");
            if ((index + 1U) != dispatch_plan.plans.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"dispatchArguments\": [";
        for (std::size_t index = 0U; index < dispatch_plan.dispatch_arguments.size(); ++index) {
            print_json_string(dispatch_plan.dispatch_arguments[index]);
            if ((index + 1U) != dispatch_plan.dispatch_arguments.size()) {
                std::cout << ", ";
            }
        }
        std::cout << "],\n";
        std::cout << "      \"dispatchAdmitted\": "
                  << (dispatch_plan.dispatch_admitted ? "true" : "false") << ",\n";
        std::cout << "      \"dryRun\": " << (dispatch_plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"executed\": " << (dispatch_plan.executed ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (dispatch_plan.mutates_asset ? "true" : "false") << "\n";
        std::cout << "    }\n";
    }
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_create_batch_plan_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchPlanCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreateBatchPlanCatalog\": ";
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
    if (result.batch_plan.ok) {
        for (const auto& plan : result.batch_plan.plan.plans) {
            plan_ready_item_ids.push_back(std::string(plan.toolbox_item.id));
        }
    } else if (!result.batch_plan.error.empty()) {
        plan_blocked_errors.push_back(result.batch_plan.error);
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
    std::cout << "    \"batchPlanOk\": " << (result.batch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"batchPlanError\": ";
    print_json_string(result.batch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"batchPlan\": ";
    if (!result.batch_plan.ok) {
        std::cout << "null\n";
    } else {
        const auto& batch_plan = result.batch_plan.plan;
        std::cout << "{\n";
        std::cout << "      \"path\": ";
        print_json_string(batch_plan.path);
        std::cout << ",\n";
        std::cout << "      \"toolboxContextProvided\": "
                  << (batch_plan.toolbox_context_provided ? "true" : "false") << ",\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(batch_plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"itemCount\": " << batch_plan.item_count << ",\n";
        std::cout << "      \"plans\": [\n";
        for (std::size_t index = 0U; index < batch_plan.plans.size(); ++index) {
            print_json_toolbox_create_batch_plan_entry(batch_plan.plans[index], "        ");
            if ((index + 1U) != batch_plan.plans.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"dryRun\": " << (batch_plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (batch_plan.mutates_asset ? "true" : "false") << "\n";
        std::cout << "    }\n";
    }
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_text_toolbox_create_batch_result(const copperfin::vfp::VisualObjectCreateBatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    for (const auto record_index : result.record_indexes) {
        std::cout << "record_index: " << record_index << "\n";
    }
    for (const auto& object : result.created_objects) {
        std::cout << "created_record_index: " << object.record_index << "\n";
        std::cout << "created_object_name: " << object.object_name << "\n";
        std::cout << "created_unique_id: " << object.unique_id << "\n";
        std::cout << "created_parent_name: " << object.parent_name << "\n";
    }
}

void print_text_toolbox_create_batch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchPlanResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "path: " << result.plan.path << "\n";
    std::cout << "toolbox_context_provided: "
              << (result.plan.toolbox_context_provided ? "true" : "false") << "\n";
    std::cout << "toolbox_context: "
              << copperfin::studio::studio_toolbox_context_name(result.plan.toolbox_context) << "\n";
    std::cout << "item_count: " << result.plan.item_count << "\n";
    std::cout << "dry_run: " << (result.plan.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.plan.mutates_asset ? "true" : "false") << "\n";
    for (const auto& plan : result.plan.plans) {
        std::cout << "plan_toolbox_item_id: " << plan.toolbox_item.id << "\n";
        std::cout << "plan_target_record_index: " << plan.target_record_index << "\n";
        std::cout << "plan_object_name: " << plan.object_name << "\n";
        std::cout << "plan_unique_id: " << plan.unique_id << "\n";
        std::cout << "plan_parent_name: " << plan.parent_name << "\n";
    }
}

void print_text_toolbox_create_batch_from_dispatch_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchFromDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "batch_plan_ok: " << (result.batch_plan.ok ? "true" : "false") << "\n";
    if (!result.batch_plan.error.empty()) {
        std::cout << "batch_plan_error: " << result.batch_plan.error << "\n";
    }
    if (result.batch_plan.ok) {
        std::cout << "item_count: " << result.batch_plan.plan.item_count << "\n";
        for (const auto& plan : result.batch_plan.plan.plans) {
            std::cout << "plan_toolbox_item_id: " << plan.toolbox_item.id << "\n";
            std::cout << "plan_target_record_index: " << plan.target_record_index << "\n";
            std::cout << "plan_object_name: " << plan.object_name << "\n";
            std::cout << "plan_unique_id: " << plan.unique_id << "\n";
            std::cout << "plan_parent_name: " << plan.parent_name << "\n";
        }
    }
    std::cout << "create_result_ok: " << (result.create_result.ok ? "true" : "false") << "\n";
    if (!result.create_result.error.empty()) {
        std::cout << "create_result_error: " << result.create_result.error << "\n";
    }
    for (const auto record_index : result.create_result.record_indexes) {
        std::cout << "record_index: " << record_index << "\n";
    }
    for (const auto& object : result.create_result.created_objects) {
        std::cout << "created_record_index: " << object.record_index << "\n";
        std::cout << "created_object_name: " << object.object_name << "\n";
        std::cout << "created_unique_id: " << object.unique_id << "\n";
        std::cout << "created_parent_name: " << object.parent_name << "\n";
    }
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
}

void print_text_toolbox_create_batch_dispatch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "path: " << result.plan.path << "\n";
    std::cout << "toolbox_context_provided: "
              << (result.plan.toolbox_context_provided ? "true" : "false") << "\n";
    std::cout << "toolbox_context: "
              << copperfin::studio::studio_toolbox_context_name(result.plan.toolbox_context) << "\n";
    std::cout << "item_count: " << result.plan.item_count << "\n";
    for (const auto& plan : result.plan.plans) {
        std::cout << "plan_toolbox_item_id: " << plan.toolbox_item.id << "\n";
        std::cout << "plan_target_record_index: " << plan.target_record_index << "\n";
        std::cout << "plan_object_name: " << plan.object_name << "\n";
        std::cout << "plan_unique_id: " << plan.unique_id << "\n";
        std::cout << "plan_parent_name: " << plan.parent_name << "\n";
    }
    for (const auto& argument : result.plan.dispatch_arguments) {
        std::cout << "dispatch_argument: " << argument << "\n";
    }
    std::cout << "dispatch_admitted: " << (result.plan.dispatch_admitted ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (result.plan.dry_run ? "true" : "false") << "\n";
    std::cout << "executed: " << (result.plan.executed ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_toolbox_create_batch_plan_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchPlanCatalogResult& result) {
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
    std::cout << "batch_plan_ok: " << (result.batch_plan.ok ? "true" : "false") << "\n";
    if (!result.batch_plan.error.empty()) {
        std::cout << "batch_plan_error: " << result.batch_plan.error << "\n";
    }
    if (result.batch_plan.ok) {
        for (const auto& plan : result.batch_plan.plan.plans) {
            std::cout << "plan_toolbox_item_id: " << plan.toolbox_item.id << "\n";
            std::cout << "plan_object_name: " << plan.object_name << "\n";
        }
    }
}

void print_text_toolbox_create_batch_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchDispatchCatalogResult& result) {
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
    std::cout << "batch_plan_ok: " << (result.batch_plan.ok ? "true" : "false") << "\n";
    if (!result.batch_plan.error.empty()) {
        std::cout << "batch_plan_error: " << result.batch_plan.error << "\n";
    }
    if (result.batch_plan.ok) {
        for (const auto& plan : result.batch_plan.plan.plans) {
            std::cout << "plan_toolbox_item_id: " << plan.toolbox_item.id << "\n";
            std::cout << "plan_object_name: " << plan.object_name << "\n";
        }
    }
    std::cout << "dispatch_ok: " << (result.dispatch.ok ? "true" : "false") << "\n";
    if (!result.dispatch.error.empty()) {
        std::cout << "dispatch_error: " << result.dispatch.error << "\n";
    }
    if (result.dispatch.ok) {
        for (const auto& argument : result.dispatch.plan.dispatch_arguments) {
            std::cout << "dispatch_argument: " << argument << "\n";
        }
    }
}

std::optional<int> try_handle_toolbox_create_batch_plan_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_batch_plan_catalog_parse =
        parse_toolbox_create_batch_plan_catalog_arguments(catalog, args);
    if (!(toolbox_create_batch_plan_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_batch_plan_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateBatchPlanCatalogResult{
                .ok = false,
                .error = toolbox_create_batch_plan_catalog_parse.error,
                .toolbox_context = {},
                .item_count = 0U,
                .plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .batch_plan = {}
            };
            if (toolbox_create_batch_plan_catalog_parse.output_json) {
                print_json_toolbox_create_batch_plan_catalog_result(result);
            } else {
                print_text_toolbox_create_batch_plan_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto catalog_result = copperfin::studio::plan_visual_object_batch_catalog_from_toolbox_context(
            toolbox_create_batch_plan_catalog_parse.request);
        if (toolbox_create_batch_plan_catalog_parse.output_json) {
            print_json_toolbox_create_batch_plan_catalog_result(catalog_result);
        } else {
            print_text_toolbox_create_batch_plan_catalog_result(catalog_result);
        }
        return catalog_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_batch_plan_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_batch_plan_catalog_parse =
        parse_selection_toolbox_create_batch_plan_catalog_arguments(catalog, args);
    if (!(selection_toolbox_create_batch_plan_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_batch_plan_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanCatalogResult{
                .ok = false,
                .error = selection_toolbox_create_batch_plan_catalog_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .item_count = 0U,
                .plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .batch_plan = {}
            };
            if (selection_toolbox_create_batch_plan_catalog_parse.output_json) {
                print_json_selection_toolbox_create_batch_plan_catalog_result(result);
            } else {
                print_text_selection_toolbox_create_batch_plan_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto catalog_result = copperfin::studio::plan_visual_object_batch_catalog_from_toolbox_selection(
            selection_toolbox_create_batch_plan_catalog_parse.request);
        if (selection_toolbox_create_batch_plan_catalog_parse.output_json) {
            print_json_selection_toolbox_create_batch_plan_catalog_result(catalog_result);
        } else {
            print_text_selection_toolbox_create_batch_plan_catalog_result(catalog_result);
        }
        return catalog_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_batch_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_batch_dispatch_catalog_parse =
        parse_toolbox_create_batch_dispatch_catalog_arguments(catalog, args);
    if (!(toolbox_create_batch_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_batch_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateBatchDispatchCatalogResult{
                .ok = false,
                .error = toolbox_create_batch_dispatch_catalog_parse.error,
                .toolbox_context = {},
                .item_count = 0U,
                .batch_plan = {},
                .dispatch = {},
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_create_batch_dispatch_catalog_parse.output_json) {
                print_json_toolbox_create_batch_dispatch_catalog_result(result);
            } else {
                print_text_toolbox_create_batch_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto catalog_result = copperfin::studio::plan_visual_object_batch_create_dispatch_catalog(
            toolbox_create_batch_dispatch_catalog_parse.request);
        if (toolbox_create_batch_dispatch_catalog_parse.output_json) {
            print_json_toolbox_create_batch_dispatch_catalog_result(catalog_result);
        } else {
            print_text_toolbox_create_batch_dispatch_catalog_result(catalog_result);
        }
        return catalog_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_batch_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_batch_dispatch_catalog_parse =
        parse_selection_toolbox_create_batch_dispatch_catalog_arguments(catalog, args);
    if (!(selection_toolbox_create_batch_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_batch_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchCatalogResult{
                .ok = false,
                .error = selection_toolbox_create_batch_dispatch_catalog_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .item_count = 0U,
                .batch_plan = {},
                .dispatch = {},
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false
            };
            if (selection_toolbox_create_batch_dispatch_catalog_parse.output_json) {
                print_json_selection_toolbox_create_batch_dispatch_catalog_result(result);
            } else {
                print_text_selection_toolbox_create_batch_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto catalog_result =
            copperfin::studio::plan_visual_object_batch_create_dispatch_catalog_from_toolbox_selection(
                selection_toolbox_create_batch_dispatch_catalog_parse.request);
        if (selection_toolbox_create_batch_dispatch_catalog_parse.output_json) {
            print_json_selection_toolbox_create_batch_dispatch_catalog_result(catalog_result);
        } else {
            print_text_selection_toolbox_create_batch_dispatch_catalog_result(catalog_result);
        }
        return catalog_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_batch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_batch_plan_parse = parse_toolbox_create_batch_plan_arguments(catalog, args);
    if (!(toolbox_create_batch_plan_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_batch_plan_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateBatchPlanResult{
                .ok = false,
                .error = toolbox_create_batch_plan_parse.error,
                .plan = {}
            };
            if (toolbox_create_batch_plan_parse.output_json) {
                print_json_toolbox_create_batch_plan_result(result);
            } else {
                print_text_toolbox_create_batch_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto batch_plan_result = copperfin::studio::plan_visual_objects_from_toolbox_items(
            toolbox_create_batch_plan_parse.request);
        if (toolbox_create_batch_plan_parse.output_json) {
            print_json_toolbox_create_batch_plan_result(batch_plan_result);
        } else {
            print_text_toolbox_create_batch_plan_result(batch_plan_result);
        }
        return batch_plan_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_batch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_batch_plan_parse =
        parse_selection_toolbox_create_batch_plan_arguments(catalog, args);
    if (!(selection_toolbox_create_batch_plan_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_batch_plan_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanResult{
                .ok = false,
                .error = selection_toolbox_create_batch_plan_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .item_count = 0U,
                .plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .batch_plan = {}
            };
            if (selection_toolbox_create_batch_plan_parse.output_json) {
                print_json_selection_toolbox_create_batch_plan_result(result);
            } else {
                print_text_selection_toolbox_create_batch_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto batch_plan_result =
            copperfin::studio::plan_visual_objects_from_toolbox_selection(
                selection_toolbox_create_batch_plan_parse.request);
        if (selection_toolbox_create_batch_plan_parse.output_json) {
            print_json_selection_toolbox_create_batch_plan_result(batch_plan_result);
        } else {
            print_text_selection_toolbox_create_batch_plan_result(batch_plan_result);
        }
        return batch_plan_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_batch_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_batch_dispatch_plan_parse =
        parse_selection_toolbox_create_batch_dispatch_plan_arguments(catalog, args);
    if (!(selection_toolbox_create_batch_dispatch_plan_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_batch_dispatch_plan_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchResult{
                .ok = false,
                .error = selection_toolbox_create_batch_dispatch_plan_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .item_count = 0U,
                .batch_plan = {},
                .dispatch = {},
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false
            };
            if (selection_toolbox_create_batch_dispatch_plan_parse.output_json) {
                print_json_selection_toolbox_create_batch_dispatch_plan_result(result);
            } else {
                print_text_selection_toolbox_create_batch_dispatch_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto batch_dispatch_result =
            copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_selection(
                selection_toolbox_create_batch_dispatch_plan_parse.request);
        if (selection_toolbox_create_batch_dispatch_plan_parse.output_json) {
            print_json_selection_toolbox_create_batch_dispatch_plan_result(batch_dispatch_result);
        } else {
            print_text_selection_toolbox_create_batch_dispatch_plan_result(batch_dispatch_result);
        }
        return batch_dispatch_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_batch_from_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_batch_from_dispatch_parse =
        parse_toolbox_create_batch_from_dispatch_arguments(catalog, args);
    if (!(toolbox_create_batch_from_dispatch_parse.requested)) {
        return std::nullopt;
    }

        auto print_batch_create_error = [&](const std::string& error, const int exit_code) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateBatchFromDispatchResult{
                .ok = false,
                .error = error,
                .batch_plan = {
                    .ok = false,
                    .error = error,
                    .plan = {}
                },
                .create_result = {
                    .ok = false,
                    .error = error,
                    .record_indexes = {},
                    .created_objects = {}
                },
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_create_batch_from_dispatch_parse.output_json) {
                print_json_toolbox_create_batch_from_dispatch_result(result);
            } else {
                print_text_toolbox_create_batch_from_dispatch_result(result);
                if (exit_code == 2) {
                    print_usage(catalog);
                }
            }
            return exit_code;
        };

        if (!toolbox_create_batch_from_dispatch_parse.ok) {
            return print_batch_create_error(toolbox_create_batch_from_dispatch_parse.error, 2);
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_create_batch_from_dispatch_parse.launch_request);
        if (!launch_result.ok) {
            return print_batch_create_error(launch_result.error, 4);
        }

        const auto admission_result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation =
                toolbox_create_batch_from_dispatch_parse.admit_palette_invocation
        });
        if (!admission_result.ok) {
            return print_batch_create_error(admission_result.error, 4);
        }

        const auto dispatch_result = copperfin::studio::plan_studio_toolbox_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            return print_batch_create_error(dispatch_result.error, 4);
        }

        auto create_request = toolbox_create_batch_from_dispatch_parse.create_request;
        create_request.dispatch_plan = dispatch_result.plan;
        const auto create_result = copperfin::studio::create_visual_objects_from_toolbox_dispatch(create_request);
        if (toolbox_create_batch_from_dispatch_parse.output_json) {
            print_json_toolbox_create_batch_from_dispatch_result(create_result);
        } else {
            print_text_toolbox_create_batch_from_dispatch_result(create_result);
        }
        return create_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_batch_from_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_batch_from_dispatch_plan_parse =
        parse_toolbox_create_batch_from_dispatch_plan_arguments(catalog, args);
    if (!(toolbox_create_batch_from_dispatch_plan_parse.requested)) {
        return std::nullopt;
    }

        auto print_batch_plan_error = [&](const std::string& error, const int exit_code) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateBatchPlanResult{
                .ok = false,
                .error = error,
                .plan = {}
            };
            if (toolbox_create_batch_from_dispatch_plan_parse.output_json) {
                print_json_toolbox_create_batch_plan_result(result);
            } else {
                print_text_toolbox_create_batch_plan_result(result);
                if (exit_code == 2) {
                    print_usage(catalog);
                }
            }
            return exit_code;
        };

        if (!toolbox_create_batch_from_dispatch_plan_parse.ok) {
            return print_batch_plan_error(toolbox_create_batch_from_dispatch_plan_parse.error, 2);
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_create_batch_from_dispatch_plan_parse.launch_request);
        if (!launch_result.ok) {
            return print_batch_plan_error(launch_result.error, 4);
        }

        const auto admission_result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation =
                toolbox_create_batch_from_dispatch_plan_parse.admit_palette_invocation
        });
        if (!admission_result.ok) {
            return print_batch_plan_error(admission_result.error, 4);
        }

        const auto dispatch_result = copperfin::studio::plan_studio_toolbox_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            return print_batch_plan_error(dispatch_result.error, 4);
        }

        auto create_request = toolbox_create_batch_from_dispatch_plan_parse.create_request;
        create_request.dispatch_plan = dispatch_result.plan;
        const auto batch_plan_result = copperfin::studio::plan_visual_objects_from_toolbox_dispatch(
            create_request);
        if (toolbox_create_batch_from_dispatch_plan_parse.output_json) {
            print_json_toolbox_create_batch_plan_result(batch_plan_result);
        } else {
            print_text_toolbox_create_batch_plan_result(batch_plan_result);
        }
        return batch_plan_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_batch_dispatch_from_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_batch_dispatch_from_dispatch_plan_parse =
        parse_toolbox_create_batch_dispatch_from_dispatch_plan_arguments(catalog, args);
    if (!(toolbox_create_batch_dispatch_from_dispatch_plan_parse.requested)) {
        return std::nullopt;
    }

        auto print_batch_dispatch_error = [&](const std::string& error, const int exit_code) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateBatchDispatchResult{
                .ok = false,
                .error = error,
                .plan = {}
            };
            if (toolbox_create_batch_dispatch_from_dispatch_plan_parse.output_json) {
                print_json_toolbox_create_batch_dispatch_plan_result(result);
            } else {
                print_text_toolbox_create_batch_dispatch_plan_result(result);
                if (exit_code == 2) {
                    print_usage(catalog);
                }
            }
            return exit_code;
        };

        if (!toolbox_create_batch_dispatch_from_dispatch_plan_parse.ok) {
            return print_batch_dispatch_error(toolbox_create_batch_dispatch_from_dispatch_plan_parse.error, 2);
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_create_batch_dispatch_from_dispatch_plan_parse.launch_request);
        if (!launch_result.ok) {
            return print_batch_dispatch_error(launch_result.error, 4);
        }

        const auto admission_result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation =
                toolbox_create_batch_dispatch_from_dispatch_plan_parse.admit_palette_invocation
        });
        if (!admission_result.ok) {
            return print_batch_dispatch_error(admission_result.error, 4);
        }

        const auto dispatch_result = copperfin::studio::plan_studio_toolbox_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            return print_batch_dispatch_error(dispatch_result.error, 4);
        }

        auto create_dispatch_request = toolbox_create_batch_dispatch_from_dispatch_plan_parse.dispatch_request;
        create_dispatch_request.create_request.dispatch_plan = dispatch_result.plan;
        const auto batch_dispatch_result =
            copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_dispatch(
                create_dispatch_request);
        if (toolbox_create_batch_dispatch_from_dispatch_plan_parse.output_json) {
            print_json_toolbox_create_batch_dispatch_plan_result(batch_dispatch_result);
        } else {
            print_text_toolbox_create_batch_dispatch_plan_result(batch_dispatch_result);
        }
        return batch_dispatch_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_batch_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_batch_dispatch_plan_parse =
        parse_toolbox_create_batch_dispatch_plan_arguments(catalog, args);
    if (!(toolbox_create_batch_dispatch_plan_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_batch_dispatch_plan_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateBatchDispatchResult{
                .ok = false,
                .error = toolbox_create_batch_dispatch_plan_parse.error,
                .plan = {}
            };
            if (toolbox_create_batch_dispatch_plan_parse.output_json) {
                print_json_toolbox_create_batch_dispatch_plan_result(result);
            } else {
                print_text_toolbox_create_batch_dispatch_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto batch_plan_result = copperfin::studio::plan_visual_objects_from_toolbox_items(
            toolbox_create_batch_dispatch_plan_parse.request);
        if (!batch_plan_result.ok) {
            const auto result = copperfin::studio::StudioToolboxObjectCreateBatchDispatchResult{
                .ok = false,
                .error = batch_plan_result.error,
                .plan = {}
            };
            if (toolbox_create_batch_dispatch_plan_parse.output_json) {
                print_json_toolbox_create_batch_dispatch_plan_result(result);
            } else {
                print_text_toolbox_create_batch_dispatch_plan_result(result);
            }
            return 4;
        }

        const auto dispatch_result = copperfin::studio::plan_visual_object_batch_create_dispatch({
            .batch_plan = batch_plan_result.plan,
            .admit_create_operation = toolbox_create_batch_dispatch_plan_parse.admit_create_operation
        });
        if (toolbox_create_batch_dispatch_plan_parse.output_json) {
            print_json_toolbox_create_batch_dispatch_plan_result(dispatch_result);
        } else {
            print_text_toolbox_create_batch_dispatch_plan_result(dispatch_result);
        }
        return dispatch_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_create_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_create_batch_parse = parse_toolbox_create_batch_arguments(catalog, args);
    if (!(toolbox_create_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_create_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectCreateBatchResult{
                .ok = false,
                .error = toolbox_create_batch_parse.error,
                .record_indexes = {},
                .created_objects = {}
            };
            if (toolbox_create_batch_parse.output_json) {
                print_json_toolbox_create_batch_result(result);
            } else {
                print_text_toolbox_create_batch_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto create_result = copperfin::studio::create_visual_objects_from_toolbox_items(
            toolbox_create_batch_parse.request);
        if (toolbox_create_batch_parse.output_json) {
            print_json_toolbox_create_batch_result(create_result);
        } else {
            print_text_toolbox_create_batch_result(create_result);
        }
        return create_result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_create_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_create_batch_parse = parse_selection_toolbox_create_batch_arguments(catalog, args);
    if (!(selection_toolbox_create_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_create_batch_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxObjectCreateBatchResult{
                .ok = false,
                .error = selection_toolbox_create_batch_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .launch_plan = {},
                .item_count = 0U,
                .batch_plan = {},
                .create_result = {},
                .dry_run = true,
                .mutates_asset = false
            };
            if (selection_toolbox_create_batch_parse.output_json) {
                print_json_selection_toolbox_create_batch_result(result);
            } else {
                print_text_selection_toolbox_create_batch_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto create_result = copperfin::studio::create_visual_objects_from_toolbox_selection(
            selection_toolbox_create_batch_parse.request);
        if (selection_toolbox_create_batch_parse.output_json) {
            print_json_selection_toolbox_create_batch_result(create_result);
        } else {
            print_text_selection_toolbox_create_batch_result(create_result);
        }
        return create_result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
