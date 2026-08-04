// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::optional<LaunchParseResult> validate_layout_actions(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog) {
if (result.request.align_object && result.request.alignment_mode.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_option(
            catalog,
            localized_layout_action_alignment(catalog),
            "--alignment-mode")};
    }

if (result.request.align_object &&
        result.request.anchor_object_name.empty() &&
        result.request.anchor_unique_id.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_either_option(
            catalog,
            localized_layout_action_alignment(catalog),
            "--anchor-object-name",
            "--anchor-unique-id")};
    }

if (result.request.align_object && result.request.align_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_target(
            catalog,
            localized_layout_action_alignment(catalog))};
    }

if (!result.request.align_object &&
        (!result.request.alignment_mode.empty() ||
         !result.request.align_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            localized_layout_action_alignment_title(catalog),
            "--align-object")};
    }

if (result.request.resize_object && result.request.resize_mode.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_option(
            catalog,
            localized_layout_action_resize(catalog),
            "--resize-mode")};
    }

if (result.request.resize_object &&
        result.request.anchor_object_name.empty() &&
        result.request.anchor_unique_id.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_either_option(
            catalog,
            localized_layout_action_resize(catalog),
            "--anchor-object-name",
            "--anchor-unique-id")};
    }

if (result.request.resize_object && result.request.resize_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_target(
            catalog,
            localized_layout_action_resize(catalog))};
    }

if (!result.request.resize_object &&
        (!result.request.resize_mode.empty() ||
         !result.request.resize_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            localized_layout_action_resize_title(catalog),
            "--resize-object")};
    }

if (result.request.distribute_object && result.request.distribution_mode.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_option(
            catalog,
            localized_layout_action_distribution(catalog),
            "--distribution-mode")};
    }

if (result.request.distribute_object && result.request.distribute_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_target(
            catalog,
            localized_layout_action_distribution(catalog))};
    }

if (!result.request.distribute_object &&
        (!result.request.distribution_mode.empty() ||
         !result.request.distribute_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            localized_layout_action_distribution_title(catalog),
            "--distribute-object")};
    }

if (result.request.snap_object && result.request.snap_mode.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_option(
            catalog,
            localized_layout_action_snap(catalog),
            "--snap-mode")};
    }

if (result.request.snap_object && result.request.snap_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_target(
            catalog,
            localized_layout_action_snap(catalog))};
    }

if (!result.request.snap_object &&
        (!result.request.snap_mode.empty() ||
         result.request.grid_width != 0.0 ||
         result.request.grid_height != 0.0 ||
         !result.request.snap_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            localized_layout_action_snap_title(catalog),
            "--snap-object")};
    }

if (result.request.nudge_object && result.request.nudge_mode.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_option(
            catalog,
            localized_layout_action_nudge(catalog),
            "--nudge-mode")};
    }

if (result.request.nudge_object && result.request.nudge_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_requires_target(
            catalog,
            localized_layout_action_nudge(catalog))};
    }

if (!result.request.nudge_object &&
        (!result.request.nudge_mode.empty() ||
         result.request.delta_hpos != 0.0 ||
         result.request.delta_vpos != 0.0 ||
         !result.request.nudge_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_action_arguments_require_mode(
            catalog,
            localized_layout_action_nudge_title(catalog),
            "--nudge-object")};
    }

if (!result.request.align_object && !result.request.resize_object &&
        (!result.request.anchor_object_name.empty() || !result.request.anchor_unique_id.empty())) {
        return LaunchParseResult{.ok = false, .error = catalog.translate(
            "StudioHost.LaunchParse.Error.AnchorSelectorsRequireAlignOrResize",
            {
                {"alignOption", "--align-object"},
                {"resizeOption", "--resize-object"}
            })};
    }
    return std::nullopt;
}

}  // namespace copperfin::studio
