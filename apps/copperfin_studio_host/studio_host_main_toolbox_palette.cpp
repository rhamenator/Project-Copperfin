// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string toolbox_palette_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.ToolboxPaletteParse.Error.MissingValue",
        {{"option", option}});
}

std::string toolbox_palette_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token) {
    return catalog.translate(
        "StudioHost.ToolboxPaletteParse.Error.UnknownSelectionContextToken",
        {{"token", token}});
}

std::string toolbox_palette_parse_unknown_toolbox_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token) {
    return catalog.translate(
        "StudioHost.ToolboxPaletteParse.Error.UnknownToolboxContextToken",
        {{"token", token}});
}

std::string toolbox_palette_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.ToolboxPaletteParse.Error.NonNegativeInteger",
        {{"option", option}});
}

std::string toolbox_palette_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument) {
    return catalog.translate(
        "StudioHost.ToolboxPaletteParse.Error.UnknownOption",
        {
            {"commandName", command_name},
            {"argument", argument}
        });
}

std::string toolbox_palette_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

ToolboxPaletteLaunchPlanParseResult parse_toolbox_palette_launch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxPaletteLaunchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-palette-launch-plan") != args.end();
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
                fail(toolbox_palette_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-palette-launch-plan") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_palette_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_palette_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(toolbox_palette_parse_unknown_option(catalog, "toolbox-palette-launch-plan", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_palette_parse_message(catalog, "StudioHost.ToolboxPaletteParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxPaletteLaunchCatalogParseResult parse_toolbox_palette_launch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxPaletteLaunchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-palette-launch-catalog") != args.end();
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
                fail(toolbox_palette_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-palette-launch-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_palette_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(toolbox_palette_parse_unknown_option(catalog, "toolbox-palette-launch-catalog", argument));
        }
    }

    return result;
}

ToolboxPaletteQueryParseResult parse_toolbox_palette_query_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxPaletteQueryParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-palette-query") != args.end();
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
                fail(toolbox_palette_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-palette-query") {
            continue;
        }
        if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_palette_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--toolbox-search") {
            result.request.search_text = require_value(argument);
        } else if (argument == "--toolbox-category") {
            result.request.category = require_value(argument);
        } else {
            fail(toolbox_palette_parse_unknown_option(catalog, "toolbox-palette-query", argument));
        }
    }

    if (result.ok && !result.context_provided) {
        fail(toolbox_palette_parse_message(catalog, "StudioHost.ToolboxPaletteParse.Error.NoToolboxContext"));
    }
    return result;
}

void print_json_toolbox_palette_query_result(
    const copperfin::studio::StudioToolboxPaletteQueryResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxPaletteQuery\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"searchText\": ";
    print_json_string(result.search_text);
    std::cout << ",\n";
    std::cout << "    \"category\": ";
    print_json_string(result.category);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < result.items.size(); ++index) {
        print_json_toolbox_item_descriptor(result.items[index], "      ");
        if ((index + 1U) != result.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_palette_launch_plan_result(
    const copperfin::studio::StudioToolboxPaletteLaunchPlanResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxPaletteLaunchPlan\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    const std::string selection_context =
        copperfin::studio::studio_editor_selection_context_name(plan.selection_context);
    std::vector<std::string> launch_ready_selection_contexts;
    launch_ready_selection_contexts.push_back(selection_context);
    const std::vector<std::string> launch_blocked_selection_contexts;
    const std::vector<std::string> launch_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(selection_context);
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << plan.item_count << ",\n";
    std::cout << "    \"launchReadySelectionContexts\": ";
    print_json_string_array(launch_ready_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedSelectionContexts\": ";
    print_json_string_array(launch_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedErrors\": ";
    print_json_string_array(launch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < plan.items.size(); ++index) {
        print_json_toolbox_item_descriptor(plan.items[index], "      ");
        if ((index + 1U) != plan.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_palette_launch_catalog_entry(
    const copperfin::studio::StudioToolboxPaletteLaunchCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
    std::cout << ",\n";
    std::cout << indent << "  \"toolboxAvailable\": " << (entry.toolbox_available ? "true" : "false") << ",\n";
    std::cout << indent << "  \"itemCount\": " << entry.item_count << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(entry.error);
    std::cout << ",\n";
    std::cout << indent << "  \"launchPlan\": ";
    if (!entry.launch_plan.ok) {
        std::cout << "null\n";
        std::cout << indent << "}";
        return;
    }

    const auto& plan = entry.launch_plan.plan;
    std::cout << "{\n";
    std::cout << indent << "    \"ok\": true,\n";
    std::cout << indent << "    \"error\": \"\",\n";
    std::cout << indent << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << indent << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << indent << "    \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << indent << "    \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << indent << "    \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << indent << "    \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << indent << "    \"itemCount\": " << plan.item_count << ",\n";
    std::cout << indent << "    \"items\": [\n";
    for (std::size_t index = 0U; index < plan.items.size(); ++index) {
        print_json_toolbox_item_descriptor(plan.items[index], indent + "      ");
        if ((index + 1U) != plan.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << indent << "    ]\n";
    std::cout << indent << "  }\n";
    std::cout << indent << "}";
}

void print_json_toolbox_palette_launch_catalog_result(
    const copperfin::studio::StudioToolboxPaletteLaunchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxPaletteLaunchCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> launch_ready_selection_contexts;
    std::vector<std::string> launch_blocked_selection_contexts;
    std::vector<std::string> launch_blocked_errors;
    for (const auto& entry : result.entries) {
        const std::string selection_context =
            copperfin::studio::studio_editor_selection_context_name(entry.selection_context);
        if (entry.launch_plan.ok) {
            launch_ready_selection_contexts.push_back(selection_context);
        } else {
            launch_blocked_selection_contexts.push_back(selection_context);
            launch_blocked_errors.push_back(entry.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"contextCount\": " << result.context_count << ",\n";
    std::cout << "    \"launchPlanCount\": " << result.launch_plan_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"launchReadySelectionContexts\": ";
    print_json_string_array(launch_ready_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedSelectionContexts\": ";
    print_json_string_array(launch_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedErrors\": ";
    print_json_string_array(launch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_toolbox_palette_launch_catalog_entry(result.entries[index], "      ");
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

void print_text_toolbox_palette_launch_plan_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const copperfin::studio::StudioToolboxPaletteLaunchPlanResult& result) {
    std::cout << catalog.translate(result.ok ? "StudioHost.ToolboxPalette.Text.Status.Ok" :
                                   "StudioHost.ToolboxPalette.Text.Status.Error") << "\n";
    if (!result.error.empty()) {
        std::cout << catalog.translate("StudioHost.Prefix.Error") << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.SelectionContext") << ": "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.ToolboxContext") << ": "
              << copperfin::studio::studio_toolbox_context_name(plan.toolbox_context) << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.AssetPath") << ": " << plan.asset_path << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.RecordIndex") << ": " << plan.record_index << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.ObjectName") << ": " << plan.object_name << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.UniqueId") << ": " << plan.unique_id << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.ItemCount") << ": " << plan.item_count << "\n";
    for (const auto& item : plan.items) {
        std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.Item") << ": "
                  << item.id << " " << item.title << "\n";
    }
}

void print_text_toolbox_palette_launch_catalog_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const copperfin::studio::StudioToolboxPaletteLaunchCatalogResult& result) {
    std::cout << catalog.translate(result.ok ? "StudioHost.ToolboxPalette.Text.Status.Ok" :
                                   "StudioHost.ToolboxPalette.Text.Status.Error") << "\n";
    if (!result.error.empty()) {
        std::cout << catalog.translate("StudioHost.Prefix.Error") << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.ContextCount") << ": " << result.context_count << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.LaunchPlanCount") << ": " << result.launch_plan_count << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.ErrorCount") << ": " << result.error_count << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.DryRun") << ": " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.MutatesAsset") << ": " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.EntrySelectionContext") << ": "
                  << copperfin::studio::studio_editor_selection_context_name(entry.selection_context) << "\n";
        std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.EntryToolboxAvailable") << ": "
                  << (entry.toolbox_available ? "true" : "false") << "\n";
        std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.EntryItemCount") << ": "
                  << entry.item_count << "\n";
        if (!entry.error.empty()) {
            std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.EntryError") << ": "
                      << entry.error << "\n";
        }
    }
}

void print_text_toolbox_palette_query_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const copperfin::studio::StudioToolboxPaletteQueryResult& result) {
    std::cout << catalog.translate(result.ok ? "StudioHost.ToolboxPalette.Text.Status.Ok" :
                                   "StudioHost.ToolboxPalette.Text.Status.Error") << "\n";
    if (!result.error.empty()) {
        std::cout << catalog.translate("StudioHost.Prefix.Error") << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.ToolboxContext") << ": "
              << copperfin::studio::studio_toolbox_context_name(result.toolbox_context) << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.SearchText") << ": " << result.search_text << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.Category") << ": " << result.category << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.ItemCount") << ": " << result.item_count << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.DryRun") << ": " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.MutatesAsset") << ": " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& item : result.items) {
        std::cout << catalog.translate("StudioHost.ToolboxPalette.Text.Label.Item") << ": "
                  << item.id << " " << item.title << "\n";
    }
}

std::optional<int> try_handle_toolbox_palette_query(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_palette_query_parse = parse_toolbox_palette_query_arguments(catalog, args);
    if (!(toolbox_palette_query_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_palette_query_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxPaletteQueryResult{
                .ok = false,
                .error = toolbox_palette_query_parse.error,
                .toolbox_context = toolbox_palette_query_parse.request.toolbox_context,
                .search_text = toolbox_palette_query_parse.request.search_text,
                .category = toolbox_palette_query_parse.request.category,
                .item_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .items = {}
            };
            if (toolbox_palette_query_parse.output_json) {
                print_json_toolbox_palette_query_result(result);
            } else {
                print_text_toolbox_palette_query_result(catalog, result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::query_studio_toolbox_palette(
            toolbox_palette_query_parse.request);
        if (toolbox_palette_query_parse.output_json) {
            print_json_toolbox_palette_query_result(result);
        } else {
            print_text_toolbox_palette_query_result(catalog, result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_palette_launch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_palette_launch_parse = parse_toolbox_palette_launch_plan_arguments(catalog, args);
    if (!(toolbox_palette_launch_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_palette_launch_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxPaletteLaunchPlanResult{
                .ok = false,
                .error = toolbox_palette_launch_parse.error,
                .plan = {}
            };
            if (toolbox_palette_launch_parse.output_json) {
                print_json_toolbox_palette_launch_plan_result(result);
            } else {
                print_text_toolbox_palette_launch_plan_result(catalog, result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_palette_launch_parse.request);
        if (toolbox_palette_launch_parse.output_json) {
            print_json_toolbox_palette_launch_plan_result(result);
        } else {
            print_text_toolbox_palette_launch_plan_result(catalog, result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_palette_launch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_palette_launch_catalog_parse = parse_toolbox_palette_launch_catalog_arguments(catalog, args);
    if (!(toolbox_palette_launch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_palette_launch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxPaletteLaunchCatalogResult{
                .ok = false,
                .error = toolbox_palette_launch_catalog_parse.error,
                .context_count = 0U,
                .launch_plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (toolbox_palette_launch_catalog_parse.output_json) {
                print_json_toolbox_palette_launch_catalog_result(result);
            } else {
                print_text_toolbox_palette_launch_catalog_result(catalog, result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_palette_launch_catalog(
            toolbox_palette_launch_catalog_parse.request);
        if (toolbox_palette_launch_catalog_parse.output_json) {
            print_json_toolbox_palette_launch_catalog_result(result);
        } else {
            print_text_toolbox_palette_launch_catalog_result(catalog, result);
        }
        return result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
