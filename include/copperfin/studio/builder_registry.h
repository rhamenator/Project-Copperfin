// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::localization {
struct LocalizedCatalog;
}

namespace copperfin::studio {

enum class StudioBuilderKind {
    builder,
    wizard
};

enum class StudioBuilderContext {
    form,
    class_designer,
    control,
    report,
    label,
    menu,
    project,
    data_environment
};

struct StudioBuilderDescriptor {
    std::string_view id;
    std::string_view title;
    StudioBuilderKind kind = StudioBuilderKind::builder;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string_view vfp9_equivalent;
    std::string_view copperfin_component;
    std::string_view entry_point;
    std::string_view description;
    std::string_view vfp9_equivalent_display;
};

struct StudioBuilderLaunchRequest {
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string builder_id;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct StudioBuilderLaunchPlan {
    StudioBuilderDescriptor builder;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string entry_point;
};

struct StudioBuilderLaunchPlanResult {
    bool ok = false;
    std::string error;
    StudioBuilderLaunchPlan plan;
};

struct StudioBuilderLaunchCatalogRequest {
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct StudioBuilderLaunchCatalogEntry {
    StudioBuilderDescriptor builder;
    StudioBuilderLaunchPlanResult launch_plan;
};

struct StudioBuilderLaunchCatalogResult {
    bool ok = false;
    std::string error;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::size_t builder_count = 0;
    std::size_t launch_plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioBuilderLaunchCatalogEntry> entries;
};

[[nodiscard]] const char* studio_builder_kind_name(StudioBuilderKind kind);
[[nodiscard]] const char* studio_builder_context_name(StudioBuilderContext context);
[[nodiscard]] std::vector<StudioBuilderDescriptor> studio_builder_registry_for_catalog(
    const localization::LocalizedCatalog& catalog);
[[nodiscard]] std::vector<StudioBuilderDescriptor> studio_builder_registry();
[[nodiscard]] std::vector<StudioBuilderDescriptor> studio_builders_for_context(StudioBuilderContext context);
[[nodiscard]] StudioBuilderLaunchPlanResult plan_studio_builder_launch(
    const StudioBuilderLaunchRequest& request);
[[nodiscard]] StudioBuilderLaunchCatalogResult plan_studio_builder_launch_catalog(
    const StudioBuilderLaunchCatalogRequest& request);

}  // namespace copperfin::studio
