// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string editor_action_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.EditorActionParse.Error.MissingValue",
        {{"option", option}});
}

std::string editor_action_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token) {
    return catalog.translate(
        "StudioHost.EditorActionParse.Error.UnknownSelectionContextToken",
        {{"token", token}});
}

std::string editor_action_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.EditorActionParse.Error.NonNegativeInteger",
        {{"option", option}});
}

std::string editor_action_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.EditorActionParse.Error.BooleanValueRequired",
        {
            {"option", option},
            {"trueValue", "true"},
            {"falseValue", "false"}
        });
}

std::string editor_action_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument) {
    return catalog.translate(
        "StudioHost.EditorActionParse.Error.UnknownOption",
        {
            {"commandName", command_name},
            {"argument", argument}
        });
}

std::string editor_action_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

void print_json_editor_action_launch_plan_result(
    const copperfin::studio::StudioEditorActionLaunchPlanResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"editorActionLaunchPlan\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    const auto& action = plan.action;
    const std::vector<std::string> launch_ready_action_ids{std::string(action.id)};
    const std::vector<std::string> launch_blocked_action_ids;
    const std::vector<std::string> launch_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"actionId\": ";
    print_json_string_view(action.id);
    std::cout << ",\n";
    std::cout << "    \"label\": ";
    print_json_string_view(action.label);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(action.kind));
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"targetSurface\": ";
    print_json_string(plan.target_surface);
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
    std::cout << "    \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << "    \"line\": " << plan.line << ",\n";
    std::cout << "    \"column\": " << plan.column << ",\n";
    std::cout << "    \"launchReadyActionIds\": ";
    print_json_string_array(launch_ready_action_ids);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedActionIds\": ";
    print_json_string_array(launch_blocked_action_ids);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedErrors\": ";
    print_json_string_array(launch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"description\": ";
    print_json_string_view(action.description);
    std::cout << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_editor_action_launch_catalog_entry(
    const copperfin::studio::StudioEditorActionLaunchCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"actionId\": ";
    print_json_string_view(entry.action.id);
    std::cout << ",\n";
    std::cout << indent << "  \"label\": ";
    print_json_string_view(entry.action.label);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(entry.action.kind));
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
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"targetSurface\": ";
    print_json_string(plan.target_surface);
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
    std::cout << indent << "  \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << indent << "  \"line\": " << plan.line << ",\n";
    std::cout << indent << "  \"column\": " << plan.column << ",\n";
    std::cout << indent << "  \"description\": ";
    print_json_string_view(plan.action.description);
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_editor_action_launch_catalog_result(
    const copperfin::studio::StudioEditorActionLaunchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"editorActionLaunchCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> launch_ready_action_ids;
    std::vector<std::string> launch_blocked_action_ids;
    std::vector<std::string> launch_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.launch_plan.ok) {
            launch_ready_action_ids.push_back(std::string(entry.action.id));
        } else {
            launch_blocked_action_ids.push_back(std::string(entry.action.id));
            launch_blocked_errors.push_back(entry.launch_plan.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"actionCount\": " << result.action_count << ",\n";
    std::cout << "    \"launchPlanCount\": " << result.launch_plan_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"launchReadyActionIds\": ";
    print_json_string_array(launch_ready_action_ids);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedActionIds\": ";
    print_json_string_array(launch_blocked_action_ids);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedErrors\": ";
    print_json_string_array(launch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_editor_action_launch_catalog_entry(result.entries[index], "      ");
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

void print_json_editor_action_invocation_admission_result(
    const copperfin::studio::StudioEditorActionInvocationAdmissionResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"editorActionInvocationAdmission\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    const auto& action = plan.action;
    const std::vector<std::string> admission_ready_action_ids{std::string(action.id)};
    const std::vector<std::string> admission_blocked_action_ids;
    const std::vector<std::string> admission_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"actionId\": ";
    print_json_string_view(action.id);
    std::cout << ",\n";
    std::cout << "    \"label\": ";
    print_json_string_view(action.label);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(action.kind));
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"targetSurface\": ";
    print_json_string(plan.target_surface);
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
    std::cout << "    \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << "    \"line\": " << plan.line << ",\n";
    std::cout << "    \"column\": " << plan.column << ",\n";
    std::cout << "    \"admissionReadyActionIds\": ";
    print_json_string_array(admission_ready_action_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedActionIds\": ";
    print_json_string_array(admission_blocked_action_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"editorInvocationAdmitted\": "
              << (plan.editor_invocation_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_editor_action_invocation_admission_catalog_entry(
    const copperfin::studio::StudioEditorActionInvocationAdmissionCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"actionId\": ";
    print_json_string_view(entry.action.id);
    std::cout << ",\n";
    std::cout << indent << "  \"label\": ";
    print_json_string_view(entry.action.label);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(entry.action.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"launchOk\": " << (entry.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"invocationAdmissionOk\": "
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
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"targetSurface\": ";
    print_json_string(plan.target_surface);
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
    std::cout << indent << "  \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << indent << "  \"line\": " << plan.line << ",\n";
    std::cout << indent << "  \"column\": " << plan.column << ",\n";
    std::cout << indent << "  \"editorInvocationAdmitted\": "
              << (plan.editor_invocation_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << ",\n";
    std::cout << indent << "  \"description\": ";
    print_json_string_view(plan.action.description);
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_editor_action_invocation_admission_catalog_result(
    const copperfin::studio::StudioEditorActionInvocationAdmissionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"editorActionInvocationAdmissionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> admission_ready_action_ids;
    std::vector<std::string> admission_blocked_action_ids;
    std::vector<std::string> admission_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.invocation_admission.ok) {
            admission_ready_action_ids.push_back(std::string(entry.action.id));
        } else {
            admission_blocked_action_ids.push_back(std::string(entry.action.id));
            admission_blocked_errors.push_back(entry.invocation_admission.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"actionCount\": " << result.action_count << ",\n";
    std::cout << "    \"admissionCount\": " << result.admission_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"admissionReadyActionIds\": ";
    print_json_string_array(admission_ready_action_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedActionIds\": ";
    print_json_string_array(admission_blocked_action_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_editor_action_invocation_admission_catalog_entry(result.entries[index], "      ");
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

void print_json_editor_action_dispatch_result(
    const copperfin::studio::StudioEditorActionDispatchResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"editorActionDispatch\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    const auto& action = plan.action;
    const std::vector<std::string> dispatch_ready_action_ids{std::string(action.id)};
    const std::vector<std::string> dispatch_blocked_action_ids;
    const std::vector<std::string> dispatch_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"actionId\": ";
    print_json_string_view(action.id);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(action.kind));
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"targetSurface\": ";
    print_json_string(plan.target_surface);
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
    std::cout << "    \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << "    \"line\": " << plan.line << ",\n";
    std::cout << "    \"column\": " << plan.column << ",\n";
    std::cout << "    \"dispatchReadyActionIds\": ";
    print_json_string_array(dispatch_ready_action_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedActionIds\": ";
    print_json_string_array(dispatch_blocked_action_ids);
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

void print_json_editor_action_execution_result(
    const copperfin::studio::StudioEditorActionDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"editorActionExecution\": ";
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
    const auto& action = plan.action;
    const std::vector<std::string> execution_ready_action_ids{std::string(action.id)};
    const std::vector<std::string> execution_blocked_action_ids;
    const std::vector<std::string> execution_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"actionId\": ";
    print_json_string_view(action.id);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(action.kind));
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"targetSurface\": ";
    print_json_string(plan.target_surface);
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
    std::cout << "    \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << "    \"line\": " << plan.line << ",\n";
    std::cout << "    \"column\": " << plan.column << ",\n";
    std::cout << "    \"executionReadyActionIds\": ";
    print_json_string_array(execution_ready_action_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedActionIds\": ";
    print_json_string_array(execution_blocked_action_ids);
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

void print_json_editor_action_dispatch_catalog_entry(
    const copperfin::studio::StudioEditorActionDispatchCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"actionId\": ";
    print_json_string_view(entry.action.id);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(entry.action.kind));
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
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"targetSurface\": ";
    print_json_string(plan.target_surface);
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
    std::cout << indent << "  \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << indent << "  \"line\": " << plan.line << ",\n";
    std::cout << indent << "  \"column\": " << plan.column << ",\n";
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

void print_json_editor_action_dispatch_catalog_result(
    const copperfin::studio::StudioEditorActionDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"editorActionDispatchCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> dispatch_ready_action_ids;
    std::vector<std::string> dispatch_blocked_action_ids;
    std::vector<std::string> dispatch_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.dispatch.ok) {
            dispatch_ready_action_ids.push_back(std::string(entry.action.id));
        } else {
            dispatch_blocked_action_ids.push_back(std::string(entry.action.id));
            dispatch_blocked_errors.push_back(entry.dispatch.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"actionCount\": " << result.action_count << ",\n";
    std::cout << "    \"dispatchCount\": " << result.dispatch_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchReadyActionIds\": ";
    print_json_string_array(dispatch_ready_action_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedActionIds\": ";
    print_json_string_array(dispatch_blocked_action_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_editor_action_dispatch_catalog_entry(result.entries[index], "      ");
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

void print_json_editor_action_dispatch_execution_catalog_entry(
    const copperfin::studio::StudioEditorActionDispatchExecutionCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"actionId\": ";
    print_json_string_view(entry.action.id);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(entry.action.kind));
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
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"targetSurface\": ";
    print_json_string(plan.target_surface);
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
    std::cout << indent << "  \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << indent << "  \"line\": " << plan.line << ",\n";
    std::cout << indent << "  \"column\": " << plan.column << ",\n";
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

void print_json_editor_action_dispatch_execution_catalog_result(
    const copperfin::studio::StudioEditorActionDispatchExecutionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"editorActionDispatchExecutionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> execution_ready_action_ids;
    std::vector<std::string> execution_blocked_action_ids;
    std::vector<std::string> execution_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.execution_ready) {
            execution_ready_action_ids.push_back(std::string(entry.action.id));
        } else {
            execution_blocked_action_ids.push_back(std::string(entry.action.id));
            execution_blocked_errors.push_back(entry.execution_error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"actionCount\": " << result.action_count << ",\n";
    std::cout << "    \"executionReadyCount\": " << result.execution_ready_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"executionReadyActionIds\": ";
    print_json_string_array(execution_ready_action_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedActionIds\": ";
    print_json_string_array(execution_blocked_action_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedErrors\": ";
    print_json_string_array(execution_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_editor_action_dispatch_execution_catalog_entry(result.entries[index], "      ");
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

void print_text_editor_action_launch_plan_result(
    const copperfin::studio::StudioEditorActionLaunchPlanResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "action_id: " << plan.action.id << "\n";
    std::cout << "kind: " << copperfin::studio::studio_editor_action_kind_name(plan.action.kind) << "\n";
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "target_surface: " << plan.target_surface << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "symbol: " << plan.symbol << "\n";
    std::cout << "line: " << plan.line << "\n";
    std::cout << "column: " << plan.column << "\n";
}

void print_text_editor_action_launch_catalog_result(
    const copperfin::studio::StudioEditorActionLaunchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "action_count: " << result.action_count << "\n";
    std::cout << "launch_plan_count: " << result.launch_plan_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_action_id: " << entry.action.id << "\n";
        std::cout << "entry_launch_ok: " << (entry.launch_plan.ok ? "true" : "false") << "\n";
        if (!entry.launch_plan.error.empty()) {
            std::cout << "entry_error: " << entry.launch_plan.error << "\n";
        }
    }
}

void print_text_editor_action_invocation_admission_result(
    const copperfin::studio::StudioEditorActionInvocationAdmissionResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "action_id: " << plan.action.id << "\n";
    std::cout << "kind: " << copperfin::studio::studio_editor_action_kind_name(plan.action.kind) << "\n";
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "target_surface: " << plan.target_surface << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "symbol: " << plan.symbol << "\n";
    std::cout << "line: " << plan.line << "\n";
    std::cout << "column: " << plan.column << "\n";
    std::cout << "editor_invocation_admitted: "
              << (plan.editor_invocation_admitted ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_editor_action_invocation_admission_catalog_result(
    const copperfin::studio::StudioEditorActionInvocationAdmissionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "action_count: " << result.action_count << "\n";
    std::cout << "admission_count: " << result.admission_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_action_id: " << entry.action.id << "\n";
        std::cout << "entry_launch_ok: " << (entry.launch_plan.ok ? "true" : "false") << "\n";
        std::cout << "entry_invocation_admission_ok: "
                  << (entry.invocation_admission.ok ? "true" : "false") << "\n";
        if (!entry.invocation_admission.error.empty()) {
            std::cout << "entry_error: " << entry.invocation_admission.error << "\n";
        }
    }
}

void print_text_editor_action_dispatch_result(
    const copperfin::studio::StudioEditorActionDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "action_id: " << plan.action.id << "\n";
    std::cout << "kind: " << copperfin::studio::studio_editor_action_kind_name(plan.action.kind) << "\n";
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "target_surface: " << plan.target_surface << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "symbol: " << plan.symbol << "\n";
    std::cout << "line: " << plan.line << "\n";
    std::cout << "column: " << plan.column << "\n";
    for (const auto& argument : plan.dispatch_arguments) {
        std::cout << "dispatch_argument: " << argument << "\n";
    }
    std::cout << "dispatch_admitted: " << (plan.dispatch_admitted ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "executed: " << (plan.executed ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_editor_action_execution_result(
    const copperfin::studio::StudioEditorActionDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command) {
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
    std::cout << "action_id: " << plan.action.id << "\n";
    std::cout << "kind: " << copperfin::studio::studio_editor_action_kind_name(plan.action.kind) << "\n";
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "target_surface: " << plan.target_surface << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "symbol: " << plan.symbol << "\n";
    std::cout << "line: " << plan.line << "\n";
    std::cout << "column: " << plan.column << "\n";
    std::cout << "dispatch_admitted: " << (plan.dispatch_admitted ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
}

void print_text_editor_action_dispatch_catalog_result(
    const copperfin::studio::StudioEditorActionDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "action_count: " << result.action_count << "\n";
    std::cout << "dispatch_count: " << result.dispatch_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_action_id: " << entry.action.id << "\n";
        std::cout << "entry_dispatch_ok: " << (entry.dispatch.ok ? "true" : "false") << "\n";
        if (!entry.dispatch.error.empty()) {
            std::cout << "entry_error: " << entry.dispatch.error << "\n";
        }
    }
}

void print_text_editor_action_dispatch_execution_catalog_result(
    const copperfin::studio::StudioEditorActionDispatchExecutionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "action_count: " << result.action_count << "\n";
    std::cout << "execution_ready_count: " << result.execution_ready_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_action_id: " << entry.action.id << "\n";
        std::cout << "entry_execution_ready: " << (entry.execution_ready ? "true" : "false") << "\n";
        if (!entry.execution_error.empty()) {
            std::cout << "entry_execution_error: " << entry.execution_error << "\n";
        }
    }
}

void print_json_editor_contexts(const std::vector<copperfin::studio::StudioEditorSelectionContext>& contexts) {
    std::cout << "[";
    for (std::size_t index = 0; index < contexts.size(); ++index) {
        print_json_string(copperfin::studio::studio_editor_selection_context_name(contexts[index]));
        if ((index + 1U) != contexts.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

std::optional<int> try_handle_editor_action_launch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto editor_action_launch_parse = parse_editor_action_launch_plan_arguments(catalog, args);
    if (!(editor_action_launch_parse.requested)) {
        return std::nullopt;
    }

        if (!editor_action_launch_parse.ok) {
            const auto result = copperfin::studio::StudioEditorActionLaunchPlanResult{
                .ok = false,
                .error = editor_action_launch_parse.error,
                .plan = {}
            };
            if (editor_action_launch_parse.output_json) {
                print_json_editor_action_launch_plan_result(result);
            } else {
                print_text_editor_action_launch_plan_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_editor_action_launch(
            editor_action_launch_parse.request);
        if (editor_action_launch_parse.output_json) {
            print_json_editor_action_launch_plan_result(result);
        } else {
            print_text_editor_action_launch_plan_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_editor_action_launch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto editor_action_launch_catalog_parse = parse_editor_action_launch_catalog_arguments(catalog, args);
    if (!(editor_action_launch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!editor_action_launch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioEditorActionLaunchCatalogResult{
                .ok = false,
                .error = editor_action_launch_catalog_parse.error,
                .selection_context = {},
                .action_count = 0U,
                .launch_plan_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (editor_action_launch_catalog_parse.output_json) {
                print_json_editor_action_launch_catalog_result(result);
            } else {
                print_text_editor_action_launch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_editor_action_launch_catalog(
            editor_action_launch_catalog_parse.request);
        if (editor_action_launch_catalog_parse.output_json) {
            print_json_editor_action_launch_catalog_result(result);
        } else {
            print_text_editor_action_launch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_editor_action_invocation_admission(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto editor_action_invocation_admission_parse =
        parse_editor_action_invocation_admission_arguments(catalog, args);
    if (!(editor_action_invocation_admission_parse.requested)) {
        return std::nullopt;
    }

        if (!editor_action_invocation_admission_parse.ok) {
            const auto result = copperfin::studio::StudioEditorActionInvocationAdmissionResult{
                .ok = false,
                .error = editor_action_invocation_admission_parse.error,
                .plan = {}
            };
            if (editor_action_invocation_admission_parse.output_json) {
                print_json_editor_action_invocation_admission_result(result);
            } else {
                print_text_editor_action_invocation_admission_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_editor_action_launch(
            editor_action_invocation_admission_parse.request);
        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioEditorActionInvocationAdmissionResult{
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            };
            if (editor_action_invocation_admission_parse.output_json) {
                print_json_editor_action_invocation_admission_result(result);
            } else {
                print_text_editor_action_invocation_admission_result(result);
            }
            return 4;
        }

        const auto result = copperfin::studio::plan_studio_editor_action_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_editor_invocation = editor_action_invocation_admission_parse.admit_editor_invocation
        });
        if (editor_action_invocation_admission_parse.output_json) {
            print_json_editor_action_invocation_admission_result(result);
        } else {
            print_text_editor_action_invocation_admission_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_editor_action_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto editor_action_invocation_admission_catalog_parse =
        parse_editor_action_invocation_admission_catalog_arguments(catalog, args);
    if (!(editor_action_invocation_admission_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!editor_action_invocation_admission_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioEditorActionInvocationAdmissionCatalogResult{
                .ok = false,
                .error = editor_action_invocation_admission_catalog_parse.error,
                .selection_context = {},
                .action_count = 0U,
                .admission_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (editor_action_invocation_admission_catalog_parse.output_json) {
                print_json_editor_action_invocation_admission_catalog_result(result);
            } else {
                print_text_editor_action_invocation_admission_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_editor_action_invocation_admission_catalog(
            editor_action_invocation_admission_catalog_parse.request);
        if (editor_action_invocation_admission_catalog_parse.output_json) {
            print_json_editor_action_invocation_admission_catalog_result(result);
        } else {
            print_text_editor_action_invocation_admission_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_editor_action_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto editor_action_dispatch_parse = parse_editor_action_dispatch_arguments(catalog, args);
    if (!(editor_action_dispatch_parse.requested)) {
        return std::nullopt;
    }

        if (!editor_action_dispatch_parse.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchResult{
                .ok = false,
                .error = editor_action_dispatch_parse.error,
                .plan = {}
            };
            if (editor_action_dispatch_parse.output_json) {
                print_json_editor_action_dispatch_result(result);
            } else {
                print_text_editor_action_dispatch_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_editor_action_launch(
            editor_action_dispatch_parse.request);
        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchResult{
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            };
            if (editor_action_dispatch_parse.output_json) {
                print_json_editor_action_dispatch_result(result);
            } else {
                print_text_editor_action_dispatch_result(result);
            }
            return 4;
        }

        const auto admission_result = copperfin::studio::plan_studio_editor_action_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_editor_invocation = editor_action_dispatch_parse.admit_editor_invocation
        });
        if (!admission_result.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchResult{
                .ok = false,
                .error = admission_result.error,
                .plan = {}
            };
            if (editor_action_dispatch_parse.output_json) {
                print_json_editor_action_dispatch_result(result);
            } else {
                print_text_editor_action_dispatch_result(result);
            }
            return 4;
        }

        const auto result = copperfin::studio::plan_studio_editor_action_dispatch({
            .admission_plan = admission_result.plan
        });
        if (editor_action_dispatch_parse.output_json) {
            print_json_editor_action_dispatch_result(result);
        } else {
            print_text_editor_action_dispatch_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_editor_action_execute(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto editor_action_execute_parse = parse_editor_action_execute_arguments(catalog, args);
    if (!(editor_action_execute_parse.requested)) {
        return std::nullopt;
    }

        if (!editor_action_execute_parse.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchExecutionResult{
                .ok = false,
                .error = editor_action_execute_parse.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = editor_action_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (editor_action_execute_parse.output_json) {
                print_json_editor_action_execution_result(result, editor_action_execute_parse.launch_command, {});
            } else {
                print_text_editor_action_execution_result(result, editor_action_execute_parse.launch_command, {});
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_editor_action_launch(
            editor_action_execute_parse.request);
        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchExecutionResult{
                .ok = false,
                .error = launch_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = editor_action_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (editor_action_execute_parse.output_json) {
                print_json_editor_action_execution_result(
                    result,
                    editor_action_execute_parse.launch_command,
                    {});
            } else {
                print_text_editor_action_execution_result(
                    result,
                    editor_action_execute_parse.launch_command,
                    {});
            }
            return 4;
        }

        const auto admission_result = copperfin::studio::plan_studio_editor_action_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_editor_invocation = editor_action_execute_parse.admit_editor_invocation
        });
        if (!admission_result.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchExecutionResult{
                .ok = false,
                .error = admission_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = editor_action_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (editor_action_execute_parse.output_json) {
                print_json_editor_action_execution_result(
                    result,
                    editor_action_execute_parse.launch_command,
                    {});
            } else {
                print_text_editor_action_execution_result(
                    result,
                    editor_action_execute_parse.launch_command,
                    {});
            }
            return 4;
        }

        const auto dispatch_result = copperfin::studio::plan_studio_editor_action_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchExecutionResult{
                .ok = false,
                .error = dispatch_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = editor_action_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (editor_action_execute_parse.output_json) {
                print_json_editor_action_execution_result(
                    result,
                    editor_action_execute_parse.launch_command,
                    {});
            } else {
                print_text_editor_action_execution_result(
                    result,
                    editor_action_execute_parse.launch_command,
                    {});
            }
            return 4;
        }

        const std::string executed_command = build_shell_command(
            editor_action_execute_parse.launch_command,
            dispatch_result.plan.dispatch_arguments);
        const auto result = copperfin::studio::execute_studio_editor_action_dispatch({
            .dispatch_plan = dispatch_result.plan,
            .admit_execution = editor_action_execute_parse.admit_execution,
            .executor = [&](const copperfin::studio::StudioEditorActionDispatchPlan& plan) {
                const int exit_code = execute_launch_command(
                    editor_action_execute_parse.launch_command,
                    plan.dispatch_arguments);
                return copperfin::studio::StudioEditorActionDispatchExecutionObservation{
                    .launched = true,
                    .exit_code = exit_code,
                    .output = {},
                    .error = exit_code == 0
                        ? std::string{}
                        : catalog.translate("StudioHost.EditorActionExecution.Error.LaunchCommandNonZeroExit"),
                    .mutates_asset = false
                };
            }
        });
        if (editor_action_execute_parse.output_json) {
            print_json_editor_action_execution_result(
                result,
                editor_action_execute_parse.launch_command,
                executed_command);
        } else {
            print_text_editor_action_execution_result(
                result,
                editor_action_execute_parse.launch_command,
                executed_command);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_editor_action_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto editor_action_dispatch_catalog_parse = parse_editor_action_dispatch_catalog_arguments(catalog, args);
    if (!(editor_action_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!editor_action_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchCatalogResult{
                .ok = false,
                .error = editor_action_dispatch_catalog_parse.error,
                .selection_context = {},
                .action_count = 0U,
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (editor_action_dispatch_catalog_parse.output_json) {
                print_json_editor_action_dispatch_catalog_result(result);
            } else {
                print_text_editor_action_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_editor_action_dispatch_catalog(
            editor_action_dispatch_catalog_parse.request);
        if (editor_action_dispatch_catalog_parse.output_json) {
            print_json_editor_action_dispatch_catalog_result(result);
        } else {
            print_text_editor_action_dispatch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_editor_action_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto editor_action_dispatch_execution_catalog_parse =
        parse_editor_action_dispatch_execution_catalog_arguments(catalog, args);
    if (!(editor_action_dispatch_execution_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!editor_action_dispatch_execution_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioEditorActionDispatchExecutionCatalogResult{
                .ok = false,
                .error = editor_action_dispatch_execution_catalog_parse.error,
                .selection_context = {},
                .action_count = 0U,
                .execution_ready_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (editor_action_dispatch_execution_catalog_parse.output_json) {
                print_json_editor_action_dispatch_execution_catalog_result(result);
            } else {
                print_text_editor_action_dispatch_execution_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_editor_action_dispatch_execution_catalog(
            editor_action_dispatch_execution_catalog_parse.request);
        if (editor_action_dispatch_execution_catalog_parse.output_json) {
            print_json_editor_action_dispatch_execution_catalog_result(result);
        } else {
            print_text_editor_action_dispatch_execution_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
