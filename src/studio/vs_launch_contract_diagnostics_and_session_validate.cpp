// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::optional<LaunchParseResult> validate_diagnostics_and_session(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog) {
if (result.request.path.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_no_asset_path_provided(catalog)};
    }

if (result.request.enabled_object && !result.request.enabled_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Enabled"),
            "--enabled")};
    }

if (result.request.enabled_object && result.request.enabled_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Enabled"))};
    }

if (!result.request.enabled_object &&
        (result.request.enabled_available ||
         !result.request.enabled_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.EnabledTitle"),
            "--enabled-object")};
    }

if (result.request.read_only_object && !result.request.object_read_only_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ReadOnly"),
            "--object-read-only")};
    }

if (result.request.read_only_object && result.request.read_only_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ReadOnly"))};
    }

if (!result.request.read_only_object &&
        (result.request.object_read_only_available ||
         !result.request.read_only_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ReadOnlyTitle"),
            "--read-only-object")};
    }

if (result.request.locked_object && !result.request.locked_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Locked"),
            "--locked")};
    }

if (result.request.locked_object && result.request.locked_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Locked"))};
    }

if (!result.request.locked_object &&
        (result.request.locked_available ||
         !result.request.locked_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockedTitle"),
            "--locked-object")};
    }
    return std::nullopt;
}

}  // namespace copperfin::studio
