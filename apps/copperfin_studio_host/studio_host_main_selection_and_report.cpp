// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
void print_json_selection_toolbox_create_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateResult& result) {
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
    std::cout << "  \"selectionToolboxCreate\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"createPlanOk\": " << (result.create_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"createPlanError\": ";
    print_json_string(result.create_plan.error);
    std::cout << ",\n";
    std::cout << "    \"createPlan\": ";
    if (!result.create_plan.ok) {
        std::cout << "null,\n";
    } else {
        const auto& plan = result.create_plan.create_plan.plan;
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

void print_json_selection_toolbox_create_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreatePlanResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxCreatePlan\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"createPlanOk\": " << (result.create_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"createPlanError\": ";
    print_json_string(result.create_plan.error);
    std::cout << ",\n";
    std::cout << "    \"createPlan\": ";
    if (!result.create_plan.ok) {
        std::cout << "null,\n";
        std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
        std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << "\n";
        std::cout << "  },\n";
        std::cout << "  \"error\": \"\"\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.create_plan.plan;
    const std::vector<std::string> plan_ready_item_ids{std::string(plan.toolbox_item.id)};
    const std::vector<std::string> plan_blocked_item_ids;
    const std::vector<std::string> plan_blocked_errors;

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
    std::cout << "      \"planReadyItemIds\": ";
    print_json_string_array(plan_ready_item_ids);
    std::cout << ",\n";
    std::cout << "      \"planBlockedItemIds\": ";
    print_json_string_array(plan_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "      \"planBlockedErrors\": ";
    print_json_string_array(plan_blocked_errors);
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
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_selection_toolbox_create_dispatch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateDispatchResult& result) {
    std::vector<std::string> dispatch_ready_item_ids;
    std::vector<std::string> dispatch_blocked_item_ids;
    std::vector<std::string> dispatch_blocked_errors;
    if (result.dispatch.ok) {
        dispatch_ready_item_ids.push_back(std::string(result.dispatch.plan.toolbox_item.id));
    } else {
        const std::string& error = !result.dispatch.error.empty() ? result.dispatch.error :
            (!result.create_plan.error.empty() ? result.create_plan.error :
                (!result.launch_plan.error.empty() ? result.launch_plan.error : result.error));
        if (!error.empty()) {
            dispatch_blocked_errors.push_back(error);
        }
    }

    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxCreateDispatchPlan\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"createPlanOk\": " << (result.create_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"createPlanError\": ";
    print_json_string(result.create_plan.error);
    std::cout << ",\n";
    std::cout << "    \"createPlan\": ";
    if (!result.create_plan.ok) {
        std::cout << "null,\n";
    } else {
        const auto& create_plan = result.create_plan.create_plan.plan;
        std::cout << "{\n";
        std::cout << "      \"toolboxItemId\": ";
        print_json_string_view(create_plan.toolbox_item.id);
        std::cout << ",\n";
        std::cout << "      \"title\": ";
        print_json_string_view(create_plan.toolbox_item.title);
        std::cout << ",\n";
        std::cout << "      \"className\": ";
        print_json_string_view(create_plan.toolbox_item.vfp_class);
        std::cout << ",\n";
        std::cout << "      \"baseClassName\": ";
        print_json_string_view(create_plan.toolbox_item.base_class);
        std::cout << ",\n";
        std::cout << "      \"toolboxContextProvided\": "
                  << (create_plan.toolbox_context_provided ? "true" : "false") << ",\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(create_plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"targetRecordIndex\": " << create_plan.target_record_index << ",\n";
        std::cout << "      \"objectName\": ";
        print_json_string(create_plan.object_name);
        std::cout << ",\n";
        std::cout << "      \"uniqueId\": ";
        print_json_string(create_plan.unique_id);
        std::cout << ",\n";
        std::cout << "      \"parentName\": ";
        print_json_string(create_plan.parent_name);
        std::cout << ",\n";
        std::cout << "      \"fieldValues\": [\n";
        for (std::size_t index = 0U; index < create_plan.field_values.size(); ++index) {
            const auto& field_value = create_plan.field_values[index];
            std::cout << "        {\"propertyName\": ";
            print_json_string(field_value.property_name);
            std::cout << ", \"propertyValue\": ";
            print_json_string(field_value.property_value);
            std::cout << "}";
            if ((index + 1U) != create_plan.field_values.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"dryRun\": " << (create_plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (create_plan.mutates_asset ? "true" : "false") << "\n";
        std::cout << "    },\n";
    }
    std::cout << "    \"dispatchOk\": " << (result.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchError\": ";
    print_json_string(result.dispatch.error);
    std::cout << ",\n";
    std::cout << "    \"dispatch\": ";
    if (!result.dispatch.ok) {
        std::cout << "null,\n";
    } else {
        const auto& dispatch_plan = result.dispatch.plan;
        std::cout << "{\n";
        std::cout << "      \"path\": ";
        print_json_string(dispatch_plan.path);
        std::cout << ",\n";
        std::cout << "      \"toolboxItemId\": ";
        print_json_string_view(dispatch_plan.toolbox_item.id);
        std::cout << ",\n";
        std::cout << "      \"title\": ";
        print_json_string_view(dispatch_plan.toolbox_item.title);
        std::cout << ",\n";
        std::cout << "      \"className\": ";
        print_json_string_view(dispatch_plan.toolbox_item.vfp_class);
        std::cout << ",\n";
        std::cout << "      \"baseClassName\": ";
        print_json_string_view(dispatch_plan.toolbox_item.base_class);
        std::cout << ",\n";
        std::cout << "      \"toolboxContextProvided\": "
                  << (dispatch_plan.toolbox_context_provided ? "true" : "false") << ",\n";
        std::cout << "      \"toolboxContext\": ";
        print_json_string(copperfin::studio::studio_toolbox_context_name(dispatch_plan.toolbox_context));
        std::cout << ",\n";
        std::cout << "      \"targetRecordIndex\": " << dispatch_plan.target_record_index << ",\n";
        std::cout << "      \"objectName\": ";
        print_json_string(dispatch_plan.object_name);
        std::cout << ",\n";
        std::cout << "      \"uniqueId\": ";
        print_json_string(dispatch_plan.unique_id);
        std::cout << ",\n";
        std::cout << "      \"parentName\": ";
        print_json_string(dispatch_plan.parent_name);
        std::cout << ",\n";
        std::cout << "      \"fieldValues\": [\n";
        for (std::size_t index = 0U; index < dispatch_plan.field_values.size(); ++index) {
            const auto& field_value = dispatch_plan.field_values[index];
            std::cout << "        {\"propertyName\": ";
            print_json_string(field_value.property_name);
            std::cout << ", \"propertyValue\": ";
            print_json_string(field_value.property_value);
            std::cout << "}";
            if ((index + 1U) != dispatch_plan.field_values.size()) {
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
        std::cout << "      \"dispatchAdmitted\": " << (dispatch_plan.dispatch_admitted ? "true" : "false") << ",\n";
        std::cout << "      \"dryRun\": " << (dispatch_plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"executed\": " << (dispatch_plan.executed ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (dispatch_plan.mutates_asset ? "true" : "false") << "\n";
        std::cout << "    },\n";
    }
    std::cout << "    \"dispatchCount\": " << result.dispatch_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dispatchReadyItemIds\": ";
    print_json_string_array(dispatch_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedItemIds\": ";
    print_json_string_array(dispatch_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": ";
    print_json_string(result.error);
    std::cout << "\n";
    std::cout << "}\n";
}

void print_json_selection_toolbox_create_batch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanResult& result) {
    std::vector<std::string> plan_ready_item_ids;
    std::vector<std::string> plan_blocked_item_ids;
    std::vector<std::string> plan_blocked_errors;
    if (result.batch_plan.ok) {
        plan_ready_item_ids.reserve(result.batch_plan.plan.plans.size());
        for (const auto& create_plan : result.batch_plan.plan.plans) {
            plan_ready_item_ids.push_back(std::string(create_plan.toolbox_item.id));
        }
    } else {
        const std::string& error = !result.batch_plan.error.empty() ? result.batch_plan.error : result.error;
        if (!error.empty()) {
            plan_blocked_errors.push_back(error);
        }
    }

    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxCreateBatchPlan\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"planCount\": " << result.plan_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"planReadyItemIds\": ";
    print_json_string_array(plan_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"planBlockedItemIds\": ";
    print_json_string_array(plan_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"planBlockedErrors\": ";
    print_json_string_array(plan_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"batchPlanOk\": " << (result.batch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"batchPlanError\": ";
    print_json_string(result.batch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"batchPlan\": ";
    if (!result.batch_plan.ok) {
        std::cout << "null\n";
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
        std::cout << "    }\n";
    }
    std::cout << "  },\n";
    std::cout << "  \"error\": ";
    print_json_string(result.error);
    std::cout << "\n";
    std::cout << "}\n";
}

void print_json_selection_toolbox_create_batch_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchResult& result) {
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
    std::cout << "  \"selectionToolboxCreateBatch\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"batchPlanOk\": " << (result.batch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"batchPlanError\": ";
    print_json_string(result.batch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"planCount\": " << result.batch_plan.plan_count << ",\n";
    std::cout << "    \"errorCount\": " << result.batch_plan.error_count << ",\n";
    std::cout << "    \"batchPlan\": ";
    if (!result.batch_plan.ok) {
        std::cout << "null,\n";
    } else {
        const auto& plan = result.batch_plan.batch_plan.plan;
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

void print_json_selection_toolbox_create_batch_dispatch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchResult& result) {
    std::vector<std::string> dispatch_ready_item_ids;
    std::vector<std::string> dispatch_blocked_item_ids;
    std::vector<std::string> dispatch_blocked_errors;
    if (result.dispatch.ok) {
        for (const auto& plan : result.dispatch.plan.plans) {
            dispatch_ready_item_ids.push_back(std::string(plan.toolbox_item.id));
        }
    } else {
        const std::string& error = !result.dispatch.error.empty() ? result.dispatch.error :
            (!result.batch_plan.error.empty() ? result.batch_plan.error :
                (!result.launch_plan.error.empty() ? result.launch_plan.error : result.error));
        if (!error.empty()) {
            dispatch_blocked_errors.push_back(error);
        }
    }

    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxCreateBatchDispatchPlan\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
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
        const auto& batch_plan = result.batch_plan.batch_plan.plan;
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
        std::cout << "      \"dryRun\": " << (batch_plan.dry_run ? "true" : "false") << ",\n";
        std::cout << "      \"mutatesAsset\": " << (batch_plan.mutates_asset ? "true" : "false") << ",\n";
        std::cout << "      \"plans\": [\n";
        for (std::size_t index = 0U; index < batch_plan.plans.size(); ++index) {
            print_json_toolbox_create_batch_plan_entry(batch_plan.plans[index], "        ");
            if ((index + 1U) != batch_plan.plans.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ]\n";
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
    std::cout << "  \"error\": ";
    print_json_string(result.error);
    std::cout << "\n";
    std::cout << "}\n";
}

void print_json_selection_toolbox_create_batch_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxCreateBatchDispatchCatalog\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
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

void print_json_selection_toolbox_create_plan_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreatePlanCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxCreatePlanCatalog\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
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

void print_json_selection_toolbox_create_batch_plan_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxCreateBatchPlanCatalog\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
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

void print_json_selection_toolbox_create_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxCreateDispatchCatalog\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
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

void print_json_selection_builder_launch_catalog_entry(
    const copperfin::studio::StudioSelectionBuilderLaunchCatalogEntry& entry,
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
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
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

void print_json_selection_builder_launch_catalog_result(
    const copperfin::studio::StudioSelectionBuilderLaunchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionBuilderLaunchCatalog\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
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
        print_json_selection_builder_launch_catalog_entry(result.entries[index], "      ");
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

void print_json_selection_builder_invocation_admission_catalog_entry(
    const copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogEntry& entry,
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
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
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

void print_json_selection_builder_invocation_admission_catalog_result(
    const copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionBuilderInvocationAdmissionCatalog\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
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
        print_json_selection_builder_invocation_admission_catalog_entry(result.entries[index], "      ");
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

void print_json_selection_builder_dispatch_catalog_entry(
    const copperfin::studio::StudioSelectionBuilderDispatchCatalogEntry& entry,
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
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
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

void print_json_selection_builder_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionBuilderDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionBuilderDispatchCatalog\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
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
        print_json_selection_builder_dispatch_catalog_entry(result.entries[index], "      ");
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

void print_json_selection_builder_dispatch_execution_catalog_entry(
    const copperfin::studio::StudioSelectionBuilderDispatchExecutionCatalogEntry& entry,
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
    std::cout << indent << "  \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(entry.selection_context));
    std::cout << ",\n";
    std::cout << indent << "  \"launchOk\": " << (entry.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << indent << "  \"admissionOk\": "
              << (entry.invocation_admission.ok ? "true" : "false") << ",\n";
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

void print_json_selection_builder_dispatch_execution_catalog_result(
    const copperfin::studio::StudioSelectionBuilderDispatchExecutionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionBuilderDispatchExecutionCatalog\": ";
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
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
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
        print_json_selection_builder_dispatch_execution_catalog_entry(result.entries[index], "      ");
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

void print_json_selection_toolbox_invocation_admission_catalog_result(
    const copperfin::studio::StudioSelectionToolboxInvocationAdmissionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxInvocationAdmissionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> admission_ready_item_ids;
    std::vector<std::string> admission_blocked_item_ids;
    std::vector<std::string> admission_blocked_errors;
    for (const auto& item : result.items) {
        if (result.invocation_admission.ok) {
            admission_ready_item_ids.push_back(std::string(item.id));
        } else {
            admission_blocked_item_ids.push_back(std::string(item.id));
            admission_blocked_errors.push_back(result.invocation_admission.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(result.command_token);
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(result.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(result.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(result.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"admissionCount\": " << result.admission_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"admissionReadyItemIds\": ";
    print_json_string_array(admission_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedItemIds\": ";
    print_json_string_array(admission_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < result.items.size(); ++index) {
        print_json_toolbox_item_descriptor(result.items[index], "      ");
        if ((index + 1U) != result.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"invocationAdmissionOk\": "
              << (result.invocation_admission.ok ? "true" : "false") << ",\n";
    std::cout << "    \"paletteInvocationAdmitted\": "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << ",\n";
    std::cout << "    \"invocationAdmissionError\": ";
    print_json_string(result.invocation_admission.error);
    std::cout << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_selection_toolbox_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxDispatchCatalog\": ";
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
    for (const auto& item : result.items) {
        if (result.dispatch.ok) {
            dispatch_ready_item_ids.push_back(std::string(item.id));
        } else {
            dispatch_blocked_item_ids.push_back(std::string(item.id));
            dispatch_blocked_errors.push_back(result.dispatch.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(result.command_token);
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(result.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(result.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(result.unique_id);
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
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < result.items.size(); ++index) {
        print_json_toolbox_item_descriptor(result.items[index], "      ");
        if ((index + 1U) != result.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"invocationAdmissionOk\": "
              << (result.invocation_admission.ok ? "true" : "false") << ",\n";
    std::cout << "    \"paletteInvocationAdmitted\": "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << ",\n";
    std::cout << "    \"invocationAdmissionError\": ";
    print_json_string(result.invocation_admission.error);
    std::cout << ",\n";
    std::cout << "    \"dispatchOk\": " << (result.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchError\": ";
    print_json_string(result.dispatch.error);
    std::cout << ",\n";
    std::cout << "    \"dispatchArguments\": [";
    if (result.dispatch.ok) {
        for (std::size_t index = 0U; index < result.dispatch.plan.dispatch_arguments.size(); ++index) {
            if (index != 0U) {
                std::cout << ", ";
            }
            print_json_string(result.dispatch.plan.dispatch_arguments[index]);
        }
    }
    std::cout << "]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_selection_toolbox_dispatch_execution_catalog_result(
    const copperfin::studio::StudioSelectionToolboxDispatchExecutionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"selectionToolboxDispatchExecutionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> execution_ready_item_ids;
    std::vector<std::string> execution_blocked_item_ids;
    std::vector<std::string> execution_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.execution_ready) {
            execution_ready_item_ids.push_back(std::string(entry.item.id));
        } else {
            execution_blocked_item_ids.push_back(std::string(entry.item.id));
            execution_blocked_errors.push_back(entry.execution_error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(result.command_token);
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(result.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(result.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(result.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"executionReadyCount\": " << result.execution_ready_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"executionReadyItemIds\": ";
    print_json_string_array(execution_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedItemIds\": ";
    print_json_string_array(execution_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedErrors\": ";
    print_json_string_array(execution_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < result.items.size(); ++index) {
        print_json_toolbox_item_descriptor(result.items[index], "      ");
        if ((index + 1U) != result.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_toolbox_dispatch_execution_catalog_entry(result.entries[index], "      ");
        if ((index + 1U) != result.entries.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"launchPlanOk\": " << (result.launch_plan.ok ? "true" : "false") << ",\n";
    std::cout << "    \"launchPlanError\": ";
    print_json_string(result.launch_plan.error);
    std::cout << ",\n";
    std::cout << "    \"invocationAdmissionOk\": "
              << (result.invocation_admission.ok ? "true" : "false") << ",\n";
    std::cout << "    \"paletteInvocationAdmitted\": "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << ",\n";
    std::cout << "    \"invocationAdmissionError\": ";
    print_json_string(result.invocation_admission.error);
    std::cout << ",\n";
    std::cout << "    \"dispatchOk\": " << (result.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchError\": ";
    print_json_string(result.dispatch.error);
    std::cout << ",\n";
    std::cout << "    \"dispatchArguments\": [";
    if (result.dispatch.ok) {
        for (std::size_t index = 0U; index < result.dispatch.plan.dispatch_arguments.size(); ++index) {
            if (index != 0U) {
                std::cout << ", ";
            }
            print_json_string(result.dispatch.plan.dispatch_arguments[index]);
        }
    }
    std::cout << "]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_text_selection_toolbox_create_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreatePlanResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
    std::cout << "create_plan_ok: " << (result.create_plan.ok ? "true" : "false") << "\n";
    if (!result.create_plan.error.empty()) {
        std::cout << "create_plan_error: " << result.create_plan.error << "\n";
    }
    if (result.create_plan.ok) {
        const auto& plan = result.create_plan.plan;
        std::cout << "toolbox_item_id: " << plan.toolbox_item.id << "\n";
        std::cout << "title: " << plan.toolbox_item.title << "\n";
        std::cout << "class_name: " << plan.toolbox_item.vfp_class << "\n";
        std::cout << "baseclass_name: " << plan.toolbox_item.base_class << "\n";
        std::cout << "target_record_index: " << plan.target_record_index << "\n";
        std::cout << "object_name: " << plan.object_name << "\n";
        std::cout << "unique_id: " << plan.unique_id << "\n";
        std::cout << "parent_name: " << plan.parent_name << "\n";
        for (const auto& field_value : plan.field_values) {
            std::cout << "field_value: " << field_value.property_name << "=" << field_value.property_value << "\n";
        }
    }
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
}

void print_text_selection_toolbox_create_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
    std::cout << "create_plan_ok: " << (result.create_plan.ok ? "true" : "false") << "\n";
    if (!result.create_plan.error.empty()) {
        std::cout << "create_plan_error: " << result.create_plan.error << "\n";
    }
    if (result.create_plan.ok) {
        const auto& plan = result.create_plan.create_plan.plan;
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

void print_text_selection_toolbox_create_dispatch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
    std::cout << "create_plan_ok: " << (result.create_plan.ok ? "true" : "false") << "\n";
    if (!result.create_plan.error.empty()) {
        std::cout << "create_plan_error: " << result.create_plan.error << "\n";
    }
    if (result.create_plan.ok) {
        const auto& create_plan = result.create_plan.create_plan.plan;
        std::cout << "toolbox_item_id: " << create_plan.toolbox_item.id << "\n";
        std::cout << "title: " << create_plan.toolbox_item.title << "\n";
        std::cout << "class_name: " << create_plan.toolbox_item.vfp_class << "\n";
        std::cout << "baseclass_name: " << create_plan.toolbox_item.base_class << "\n";
        std::cout << "target_record_index: " << create_plan.target_record_index << "\n";
        std::cout << "object_name: " << create_plan.object_name << "\n";
        std::cout << "unique_id: " << create_plan.unique_id << "\n";
        std::cout << "parent_name: " << create_plan.parent_name << "\n";
        for (const auto& field_value : create_plan.field_values) {
            std::cout << "field_value: " << field_value.property_name << "=" << field_value.property_value << "\n";
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
        std::cout << "dispatch_admitted: "
                  << (result.dispatch.plan.dispatch_admitted ? "true" : "false") << "\n";
        std::cout << "executed: " << (result.dispatch.plan.executed ? "true" : "false") << "\n";
    }
    std::cout << "dispatch_count: " << result.dispatch_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
}

void print_text_selection_toolbox_create_batch_dispatch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
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
        for (const auto& plan : result.batch_plan.batch_plan.plan.plans) {
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
        std::cout << "dispatch_admitted: "
                  << (result.dispatch.plan.dispatch_admitted ? "true" : "false") << "\n";
        std::cout << "executed: " << (result.dispatch.plan.executed ? "true" : "false") << "\n";
    }
}

void print_text_selection_toolbox_create_plan_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreatePlanCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
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

void print_text_selection_toolbox_create_batch_plan_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
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

void print_text_selection_toolbox_create_batch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
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

void print_text_selection_toolbox_create_batch_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "batch_plan_ok: " << (result.batch_plan.ok ? "true" : "false") << "\n";
    if (!result.batch_plan.error.empty()) {
        std::cout << "batch_plan_error: " << result.batch_plan.error << "\n";
    }
    if (result.batch_plan.ok) {
        for (const auto& plan : result.batch_plan.batch_plan.plan.plans) {
            std::cout << "plan_toolbox_item_id: " << plan.toolbox_item.id << "\n";
            std::cout << "plan_object_name: " << plan.object_name << "\n";
        }
    }
    std::cout << "create_result_ok: " << (result.create_result.ok ? "true" : "false") << "\n";
    if (!result.create_result.error.empty()) {
        std::cout << "create_result_error: " << result.create_result.error << "\n";
    }
    for (const auto record_index : result.create_result.record_indexes) {
        std::cout << "record_index: " << record_index << "\n";
    }
    for (const auto& created_object : result.create_result.created_objects) {
        std::cout << "created_object_name: " << created_object.object_name << "\n";
        std::cout << "created_unique_id: " << created_object.unique_id << "\n";
        std::cout << "created_parent_name: " << created_object.parent_name << "\n";
    }
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
}

void print_text_selection_toolbox_create_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
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

void print_text_selection_toolbox_create_batch_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
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

void print_text_selection_builder_launch_catalog_result(
    const copperfin::studio::StudioSelectionBuilderLaunchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "builder_count: " << result.builder_count << "\n";
    std::cout << "launch_plan_count: " << result.launch_plan_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_builder_id: " << entry.builder.id << "\n";
        std::cout << "entry_selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(entry.selection_context) << "\n";
        std::cout << "entry_launch_ok: " << (entry.launch_plan.ok ? "true" : "false") << "\n";
        if (!entry.launch_plan.error.empty()) {
            std::cout << "entry_error: " << entry.launch_plan.error << "\n";
        }
    }
}

void print_text_selection_builder_invocation_admission_catalog_result(
    const copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "builder_count: " << result.builder_count << "\n";
    std::cout << "admission_count: " << result.admission_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_builder_id: " << entry.builder.id << "\n";
        std::cout << "entry_selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(entry.selection_context) << "\n";
        std::cout << "entry_admission_ok: " << (entry.invocation_admission.ok ? "true" : "false") << "\n";
        if (!entry.invocation_admission.error.empty()) {
            std::cout << "entry_error: " << entry.invocation_admission.error << "\n";
        }
    }
}

void print_text_selection_builder_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionBuilderDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "builder_count: " << result.builder_count << "\n";
    std::cout << "dispatch_count: " << result.dispatch_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_builder_id: " << entry.builder.id << "\n";
        std::cout << "entry_selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(entry.selection_context) << "\n";
        std::cout << "entry_dispatch_ok: " << (entry.dispatch.ok ? "true" : "false") << "\n";
        if (!entry.dispatch.error.empty()) {
            std::cout << "entry_error: " << entry.dispatch.error << "\n";
        }
    }
}

void print_text_selection_builder_dispatch_execution_catalog_result(
    const copperfin::studio::StudioSelectionBuilderDispatchExecutionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "builder_count: " << result.builder_count << "\n";
    std::cout << "execution_ready_count: " << result.execution_ready_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_builder_id: " << entry.builder.id << "\n";
        std::cout << "entry_selection_context: "
                  << copperfin::studio::studio_editor_selection_context_name(entry.selection_context) << "\n";
        std::cout << "entry_execution_ready: " << (entry.execution_ready ? "true" : "false") << "\n";
        if (!entry.execution_error.empty()) {
            std::cout << "entry_execution_error: " << entry.execution_error << "\n";
        }
    }
}

void print_text_selection_toolbox_invocation_admission_catalog_result(
    const copperfin::studio::StudioSelectionToolboxInvocationAdmissionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "command_token: " << result.command_token << "\n";
    std::cout << "asset_path: " << result.asset_path << "\n";
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "object_name: " << result.object_name << "\n";
    std::cout << "unique_id: " << result.unique_id << "\n";
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "admission_count: " << result.admission_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& item : result.items) {
        std::cout << "item: " << item.id << " " << item.title << "\n";
    }
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
    std::cout << "invocation_admission_ok: " << (result.invocation_admission.ok ? "true" : "false") << "\n";
    std::cout << "palette_invocation_admitted: "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << "\n";
    if (!result.invocation_admission.error.empty()) {
        std::cout << "invocation_admission_error: " << result.invocation_admission.error << "\n";
    }
}

void print_text_selection_toolbox_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "command_token: " << result.command_token << "\n";
    std::cout << "asset_path: " << result.asset_path << "\n";
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "object_name: " << result.object_name << "\n";
    std::cout << "unique_id: " << result.unique_id << "\n";
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "dispatch_count: " << result.dispatch_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& item : result.items) {
        std::cout << "item: " << item.id << " " << item.title << "\n";
    }
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
    std::cout << "invocation_admission_ok: " << (result.invocation_admission.ok ? "true" : "false") << "\n";
    std::cout << "palette_invocation_admitted: "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << "\n";
    if (!result.invocation_admission.error.empty()) {
        std::cout << "invocation_admission_error: " << result.invocation_admission.error << "\n";
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

void print_text_selection_toolbox_dispatch_execution_catalog_result(
    const copperfin::studio::StudioSelectionToolboxDispatchExecutionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "command_token: " << result.command_token << "\n";
    std::cout << "asset_path: " << result.asset_path << "\n";
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "object_name: " << result.object_name << "\n";
    std::cout << "unique_id: " << result.unique_id << "\n";
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "execution_ready_count: " << result.execution_ready_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& item : result.items) {
        std::cout << "item: " << item.id << " " << item.title << "\n";
    }
    for (const auto& entry : result.entries) {
        std::cout << "entry_item: " << entry.item.id << " " << entry.item.title << "\n";
        std::cout << "entry_execution_ready: " << (entry.execution_ready ? "true" : "false") << "\n";
        if (!entry.execution_error.empty()) {
            std::cout << "entry_execution_error: " << entry.execution_error << "\n";
        }
    }
    std::cout << "launch_plan_ok: " << (result.launch_plan.ok ? "true" : "false") << "\n";
    if (!result.launch_plan.error.empty()) {
        std::cout << "launch_plan_error: " << result.launch_plan.error << "\n";
    }
    std::cout << "invocation_admission_ok: " << (result.invocation_admission.ok ? "true" : "false") << "\n";
    std::cout << "palette_invocation_admitted: "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << "\n";
    std::cout << "dispatch_ok: " << (result.dispatch.ok ? "true" : "false") << "\n";
    if (!result.dispatch.error.empty()) {
        std::cout << "dispatch_error: " << result.dispatch.error << "\n";
    }
}

void print_json_report_field_index_or_null(std::size_t field_index) {
    if (field_index == copperfin::studio::StudioReportMissingFieldIndex) {
        std::cout << "null";
    } else {
        std::cout << field_index;
    }
}

void print_json_report_line_index_or_null(std::size_t line_index) {
    if (line_index == copperfin::studio::StudioReportMissingLineIndex) {
        std::cout << "null";
    } else {
        std::cout << line_index;
    }
}

void print_json_report_record_index_or_null(std::size_t record_index) {
    if (record_index == copperfin::studio::StudioReportMissingRecordIndex) {
        std::cout << "null";
    } else {
        std::cout << record_index;
    }
}

void print_json_report_named_values(
    const std::vector<copperfin::studio::StudioNamedValue>& values,
    const std::string& indent) {
    std::cout << "[\n";
    for (std::size_t index = 0; index < values.size(); ++index) {
        const auto& value = values[index];
        std::cout << indent << "  {\"name\": ";
        print_json_string(value.name);
        std::cout << ", \"recordIndex\": " << value.record_index;
        std::cout << ", \"fieldIndex\": ";
        print_json_report_field_index_or_null(value.field_index);
        std::cout << ", \"sourceLineIndex\": ";
        print_json_report_line_index_or_null(value.source_line_index);
        std::cout << ", \"memoBlockNumber\": " << value.memo_block_number;
        std::cout << ", \"value\": ";
        print_json_string(value.value);
        std::cout << "}";
        if ((index + 1U) != values.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << indent << "]";
}

void print_json_report_kind_counts(
    const std::vector<copperfin::studio::StudioReportKindCount>& counts,
    const std::string& indent) {
    std::cout << "[\n";
    for (std::size_t index = 0; index < counts.size(); ++index) {
        const auto& count = counts[index];
        std::cout << indent << "  {\"kind\": ";
        print_json_string(count.kind);
        std::cout << ", \"count\": " << count.count << "}";
        if ((index + 1U) != counts.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << indent << "]";
}

const copperfin::studio::StudioObjectSnapshot* find_selected_object(
    const std::vector<copperfin::studio::StudioObjectSnapshot>& objects,
    std::size_t record_index) {
    const auto selected = std::find_if(objects.begin(), objects.end(), [&](const auto& object) {
        return object.record_index == record_index;
    });
    return selected == objects.end() ? nullptr : &*selected;
}

const copperfin::studio::StudioReportSectionSnapshot* find_selected_report_section(
    const copperfin::studio::StudioReportLayoutSnapshot& report_layout,
    std::size_t record_index) {
    if (!report_layout.available) {
        return nullptr;
    }
    const auto live_section = std::find_if(
        report_layout.sections.begin(),
        report_layout.sections.end(),
        [&](const copperfin::studio::StudioReportSectionSnapshot& section) {
            return section.record_index == record_index;
        });
    if (live_section != report_layout.sections.end()) {
        return &*live_section;
    }
    const auto deleted_section = std::find_if(
        report_layout.deleted_sections.begin(),
        report_layout.deleted_sections.end(),
        [&](const copperfin::studio::StudioReportSectionSnapshot& section) {
            return section.record_index == record_index;
        });
    return deleted_section == report_layout.deleted_sections.end() ? nullptr : &*deleted_section;
}

const copperfin::studio::StudioLayoutObjectSnapshot* find_selected_report_object(
    const copperfin::studio::StudioReportLayoutSnapshot& report_layout,
    std::size_t record_index) {
    if (!report_layout.available) {
        return nullptr;
    }
    for (const auto& section : report_layout.sections) {
        const auto object = std::find_if(
            section.objects.begin(),
            section.objects.end(),
            [&](const copperfin::studio::StudioLayoutObjectSnapshot& item) {
                return item.record_index == record_index;
            });
        if (object != section.objects.end()) {
            return &*object;
        }
    }
    for (const auto& section : report_layout.deleted_sections) {
        const auto object = std::find_if(
            section.objects.begin(),
            section.objects.end(),
            [&](const copperfin::studio::StudioLayoutObjectSnapshot& item) {
                return item.record_index == record_index;
            });
        if (object != section.objects.end()) {
            return &*object;
        }
    }
    const auto unplaced_object = std::find_if(
        report_layout.unplaced_objects.begin(),
        report_layout.unplaced_objects.end(),
        [&](const copperfin::studio::StudioLayoutObjectSnapshot& object) {
            return object.record_index == record_index;
        });
    if (unplaced_object != report_layout.unplaced_objects.end()) {
        return &*unplaced_object;
    }
    const auto deleted_object = std::find_if(
        report_layout.deleted_objects.begin(),
        report_layout.deleted_objects.end(),
        [&](const copperfin::studio::StudioLayoutObjectSnapshot& object) {
            return object.record_index == record_index;
        });
    return deleted_object == report_layout.deleted_objects.end() ? nullptr : &*deleted_object;
}

const copperfin::studio::StudioReportSectionSnapshot* find_selected_report_object_section(
    const copperfin::studio::StudioReportLayoutSnapshot& report_layout,
    std::size_t record_index) {
    if (!report_layout.available) {
        return nullptr;
    }
    const auto* object = find_selected_report_object(report_layout, record_index);
    if (object == nullptr ||
        object->containing_section_record_index == copperfin::studio::StudioReportMissingRecordIndex) {
        return nullptr;
    }
    return find_selected_report_section(report_layout, object->containing_section_record_index);
}

std::vector<copperfin::studio::StudioNamedValue> find_selected_report_settings(
    const copperfin::studio::StudioReportLayoutSnapshot& report_layout,
    std::size_t record_index) {
    std::vector<copperfin::studio::StudioNamedValue> settings;
    if (!report_layout.available) {
        return settings;
    }
    for (const auto& setting : report_layout.settings) {
        if (setting.record_index == record_index) {
            settings.push_back(setting);
        }
    }
    for (const auto& setting : report_layout.deleted_settings) {
        if (setting.record_index == record_index) {
            settings.push_back(setting);
        }
    }
    return settings;
}

void print_json_report_layout_object(
    const copperfin::studio::StudioLayoutObjectSnapshot& object,
    const std::string& indent) {
    const std::string picture_alignment = [&]() {
        if (object.objtype_code != 5 || object.picture.empty()) {
            return std::string("left");
        }
        if (object.picture == "@I") {
            return std::string("center");
        }
        if (object.picture == "@J") {
            return std::string("right");
        }
        return std::string("other");
    }();
    std::cout << "{\n";
    std::cout << indent << "  \"recordIndex\": " << object.record_index << ",\n";
    std::cout << indent << "  \"deleted\": " << (object.deleted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"containingSectionId\": ";
    print_json_string(object.containing_section_id);
    std::cout << ",\n";
    std::cout << indent << "  \"containingSectionRecordIndex\": ";
    print_json_report_record_index_or_null(object.containing_section_record_index);
    std::cout << ",\n";
    std::cout << indent << "  \"sectionRelativeTop\": " << object.section_relative_top << ",\n";
    std::cout << indent << "  \"sectionRelativeBottom\": " << object.section_relative_bottom << ",\n";
    std::cout << indent << "  \"sectionObjectIndex\": ";
    print_json_report_record_index_or_null(object.section_object_index);
    std::cout << ",\n";
    std::cout << indent << "  \"sectionObjectCount\": " << object.section_object_count << ",\n";
    std::cout << indent << "  \"objectTypeCode\": " << object.objtype_code << ",\n";
    std::cout << indent << "  \"objectTypeFieldIndex\": ";
    print_json_report_field_index_or_null(object.objtype_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"objectTypeMemoBlockNumber\": " << object.objtype_memo_block_number << ",\n";
    std::cout << indent << "  \"objectCode\": " << object.objcode_code << ",\n";
    std::cout << indent << "  \"objectCodeFieldIndex\": ";
    print_json_report_field_index_or_null(object.objcode_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"objectCodeMemoBlockNumber\": " << object.objcode_memo_block_number << ",\n";
    std::cout << indent << "  \"objectKind\": ";
    print_json_string(object.object_kind);
    std::cout << ",\n";
    std::cout << indent << "  \"objectKindFieldIndex\": ";
    print_json_report_field_index_or_null(object.object_kind_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"objectKindMemoBlockNumber\": " << object.object_kind_memo_block_number << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string(object.title);
    std::cout << ",\n";
    std::cout << indent << "  \"titleFieldIndex\": ";
    print_json_report_field_index_or_null(object.title_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"titleMemoBlockNumber\": " << object.title_memo_block_number << ",\n";
    std::cout << indent << "  \"expression\": ";
    print_json_string(object.expression);
    std::cout << ",\n";
    std::cout << indent << "  \"expressionFieldIndex\": ";
    print_json_report_field_index_or_null(object.expression_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"expressionMemoBlockNumber\": " << object.expression_memo_block_number << ",\n";
    std::cout << indent << "  \"picture\": ";
    print_json_string(object.picture);
    std::cout << ",\n";
    std::cout << indent << "  \"pictureFieldIndex\": ";
    print_json_report_field_index_or_null(object.picture_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"pictureMemoBlockNumber\": " << object.picture_memo_block_number << ",\n";
    std::cout << indent << "  \"pictureAlignment\": ";
    print_json_string(picture_alignment);
    std::cout << ",\n";
    std::cout << indent << "  \"left\": " << object.left << ",\n";
    std::cout << indent << "  \"leftFieldIndex\": ";
    print_json_report_field_index_or_null(object.left_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"leftMemoBlockNumber\": " << object.left_memo_block_number << ",\n";
    std::cout << indent << "  \"top\": " << object.top << ",\n";
    std::cout << indent << "  \"topFieldIndex\": ";
    print_json_report_field_index_or_null(object.top_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"topMemoBlockNumber\": " << object.top_memo_block_number << ",\n";
    std::cout << indent << "  \"width\": " << object.width << ",\n";
    std::cout << indent << "  \"widthFieldIndex\": ";
    print_json_report_field_index_or_null(object.width_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"widthMemoBlockNumber\": " << object.width_memo_block_number << ",\n";
    std::cout << indent << "  \"right\": " << object.right << ",\n";
    std::cout << indent << "  \"height\": " << object.height << ",\n";
    std::cout << indent << "  \"heightFieldIndex\": ";
    print_json_report_field_index_or_null(object.height_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"heightMemoBlockNumber\": " << object.height_memo_block_number << ",\n";
    std::cout << indent << "  \"bottom\": " << object.bottom << ",\n";
    std::cout << indent << "  \"highlightCount\": " << object.highlights.size() << ",\n";
    std::cout << indent << "  \"highlights\": ";
    print_json_report_named_values(object.highlights, indent + "  ");
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_report_layout_objects(
    const std::vector<copperfin::studio::StudioLayoutObjectSnapshot>& objects,
    const std::string& indent) {
    std::cout << "[\n";
    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        std::cout << indent << "  ";
        print_json_report_layout_object(objects[object_index], indent + "  ");
        if ((object_index + 1U) != objects.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << indent << "]";
}

void print_json_report_layout_section(
    const copperfin::studio::StudioReportSectionSnapshot& section,
    const std::string& indent) {
    std::cout << "{\n";
    std::cout << indent << "  \"id\": ";
    print_json_string(section.id);
    std::cout << ",\n";
    std::cout << indent << "  \"idFieldIndex\": ";
    print_json_report_field_index_or_null(section.id_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"idMemoBlockNumber\": " << section.id_memo_block_number << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string(section.title);
    std::cout << ",\n";
    std::cout << indent << "  \"titleFieldIndex\": ";
    print_json_report_field_index_or_null(section.title_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"titleMemoBlockNumber\": " << section.title_memo_block_number << ",\n";
    std::cout << indent << "  \"bandKind\": ";
    print_json_string(section.band_kind);
    std::cout << ",\n";
    std::cout << indent << "  \"bandKindFieldIndex\": ";
    print_json_report_field_index_or_null(section.band_kind_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"bandKindMemoBlockNumber\": " << section.band_kind_memo_block_number << ",\n";
    std::cout << indent << "  \"expression\": ";
    print_json_string(section.expression);
    std::cout << ",\n";
    std::cout << indent << "  \"expressionFieldIndex\": ";
    print_json_report_field_index_or_null(section.expression_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"expressionMemoBlockNumber\": " << section.expression_memo_block_number << ",\n";
    std::cout << indent << "  \"comment\": ";
    print_json_string(section.comment);
    std::cout << ",\n";
    std::cout << indent << "  \"commentFieldIndex\": ";
    print_json_report_field_index_or_null(section.comment_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"commentMemoBlockNumber\": " << section.comment_memo_block_number << ",\n";
    std::cout << indent << "  \"userComment\": ";
    print_json_string(section.user_comment);
    std::cout << ",\n";
    std::cout << indent << "  \"userCommentFieldIndex\": ";
    print_json_report_field_index_or_null(section.user_comment_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"userCommentMemoBlockNumber\": " << section.user_comment_memo_block_number << ",\n";
    std::cout << indent << "  \"recordIndex\": " << section.record_index << ",\n";
    std::cout << indent << "  \"deleted\": " << (section.deleted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"sectionIndex\": ";
    print_json_report_record_index_or_null(section.section_index);
    std::cout << ",\n";
    std::cout << indent << "  \"sectionCount\": " << section.section_count << ",\n";
    std::cout << indent << "  \"groupingContextAvailable\": "
              << (section.grouping_context_available ? "true" : "false") << ",\n";
    std::cout << indent << "  \"groupingIndex\": ";
    print_json_report_record_index_or_null(section.grouping_index);
    std::cout << ",\n";
    std::cout << indent << "  \"groupingNestingDepth\": ";
    print_json_report_record_index_or_null(section.grouping_nesting_depth);
    std::cout << ",\n";
    std::cout << indent << "  \"groupRole\": ";
    if (section.grouping_context_available) {
        print_json_string(section.grouping_role);
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << indent << "  \"groupingExpression\": ";
    if (section.grouping_context_available) {
        print_json_string(section.grouping_expression);
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << indent << "  \"groupingExpressionFieldIndex\": ";
    print_json_report_field_index_or_null(section.grouping_expression_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"groupingExpressionMemoBlockNumber\": "
              << section.grouping_expression_memo_block_number << ",\n";
    std::cout << indent << "  \"groupPartnerSectionId\": ";
    if (section.grouping_partner_section_id.empty()) {
        std::cout << "null";
    } else {
        print_json_string(section.grouping_partner_section_id);
    }
    std::cout << ",\n";
    std::cout << indent << "  \"groupPartnerRecordIndex\": ";
    print_json_report_record_index_or_null(section.grouping_partner_record_index);
    std::cout << ",\n";
    std::cout << indent << "  \"groupPartnerDeleted\": " << (section.grouping_partner_deleted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"objectCode\": " << section.objcode_code << ",\n";
    std::cout << indent << "  \"objectCodeFieldIndex\": ";
    print_json_report_field_index_or_null(section.objcode_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"objectCodeMemoBlockNumber\": " << section.objcode_memo_block_number << ",\n";
    std::cout << indent << "  \"top\": " << section.top << ",\n";
    std::cout << indent << "  \"topFieldIndex\": ";
    print_json_report_field_index_or_null(section.top_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"topMemoBlockNumber\": " << section.top_memo_block_number << ",\n";
    std::cout << indent << "  \"height\": " << section.height << ",\n";
    std::cout << indent << "  \"heightFieldIndex\": ";
    print_json_report_field_index_or_null(section.height_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"heightMemoBlockNumber\": " << section.height_memo_block_number << ",\n";
    std::cout << indent << "  \"bottom\": " << section.bottom << ",\n";
    std::cout << indent << "  \"pageBreak\": ";
    print_json_string(section.page_break);
    std::cout << ",\n";
    std::cout << indent << "  \"pageBreakFieldIndex\": ";
    print_json_report_field_index_or_null(section.page_break_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"pageBreakMemoBlockNumber\": " << section.page_break_memo_block_number << ",\n";
    std::cout << indent << "  \"columnBreak\": ";
    print_json_string(section.column_break);
    std::cout << ",\n";
    std::cout << indent << "  \"columnBreakFieldIndex\": ";
    print_json_report_field_index_or_null(section.column_break_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"columnBreakMemoBlockNumber\": " << section.column_break_memo_block_number << ",\n";
    std::cout << indent << "  \"resetPage\": ";
    print_json_string(section.reset_page);
    std::cout << ",\n";
    std::cout << indent << "  \"resetPageFieldIndex\": ";
    print_json_report_field_index_or_null(section.reset_page_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"resetPageMemoBlockNumber\": " << section.reset_page_memo_block_number << ",\n";
    std::cout << indent << "  \"ejectBefore\": ";
    print_json_string(section.eject_before);
    std::cout << ",\n";
    std::cout << indent << "  \"ejectBeforeFieldIndex\": ";
    print_json_report_field_index_or_null(section.eject_before_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"ejectBeforeMemoBlockNumber\": " << section.eject_before_memo_block_number << ",\n";
    std::cout << indent << "  \"ejectAfter\": ";
    print_json_string(section.eject_after);
    std::cout << ",\n";
    std::cout << indent << "  \"ejectAfterFieldIndex\": ";
    print_json_report_field_index_or_null(section.eject_after_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"ejectAfterMemoBlockNumber\": " << section.eject_after_memo_block_number << ",\n";
    std::cout << indent << "  \"plain\": ";
    print_json_string(section.plain);
    std::cout << ",\n";
    std::cout << indent << "  \"plainFieldIndex\": ";
    print_json_report_field_index_or_null(section.plain_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"plainMemoBlockNumber\": " << section.plain_memo_block_number << ",\n";
    std::cout << indent << "  \"onEntryExpression\": ";
    print_json_string(section.on_entry_expression);
    std::cout << ",\n";
    std::cout << indent << "  \"onEntryExpressionFieldIndex\": ";
    print_json_report_field_index_or_null(section.on_entry_expression_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"onEntryExpressionMemoBlockNumber\": " << section.on_entry_expression_memo_block_number << ",\n";
    std::cout << indent << "  \"onExitExpression\": ";
    print_json_string(section.on_exit_expression);
    std::cout << ",\n";
    std::cout << indent << "  \"onExitExpressionFieldIndex\": ";
    print_json_report_field_index_or_null(section.on_exit_expression_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"onExitExpressionMemoBlockNumber\": " << section.on_exit_expression_memo_block_number << ",\n";
    std::cout << indent << "  \"objectCount\": " << section.objects.size() << ",\n";
    std::cout << indent << "  \"deletedObjectCount\": " << section.deleted_object_count << ",\n";
    std::cout << indent << "  \"objects\": ";
    print_json_report_layout_objects(section.objects, indent + "  ");
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_report_layout_sections(
    const std::vector<copperfin::studio::StudioReportSectionSnapshot>& sections,
    const std::string& indent) {
    std::cout << "[\n";
    for (std::size_t section_index = 0; section_index < sections.size(); ++section_index) {
        std::cout << indent << "  ";
        print_json_report_layout_section(sections[section_index], indent + "  ");
        if ((section_index + 1U) != sections.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << indent << "]";
}

void print_json_report_layout_grouping(
    const copperfin::studio::StudioReportGroupingSnapshot& grouping,
    const std::string& indent) {
    std::cout << "{\n";
    std::cout << indent << "  \"groupingIndex\": " << grouping.grouping_index << ",\n";
    std::cout << indent << "  \"nestingDepth\": " << grouping.nesting_depth << ",\n";
    std::cout << indent << "  \"expression\": ";
    print_json_string(grouping.expression);
    std::cout << ",\n";
    std::cout << indent << "  \"expressionFieldIndex\": ";
    print_json_report_field_index_or_null(grouping.expression_field_index);
    std::cout << ",\n";
    std::cout << indent << "  \"expressionMemoBlockNumber\": " << grouping.expression_memo_block_number << ",\n";
    std::cout << indent << "  \"headerSectionId\": ";
    print_json_string(grouping.header_section_id);
    std::cout << ",\n";
    std::cout << indent << "  \"headerRecordIndex\": ";
    print_json_report_record_index_or_null(grouping.header_record_index);
    std::cout << ",\n";
    std::cout << indent << "  \"headerDeleted\": " << (grouping.header_deleted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"footerSectionId\": ";
    print_json_string(grouping.footer_section_id);
    std::cout << ",\n";
    std::cout << indent << "  \"footerRecordIndex\": ";
    print_json_report_record_index_or_null(grouping.footer_record_index);
    std::cout << ",\n";
    std::cout << indent << "  \"footerDeleted\": " << (grouping.footer_deleted ? "true" : "false") << "\n";
    std::cout << indent << "}";
}

void print_json_report_layout_groupings(
    const std::vector<copperfin::studio::StudioReportGroupingSnapshot>& groupings,
    const std::string& indent) {
    std::cout << "[\n";
    for (std::size_t grouping_index = 0; grouping_index < groupings.size(); ++grouping_index) {
        std::cout << indent << "  ";
        print_json_report_layout_grouping(groupings[grouping_index], indent + "  ");
        if ((grouping_index + 1U) != groupings.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << indent << "]";
}

}  // namespace cf_studio_host_main_detail
