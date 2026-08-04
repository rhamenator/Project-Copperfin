// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string builder_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.BuilderParse.Error.MissingValue",
        {{"option", option}});
}

std::string builder_parse_unknown_builder_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token) {
    return catalog.translate(
        "StudioHost.BuilderParse.Error.UnknownBuilderContextToken",
        {{"token", token}});
}

std::string builder_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token) {
    return catalog.translate(
        "StudioHost.BuilderParse.Error.UnknownSelectionContextToken",
        {{"token", token}});
}

std::string builder_parse_record_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.BuilderParse.Error.RecordNonNegativeInteger",
        {{"option", "--record"}});
}

std::string builder_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.BuilderParse.Error.BooleanValueRequired",
        {
            {"option", option},
            {"trueValue", "true"},
            {"falseValue", "false"}
        });
}

std::string builder_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument) {
    return catalog.translate(
        "StudioHost.BuilderParse.Error.UnknownOption",
        {
            {"commandName", command_name},
            {"argument", argument}
        });
}

std::string builder_parse_context_conflict(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view request_name_key) {
    return catalog.translate(
        "StudioHost.BuilderParse.Error.ContextConflict",
        {
            {"requestName", catalog.translate(request_name_key)},
            {"builderContextOption", "--builder-context"},
            {"selectionContextOption", "--selection-context"}
        });
}

std::string builder_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

SelectionBuilderLaunchCatalogParseResult
parse_selection_builder_launch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionBuilderLaunchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-builder-launch-catalog") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-builder-launch-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_selection_context_token(catalog, token));
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
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(builder_parse_unknown_option(catalog, "selection-builder-launch-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoSelectionContext"));
    }
    return result;
}

SelectionBuilderInvocationAdmissionCatalogParseResult
parse_selection_builder_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionBuilderInvocationAdmissionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--selection-builder-invocation-admission-catalog") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-builder-invocation-admission-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_selection_context_token(catalog, token));
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
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.request.admit_ui_launches = admitted;
        } else {
            fail(builder_parse_unknown_option(catalog, "selection-builder-invocation-admission-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoSelectionContext"));
    }
    return result;
}

SelectionBuilderDispatchExecutionCatalogParseResult
parse_selection_builder_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionBuilderDispatchExecutionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-builder-dispatch-execution-catalog")
        != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-builder-dispatch-execution-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_selection_context_token(catalog, token));
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
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.request.admit_ui_launches = admitted;
        } else if (argument == "--admit-builder-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-builder-execution"));
                continue;
            }
            result.request.admit_execution = admitted;
        } else {
            fail(builder_parse_unknown_option(catalog, "selection-builder-dispatch-execution-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoSelectionContext"));
    }
    return result;
}

SelectionBuilderDispatchCatalogParseResult
parse_selection_builder_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionBuilderDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-builder-dispatch-catalog") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-builder-dispatch-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_selection_context_token(catalog, token));
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
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.request.admit_ui_launches = admitted;
        } else {
            fail(builder_parse_unknown_option(catalog, "selection-builder-dispatch-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoSelectionContext"));
    }
    return result;
}

void print_json_builder_launch_plan_result(
    const copperfin::studio::StudioBuilderLaunchPlanResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"builderLaunchPlan\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    const auto& builder = plan.builder;
    const std::vector<std::string> launch_ready_builder_ids{std::string(builder.id)};
    const std::vector<std::string> launch_blocked_builder_ids;
    const std::vector<std::string> launch_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"builderId\": ";
    print_json_string_view(builder.id);
    std::cout << ",\n";
    std::cout << "    \"title\": ";
    print_json_string_view(builder.title);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(builder.kind));
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    if (selection_context != nullptr) {
        print_json_string(copperfin::studio::studio_editor_selection_context_name(*selection_context));
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << "    \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << "    \"vfp9Equivalent\": ";
    print_json_string_view(builder.vfp9_equivalent);
    std::cout << ",\n";
    std::cout << "    \"vfp9EquivalentDisplay\": ";
    print_json_string_view(builder.vfp9_equivalent_display);
    std::cout << ",\n";
    std::cout << "    \"copperfinComponent\": ";
    print_json_string_view(builder.copperfin_component);
    std::cout << ",\n";
    std::cout << "    \"entryPoint\": ";
    print_json_string(plan.entry_point);
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
    std::cout << "    \"launchReadyBuilderIds\": ";
    print_json_string_array(launch_ready_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedBuilderIds\": ";
    print_json_string_array(launch_blocked_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedErrors\": ";
    print_json_string_array(launch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"description\": ";
    print_json_string_view(builder.description);
    std::cout << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_builder_launch_catalog_entry(
    const copperfin::studio::StudioBuilderLaunchCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"builderId\": ";
    print_json_string_view(entry.builder.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(entry.builder.title);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(entry.builder.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"launchOk\": " << (entry.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    if (!entry.launch_plan.ok) {
        print_json_string(entry.launch_plan.error);
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }

    print_json_string("");
    std::cout << ",\n";
    const auto& plan = entry.launch_plan.plan;
    const auto& builder = plan.builder;
    std::cout << indent << "  \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << indent << "  \"vfp9Equivalent\": ";
    print_json_string_view(builder.vfp9_equivalent);
    std::cout << ",\n";
    std::cout << indent << "  \"vfp9EquivalentDisplay\": ";
    print_json_string_view(builder.vfp9_equivalent_display);
    std::cout << ",\n";
    std::cout << indent << "  \"copperfinComponent\": ";
    print_json_string_view(builder.copperfin_component);
    std::cout << ",\n";
    std::cout << indent << "  \"entryPoint\": ";
    print_json_string(plan.entry_point);
    std::cout << ",\n";
    std::cout << indent << "  \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << indent << "  \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << indent << "  \"description\": ";
    print_json_string_view(builder.description);
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_builder_launch_catalog_result(
    const copperfin::studio::StudioBuilderLaunchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"builderLaunchCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> launch_ready_builder_ids;
    std::vector<std::string> launch_blocked_builder_ids;
    std::vector<std::string> launch_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.launch_plan.ok) {
            launch_ready_builder_ids.push_back(std::string(entry.builder.id));
        } else {
            launch_blocked_builder_ids.push_back(std::string(entry.builder.id));
            launch_blocked_errors.push_back(entry.launch_plan.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(result.context));
    std::cout << ",\n";
    std::cout << "    \"builderCount\": " << result.builder_count << ",\n";
    std::cout << "    \"launchPlanCount\": " << result.launch_plan_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"launchReadyBuilderIds\": ";
    print_json_string_array(launch_ready_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedBuilderIds\": ";
    print_json_string_array(launch_blocked_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedErrors\": ";
    print_json_string_array(launch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_builder_launch_catalog_entry(result.entries[index], "      ");
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

void print_json_builder_invocation_admission_result(
    const copperfin::studio::StudioBuilderInvocationAdmissionResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"builderInvocationAdmission\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    const auto& builder = plan.builder;
    const std::vector<std::string> admission_ready_builder_ids{std::string(builder.id)};
    const std::vector<std::string> admission_blocked_builder_ids;
    const std::vector<std::string> admission_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"builderId\": ";
    print_json_string_view(builder.id);
    std::cout << ",\n";
    std::cout << "    \"title\": ";
    print_json_string_view(builder.title);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(builder.kind));
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    if (selection_context != nullptr) {
        print_json_string(copperfin::studio::studio_editor_selection_context_name(*selection_context));
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << "    \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"entryPoint\": ";
    print_json_string(plan.entry_point);
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
    std::cout << "    \"admissionReadyBuilderIds\": ";
    print_json_string_array(admission_ready_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedBuilderIds\": ";
    print_json_string_array(admission_blocked_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"uiLaunchAdmitted\": " << (plan.ui_launch_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_builder_invocation_admission_catalog_entry(
    const copperfin::studio::StudioBuilderInvocationAdmissionCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"builderId\": ";
    print_json_string_view(entry.builder.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(entry.builder.title);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(entry.builder.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"launchOk\": " << (entry.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"admissionOk\": "
              << (entry.invocation_admission.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    if (!entry.invocation_admission.ok) {
        print_json_string(entry.invocation_admission.error);
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }

    print_json_string("");
    std::cout << ",\n";
    const auto& plan = entry.invocation_admission.plan;
    std::cout << indent << "  \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"entryPoint\": ";
    print_json_string(plan.entry_point);
    std::cout << ",\n";
    std::cout << indent << "  \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << indent << "  \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << indent << "  \"uiLaunchAdmitted\": "
              << (plan.ui_launch_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_builder_invocation_admission_catalog_result(
    const copperfin::studio::StudioBuilderInvocationAdmissionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"builderInvocationAdmissionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> admission_ready_builder_ids;
    std::vector<std::string> admission_blocked_builder_ids;
    std::vector<std::string> admission_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.invocation_admission.ok) {
            admission_ready_builder_ids.push_back(std::string(entry.builder.id));
        } else {
            admission_blocked_builder_ids.push_back(std::string(entry.builder.id));
            admission_blocked_errors.push_back(entry.invocation_admission.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(result.context));
    std::cout << ",\n";
    std::cout << "    \"builderCount\": " << result.builder_count << ",\n";
    std::cout << "    \"admissionCount\": " << result.admission_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"admissionReadyBuilderIds\": ";
    print_json_string_array(admission_ready_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedBuilderIds\": ";
    print_json_string_array(admission_blocked_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_builder_invocation_admission_catalog_entry(result.entries[index], "      ");
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

void print_json_builder_dispatch_result(
    const copperfin::studio::StudioBuilderDispatchResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"builderDispatch\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    const auto& builder = plan.builder;
    const std::vector<std::string> dispatch_ready_builder_ids{std::string(builder.id)};
    const std::vector<std::string> dispatch_blocked_builder_ids;
    const std::vector<std::string> dispatch_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"builderId\": ";
    print_json_string_view(builder.id);
    std::cout << ",\n";
    std::cout << "    \"title\": ";
    print_json_string_view(builder.title);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(builder.kind));
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    if (selection_context != nullptr) {
        print_json_string(copperfin::studio::studio_editor_selection_context_name(*selection_context));
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << "    \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"entryPoint\": ";
    print_json_string(plan.entry_point);
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
    std::cout << "    \"dispatchReadyBuilderIds\": ";
    print_json_string_array(dispatch_ready_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedBuilderIds\": ";
    print_json_string_array(dispatch_blocked_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"dispatchArguments\": [";
    for (std::size_t index = 0U; index < plan.dispatch_arguments.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        print_json_string(plan.dispatch_arguments[index]);
    }
    std::cout << "],\n";
    std::cout << "    \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_builder_execution_result(
    const copperfin::studio::StudioBuilderDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command,
    const copperfin::studio::StudioEditorSelectionContext* selection_context) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"builderExecution\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << ",\n";
        std::cout << "  \"executionAdmitted\": " << (result.execution_admitted ? "true" : "false") << ",\n";
        std::cout << "  \"executed\": " << (result.executed ? "true" : "false") << ",\n";
        std::cout << "  \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
        std::cout << "  \"launchCommand\": ";
        print_json_string(launch_command);
        std::cout << ",\n";
        std::cout << "  \"executedCommand\": ";
        print_json_string(executed_command);
        std::cout << ",\n";
        std::cout << "  \"observedExitCode\": " << result.observation.exit_code << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.dispatch_plan;
    const std::vector<std::string> execution_ready_builder_ids{std::string(plan.builder.id)};
    const std::vector<std::string> execution_blocked_builder_ids;
    const std::vector<std::string> execution_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"builderId\": ";
    print_json_string_view(plan.builder.id);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(plan.builder.kind));
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    if (selection_context != nullptr) {
        print_json_string(copperfin::studio::studio_editor_selection_context_name(*selection_context));
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << "    \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"entryPoint\": ";
    print_json_string(plan.entry_point);
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
    std::cout << "    \"executionReadyBuilderIds\": ";
    print_json_string_array(execution_ready_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedBuilderIds\": ";
    print_json_string_array(execution_blocked_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedErrors\": ";
    print_json_string_array(execution_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"launchCommand\": ";
    print_json_string(launch_command);
    std::cout << ",\n";
    std::cout << "    \"executedCommand\": ";
    print_json_string(executed_command);
    std::cout << ",\n";
    std::cout << "    \"observedExitCode\": " << result.observation.exit_code << ",\n";
    std::cout << "    \"executionAdmitted\": " << (result.execution_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"executed\": " << (result.executed ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_builder_dispatch_catalog_entry(
    const copperfin::studio::StudioBuilderDispatchCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"builderId\": ";
    print_json_string_view(entry.builder.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(entry.builder.title);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(entry.builder.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"launchOk\": " << (entry.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"admissionOk\": " << (entry.invocation_admission.ok ? "true" : "false")
              << ",\n";
    std::cout << indent << "  \"dispatchOk\": " << (entry.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    if (!entry.dispatch.ok) {
        print_json_string(entry.dispatch.error);
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }

    print_json_string("");
    std::cout << ",\n";
    const auto& plan = entry.dispatch.plan;
    std::cout << indent << "  \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"entryPoint\": ";
    print_json_string(plan.entry_point);
    std::cout << ",\n";
    std::cout << indent << "  \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << indent << "  \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchArguments\": [";
    for (std::size_t index = 0U; index < plan.dispatch_arguments.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        print_json_string(plan.dispatch_arguments[index]);
    }
    std::cout << "],\n";
    std::cout << indent << "  \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_builder_dispatch_catalog_result(
    const copperfin::studio::StudioBuilderDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"builderDispatchCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> dispatch_ready_builder_ids;
    std::vector<std::string> dispatch_blocked_builder_ids;
    std::vector<std::string> dispatch_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.dispatch.ok) {
            dispatch_ready_builder_ids.push_back(std::string(entry.builder.id));
        } else {
            dispatch_blocked_builder_ids.push_back(std::string(entry.builder.id));
            dispatch_blocked_errors.push_back(entry.dispatch.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(result.context));
    std::cout << ",\n";
    std::cout << "    \"builderCount\": " << result.builder_count << ",\n";
    std::cout << "    \"dispatchCount\": " << result.dispatch_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchReadyBuilderIds\": ";
    print_json_string_array(dispatch_ready_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedBuilderIds\": ";
    print_json_string_array(dispatch_blocked_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_builder_dispatch_catalog_entry(result.entries[index], "      ");
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

void print_json_builder_dispatch_execution_catalog_entry(
    const copperfin::studio::StudioBuilderDispatchExecutionCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"builderId\": ";
    print_json_string_view(entry.builder.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(entry.builder.title);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(entry.builder.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"launchOk\": " << (entry.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"admissionOk\": " << (entry.invocation_admission.ok ? "true" : "false")
              << ",\n";
    std::cout << indent << "  \"dispatchOk\": " << (entry.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executionAdmitted\": "
              << (entry.execution_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executionReady\": " << (entry.execution_ready ? "true" : "false")
              << ",\n";
    std::cout << indent << "  \"executionError\": ";
    print_json_string(entry.execution_error);
    std::cout << ",\n";
    std::cout << indent << "  \"error\": ";
    if (!entry.dispatch.ok) {
        print_json_string(entry.dispatch.error);
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }

    print_json_string("");
    std::cout << ",\n";
    const auto& plan = entry.dispatch.plan;
    std::cout << indent << "  \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"entryPoint\": ";
    print_json_string(plan.entry_point);
    std::cout << ",\n";
    std::cout << indent << "  \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << indent << "  \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchArguments\": [";
    for (std::size_t index = 0U; index < plan.dispatch_arguments.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        print_json_string(plan.dispatch_arguments[index]);
    }
    std::cout << "],\n";
    std::cout << indent << "  \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_builder_dispatch_execution_catalog_result(
    const copperfin::studio::StudioBuilderDispatchExecutionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"builderDispatchExecutionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> execution_ready_builder_ids;
    std::vector<std::string> execution_blocked_builder_ids;
    std::vector<std::string> execution_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.execution_ready) {
            execution_ready_builder_ids.push_back(std::string(entry.builder.id));
        } else {
            execution_blocked_builder_ids.push_back(std::string(entry.builder.id));
            execution_blocked_errors.push_back(entry.execution_error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(result.context));
    std::cout << ",\n";
    std::cout << "    \"builderCount\": " << result.builder_count << ",\n";
    std::cout << "    \"executionReadyCount\": " << result.execution_ready_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"executionReadyBuilderIds\": ";
    print_json_string_array(execution_ready_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedBuilderIds\": ";
    print_json_string_array(execution_blocked_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedErrors\": ";
    print_json_string_array(execution_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_builder_dispatch_execution_catalog_entry(result.entries[index], "      ");
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

void print_text_builder_launch_plan_result(
    const copperfin::studio::StudioBuilderLaunchPlanResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "builder_id: " << plan.builder.id << "\n";
    std::cout << "kind: " << copperfin::studio::studio_builder_kind_name(plan.builder.kind) << "\n";
    if (selection_context != nullptr) {
        std::cout << "selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(*selection_context) << "\n";
    }
    std::cout << "context: " << copperfin::studio::studio_builder_context_name(plan.context) << "\n";
    std::cout << "vfp9_equivalent: " << plan.builder.vfp9_equivalent_display << "\n";
    std::cout << "copperfin_component: " << plan.builder.copperfin_component << "\n";
    std::cout << "entry_point: " << plan.entry_point << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
}

void print_text_builder_launch_catalog_result(
    const copperfin::studio::StudioBuilderLaunchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "context: " << copperfin::studio::studio_builder_context_name(result.context) << "\n";
    std::cout << "builder_count: " << result.builder_count << "\n";
    std::cout << "launch_plan_count: " << result.launch_plan_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_builder_id: " << entry.builder.id << "\n";
        std::cout << "entry_launch_ok: " << (entry.launch_plan.ok ? "true" : "false") << "\n";
        if (!entry.launch_plan.error.empty()) {
            std::cout << "entry_error: " << entry.launch_plan.error << "\n";
        }
    }
}

void print_text_builder_invocation_admission_result(
    const copperfin::studio::StudioBuilderInvocationAdmissionResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "builder_id: " << plan.builder.id << "\n";
    std::cout << "kind: " << copperfin::studio::studio_builder_kind_name(plan.builder.kind) << "\n";
    if (selection_context != nullptr) {
        std::cout << "selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(*selection_context) << "\n";
    }
    std::cout << "context: " << copperfin::studio::studio_builder_context_name(plan.context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "entry_point: " << plan.entry_point << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "ui_launch_admitted: " << (plan.ui_launch_admitted ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_builder_invocation_admission_catalog_result(
    const copperfin::studio::StudioBuilderInvocationAdmissionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "context: " << copperfin::studio::studio_builder_context_name(result.context) << "\n";
    std::cout << "builder_count: " << result.builder_count << "\n";
    std::cout << "admission_count: " << result.admission_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_builder_id: " << entry.builder.id << "\n";
        std::cout << "entry_admission_ok: " << (entry.invocation_admission.ok ? "true" : "false") << "\n";
        if (!entry.invocation_admission.error.empty()) {
            std::cout << "entry_error: " << entry.invocation_admission.error << "\n";
        }
    }
}

void print_text_builder_dispatch_result(
    const copperfin::studio::StudioBuilderDispatchResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "builder_id: " << plan.builder.id << "\n";
    std::cout << "kind: " << copperfin::studio::studio_builder_kind_name(plan.builder.kind) << "\n";
    if (selection_context != nullptr) {
        std::cout << "selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(*selection_context) << "\n";
    }
    std::cout << "context: " << copperfin::studio::studio_builder_context_name(plan.context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "entry_point: " << plan.entry_point << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    for (const auto& argument : plan.dispatch_arguments) {
        std::cout << "dispatch_argument: " << argument << "\n";
    }
    std::cout << "dispatch_admitted: " << (plan.dispatch_admitted ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "executed: " << (plan.executed ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_builder_execution_result(
    const copperfin::studio::StudioBuilderDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command,
    const copperfin::studio::StudioEditorSelectionContext* selection_context) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "execution_admitted: " << (result.execution_admitted ? "true" : "false") << "\n";
    std::cout << "executed: " << (result.executed ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "launch_command: " << launch_command << "\n";
    std::cout << "executed_command: " << executed_command << "\n";
    std::cout << "observed_exit_code: " << result.observation.exit_code << "\n";
    if (!result.ok) {
        return;
    }
    const auto& plan = result.dispatch_plan;
    std::cout << "builder_id: " << plan.builder.id << "\n";
    std::cout << "kind: " << copperfin::studio::studio_builder_kind_name(plan.builder.kind) << "\n";
    if (selection_context != nullptr) {
        std::cout << "selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(*selection_context) << "\n";
    }
    std::cout << "context: " << copperfin::studio::studio_builder_context_name(plan.context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "entry_point: " << plan.entry_point << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "dispatch_admitted: " << (plan.dispatch_admitted ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
}

void print_text_builder_dispatch_catalog_result(
    const copperfin::studio::StudioBuilderDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "context: " << copperfin::studio::studio_builder_context_name(result.context) << "\n";
    std::cout << "builder_count: " << result.builder_count << "\n";
    std::cout << "dispatch_count: " << result.dispatch_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_builder_id: " << entry.builder.id << "\n";
        std::cout << "entry_dispatch_ok: " << (entry.dispatch.ok ? "true" : "false") << "\n";
        if (!entry.dispatch.error.empty()) {
            std::cout << "entry_error: " << entry.dispatch.error << "\n";
        }
    }
}

void print_text_builder_dispatch_execution_catalog_result(
    const copperfin::studio::StudioBuilderDispatchExecutionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "context: " << copperfin::studio::studio_builder_context_name(result.context) << "\n";
    std::cout << "builder_count: " << result.builder_count << "\n";
    std::cout << "execution_ready_count: " << result.execution_ready_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_builder_id: " << entry.builder.id << "\n";
        std::cout << "entry_execution_ready: " << (entry.execution_ready ? "true" : "false") << "\n";
        if (!entry.execution_error.empty()) {
            std::cout << "entry_execution_error: " << entry.execution_error << "\n";
        }
    }
}

std::optional<int> try_handle_builder_launch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto builder_launch_parse = parse_builder_launch_plan_arguments(catalog, args);
    if (!(builder_launch_parse.requested)) {
        return std::nullopt;
    }

        if (!builder_launch_parse.ok) {
            const auto result = copperfin::studio::StudioBuilderLaunchPlanResult{
                .ok = false,
                .error = builder_launch_parse.error,
                .plan = {}
            };
            if (builder_launch_parse.output_json) {
                print_json_builder_launch_plan_result(result);
            } else {
                print_text_builder_launch_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        if (builder_launch_parse.selection_context_provided) {
            const auto selection_result = copperfin::studio::plan_studio_builder_launch_for_selection({
                .selection_context = builder_launch_parse.selection_context,
                .builder_id = builder_launch_parse.request.builder_id,
                .asset_path = builder_launch_parse.request.asset_path,
                .record_index = builder_launch_parse.request.record_index,
                .object_name = builder_launch_parse.request.object_name,
                .unique_id = builder_launch_parse.request.unique_id
            });
            const auto result = copperfin::studio::StudioBuilderLaunchPlanResult{
                .ok = selection_result.ok,
                .error = selection_result.error,
                .plan = selection_result.plan
            };
            if (builder_launch_parse.output_json) {
                print_json_builder_launch_plan_result(result, &selection_result.selection_context);
            } else {
                print_text_builder_launch_plan_result(result, &selection_result.selection_context);
            }
            return result.ok ? 0 : 4;
        }

        const auto result = copperfin::studio::plan_studio_builder_launch(builder_launch_parse.request);
        if (builder_launch_parse.output_json) {
            print_json_builder_launch_plan_result(result);
        } else {
            print_text_builder_launch_plan_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_builder_launch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto builder_launch_catalog_parse = parse_builder_launch_catalog_arguments(catalog, args);
    if (!(builder_launch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!builder_launch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioBuilderLaunchCatalogResult{
                .ok = false,
                .error = builder_launch_catalog_parse.error,
                .context = {},
                .builder_count = 0U,
                .launch_plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (builder_launch_catalog_parse.output_json) {
                print_json_builder_launch_catalog_result(result);
            } else {
                print_text_builder_launch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_builder_launch_catalog(
            builder_launch_catalog_parse.request);
        if (builder_launch_catalog_parse.output_json) {
            print_json_builder_launch_catalog_result(result);
        } else {
            print_text_builder_launch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_builder_launch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_builder_launch_catalog_parse =
        parse_selection_builder_launch_catalog_arguments(catalog, args);
    if (!(selection_builder_launch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_builder_launch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionBuilderLaunchCatalogResult{
                .ok = false,
                .error = selection_builder_launch_catalog_parse.error,
                .selection_context = {},
                .builder_count = 0U,
                .launch_plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (selection_builder_launch_catalog_parse.output_json) {
                print_json_selection_builder_launch_catalog_result(result);
            } else {
                print_text_selection_builder_launch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_builder_launch_catalog_for_selection(
            selection_builder_launch_catalog_parse.request);
        if (selection_builder_launch_catalog_parse.output_json) {
            print_json_selection_builder_launch_catalog_result(result);
        } else {
            print_text_selection_builder_launch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_builder_invocation_admission(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto builder_invocation_admission_parse = parse_builder_invocation_admission_arguments(catalog, args);
    if (!(builder_invocation_admission_parse.requested)) {
        return std::nullopt;
    }

        if (!builder_invocation_admission_parse.ok) {
            const auto result = copperfin::studio::StudioBuilderInvocationAdmissionResult{
                .ok = false,
                .error = builder_invocation_admission_parse.error,
                .plan = {}
            };
            if (builder_invocation_admission_parse.output_json) {
                print_json_builder_invocation_admission_result(result);
            } else {
                print_text_builder_invocation_admission_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        copperfin::studio::StudioBuilderLaunchPlanResult launch_result{};
        const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr;
        if (builder_invocation_admission_parse.selection_context_provided) {
            const auto selection_result = copperfin::studio::plan_studio_builder_launch_for_selection({
                .selection_context = builder_invocation_admission_parse.selection_context,
                .builder_id = builder_invocation_admission_parse.request.builder_id,
                .asset_path = builder_invocation_admission_parse.request.asset_path,
                .record_index = builder_invocation_admission_parse.request.record_index,
                .object_name = builder_invocation_admission_parse.request.object_name,
                .unique_id = builder_invocation_admission_parse.request.unique_id
            });
            launch_result = {
                .ok = selection_result.ok,
                .error = selection_result.error,
                .plan = selection_result.plan
            };
            selection_context = &builder_invocation_admission_parse.selection_context;
        } else {
            launch_result = copperfin::studio::plan_studio_builder_launch(builder_invocation_admission_parse.request);
        }

        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioBuilderInvocationAdmissionResult{
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            };
            if (builder_invocation_admission_parse.output_json) {
                print_json_builder_invocation_admission_result(result, selection_context);
            } else {
                print_text_builder_invocation_admission_result(result, selection_context);
            }
            return 4;
        }

        const auto result = copperfin::studio::plan_studio_builder_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_ui_launch = builder_invocation_admission_parse.admit_ui_launch
        });
        if (builder_invocation_admission_parse.output_json) {
            print_json_builder_invocation_admission_result(result, selection_context);
        } else {
            print_text_builder_invocation_admission_result(result, selection_context);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_builder_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto builder_invocation_admission_catalog_parse =
        parse_builder_invocation_admission_catalog_arguments(catalog, args);
    if (!(builder_invocation_admission_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!builder_invocation_admission_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioBuilderInvocationAdmissionCatalogResult{
                .ok = false,
                .error = builder_invocation_admission_catalog_parse.error,
                .context = {},
                .builder_count = 0U,
                .admission_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (builder_invocation_admission_catalog_parse.output_json) {
                print_json_builder_invocation_admission_catalog_result(result);
            } else {
                print_text_builder_invocation_admission_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_builder_invocation_admission_catalog(
            builder_invocation_admission_catalog_parse.request);
        if (builder_invocation_admission_catalog_parse.output_json) {
            print_json_builder_invocation_admission_catalog_result(result);
        } else {
            print_text_builder_invocation_admission_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_builder_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_builder_invocation_admission_catalog_parse =
        parse_selection_builder_invocation_admission_catalog_arguments(catalog, args);
    if (!(selection_builder_invocation_admission_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_builder_invocation_admission_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogResult{
                .ok = false,
                .error = selection_builder_invocation_admission_catalog_parse.error,
                .selection_context = {},
                .builder_count = 0U,
                .admission_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (selection_builder_invocation_admission_catalog_parse.output_json) {
                print_json_selection_builder_invocation_admission_catalog_result(result);
            } else {
                print_text_selection_builder_invocation_admission_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_builder_invocation_admission_catalog_for_selection(
            selection_builder_invocation_admission_catalog_parse.request);
        if (selection_builder_invocation_admission_catalog_parse.output_json) {
            print_json_selection_builder_invocation_admission_catalog_result(result);
        } else {
            print_text_selection_builder_invocation_admission_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_builder_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto builder_dispatch_parse = parse_builder_dispatch_arguments(catalog, args);
    if (!(builder_dispatch_parse.requested)) {
        return std::nullopt;
    }

        if (!builder_dispatch_parse.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchResult{
                .ok = false,
                .error = builder_dispatch_parse.error,
                .plan = {}
            };
            if (builder_dispatch_parse.output_json) {
                print_json_builder_dispatch_result(result);
            } else {
                print_text_builder_dispatch_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        copperfin::studio::StudioBuilderLaunchPlanResult launch_result{};
        const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr;
        if (builder_dispatch_parse.selection_context_provided) {
            const auto selection_result = copperfin::studio::plan_studio_builder_launch_for_selection({
                .selection_context = builder_dispatch_parse.selection_context,
                .builder_id = builder_dispatch_parse.request.builder_id,
                .asset_path = builder_dispatch_parse.request.asset_path,
                .record_index = builder_dispatch_parse.request.record_index,
                .object_name = builder_dispatch_parse.request.object_name,
                .unique_id = builder_dispatch_parse.request.unique_id
            });
            launch_result = {
                .ok = selection_result.ok,
                .error = selection_result.error,
                .plan = selection_result.plan
            };
            selection_context = &builder_dispatch_parse.selection_context;
        } else {
            launch_result = copperfin::studio::plan_studio_builder_launch(builder_dispatch_parse.request);
        }

        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchResult{
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            };
            if (builder_dispatch_parse.output_json) {
                print_json_builder_dispatch_result(result, selection_context);
            } else {
                print_text_builder_dispatch_result(result, selection_context);
            }
            return 4;
        }

        const auto admission_result = copperfin::studio::plan_studio_builder_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_ui_launch = builder_dispatch_parse.admit_ui_launch
        });
        if (!admission_result.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchResult{
                .ok = false,
                .error = admission_result.error,
                .plan = {}
            };
            if (builder_dispatch_parse.output_json) {
                print_json_builder_dispatch_result(result, selection_context);
            } else {
                print_text_builder_dispatch_result(result, selection_context);
            }
            return 4;
        }

        const auto result = copperfin::studio::plan_studio_builder_dispatch({
            .admission_plan = admission_result.plan
        });
        if (builder_dispatch_parse.output_json) {
            print_json_builder_dispatch_result(result, selection_context);
        } else {
            print_text_builder_dispatch_result(result, selection_context);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_builder_execute(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto builder_execute_parse = parse_builder_execute_arguments(catalog, args);
    if (!(builder_execute_parse.requested)) {
        return std::nullopt;
    }

        if (!builder_execute_parse.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchExecutionResult{
                .ok = false,
                .error = builder_execute_parse.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = builder_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (builder_execute_parse.output_json) {
                print_json_builder_execution_result(result, builder_execute_parse.launch_command, {});
            } else {
                print_text_builder_execution_result(result, builder_execute_parse.launch_command, {});
                print_usage(catalog);
            }
            return 2;
        }

        copperfin::studio::StudioBuilderLaunchPlanResult launch_result{};
        const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr;
        if (builder_execute_parse.selection_context_provided) {
            const auto selection_result = copperfin::studio::plan_studio_builder_launch_for_selection({
                .selection_context = builder_execute_parse.selection_context,
                .builder_id = builder_execute_parse.request.builder_id,
                .asset_path = builder_execute_parse.request.asset_path,
                .record_index = builder_execute_parse.request.record_index,
                .object_name = builder_execute_parse.request.object_name,
                .unique_id = builder_execute_parse.request.unique_id
            });
            launch_result = {
                .ok = selection_result.ok,
                .error = selection_result.error,
                .plan = selection_result.plan
            };
            selection_context = &builder_execute_parse.selection_context;
        } else {
            launch_result = copperfin::studio::plan_studio_builder_launch(builder_execute_parse.request);
        }

        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchExecutionResult{
                .ok = false,
                .error = launch_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = builder_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (builder_execute_parse.output_json) {
                print_json_builder_execution_result(result, builder_execute_parse.launch_command, {}, selection_context);
            } else {
                print_text_builder_execution_result(result, builder_execute_parse.launch_command, {}, selection_context);
            }
            return 4;
        }

        const auto admission_result = copperfin::studio::plan_studio_builder_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_ui_launch = builder_execute_parse.admit_ui_launch
        });
        if (!admission_result.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchExecutionResult{
                .ok = false,
                .error = admission_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = builder_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (builder_execute_parse.output_json) {
                print_json_builder_execution_result(result, builder_execute_parse.launch_command, {}, selection_context);
            } else {
                print_text_builder_execution_result(result, builder_execute_parse.launch_command, {}, selection_context);
            }
            return 4;
        }

        const auto dispatch_result = copperfin::studio::plan_studio_builder_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchExecutionResult{
                .ok = false,
                .error = dispatch_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = builder_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (builder_execute_parse.output_json) {
                print_json_builder_execution_result(result, builder_execute_parse.launch_command, {}, selection_context);
            } else {
                print_text_builder_execution_result(result, builder_execute_parse.launch_command, {}, selection_context);
            }
            return 4;
        }

        const std::string executed_command = build_shell_command(
            builder_execute_parse.launch_command,
            dispatch_result.plan.dispatch_arguments);
        const auto result = copperfin::studio::execute_studio_builder_dispatch({
            .dispatch_plan = dispatch_result.plan,
            .admit_execution = builder_execute_parse.admit_execution,
            .executor = [&](const copperfin::studio::StudioBuilderDispatchPlan& plan) {
                const int exit_code = execute_launch_command(
                    builder_execute_parse.launch_command,
                    plan.dispatch_arguments);
                return copperfin::studio::StudioBuilderDispatchExecutionObservation{
                    .launched = true,
                    .exit_code = exit_code,
                    .output = {},
                    .error = exit_code == 0
                        ? std::string{}
                        : catalog.translate("StudioHost.BuilderExecution.Error.LaunchCommandNonZeroExit"),
                    .mutates_asset = false
                };
            }
        });
        if (builder_execute_parse.output_json) {
            print_json_builder_execution_result(
                result,
                builder_execute_parse.launch_command,
                executed_command,
                selection_context);
        } else {
            print_text_builder_execution_result(
                result,
                builder_execute_parse.launch_command,
                executed_command,
                selection_context);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_builder_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto builder_dispatch_catalog_parse = parse_builder_dispatch_catalog_arguments(catalog, args);
    if (!(builder_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!builder_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchCatalogResult{
                .ok = false,
                .error = builder_dispatch_catalog_parse.error,
                .context = {},
                .builder_count = 0U,
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (builder_dispatch_catalog_parse.output_json) {
                print_json_builder_dispatch_catalog_result(result);
            } else {
                print_text_builder_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_builder_dispatch_catalog(
            builder_dispatch_catalog_parse.request);
        if (builder_dispatch_catalog_parse.output_json) {
            print_json_builder_dispatch_catalog_result(result);
        } else {
            print_text_builder_dispatch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_builder_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto builder_dispatch_execution_catalog_parse =
        parse_builder_dispatch_execution_catalog_arguments(catalog, args);
    if (!(builder_dispatch_execution_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!builder_dispatch_execution_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioBuilderDispatchExecutionCatalogResult{
                .ok = false,
                .error = builder_dispatch_execution_catalog_parse.error,
                .context = {},
                .builder_count = 0U,
                .execution_ready_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (builder_dispatch_execution_catalog_parse.output_json) {
                print_json_builder_dispatch_execution_catalog_result(result);
            } else {
                print_text_builder_dispatch_execution_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_builder_dispatch_execution_catalog(
            builder_dispatch_execution_catalog_parse.request);
        if (builder_dispatch_execution_catalog_parse.output_json) {
            print_json_builder_dispatch_execution_catalog_result(result);
        } else {
            print_text_builder_dispatch_execution_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_builder_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_builder_dispatch_execution_catalog_parse =
        parse_selection_builder_dispatch_execution_catalog_arguments(catalog, args);
    if (!(selection_builder_dispatch_execution_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_builder_dispatch_execution_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionBuilderDispatchExecutionCatalogResult{
                .ok = false,
                .error = selection_builder_dispatch_execution_catalog_parse.error,
                .selection_context = {},
                .builder_count = 0U,
                .execution_ready_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (selection_builder_dispatch_execution_catalog_parse.output_json) {
                print_json_selection_builder_dispatch_execution_catalog_result(result);
            } else {
                print_text_selection_builder_dispatch_execution_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_builder_dispatch_execution_catalog_for_selection(
            selection_builder_dispatch_execution_catalog_parse.request);
        if (selection_builder_dispatch_execution_catalog_parse.output_json) {
            print_json_selection_builder_dispatch_execution_catalog_result(result);
        } else {
            print_text_selection_builder_dispatch_execution_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_builder_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_builder_dispatch_catalog_parse =
        parse_selection_builder_dispatch_catalog_arguments(catalog, args);
    if (!(selection_builder_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_builder_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionBuilderDispatchCatalogResult{
                .ok = false,
                .error = selection_builder_dispatch_catalog_parse.error,
                .selection_context = {},
                .builder_count = 0U,
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (selection_builder_dispatch_catalog_parse.output_json) {
                print_json_selection_builder_dispatch_catalog_result(result);
            } else {
                print_text_selection_builder_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_builder_dispatch_catalog_for_selection(
            selection_builder_dispatch_catalog_parse.request);
        if (selection_builder_dispatch_catalog_parse.output_json) {
            print_json_selection_builder_dispatch_catalog_result(result);
        } else {
            print_text_selection_builder_dispatch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
