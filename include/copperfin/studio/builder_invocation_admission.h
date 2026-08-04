// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/studio/builder_registry.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioBuilderInvocationAdmissionRequest {
    StudioBuilderLaunchPlan launch_plan;
    bool admit_ui_launch = false;
};

struct StudioBuilderInvocationAdmissionPlan {
    StudioBuilderDescriptor builder;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string command_token;
    std::string entry_point;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool ui_launch_admitted = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioBuilderInvocationAdmissionResult {
    bool ok = false;
    std::string error;
    StudioBuilderInvocationAdmissionPlan plan;
};

struct StudioBuilderInvocationAdmissionCatalogRequest {
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_ui_launches = false;
};

struct StudioBuilderInvocationAdmissionCatalogEntry {
    StudioBuilderDescriptor builder;
    StudioBuilderLaunchPlanResult launch_plan;
    StudioBuilderInvocationAdmissionResult invocation_admission;
};

struct StudioBuilderInvocationAdmissionCatalogResult {
    bool ok = false;
    std::string error;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::size_t builder_count = 0;
    std::size_t admission_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioBuilderInvocationAdmissionCatalogEntry> entries;
};

[[nodiscard]] StudioBuilderInvocationAdmissionResult plan_studio_builder_invocation_admission(
    const StudioBuilderInvocationAdmissionRequest& request);
[[nodiscard]] StudioBuilderInvocationAdmissionCatalogResult plan_studio_builder_invocation_admission_catalog(
    const StudioBuilderInvocationAdmissionCatalogRequest& request);

}  // namespace copperfin::studio
