// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string designer_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.DesignerParse.Error.MissingValue",
        {{"option", option}});
}

std::string designer_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token) {
    return catalog.translate(
        "StudioHost.DesignerParse.Error.UnknownSelectionContextToken",
        {{"token", token}});
}

std::string designer_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.DesignerParse.Error.NonNegativeInteger",
        {{"option", option}});
}

std::string designer_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.DesignerParse.Error.BooleanValueRequired",
        {
            {"option", option},
            {"trueValue", "true"},
            {"falseValue", "false"}
        });
}

std::string designer_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument) {
    return catalog.translate(
        "StudioHost.DesignerParse.Error.UnknownOption",
        {
            {"commandName", command_name},
            {"argument", argument}
        });
}

std::string designer_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

void print_json_designer_launch_surface_action(
    const copperfin::studio::StudioEditorActionLaunchPlanResult& result,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(result.error);
    if (!result.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }
    const auto& plan = result.plan;
    const auto& action = plan.action;
    std::cout << ",\n";
    std::cout << indent << "  \"actionId\": ";
    print_json_string_view(action.id);
    std::cout << ",\n";
    std::cout << indent << "  \"label\": ";
    print_json_string_view(action.label);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(action.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"targetSurface\": ";
    print_json_string(plan.target_surface);
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_designer_launch_surface_builder(
    const copperfin::studio::StudioSelectionBuilderLaunchPlanResult& result,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(result.error);
    if (!result.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }
    const auto& plan = result.plan;
    const auto& builder = plan.builder;
    std::cout << ",\n";
    std::cout << indent << "  \"builderId\": ";
    print_json_string_view(builder.id);
    std::cout << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string_view(builder.title);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(builder.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << indent << "  \"entryPoint\": ";
    print_json_string(plan.entry_point);
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_designer_launch_surfaces_result(
    const copperfin::studio::StudioDesignerLaunchSurfacePlanResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"designerLaunchSurfaces\": ";
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
    std::vector<std::string> launch_blocked_selection_contexts;
    std::vector<std::string> launch_blocked_errors;
    if (plan.toolbox_error.empty()) {
        launch_ready_selection_contexts.push_back(selection_context);
    } else {
        launch_blocked_selection_contexts.push_back(selection_context);
        launch_blocked_errors.push_back(plan.toolbox_error);
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(selection_context);
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
    std::cout << "    \"launchReadySelectionContexts\": ";
    print_json_string_array(launch_ready_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedSelectionContexts\": ";
    print_json_string_array(launch_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedErrors\": ";
    print_json_string_array(launch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"editorActionLaunchPlanCount\": " << plan.editor_action_launch_plan_count << ",\n";
    std::cout << "    \"builderLaunchPlanCount\": " << plan.builder_launch_plan_count << ",\n";
    std::cout << "    \"toolboxAvailable\": " << (plan.toolbox_available ? "true" : "false") << ",\n";
    std::cout << "    \"toolboxItemCount\": " << plan.toolbox_item_count << ",\n";
    std::cout << "    \"toolboxError\": ";
    print_json_string(plan.toolbox_error);
    std::cout << ",\n";
    std::cout << "    \"editorActionLaunchPlans\": [\n";
    for (std::size_t index = 0U; index < plan.editor_action_launch_plans.size(); ++index) {
        print_json_designer_launch_surface_action(plan.editor_action_launch_plans[index], "      ");
        if ((index + 1U) != plan.editor_action_launch_plans.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"builderLaunchPlans\": [\n";
    for (std::size_t index = 0U; index < plan.builder_launch_plans.size(); ++index) {
        print_json_designer_launch_surface_builder(plan.builder_launch_plans[index], "      ");
        if ((index + 1U) != plan.builder_launch_plans.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"toolboxPaletteLaunchPlan\": ";
    if (!plan.toolbox_palette_launch_plan.ok) {
        std::cout << "null\n";
    } else {
        const auto& toolbox_plan = plan.toolbox_palette_launch_plan.plan;
        std::cout << "{\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(toolbox_plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"itemCount\": " << toolbox_plan.item_count << ",\n";
        std::cout << "      \"items\": [\n";
        for (std::size_t index = 0U; index < toolbox_plan.items.size(); ++index) {
            print_json_toolbox_item_descriptor(toolbox_plan.items[index], "        ");
            if ((index + 1U) != toolbox_plan.items.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ]\n";
        std::cout << "    }\n";
    }
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_designer_invocation_admission_action(
    const copperfin::studio::StudioEditorActionInvocationAdmissionResult& result,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(result.error);
    if (!result.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }
    const auto& plan = result.plan;
    std::cout << ",\n";
    std::cout << indent << "  \"actionId\": ";
    print_json_string_view(plan.action.id);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_editor_action_kind_name(plan.action.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"targetSurface\": ";
    print_json_string(plan.target_surface);
    std::cout << ",\n";
    std::cout << indent << "  \"editorInvocationAdmitted\": "
              << (plan.editor_invocation_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_designer_invocation_admission_builder(
    const copperfin::studio::StudioBuilderInvocationAdmissionResult& result,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(result.error);
    if (!result.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }
    const auto& plan = result.plan;
    std::cout << ",\n";
    std::cout << indent << "  \"builderId\": ";
    print_json_string_view(plan.builder.id);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(copperfin::studio::studio_builder_kind_name(plan.builder.kind));
    std::cout << ",\n";
    std::cout << indent << "  \"context\": ";
    print_json_string(copperfin::studio::studio_builder_context_name(plan.context));
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"entryPoint\": ";
    print_json_string(plan.entry_point);
    std::cout << ",\n";
    std::cout << indent << "  \"uiLaunchAdmitted\": " << (plan.ui_launch_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_designer_invocation_admission_result(
    const copperfin::studio::StudioDesignerInvocationAdmissionResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"designerInvocationAdmission\": ";
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
    std::vector<std::string> admission_ok_selection_contexts;
    std::vector<std::string> admission_blocked_selection_contexts;
    std::vector<std::string> admission_blocked_errors;
    if (plan.toolbox_error.empty()) {
        admission_ok_selection_contexts.push_back(selection_context);
    } else {
        admission_blocked_selection_contexts.push_back(selection_context);
        admission_blocked_errors.push_back(plan.toolbox_error);
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(selection_context);
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
    std::cout << "    \"admissionOkSelectionContexts\": ";
    print_json_string_array(admission_ok_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedSelectionContexts\": ";
    print_json_string_array(admission_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"editorActionInvocationCount\": " << plan.editor_action_invocation_count << ",\n";
    std::cout << "    \"builderInvocationCount\": " << plan.builder_invocation_count << ",\n";
    std::cout << "    \"toolboxAvailable\": " << (plan.toolbox_available ? "true" : "false") << ",\n";
    std::cout << "    \"toolboxItemCount\": " << plan.toolbox_item_count << ",\n";
    std::cout << "    \"toolboxError\": ";
    print_json_string(plan.toolbox_error);
    std::cout << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"editorActionInvocations\": [\n";
    for (std::size_t index = 0U; index < plan.editor_action_invocations.size(); ++index) {
        print_json_designer_invocation_admission_action(plan.editor_action_invocations[index], "      ");
        if ((index + 1U) != plan.editor_action_invocations.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"builderInvocations\": [\n";
    for (std::size_t index = 0U; index < plan.builder_invocations.size(); ++index) {
        print_json_designer_invocation_admission_builder(plan.builder_invocations[index], "      ");
        if ((index + 1U) != plan.builder_invocations.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"toolboxInvocation\": ";
    if (!plan.toolbox_invocation.ok) {
        std::cout << "null\n";
    } else {
        const auto& toolbox_plan = plan.toolbox_invocation.plan;
        std::cout << "{\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(toolbox_plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"itemCount\": " << toolbox_plan.item_count << ",\n";
        std::cout << "      \"paletteInvocationAdmitted\": "
                  << (toolbox_plan.palette_invocation_admitted ? "true" : "false") << ",\n";
        std::cout << "      \"dryRun\": " << (toolbox_plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (toolbox_plan.mutates_asset ? "true" : "false") << "\n";
        std::cout << "    }\n";
    }
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_designer_dispatch_action(
    const copperfin::studio::StudioEditorActionDispatchResult& result,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"dispatchOk\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(result.error);
    if (!result.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }
    const auto& plan = result.plan;
    std::cout << ",\n";
    std::cout << indent << "  \"actionId\": ";
    print_json_string_view(plan.action.id);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_designer_dispatch_builder(
    const copperfin::studio::StudioBuilderDispatchResult& result,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"dispatchOk\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(result.error);
    if (!result.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }
    const auto& plan = result.plan;
    std::cout << ",\n";
    std::cout << indent << "  \"builderId\": ";
    print_json_string_view(plan.builder.id);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_designer_dispatch_result(
    const copperfin::studio::StudioDesignerDispatchResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"designerDispatch\": ";
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
    std::vector<std::string> dispatch_ok_selection_contexts;
    std::vector<std::string> dispatch_blocked_selection_contexts;
    std::vector<std::string> dispatch_blocked_errors;
    if (plan.error_count == 0U) {
        dispatch_ok_selection_contexts.push_back(selection_context);
    } else {
        dispatch_blocked_selection_contexts.push_back(selection_context);
        std::string blocked_error;
        for (const auto& dispatch : plan.editor_action_dispatches) {
            if (!dispatch.ok) {
                blocked_error = dispatch.error;
                break;
            }
        }
        if (blocked_error.empty()) {
            for (const auto& dispatch : plan.builder_dispatches) {
                if (!dispatch.ok) {
                    blocked_error = dispatch.error;
                    break;
                }
            }
        }
        if (blocked_error.empty() && !plan.toolbox_dispatch.ok) {
            blocked_error = plan.toolbox_dispatch.error;
        }
        dispatch_blocked_errors.push_back(blocked_error);
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(selection_context);
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
    std::cout << "    \"dispatchOkSelectionContexts\": ";
    print_json_string_array(dispatch_ok_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedSelectionContexts\": ";
    print_json_string_array(dispatch_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"editorActionDispatchCount\": " << plan.editor_action_dispatch_count << ",\n";
    std::cout << "    \"builderDispatchCount\": " << plan.builder_dispatch_count << ",\n";
    std::cout << "    \"toolboxDispatchCount\": " << plan.toolbox_dispatch_count << ",\n";
    std::cout << "    \"dispatchCount\": " << plan.dispatch_count << ",\n";
    std::cout << "    \"errorCount\": " << plan.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"editorActionDispatches\": [\n";
    for (std::size_t index = 0U; index < plan.editor_action_dispatches.size(); ++index) {
        print_json_designer_dispatch_action(plan.editor_action_dispatches[index], "      ");
        if ((index + 1U) != plan.editor_action_dispatches.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"builderDispatches\": [\n";
    for (std::size_t index = 0U; index < plan.builder_dispatches.size(); ++index) {
        print_json_designer_dispatch_builder(plan.builder_dispatches[index], "      ");
        if ((index + 1U) != plan.builder_dispatches.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"toolboxDispatch\": ";
    if (!plan.toolbox_dispatch.ok) {
        std::cout << "{\n";
        std::cout << "      \"dispatchOk\": false,\n";
        std::cout << "      \"error\": ";
        print_json_string(plan.toolbox_dispatch.error);
        std::cout << "\n";
        std::cout << "    }\n";
    } else {
        const auto& toolbox_plan = plan.toolbox_dispatch.plan;
        std::cout << "{\n";
        std::cout << "      \"dispatchOk\": true,\n";
        std::cout << "      \"error\": \"\",\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(toolbox_plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"itemCount\": " << toolbox_plan.item_count << ",\n";
        std::cout << "      \"dispatchAdmitted\": " << (toolbox_plan.dispatch_admitted ? "true" : "false")
                  << ",\n";
        std::cout << "      \"dryRun\": " << (toolbox_plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"executed\": " << (toolbox_plan.executed ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (toolbox_plan.mutates_asset ? "true" : "false") << "\n";
        std::cout << "    }\n";
    }
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_designer_execution_result(
    const copperfin::studio::StudioDesignerDispatchExecutionResult& result,
    const copperfin::studio::StudioDesignerDispatchPlan* planned_dispatch_plan,
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& editor_action_launch_command,
    const std::string& builder_launch_command,
    const std::string& toolbox_launch_command) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok && result.error_count == 0U ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"designerExecution\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << ",\n";
        std::cout << "  \"executionAdmitted\": " << (result.execution_admitted ? "true" : "false") << ",\n";
        std::cout << "  \"executed\": " << (result.executed ? "true" : "false") << ",\n";
        std::cout << "  \"dryRun\": " << (result.dry_run ? "true" : "false") << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& child_dispatch_plan =
        result.dispatch_plan.dispatch_count == 0U && planned_dispatch_plan != nullptr
            ? *planned_dispatch_plan
            : result.dispatch_plan;
    const std::size_t toolbox_execution_count = child_dispatch_plan.toolbox_dispatch.ok ? 1U : 0U;
    std::size_t editor_action_executed_count = 0U;
    std::size_t editor_action_error_count = 0U;
    std::vector<std::string> failed_editor_action_ids;
    std::vector<std::string> failed_editor_action_command_tokens;
    std::vector<std::string> failed_editor_action_executed_commands;
    std::vector<int> failed_editor_action_exit_codes;
    std::vector<std::string> failed_editor_action_errors;
    for (std::size_t index = 0U; index < result.editor_action_executions.size(); ++index) {
        const auto& execution = result.editor_action_executions[index];
        if (execution.executed) {
            ++editor_action_executed_count;
        }
        if (!execution.ok) {
            ++editor_action_error_count;
            if (index < child_dispatch_plan.editor_action_dispatches.size() &&
                child_dispatch_plan.editor_action_dispatches[index].ok) {
                failed_editor_action_ids.push_back(
                    std::string(child_dispatch_plan.editor_action_dispatches[index].plan.action.id));
                failed_editor_action_command_tokens.push_back(
                    child_dispatch_plan.editor_action_dispatches[index].plan.command_token);
                failed_editor_action_executed_commands.push_back(build_shell_command(
                    editor_action_launch_command,
                    child_dispatch_plan.editor_action_dispatches[index].plan.dispatch_arguments));
            }
            failed_editor_action_exit_codes.push_back(execution.observation.exit_code);
            failed_editor_action_errors.push_back(execution.error);
        }
    }
    std::size_t builder_executed_count = 0U;
    std::size_t builder_error_count = 0U;
    std::vector<std::string> failed_builder_ids;
    std::vector<std::string> failed_builder_command_tokens;
    std::vector<std::string> failed_builder_executed_commands;
    std::vector<int> failed_builder_exit_codes;
    std::vector<std::string> failed_builder_errors;
    for (std::size_t index = 0U; index < result.builder_executions.size(); ++index) {
        const auto& execution = result.builder_executions[index];
        if (execution.executed) {
            ++builder_executed_count;
        }
        if (!execution.ok) {
            ++builder_error_count;
            if (index < child_dispatch_plan.builder_dispatches.size() &&
                child_dispatch_plan.builder_dispatches[index].ok) {
                failed_builder_ids.push_back(
                    std::string(child_dispatch_plan.builder_dispatches[index].plan.builder.id));
                failed_builder_command_tokens.push_back(
                    child_dispatch_plan.builder_dispatches[index].plan.command_token);
                failed_builder_executed_commands.push_back(build_shell_command(
                    builder_launch_command,
                    child_dispatch_plan.builder_dispatches[index].plan.dispatch_arguments));
            }
            failed_builder_exit_codes.push_back(execution.observation.exit_code);
            failed_builder_errors.push_back(execution.error);
        }
    }
    const std::size_t toolbox_executed_count =
        toolbox_execution_count != 0U && result.toolbox_execution.executed ? 1U : 0U;
    const std::size_t toolbox_error_count =
        toolbox_execution_count != 0U && !result.toolbox_execution.ok ? 1U : 0U;
    const std::string toolbox_error = toolbox_error_count != 0U ? result.toolbox_execution.error : std::string{};
    const std::string toolbox_command_token =
        child_dispatch_plan.toolbox_dispatch.ok ? child_dispatch_plan.toolbox_dispatch.plan.command_token : std::string{};
    const std::string toolbox_executed_command =
        child_dispatch_plan.toolbox_dispatch.ok
            ? build_shell_command(toolbox_launch_command, child_dispatch_plan.toolbox_dispatch.plan.dispatch_arguments)
            : std::string{};
    const int toolbox_exit_code = result.toolbox_execution.observation.exit_code;
    const std::string selection_context =
        copperfin::studio::studio_editor_selection_context_name(child_dispatch_plan.selection_context);
    std::vector<std::string> execution_ready_selection_contexts;
    std::vector<std::string> execution_blocked_selection_contexts;
    std::vector<std::string> execution_blocked_errors;
    if (result.error_count == 0U) {
        execution_ready_selection_contexts.push_back(selection_context);
    } else {
        execution_blocked_selection_contexts.push_back(selection_context);
        std::string blocked_error;
        if (!failed_editor_action_errors.empty()) {
            blocked_error = failed_editor_action_errors.front();
        } else if (!failed_builder_errors.empty()) {
            blocked_error = failed_builder_errors.front();
        } else if (!toolbox_error.empty()) {
            blocked_error = toolbox_error;
        } else {
            blocked_error = result.error;
        }
        execution_blocked_errors.push_back(blocked_error);
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": " << (result.error_count == 0U ? "true" : "false") << ",\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"executionAdmitted\": " << (result.execution_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"executed\": " << (result.executed ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"executionCount\": " << result.execution_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"executionReadySelectionContexts\": ";
    print_json_string_array(execution_ready_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedSelectionContexts\": ";
    print_json_string_array(execution_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedErrors\": ";
    print_json_string_array(execution_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"editorActionExecutionCount\": " << result.editor_action_executions.size() << ",\n";
    std::cout << "    \"builderExecutionCount\": " << result.builder_executions.size() << ",\n";
    std::cout << "    \"toolboxExecutionCount\": " << toolbox_execution_count << ",\n";
    std::cout << "    \"editorActionExecutedCount\": " << editor_action_executed_count << ",\n";
    std::cout << "    \"builderExecutedCount\": " << builder_executed_count << ",\n";
    std::cout << "    \"toolboxExecutedCount\": " << toolbox_executed_count << ",\n";
    std::cout << "    \"editorActionErrorCount\": " << editor_action_error_count << ",\n";
    std::cout << "    \"builderErrorCount\": " << builder_error_count << ",\n";
    std::cout << "    \"toolboxErrorCount\": " << toolbox_error_count << ",\n";
    std::cout << "    \"failedEditorActionIds\": ";
    print_json_string_array(failed_editor_action_ids);
    std::cout << ",\n";
    std::cout << "    \"failedEditorActionCommandTokens\": ";
    print_json_string_array(failed_editor_action_command_tokens);
    std::cout << ",\n";
    std::cout << "    \"failedEditorActionExecutedCommands\": ";
    print_json_string_array(failed_editor_action_executed_commands);
    std::cout << ",\n";
    std::cout << "    \"failedEditorActionExitCodes\": ";
    print_json_int_array(failed_editor_action_exit_codes);
    std::cout << ",\n";
    std::cout << "    \"failedEditorActionErrors\": ";
    print_json_string_array(failed_editor_action_errors);
    std::cout << ",\n";
    std::cout << "    \"failedBuilderIds\": ";
    print_json_string_array(failed_builder_ids);
    std::cout << ",\n";
    std::cout << "    \"failedBuilderCommandTokens\": ";
    print_json_string_array(failed_builder_command_tokens);
    std::cout << ",\n";
    std::cout << "    \"failedBuilderExecutedCommands\": ";
    print_json_string_array(failed_builder_executed_commands);
    std::cout << ",\n";
    std::cout << "    \"failedBuilderExitCodes\": ";
    print_json_int_array(failed_builder_exit_codes);
    std::cout << ",\n";
    std::cout << "    \"failedBuilderErrors\": ";
    print_json_string_array(failed_builder_errors);
    std::cout << ",\n";
    std::cout << "    \"toolboxFailed\": " << (toolbox_error_count != 0U ? "true" : "false") << ",\n";
    std::cout << "    \"toolboxCommandToken\": ";
    print_json_string(toolbox_command_token);
    std::cout << ",\n";
    std::cout << "    \"toolboxExecutedCommand\": ";
    print_json_string(toolbox_executed_command);
    std::cout << ",\n";
    std::cout << "    \"toolboxExitCode\": " << toolbox_exit_code << ",\n";
    std::cout << "    \"toolboxError\": ";
    print_json_string(toolbox_error);
    std::cout << ",\n";
    std::cout << "    \"editorActionLaunchCommand\": ";
    print_json_string(editor_action_launch_command);
    std::cout << ",\n";
    std::cout << "    \"builderLaunchCommand\": ";
    print_json_string(builder_launch_command);
    std::cout << ",\n";
    std::cout << "    \"toolboxLaunchCommand\": ";
    print_json_string(toolbox_launch_command);
    if (result.executed) {
        const auto& plan = result.dispatch_plan;
        std::cout << ",\n";
        std::cout << "    \"selectionContext\": ";
        print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
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
        std::cout << "    \"dispatchCount\": " << plan.dispatch_count;
    }
    std::cout << ",\n";
    std::cout << "    \"editorActionExecutions\": [\n";
    for (std::size_t index = 0U; index < result.editor_action_executions.size(); ++index) {
        const auto& execution = result.editor_action_executions[index];
        const copperfin::studio::StudioEditorActionDispatchPlan* plan = nullptr;
        if (index < child_dispatch_plan.editor_action_dispatches.size() &&
            child_dispatch_plan.editor_action_dispatches[index].ok) {
            plan = &child_dispatch_plan.editor_action_dispatches[index].plan;
        }
        std::string executed_command;
        if (plan != nullptr) {
            executed_command = build_shell_command(
                editor_action_launch_command,
                plan->dispatch_arguments);
        }
        std::cout << "      {\n";
        std::cout << "        \"ok\": " << (execution.ok ? "true" : "false") << ",\n";
        std::cout << "        \"error\": ";
        print_json_string(execution.error);
        std::cout << ",\n";
        std::cout << "        \"actionId\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->action.id);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"label\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->action.label);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"kind\": ";
        if (plan != nullptr) {
            print_json_string(copperfin::studio::studio_editor_action_kind_name(plan->action.kind));
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"description\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->action.description);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"commandToken\": ";
        print_json_string(plan != nullptr ? plan->command_token : std::string{});
        std::cout << ",\n";
        std::cout << "        \"selectionContext\": ";
        if (plan != nullptr) {
            print_json_string(copperfin::studio::studio_editor_selection_context_name(plan->selection_context));
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"targetSurface\": ";
        print_json_string(plan != nullptr ? plan->target_surface : std::string{});
        std::cout << ",\n";
        std::cout << "        \"assetPath\": ";
        print_json_string(plan != nullptr ? plan->asset_path : std::string{});
        std::cout << ",\n";
        std::cout << "        \"recordIndex\": " << (plan != nullptr ? plan->record_index : 0U) << ",\n";
        std::cout << "        \"objectName\": ";
        print_json_string(plan != nullptr ? plan->object_name : std::string{});
        std::cout << ",\n";
        std::cout << "        \"uniqueId\": ";
        print_json_string(plan != nullptr ? plan->unique_id : std::string{});
        std::cout << ",\n";
        std::cout << "        \"symbol\": ";
        print_json_string(plan != nullptr ? plan->symbol : std::string{});
        std::cout << ",\n";
        std::cout << "        \"line\": " << (plan != nullptr ? plan->line : 0U) << ",\n";
        std::cout << "        \"column\": " << (plan != nullptr ? plan->column : 0U) << ",\n";
        std::cout << "        \"dispatchArguments\": ";
        if (plan != nullptr) {
            print_json_string_array(plan->dispatch_arguments);
        } else {
            std::cout << "[]";
        }
        std::cout << ",\n";
        std::cout << "        \"launchCommand\": ";
        print_json_string(editor_action_launch_command);
        std::cout << ",\n";
        std::cout << "        \"executedCommand\": ";
        print_json_string(executed_command);
        std::cout << ",\n";
        std::cout << "        \"executionAdmitted\": "
                  << (execution.execution_admitted ? "true" : "false") << ",\n";
        std::cout << "        \"launched\": "
                  << (execution.observation.launched ? "true" : "false") << ",\n";
        std::cout << "        \"observedExitCode\": " << execution.observation.exit_code << ",\n";
        std::cout << "        \"executed\": " << (execution.executed ? "true" : "false") << ",\n";
        std::cout << "        \"dryRun\": " << (execution.dry_run ? "true" : "false") << ",\n";
        std::cout << "        \"mutatesAsset\": " << (execution.mutates_asset ? "true" : "false") << "\n";
        std::cout << "      }";
        if ((index + 1U) != result.editor_action_executions.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"builderExecutions\": [\n";
    for (std::size_t index = 0U; index < result.builder_executions.size(); ++index) {
        const auto& execution = result.builder_executions[index];
        const copperfin::studio::StudioBuilderDispatchPlan* plan = nullptr;
        if (index < child_dispatch_plan.builder_dispatches.size() &&
            child_dispatch_plan.builder_dispatches[index].ok) {
            plan = &child_dispatch_plan.builder_dispatches[index].plan;
        }
        std::string executed_command;
        if (plan != nullptr) {
            executed_command = build_shell_command(
                builder_launch_command,
                plan->dispatch_arguments);
        }
        std::cout << "      {\n";
        std::cout << "        \"ok\": " << (execution.ok ? "true" : "false") << ",\n";
        std::cout << "        \"error\": ";
        print_json_string(execution.error);
        std::cout << ",\n";
        std::cout << "        \"builderId\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->builder.id);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"title\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->builder.title);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"kind\": ";
        if (plan != nullptr) {
            print_json_string(copperfin::studio::studio_builder_kind_name(plan->builder.kind));
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"vfp9Equivalent\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->builder.vfp9_equivalent);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"vfp9EquivalentDisplay\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->builder.vfp9_equivalent_display);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"copperfinComponent\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->builder.copperfin_component);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"description\": ";
        if (plan != nullptr) {
            print_json_string_view(plan->builder.description);
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"commandToken\": ";
        print_json_string(plan != nullptr ? plan->command_token : std::string{});
        std::cout << ",\n";
        std::cout << "        \"context\": ";
        if (plan != nullptr) {
            print_json_string(copperfin::studio::studio_builder_context_name(plan->context));
        } else {
            print_json_string("");
        }
        std::cout << ",\n";
        std::cout << "        \"entryPoint\": ";
        print_json_string(plan != nullptr ? plan->entry_point : std::string{});
        std::cout << ",\n";
        std::cout << "        \"assetPath\": ";
        print_json_string(plan != nullptr ? plan->asset_path : std::string{});
        std::cout << ",\n";
        std::cout << "        \"recordIndex\": " << (plan != nullptr ? plan->record_index : 0U) << ",\n";
        std::cout << "        \"objectName\": ";
        print_json_string(plan != nullptr ? plan->object_name : std::string{});
        std::cout << ",\n";
        std::cout << "        \"uniqueId\": ";
        print_json_string(plan != nullptr ? plan->unique_id : std::string{});
        std::cout << ",\n";
        std::cout << "        \"dispatchArguments\": ";
        if (plan != nullptr) {
            print_json_string_array(plan->dispatch_arguments);
        } else {
            std::cout << "[]";
        }
        std::cout << ",\n";
        std::cout << "        \"launchCommand\": ";
        print_json_string(builder_launch_command);
        std::cout << ",\n";
        std::cout << "        \"executedCommand\": ";
        print_json_string(executed_command);
        std::cout << ",\n";
        std::cout << "        \"executionAdmitted\": "
                  << (execution.execution_admitted ? "true" : "false") << ",\n";
        std::cout << "        \"launched\": "
                  << (execution.observation.launched ? "true" : "false") << ",\n";
        std::cout << "        \"observedExitCode\": " << execution.observation.exit_code << ",\n";
        std::cout << "        \"executed\": " << (execution.executed ? "true" : "false") << ",\n";
        std::cout << "        \"dryRun\": " << (execution.dry_run ? "true" : "false") << ",\n";
        std::cout << "        \"mutatesAsset\": " << (execution.mutates_asset ? "true" : "false") << "\n";
        std::cout << "      }";
        if ((index + 1U) != result.builder_executions.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"toolboxExecution\": {\n";
    std::cout << "      \"ok\": " << (result.toolbox_execution.ok ? "true" : "false") << ",\n";
    std::cout << "      \"error\": ";
    print_json_string(result.toolbox_execution.error);
    std::cout << ",\n";
    std::cout << "      \"selectionContext\": ";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        print_json_string(copperfin::studio::studio_editor_selection_context_name(
            child_dispatch_plan.toolbox_dispatch.plan.selection_context));
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << "      \"toolboxContext\": ";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        print_json_string(copperfin::studio::studio_toolbox_context_name(
            child_dispatch_plan.toolbox_dispatch.plan.toolbox_context));
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << "      \"commandToken\": ";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        print_json_string(child_dispatch_plan.toolbox_dispatch.plan.command_token);
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << "      \"assetPath\": ";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        print_json_string(child_dispatch_plan.toolbox_dispatch.plan.asset_path);
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << "      \"recordIndex\": "
              << (child_dispatch_plan.toolbox_dispatch.ok
                      ? child_dispatch_plan.toolbox_dispatch.plan.record_index
                      : 0U)
              << ",\n";
    std::cout << "      \"objectName\": ";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        print_json_string(child_dispatch_plan.toolbox_dispatch.plan.object_name);
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << "      \"uniqueId\": ";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        print_json_string(child_dispatch_plan.toolbox_dispatch.plan.unique_id);
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << "      \"itemCount\": "
              << (child_dispatch_plan.toolbox_dispatch.ok
                      ? child_dispatch_plan.toolbox_dispatch.plan.item_count
                      : 0U)
              << ",\n";
    std::cout << "      \"items\": [\n";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        const auto& items = child_dispatch_plan.toolbox_dispatch.plan.items;
        for (std::size_t index = 0U; index < items.size(); ++index) {
            print_json_toolbox_item_descriptor(items[index], "        ");
            if ((index + 1U) != items.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
    }
    std::cout << "      ],\n";
    std::cout << "      \"dispatchArguments\": ";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        print_json_string_array(child_dispatch_plan.toolbox_dispatch.plan.dispatch_arguments);
    } else {
        std::cout << "[]";
    }
    std::cout << ",\n";
    std::cout << "      \"launchCommand\": ";
    print_json_string(toolbox_launch_command);
    std::cout << ",\n";
    std::cout << "      \"executedCommand\": ";
    if (child_dispatch_plan.toolbox_dispatch.ok) {
        print_json_string(build_shell_command(
            toolbox_launch_command,
            child_dispatch_plan.toolbox_dispatch.plan.dispatch_arguments));
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << "      \"executionAdmitted\": "
              << (result.toolbox_execution.execution_admitted ? "true" : "false") << ",\n";
    std::cout << "      \"launched\": "
              << (result.toolbox_execution.observation.launched ? "true" : "false") << ",\n";
    std::cout << "      \"observedExitCode\": " << result.toolbox_execution.observation.exit_code << ",\n";
    std::cout << "      \"executed\": " << (result.toolbox_execution.executed ? "true" : "false") << ",\n";
    std::cout << "      \"dryRun\": " << (result.toolbox_execution.dry_run ? "true" : "false") << ",\n";
    std::cout << "      \"mutatesAsset\": " << (result.toolbox_execution.mutates_asset ? "true" : "false") << "\n";
    std::cout << "    }\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": ";
    print_json_string(
        result.error_count == 0U
            ? ""
            : catalog.translate("StudioHost.DesignerExecution.Error.ChildExecutionsFailed"));
    std::cout << "\n";
    std::cout << "}\n";
}

void print_json_designer_dispatch_catalog_context(
    const copperfin::studio::StudioDesignerDispatchCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
    std::cout << ",\n";
    std::cout << indent << "  \"editorActionDispatchCount\": " << entry.editor_action_dispatch_count << ",\n";
    std::cout << indent << "  \"builderDispatchCount\": " << entry.builder_dispatch_count << ",\n";
    std::cout << indent << "  \"toolboxDispatchCount\": " << entry.toolbox_dispatch_count << ",\n";
    std::cout << indent << "  \"dispatchCount\": " << entry.dispatch_count << ",\n";
    std::cout << indent << "  \"errorCount\": " << entry.error_count << ",\n";
    std::cout << indent << "  \"dryRun\": " << (entry.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (entry.mutates_asset ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dispatchOk\": " << (entry.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"editorActionIds\": [";
    if (entry.dispatch.ok) {
        const auto& dispatches = entry.dispatch.plan.editor_action_dispatches;
        bool first = true;
        for (const auto& dispatch : dispatches) {
            if (!dispatch.ok) {
                continue;
            }
            if (!first) {
                std::cout << ", ";
            }
            first = false;
            print_json_string_view(dispatch.plan.action.id);
        }
    }
    std::cout << "],\n";
    std::cout << indent << "  \"editorActionDispatches\": [\n";
    if (entry.dispatch.ok) {
        const auto& dispatches = entry.dispatch.plan.editor_action_dispatches;
        for (std::size_t index = 0U; index < dispatches.size(); ++index) {
            print_json_designer_execution_catalog_editor_dispatch(dispatches[index], indent + "    ");
            if ((index + 1U) != dispatches.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
    }
    std::cout << indent << "  ],\n";
    std::cout << indent << "  \"builderIds\": [";
    if (entry.dispatch.ok) {
        const auto& dispatches = entry.dispatch.plan.builder_dispatches;
        bool first = true;
        for (const auto& dispatch : dispatches) {
            if (!dispatch.ok) {
                continue;
            }
            if (!first) {
                std::cout << ", ";
            }
            first = false;
            print_json_string_view(dispatch.plan.builder.id);
        }
    }
    std::cout << "],\n";
    std::cout << indent << "  \"builderDispatches\": [\n";
    if (entry.dispatch.ok) {
        const auto& dispatches = entry.dispatch.plan.builder_dispatches;
        for (std::size_t index = 0U; index < dispatches.size(); ++index) {
            print_json_designer_execution_catalog_builder_dispatch(dispatches[index], indent + "    ");
            if ((index + 1U) != dispatches.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
    }
    std::cout << indent << "  ],\n";
    std::cout << indent << "  \"toolboxCommandToken\": ";
    if (entry.dispatch.ok && entry.dispatch.plan.toolbox_dispatch.ok) {
        print_json_string(entry.dispatch.plan.toolbox_dispatch.plan.command_token);
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << indent << "  \"toolboxDispatchArguments\": ";
    if (entry.dispatch.ok && entry.dispatch.plan.toolbox_dispatch.ok) {
        print_json_string_array(entry.dispatch.plan.toolbox_dispatch.plan.dispatch_arguments);
    } else {
        std::cout << "[]";
    }
    std::cout << ",\n";
    std::cout << indent << "  \"toolboxDispatchOk\": "
              << (entry.dispatch.ok && entry.dispatch.plan.toolbox_dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"toolboxError\": ";
    if (entry.dispatch.ok && !entry.dispatch.plan.toolbox_dispatch.ok) {
        print_json_string(entry.dispatch.plan.toolbox_dispatch.error);
    } else {
        print_json_string("");
    }
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_designer_execution_catalog_editor_dispatch(
    const copperfin::studio::StudioEditorActionDispatchResult& dispatch,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"dispatchOk\": " << (dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(dispatch.error);
    if (!dispatch.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }

    const auto& plan = dispatch.plan;
    std::cout << ",\n";
    std::cout << indent << "  \"actionId\": ";
    print_json_string_view(plan.action.id);
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"targetSurface\": ";
    print_json_string(plan.target_surface);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchArguments\": ";
    print_json_string_array(plan.dispatch_arguments);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false")
              << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_designer_execution_catalog_builder_dispatch(
    const copperfin::studio::StudioBuilderDispatchResult& dispatch,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"dispatchOk\": " << (dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"error\": ";
    print_json_string(dispatch.error);
    if (!dispatch.ok) {
        std::cout << "\n";
        std::cout << indent << "}";
        return;
    }

    const auto& plan = dispatch.plan;
    std::cout << ",\n";
    std::cout << indent << "  \"builderId\": ";
    print_json_string_view(plan.builder.id);
    std::cout << ",\n";
    std::cout << indent << "  \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << indent << "  \"entryPoint\": ";
    print_json_string(plan.entry_point);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchArguments\": ";
    print_json_string_array(plan.dispatch_arguments);
    std::cout << ",\n";
    std::cout << indent << "  \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false")
              << ",\n";
    std::cout << indent << "  \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_designer_dispatch_catalog_result(
    const copperfin::studio::StudioDesignerDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"designerDispatchCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> dispatch_ok_selection_contexts;
    std::vector<std::string> dispatch_blocked_selection_contexts;
    std::vector<std::string> dispatch_blocked_errors;
    for (const auto& context : result.contexts) {
        const std::string selection_context =
            copperfin::studio::studio_editor_selection_context_name(context.selection_context);
        if (context.error_count == 0U) {
            dispatch_ok_selection_contexts.push_back(selection_context);
        } else {
            dispatch_blocked_selection_contexts.push_back(selection_context);
            std::string blocked_error;
            if (!context.dispatch.ok) {
                blocked_error = context.dispatch.error;
            } else {
                for (const auto& dispatch : context.dispatch.plan.editor_action_dispatches) {
                    if (!dispatch.ok) {
                        blocked_error = dispatch.error;
                        break;
                    }
                }
                if (blocked_error.empty()) {
                    for (const auto& dispatch : context.dispatch.plan.builder_dispatches) {
                        if (!dispatch.ok) {
                            blocked_error = dispatch.error;
                            break;
                        }
                    }
                }
                if (blocked_error.empty() && !context.dispatch.plan.toolbox_dispatch.ok) {
                    blocked_error = context.dispatch.plan.toolbox_dispatch.error;
                }
            }
            dispatch_blocked_errors.push_back(blocked_error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"contextCount\": " << result.context_count << ",\n";
    std::cout << "    \"dispatchOkSelectionContexts\": ";
    print_json_string_array(dispatch_ok_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedSelectionContexts\": ";
    print_json_string_array(dispatch_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"contexts\": [\n";
    for (std::size_t index = 0U; index < result.contexts.size(); ++index) {
        print_json_designer_dispatch_catalog_context(result.contexts[index], "      ");
        if ((index + 1U) != result.contexts.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_designer_dispatch_execution_catalog_context(
    const copperfin::studio::StudioDesignerDispatchExecutionCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
    std::cout << ",\n";
    std::cout << indent << "  \"editorActionDispatchCount\": " << entry.editor_action_dispatch_count << ",\n";
    std::cout << indent << "  \"builderDispatchCount\": " << entry.builder_dispatch_count << ",\n";
    std::cout << indent << "  \"toolboxDispatchCount\": " << entry.toolbox_dispatch_count << ",\n";
    std::cout << indent << "  \"dispatchCount\": " << entry.dispatch_count << ",\n";
    std::cout << indent << "  \"dispatchErrorCount\": " << entry.dispatch_error_count << ",\n";
    std::cout << indent << "  \"dispatchDryRun\": " << (entry.dispatch_dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dispatchMutatesAsset\": "
              << (entry.dispatch_mutates_asset ? "true" : "false") << ",\n";
    std::cout << indent << "  \"dispatchOk\": " << (entry.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executionAdmitted\": " << (entry.execution_admitted ? "true" : "false")
              << ",\n";
    std::cout << indent << "  \"executionReady\": " << (entry.execution_ready ? "true" : "false")
              << ",\n";
    std::cout << indent << "  \"executionError\": ";
    print_json_string(entry.execution_error);
    std::cout << ",\n";
    std::cout << indent << "  \"editorActionIds\": [";
    if (entry.dispatch.ok) {
        const auto& dispatches = entry.dispatch.plan.editor_action_dispatches;
        bool first = true;
        for (const auto& dispatch : dispatches) {
            if (!dispatch.ok) {
                continue;
            }
            if (!first) {
                std::cout << ", ";
            }
            first = false;
            print_json_string_view(dispatch.plan.action.id);
        }
    }
    std::cout << "],\n";
    std::cout << indent << "  \"editorActionDispatches\": [\n";
    if (entry.dispatch.ok) {
        const auto& dispatches = entry.dispatch.plan.editor_action_dispatches;
        for (std::size_t index = 0U; index < dispatches.size(); ++index) {
            print_json_designer_execution_catalog_editor_dispatch(dispatches[index], indent + "    ");
            if ((index + 1U) != dispatches.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
    }
    std::cout << indent << "  ],\n";
    std::cout << indent << "  \"builderIds\": [";
    if (entry.dispatch.ok) {
        const auto& dispatches = entry.dispatch.plan.builder_dispatches;
        bool first = true;
        for (const auto& dispatch : dispatches) {
            if (!dispatch.ok) {
                continue;
            }
            if (!first) {
                std::cout << ", ";
            }
            first = false;
            print_json_string_view(dispatch.plan.builder.id);
        }
    }
    std::cout << "],\n";
    std::cout << indent << "  \"builderDispatches\": [\n";
    if (entry.dispatch.ok) {
        const auto& dispatches = entry.dispatch.plan.builder_dispatches;
        for (std::size_t index = 0U; index < dispatches.size(); ++index) {
            print_json_designer_execution_catalog_builder_dispatch(dispatches[index], indent + "    ");
            if ((index + 1U) != dispatches.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
    }
    std::cout << indent << "  ],\n";
    std::cout << indent << "  \"toolboxCommandToken\": ";
    if (entry.dispatch.ok && entry.dispatch.plan.toolbox_dispatch.ok) {
        print_json_string(entry.dispatch.plan.toolbox_dispatch.plan.command_token);
    } else {
        print_json_string("");
    }
    std::cout << ",\n";
    std::cout << indent << "  \"toolboxDispatchArguments\": ";
    if (entry.dispatch.ok && entry.dispatch.plan.toolbox_dispatch.ok) {
        print_json_string_array(entry.dispatch.plan.toolbox_dispatch.plan.dispatch_arguments);
    } else {
        std::cout << "[]";
    }
    std::cout << ",\n";
    std::cout << indent << "  \"toolboxDispatchOk\": "
              << (entry.dispatch.ok && entry.dispatch.plan.toolbox_dispatch.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"toolboxError\": ";
    if (entry.dispatch.ok && !entry.dispatch.plan.toolbox_dispatch.ok) {
        print_json_string(entry.dispatch.plan.toolbox_dispatch.error);
    } else {
        print_json_string("");
    }
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_designer_dispatch_execution_catalog_result(
    const copperfin::studio::StudioDesignerDispatchExecutionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"designerDispatchExecutionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> ready_selection_contexts;
    std::vector<std::string> blocked_selection_contexts;
    std::vector<std::string> blocked_execution_errors;
    for (const auto& context : result.contexts) {
        const std::string selection_context =
            copperfin::studio::studio_editor_selection_context_name(context.selection_context);
        if (context.execution_ready) {
            ready_selection_contexts.push_back(selection_context);
        } else {
            blocked_selection_contexts.push_back(selection_context);
            blocked_execution_errors.push_back(context.execution_error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"contextCount\": " << result.context_count << ",\n";
    std::cout << "    \"executionReadyCount\": " << result.execution_ready_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"readySelectionContexts\": ";
    print_json_string_array(ready_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"blockedSelectionContexts\": ";
    print_json_string_array(blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"blockedExecutionErrors\": ";
    print_json_string_array(blocked_execution_errors);
    std::cout << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"contexts\": [\n";
    for (std::size_t index = 0U; index < result.contexts.size(); ++index) {
        print_json_designer_dispatch_execution_catalog_context(result.contexts[index], "      ");
        if ((index + 1U) != result.contexts.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_designer_invocation_admission_catalog_context(
    const copperfin::studio::StudioDesignerInvocationAdmissionCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
    std::cout << ",\n";
    std::cout << indent << "  \"editorActionInvocationCount\": " << entry.editor_action_invocation_count << ",\n";
    std::cout << indent << "  \"builderInvocationCount\": " << entry.builder_invocation_count << ",\n";
    std::cout << indent << "  \"toolboxAvailable\": " << (entry.toolbox_available ? "true" : "false") << ",\n";
    std::cout << indent << "  \"toolboxItemCount\": " << entry.toolbox_item_count << ",\n";
    std::cout << indent << "  \"toolboxError\": ";
    print_json_string(entry.toolbox_error);
    std::cout << ",\n";
    std::cout << indent << "  \"dryRun\": " << (entry.dry_run ? "true" : "false") << ",\n";
    std::cout << indent << "  \"mutatesAsset\": " << (entry.mutates_asset ? "true" : "false") << ",\n";
    std::cout << indent << "  \"editorActionIds\": [";
    if (entry.invocation_admission.ok) {
        const auto& actions = entry.invocation_admission.plan.editor_action_invocations;
        for (std::size_t index = 0U; index < actions.size(); ++index) {
            if (index != 0U) {
                std::cout << ", ";
            }
            if (actions[index].ok) {
                print_json_string_view(actions[index].plan.action.id);
            } else {
                std::cout << "null";
            }
        }
    }
    std::cout << "],\n";
    std::cout << indent << "  \"builderIds\": [";
    if (entry.invocation_admission.ok) {
        const auto& builders = entry.invocation_admission.plan.builder_invocations;
        for (std::size_t index = 0U; index < builders.size(); ++index) {
            if (index != 0U) {
                std::cout << ", ";
            }
            if (builders[index].ok) {
                print_json_string_view(builders[index].plan.builder.id);
            } else {
                std::cout << "null";
            }
        }
    }
    std::cout << "],\n";
    std::cout << indent << "  \"editorInvocationsAdmitted\": ";
    if (entry.invocation_admission.ok && !entry.invocation_admission.plan.editor_action_invocations.empty()) {
        std::cout << (entry.invocation_admission.plan.editor_action_invocations.front().plan.editor_invocation_admitted
                         ? "true"
                         : "false");
    } else {
        std::cout << "false";
    }
    std::cout << ",\n";
    std::cout << indent << "  \"builderInvocationsAdmitted\": ";
    if (entry.invocation_admission.ok && !entry.invocation_admission.plan.builder_invocations.empty()) {
        std::cout << (entry.invocation_admission.plan.builder_invocations.front().plan.ui_launch_admitted
                         ? "true"
                         : "false");
    } else {
        std::cout << "false";
    }
    std::cout << ",\n";
    std::cout << indent << "  \"toolboxInvocationAdmitted\": ";
    if (entry.invocation_admission.ok && entry.invocation_admission.plan.toolbox_invocation.ok) {
        std::cout << (entry.invocation_admission.plan.toolbox_invocation.plan.palette_invocation_admitted
                         ? "true"
                         : "false");
    } else {
        std::cout << "false";
    }
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_designer_invocation_admission_catalog_result(
    const copperfin::studio::StudioDesignerInvocationAdmissionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"designerInvocationAdmissionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> admission_ok_selection_contexts;
    std::vector<std::string> admission_blocked_selection_contexts;
    std::vector<std::string> admission_blocked_errors;
    for (const auto& context : result.contexts) {
        const std::string selection_context =
            copperfin::studio::studio_editor_selection_context_name(context.selection_context);
        if (context.invocation_admission.ok && context.toolbox_error.empty()) {
            admission_ok_selection_contexts.push_back(selection_context);
        } else {
            admission_blocked_selection_contexts.push_back(selection_context);
            if (!context.invocation_admission.ok) {
                admission_blocked_errors.push_back(context.invocation_admission.error);
            } else {
                admission_blocked_errors.push_back(context.toolbox_error);
            }
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"contextCount\": " << result.context_count << ",\n";
    std::cout << "    \"admissionOkSelectionContexts\": ";
    print_json_string_array(admission_ok_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedSelectionContexts\": ";
    print_json_string_array(admission_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"contexts\": [\n";
    for (std::size_t index = 0U; index < result.contexts.size(); ++index) {
        print_json_designer_invocation_admission_catalog_context(result.contexts[index], "      ");
        if ((index + 1U) != result.contexts.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_designer_launch_surface_catalog_context(
    const copperfin::studio::StudioDesignerLaunchSurfaceCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
    std::cout << ",\n";
    std::cout << indent << "  \"editorActionLaunchPlanCount\": " << entry.editor_action_launch_plan_count << ",\n";
    std::cout << indent << "  \"builderLaunchPlanCount\": " << entry.builder_launch_plan_count << ",\n";
    std::cout << indent << "  \"toolboxAvailable\": " << (entry.toolbox_available ? "true" : "false") << ",\n";
    std::cout << indent << "  \"toolboxItemCount\": " << entry.toolbox_item_count << ",\n";
    std::cout << indent << "  \"toolboxError\": ";
    print_json_string(entry.toolbox_error);
    std::cout << ",\n";
    std::cout << indent << "  \"launchSurfacePlan\": ";
    if (!entry.launch_surface_plan.ok) {
        std::cout << "null\n";
        std::cout << indent << "}";
        return;
    }

    const auto& plan = entry.launch_surface_plan.plan;
    std::cout << "{\n";
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
    std::cout << indent << "    \"symbol\": ";
    print_json_string(plan.symbol);
    std::cout << ",\n";
    std::cout << indent << "    \"line\": " << plan.line << ",\n";
    std::cout << indent << "    \"column\": " << plan.column << ",\n";
    std::cout << indent << "    \"editorActionIds\": [";
    for (std::size_t index = 0U; index < plan.editor_action_launch_plans.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        if (plan.editor_action_launch_plans[index].ok) {
            print_json_string_view(plan.editor_action_launch_plans[index].plan.action.id);
        } else {
            std::cout << "null";
        }
    }
    std::cout << "],\n";
    std::cout << indent << "    \"builderIds\": [";
    for (std::size_t index = 0U; index < plan.builder_launch_plans.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        if (plan.builder_launch_plans[index].ok) {
            print_json_string_view(plan.builder_launch_plans[index].plan.builder.id);
        } else {
            std::cout << "null";
        }
    }
    std::cout << "],\n";
    std::cout << indent << "    \"toolboxContext\": ";
    if (plan.toolbox_palette_launch_plan.ok) {
        print_json_string(
            copperfin::studio::studio_toolbox_context_name(plan.toolbox_palette_launch_plan.plan.toolbox_context));
    } else {
        std::cout << "null";
    }
    std::cout << "\n";
    std::cout << indent << "  }\n";
    std::cout << indent << "}";
}

void print_json_designer_launch_surface_catalog_result(
    const copperfin::studio::StudioDesignerLaunchSurfaceCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"designerLaunchSurfaceCatalog\": ";
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
    for (const auto& context : result.contexts) {
        const std::string selection_context =
            copperfin::studio::studio_editor_selection_context_name(context.selection_context);
        if (context.launch_surface_plan.ok && context.toolbox_error.empty()) {
            launch_ready_selection_contexts.push_back(selection_context);
        } else {
            launch_blocked_selection_contexts.push_back(selection_context);
            if (!context.launch_surface_plan.ok) {
                launch_blocked_errors.push_back(context.launch_surface_plan.error);
            } else {
                launch_blocked_errors.push_back(context.toolbox_error);
            }
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"contextCount\": " << result.context_count << ",\n";
    std::cout << "    \"launchReadySelectionContexts\": ";
    print_json_string_array(launch_ready_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedSelectionContexts\": ";
    print_json_string_array(launch_blocked_selection_contexts);
    std::cout << ",\n";
    std::cout << "    \"launchBlockedErrors\": ";
    print_json_string_array(launch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"contexts\": [\n";
    for (std::size_t index = 0U; index < result.contexts.size(); ++index) {
        print_json_designer_launch_surface_catalog_context(result.contexts[index], "      ");
        if ((index + 1U) != result.contexts.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_text_designer_launch_surfaces_result(
    const copperfin::studio::StudioDesignerLaunchSurfacePlanResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "symbol: " << plan.symbol << "\n";
    std::cout << "line: " << plan.line << "\n";
    std::cout << "column: " << plan.column << "\n";
    std::cout << "editor_action_launch_plan_count: " << plan.editor_action_launch_plan_count << "\n";
    std::cout << "builder_launch_plan_count: " << plan.builder_launch_plan_count << "\n";
    std::cout << "toolbox_available: " << (plan.toolbox_available ? "true" : "false") << "\n";
    std::cout << "toolbox_item_count: " << plan.toolbox_item_count << "\n";
    std::cout << "toolbox_error: " << plan.toolbox_error << "\n";
}

void print_text_designer_invocation_admission_result(
    const copperfin::studio::StudioDesignerInvocationAdmissionResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "symbol: " << plan.symbol << "\n";
    std::cout << "line: " << plan.line << "\n";
    std::cout << "column: " << plan.column << "\n";
    std::cout << "editor_action_invocation_count: " << plan.editor_action_invocation_count << "\n";
    std::cout << "builder_invocation_count: " << plan.builder_invocation_count << "\n";
    std::cout << "toolbox_available: " << (plan.toolbox_available ? "true" : "false") << "\n";
    std::cout << "toolbox_item_count: " << plan.toolbox_item_count << "\n";
    std::cout << "toolbox_error: " << plan.toolbox_error << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_designer_dispatch_result(
    const copperfin::studio::StudioDesignerDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "symbol: " << plan.symbol << "\n";
    std::cout << "line: " << plan.line << "\n";
    std::cout << "column: " << plan.column << "\n";
    std::cout << "editor_action_dispatch_count: " << plan.editor_action_dispatch_count << "\n";
    std::cout << "builder_dispatch_count: " << plan.builder_dispatch_count << "\n";
    std::cout << "toolbox_dispatch_count: " << plan.toolbox_dispatch_count << "\n";
    std::cout << "dispatch_count: " << plan.dispatch_count << "\n";
    std::cout << "error_count: " << plan.error_count << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
    for (const auto& dispatch : plan.editor_action_dispatches) {
        std::cout << "editor_action_dispatch_ok: " << (dispatch.ok ? "true" : "false") << "\n";
        if (dispatch.ok) {
            std::cout << "editor_action_id: " << dispatch.plan.action.id << "\n";
        } else if (!dispatch.error.empty()) {
            std::cout << "editor_action_error: " << dispatch.error << "\n";
        }
    }
    for (const auto& dispatch : plan.builder_dispatches) {
        std::cout << "builder_dispatch_ok: " << (dispatch.ok ? "true" : "false") << "\n";
        if (dispatch.ok) {
            std::cout << "builder_id: " << dispatch.plan.builder.id << "\n";
        } else if (!dispatch.error.empty()) {
            std::cout << "builder_error: " << dispatch.error << "\n";
        }
    }
    std::cout << "toolbox_dispatch_ok: " << (plan.toolbox_dispatch.ok ? "true" : "false") << "\n";
    if (plan.toolbox_dispatch.ok) {
        std::cout << "toolbox_context: "
                  << copperfin::studio::studio_toolbox_context_name(plan.toolbox_dispatch.plan.toolbox_context)
                  << "\n";
    } else if (!plan.toolbox_dispatch.error.empty()) {
        std::cout << "toolbox_error: " << plan.toolbox_dispatch.error << "\n";
    }
}

void print_text_designer_execution_result(
    const copperfin::studio::StudioDesignerDispatchExecutionResult& result) {
    std::cout << "status: " << (result.ok && result.error_count == 0U ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "execution_admitted: " << (result.execution_admitted ? "true" : "false") << "\n";
    std::cout << "executed: " << (result.executed ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "execution_count: " << result.execution_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    if (!result.ok) {
        return;
    }
    if (result.executed) {
        const auto& plan = result.dispatch_plan;
        std::cout << "selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
        std::cout << "dispatch_count: " << plan.dispatch_count << "\n";
    }
}

void print_text_designer_dispatch_catalog_result(
    const copperfin::studio::StudioDesignerDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "context_count: " << result.context_count << "\n";
    for (const auto& entry : result.contexts) {
        std::cout << "context: " << copperfin::studio::studio_editor_selection_context_name(entry.selection_context)
                  << " editor_dispatches=" << entry.editor_action_dispatch_count
                  << " builder_dispatches=" << entry.builder_dispatch_count
                  << " toolbox_dispatches=" << entry.toolbox_dispatch_count
                  << " dispatches=" << entry.dispatch_count
                  << " errors=" << entry.error_count
                  << " dry_run=" << (entry.dry_run ? "true" : "false")
                  << " mutates=" << (entry.mutates_asset ? "true" : "false") << "\n";
    }
}

void print_text_designer_dispatch_execution_catalog_result(
    const copperfin::studio::StudioDesignerDispatchExecutionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "context_count: " << result.context_count << "\n";
    std::cout << "execution_ready_count: " << result.execution_ready_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.contexts) {
        std::cout << "context: " << copperfin::studio::studio_editor_selection_context_name(entry.selection_context)
                  << " dispatches=" << entry.dispatch_count
                  << " dispatch_errors=" << entry.dispatch_error_count
                  << " execution_admitted=" << (entry.execution_admitted ? "true" : "false")
                  << " execution_ready=" << (entry.execution_ready ? "true" : "false") << "\n";
        if (!entry.execution_error.empty()) {
            std::cout << "execution_error: " << entry.execution_error << "\n";
        }
    }
}

void print_text_designer_launch_surface_catalog_result(
    const copperfin::studio::StudioDesignerLaunchSurfaceCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "context_count: " << result.context_count << "\n";
    for (const auto& entry : result.contexts) {
        std::cout << "context: " << copperfin::studio::studio_editor_selection_context_name(entry.selection_context)
                  << " actions=" << entry.editor_action_launch_plan_count
                  << " builders=" << entry.builder_launch_plan_count
                  << " toolbox=" << (entry.toolbox_available ? "true" : "false")
                  << " toolbox_items=" << entry.toolbox_item_count << "\n";
        if (!entry.toolbox_error.empty()) {
            std::cout << "toolbox_error: " << entry.toolbox_error << "\n";
        }
    }
}

void print_text_designer_invocation_admission_catalog_result(
    const copperfin::studio::StudioDesignerInvocationAdmissionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "context_count: " << result.context_count << "\n";
    for (const auto& entry : result.contexts) {
        std::cout << "context: " << copperfin::studio::studio_editor_selection_context_name(entry.selection_context)
                  << " actions=" << entry.editor_action_invocation_count
                  << " builders=" << entry.builder_invocation_count
                  << " toolbox=" << (entry.toolbox_available ? "true" : "false")
                  << " toolbox_items=" << entry.toolbox_item_count
                  << " dry_run=" << (entry.dry_run ? "true" : "false")
                  << " mutates_asset=" << (entry.mutates_asset ? "true" : "false") << "\n";
        if (!entry.toolbox_error.empty()) {
            std::cout << "toolbox_error: " << entry.toolbox_error << "\n";
        }
    }
}

void print_json_designer_contexts(const std::vector<copperfin::studio::StudioDesignerContextResult>& contexts) {
    std::cout << "[\n";
    for (std::size_t context_index = 0; context_index < contexts.size(); ++context_index) {
        const auto& context = contexts[context_index];
        std::cout << "      {\n";
        std::cout << "        \"selectionContext\": ";
        print_json_string(copperfin::studio::studio_editor_selection_context_name(context.selection_context));
        std::cout << ",\n";
        std::cout << "        \"editorActionCount\": " << context.editor_action_count << ",\n";
        std::cout << "        \"builderCount\": " << context.builder_count << ",\n";
        std::cout << "        \"toolboxItemCount\": " << context.toolbox_item_count << ",\n";
        std::cout << "        \"editorActions\": [\n";
        for (std::size_t action_index = 0; action_index < context.editor_actions.size(); ++action_index) {
            const auto& action = context.editor_actions[action_index];
            std::cout << "          {\n";
            std::cout << "            \"id\": ";
            print_json_string_view(action.id);
            std::cout << ",\n";
            std::cout << "            \"label\": ";
            print_json_string_view(action.label);
            std::cout << ",\n";
            std::cout << "            \"kind\": ";
            print_json_string(copperfin::studio::studio_editor_action_kind_name(action.kind));
            std::cout << ",\n";
            std::cout << "            \"contexts\": ";
            print_json_editor_contexts(action.contexts);
            std::cout << ",\n";
            std::cout << "            \"commandToken\": ";
            print_json_string_view(action.command_token);
            std::cout << ",\n";
            std::cout << "            \"targetSurface\": ";
            print_json_string_view(action.target_surface);
            std::cout << ",\n";
            std::cout << "            \"description\": ";
            print_json_string_view(action.description);
            std::cout << "\n";
            std::cout << "          }";
            if ((action_index + 1U) != context.editor_actions.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "        ],\n";
        std::cout << "        \"builders\": [\n";
        for (std::size_t builder_index = 0; builder_index < context.builders.size(); ++builder_index) {
            const auto& builder = context.builders[builder_index];
            std::cout << "          {\n";
            std::cout << "            \"id\": ";
            print_json_string_view(builder.id);
            std::cout << ",\n";
            std::cout << "            \"title\": ";
            print_json_string_view(builder.title);
            std::cout << ",\n";
            std::cout << "            \"kind\": ";
            print_json_string(copperfin::studio::studio_builder_kind_name(builder.kind));
            std::cout << ",\n";
            std::cout << "            \"context\": ";
            print_json_string(copperfin::studio::studio_builder_context_name(builder.context));
            std::cout << ",\n";
            std::cout << "            \"vfp9Equivalent\": ";
            print_json_string_view(builder.vfp9_equivalent);
            std::cout << ",\n";
            std::cout << "            \"vfp9EquivalentDisplay\": ";
            print_json_string_view(builder.vfp9_equivalent_display);
            std::cout << ",\n";
            std::cout << "            \"copperfinComponent\": ";
            print_json_string_view(builder.copperfin_component);
            std::cout << ",\n";
            std::cout << "            \"entryPoint\": ";
            print_json_string_view(builder.entry_point);
            std::cout << ",\n";
            std::cout << "            \"description\": ";
            print_json_string_view(builder.description);
            std::cout << "\n";
            std::cout << "          }";
            if ((builder_index + 1U) != context.builders.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "        ],\n";
        std::cout << "        \"toolboxItems\": [\n";
        for (std::size_t toolbox_index = 0; toolbox_index < context.toolbox_items.size(); ++toolbox_index) {
            const auto& toolbox_item = context.toolbox_items[toolbox_index];
            std::cout << "          {\n";
            std::cout << "            \"id\": ";
            print_json_string_view(toolbox_item.id);
            std::cout << ",\n";
            std::cout << "            \"title\": ";
            print_json_string_view(toolbox_item.title);
            std::cout << ",\n";
            std::cout << "            \"category\": ";
            print_json_string_view(toolbox_item.category);
            std::cout << ",\n";
            std::cout << "            \"vfpClass\": ";
            print_json_string_view(toolbox_item.vfp_class);
            std::cout << ",\n";
            std::cout << "            \"baseClass\": ";
            print_json_string_view(toolbox_item.base_class);
            std::cout << ",\n";
            std::cout << "            \"defaultNamePrefix\": ";
            print_json_string_view(toolbox_item.default_name_prefix);
            std::cout << ",\n";
            std::cout << "            \"contexts\": ";
            print_json_toolbox_contexts(toolbox_item.contexts);
            std::cout << ",\n";
            std::cout << "            \"container\": " << (toolbox_item.container ? "true" : "false") << ",\n";
            std::cout << "            \"description\": ";
            print_json_string_view(toolbox_item.description);
            std::cout << "\n";
            std::cout << "          }";
            if ((toolbox_index + 1U) != context.toolbox_items.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "        ]\n";
        std::cout << "      }";
        if ((context_index + 1U) != contexts.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]";
}

std::optional<int> try_handle_designer_launch_surfaces(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto designer_launch_surfaces_parse = parse_designer_launch_surfaces_arguments(catalog, args);
    if (!(designer_launch_surfaces_parse.requested)) {
        return std::nullopt;
    }

        if (!designer_launch_surfaces_parse.ok) {
            const auto result = copperfin::studio::StudioDesignerLaunchSurfacePlanResult{
                .ok = false,
                .error = designer_launch_surfaces_parse.error,
                .plan = {}
            };
            if (designer_launch_surfaces_parse.output_json) {
                print_json_designer_launch_surfaces_result(result);
            } else {
                print_text_designer_launch_surfaces_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_designer_launch_surfaces(
            designer_launch_surfaces_parse.request);
        if (designer_launch_surfaces_parse.output_json) {
            print_json_designer_launch_surfaces_result(result);
        } else {
            print_text_designer_launch_surfaces_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_designer_invocation_admission(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto designer_invocation_admission_parse = parse_designer_invocation_admission_arguments(catalog, args);
    if (!(designer_invocation_admission_parse.requested)) {
        return std::nullopt;
    }

        if (!designer_invocation_admission_parse.ok) {
            const auto result = copperfin::studio::StudioDesignerInvocationAdmissionResult{
                .ok = false,
                .error = designer_invocation_admission_parse.error,
                .plan = {}
            };
            if (designer_invocation_admission_parse.output_json) {
                print_json_designer_invocation_admission_result(result);
            } else {
                print_text_designer_invocation_admission_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_designer_launch_surfaces(
            designer_invocation_admission_parse.request);
        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioDesignerInvocationAdmissionResult{
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            };
            if (designer_invocation_admission_parse.output_json) {
                print_json_designer_invocation_admission_result(result);
            } else {
                print_text_designer_invocation_admission_result(result);
            }
            return 4;
        }

        const auto result = copperfin::studio::plan_studio_designer_invocation_admission({
            .launch_surface_plan = launch_result.plan,
            .admit_editor_invocations = designer_invocation_admission_parse.admit_editor_invocations,
            .admit_builder_invocations = designer_invocation_admission_parse.admit_builder_invocations,
            .admit_toolbox_invocation = designer_invocation_admission_parse.admit_toolbox_invocation
        });
        if (designer_invocation_admission_parse.output_json) {
            print_json_designer_invocation_admission_result(result);
        } else {
            print_text_designer_invocation_admission_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_designer_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto designer_dispatch_parse = parse_designer_dispatch_arguments(catalog, args);
    if (!(designer_dispatch_parse.requested)) {
        return std::nullopt;
    }

        if (!designer_dispatch_parse.ok) {
            const auto result = copperfin::studio::StudioDesignerDispatchResult{
                .ok = false,
                .error = designer_dispatch_parse.error,
                .plan = {}
            };
            if (designer_dispatch_parse.output_json) {
                print_json_designer_dispatch_result(result);
            } else {
                print_text_designer_dispatch_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_designer_launch_surfaces(
            designer_dispatch_parse.request);
        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioDesignerDispatchResult{
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            };
            if (designer_dispatch_parse.output_json) {
                print_json_designer_dispatch_result(result);
            } else {
                print_text_designer_dispatch_result(result);
            }
            return 4;
        }

        const auto admission_result = copperfin::studio::plan_studio_designer_invocation_admission({
            .launch_surface_plan = launch_result.plan,
            .admit_editor_invocations = designer_dispatch_parse.admit_editor_invocations,
            .admit_builder_invocations = designer_dispatch_parse.admit_builder_invocations,
            .admit_toolbox_invocation = designer_dispatch_parse.admit_toolbox_invocation
        });
        if (!admission_result.ok) {
            const auto result = copperfin::studio::StudioDesignerDispatchResult{
                .ok = false,
                .error = admission_result.error,
                .plan = {}
            };
            if (designer_dispatch_parse.output_json) {
                print_json_designer_dispatch_result(result);
            } else {
                print_text_designer_dispatch_result(result);
            }
            return 4;
        }

        const auto result = copperfin::studio::plan_studio_designer_dispatch({
            .invocation_admission_plan = admission_result.plan
        });
        if (designer_dispatch_parse.output_json) {
            print_json_designer_dispatch_result(result);
        } else {
            print_text_designer_dispatch_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_designer_execute(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto designer_execute_parse = parse_designer_execute_arguments(catalog, args);
    if (!(designer_execute_parse.requested)) {
        return std::nullopt;
    }

        auto print_designer_execution = [&](
            const copperfin::studio::StudioDesignerDispatchExecutionResult& result,
            const copperfin::studio::StudioDesignerDispatchPlan* planned_dispatch_plan) {
            if (designer_execute_parse.output_json) {
                print_json_designer_execution_result(
                    result,
                    planned_dispatch_plan,
                    catalog,
                    designer_execute_parse.editor_action_launch_command,
                    designer_execute_parse.builder_launch_command,
                    designer_execute_parse.toolbox_launch_command);
            } else {
                print_text_designer_execution_result(result);
            }
        };
        auto failed_execution = [&](std::string error) {
            return copperfin::studio::StudioDesignerDispatchExecutionResult{
                .ok = false,
                .error = std::move(error),
                .dispatch_plan = {},
                .editor_action_executions = {},
                .builder_executions = {},
                .toolbox_execution = {},
                .execution_count = 0U,
                .error_count = 0U,
                .execution_admitted = designer_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
        };

        if (!designer_execute_parse.ok) {
            const auto result = failed_execution(designer_execute_parse.error);
            print_designer_execution(result, nullptr);
            if (!designer_execute_parse.output_json) {
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_designer_launch_surfaces(
            designer_execute_parse.request);
        if (!launch_result.ok) {
            const auto result = failed_execution(launch_result.error);
            print_designer_execution(result, nullptr);
            return 4;
        }

        const auto admission_result = copperfin::studio::plan_studio_designer_invocation_admission({
            .launch_surface_plan = launch_result.plan,
            .admit_editor_invocations = designer_execute_parse.admit_editor_invocations,
            .admit_builder_invocations = designer_execute_parse.admit_builder_invocations,
            .admit_toolbox_invocation = designer_execute_parse.admit_toolbox_invocation
        });
        if (!admission_result.ok) {
            const auto result = failed_execution(admission_result.error);
            print_designer_execution(result, nullptr);
            return 4;
        }

        const auto dispatch_result = copperfin::studio::plan_studio_designer_dispatch({
            .invocation_admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            const auto result = failed_execution(dispatch_result.error);
            print_designer_execution(result, nullptr);
            return 4;
        }

        const auto result = copperfin::studio::execute_studio_designer_dispatch({
            .dispatch_plan = dispatch_result.plan,
            .admit_execution = designer_execute_parse.admit_execution,
            .editor_action_executor = [&](const copperfin::studio::StudioEditorActionDispatchPlan& plan) {
                const int exit_code = execute_launch_command(
                    designer_execute_parse.editor_action_launch_command,
                    plan.dispatch_arguments);
                return copperfin::studio::StudioEditorActionDispatchExecutionObservation{
                    .launched = true,
                    .exit_code = exit_code,
                    .output = {},
                    .error = exit_code == 0
                        ? std::string{}
                        : catalog.translate(
                            "StudioHost.DesignerExecution.Error.EditorActionLaunchCommandNonZeroExit"),
                    .mutates_asset = false
                };
            },
            .builder_executor = [&](const copperfin::studio::StudioBuilderDispatchPlan& plan) {
                const int exit_code = execute_launch_command(
                    designer_execute_parse.builder_launch_command,
                    plan.dispatch_arguments);
                return copperfin::studio::StudioBuilderDispatchExecutionObservation{
                    .launched = true,
                    .exit_code = exit_code,
                    .output = {},
                    .error = exit_code == 0
                        ? std::string{}
                        : catalog.translate(
                            "StudioHost.DesignerExecution.Error.BuilderLaunchCommandNonZeroExit"),
                    .mutates_asset = false
                };
            },
            .toolbox_executor = [&](const copperfin::studio::StudioToolboxDispatchPlan& plan) {
                const int exit_code = execute_launch_command(
                    designer_execute_parse.toolbox_launch_command,
                    plan.dispatch_arguments);
                return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                    .launched = true,
                    .exit_code = exit_code,
                    .output = {},
                    .error = exit_code == 0
                        ? std::string{}
                        : catalog.translate(
                            "StudioHost.DesignerExecution.Error.ToolboxLaunchCommandNonZeroExit"),
                    .mutates_asset = false
                };
            }
        });
        print_designer_execution(result, &dispatch_result.plan);
        return result.ok && result.error_count == 0U ? 0 : 4;
    }

std::optional<int> try_handle_designer_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto designer_dispatch_execution_catalog_parse =
        parse_designer_dispatch_execution_catalog_arguments(catalog, args);
    if (!(designer_dispatch_execution_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!designer_dispatch_execution_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioDesignerDispatchExecutionCatalogResult{
                .ok = false,
                .error = designer_dispatch_execution_catalog_parse.error,
                .context_count = 0U,
                .execution_ready_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .contexts = {}
            };
            if (designer_dispatch_execution_catalog_parse.output_json) {
                print_json_designer_dispatch_execution_catalog_result(result);
            } else {
                print_text_designer_dispatch_execution_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_designer_dispatch_execution_catalog(
            designer_dispatch_execution_catalog_parse.request);
        if (designer_dispatch_execution_catalog_parse.output_json) {
            print_json_designer_dispatch_execution_catalog_result(result);
        } else {
            print_text_designer_dispatch_execution_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_designer_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto designer_dispatch_catalog_parse = parse_designer_dispatch_catalog_arguments(catalog, args);
    if (!(designer_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!designer_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioDesignerDispatchCatalogResult{
                .ok = false,
                .error = designer_dispatch_catalog_parse.error,
                .context_count = 0U,
                .contexts = {}
            };
            if (designer_dispatch_catalog_parse.output_json) {
                print_json_designer_dispatch_catalog_result(result);
            } else {
                print_text_designer_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_designer_dispatch_catalog(
            designer_dispatch_catalog_parse.request);
        if (designer_dispatch_catalog_parse.output_json) {
            print_json_designer_dispatch_catalog_result(result);
        } else {
            print_text_designer_dispatch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_designer_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto designer_invocation_admission_catalog_parse =
        parse_designer_invocation_admission_catalog_arguments(catalog, args);
    if (!(designer_invocation_admission_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!designer_invocation_admission_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioDesignerInvocationAdmissionCatalogResult{
                .ok = false,
                .error = designer_invocation_admission_catalog_parse.error,
                .context_count = 0U,
                .contexts = {}
            };
            if (designer_invocation_admission_catalog_parse.output_json) {
                print_json_designer_invocation_admission_catalog_result(result);
            } else {
                print_text_designer_invocation_admission_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_designer_invocation_admission_catalog(
            designer_invocation_admission_catalog_parse.request);
        if (designer_invocation_admission_catalog_parse.output_json) {
            print_json_designer_invocation_admission_catalog_result(result);
        } else {
            print_text_designer_invocation_admission_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_designer_launch_surface_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto designer_launch_surface_catalog_parse = parse_designer_launch_surface_catalog_arguments(catalog, args);
    if (!(designer_launch_surface_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!designer_launch_surface_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioDesignerLaunchSurfaceCatalogResult{
                .ok = false,
                .error = designer_launch_surface_catalog_parse.error,
                .context_count = 0U,
                .contexts = {}
            };
            if (designer_launch_surface_catalog_parse.output_json) {
                print_json_designer_launch_surface_catalog_result(result);
            } else {
                print_text_designer_launch_surface_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_designer_launch_surface_catalog(
            designer_launch_surface_catalog_parse.request);
        if (designer_launch_surface_catalog_parse.output_json) {
            print_json_designer_launch_surface_catalog_result(result);
        } else {
            print_text_designer_launch_surface_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
