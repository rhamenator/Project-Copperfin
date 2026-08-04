// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/studio/builder_invocation_admission.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioBuilderDispatchRequest {
    StudioBuilderInvocationAdmissionPlan admission_plan;
};

struct StudioBuilderDispatchPlan {
    StudioBuilderDescriptor builder;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string command_token;
    std::string entry_point;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::vector<std::string> dispatch_arguments;
    bool dispatch_admitted = false;
    bool dry_run = true;
    bool executed = false;
    bool mutates_asset = false;
};

struct StudioBuilderDispatchResult {
    bool ok = false;
    std::string error;
    StudioBuilderDispatchPlan plan;
};

struct StudioBuilderDispatchCatalogRequest {
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_ui_launches = false;
};

struct StudioBuilderDispatchCatalogEntry {
    StudioBuilderDescriptor builder;
    StudioBuilderLaunchPlanResult launch_plan;
    StudioBuilderInvocationAdmissionResult invocation_admission;
    StudioBuilderDispatchResult dispatch;
};

struct StudioBuilderDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::size_t builder_count = 0;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioBuilderDispatchCatalogEntry> entries;
};

struct StudioBuilderDispatchExecutionObservation {
    bool launched = false;
    int exit_code = 0;
    std::string output;
    std::string error;
    bool mutates_asset = false;
};

using StudioBuilderDispatchExecutor =
    std::function<StudioBuilderDispatchExecutionObservation(const StudioBuilderDispatchPlan&)>;

struct StudioBuilderDispatchExecutionRequest {
    StudioBuilderDispatchPlan dispatch_plan;
    bool admit_execution = false;
    StudioBuilderDispatchExecutor executor;
};

struct StudioBuilderDispatchExecutionResult {
    bool ok = false;
    std::string error;
    StudioBuilderDispatchPlan dispatch_plan;
    StudioBuilderDispatchExecutionObservation observation;
    bool execution_admitted = false;
    bool executed = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioBuilderDispatchExecutionCatalogRequest {
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_ui_launches = false;
    bool admit_execution = false;
};

struct StudioBuilderDispatchExecutionCatalogEntry {
    StudioBuilderDescriptor builder;
    StudioBuilderLaunchPlanResult launch_plan;
    StudioBuilderInvocationAdmissionResult invocation_admission;
    StudioBuilderDispatchResult dispatch;
    bool execution_admitted = false;
    bool execution_ready = false;
    std::string execution_error;
};

struct StudioBuilderDispatchExecutionCatalogResult {
    bool ok = false;
    std::string error;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::size_t builder_count = 0;
    std::size_t execution_ready_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioBuilderDispatchExecutionCatalogEntry> entries;
};

[[nodiscard]] StudioBuilderDispatchResult plan_studio_builder_dispatch(
    const StudioBuilderDispatchRequest& request);
[[nodiscard]] StudioBuilderDispatchCatalogResult plan_studio_builder_dispatch_catalog(
    const StudioBuilderDispatchCatalogRequest& request);
[[nodiscard]] StudioBuilderDispatchExecutionResult execute_studio_builder_dispatch(
    const StudioBuilderDispatchExecutionRequest& request);
[[nodiscard]] StudioBuilderDispatchExecutionCatalogResult plan_studio_builder_dispatch_execution_catalog(
    const StudioBuilderDispatchExecutionCatalogRequest& request);

}  // namespace copperfin::studio
