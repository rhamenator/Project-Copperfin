// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::optional<LaunchParseResult> validate_property_commands(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog) {
if (result.request.apply_property_update && result.request.property_name.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_property_command_requires_option(
            catalog,
            localized_property_command_update(catalog),
            "--property-name")};
    }

if (result.request.clear_property && result.request.property_name.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_property_command_requires_option(
            catalog,
            localized_property_command_clear(catalog),
            "--property-name")};
    }

if (result.request.rename_property && result.request.property_name.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_property_command_requires_option(
            catalog,
            localized_property_command_rename(catalog),
            "--property-name")};
    }

if (result.request.rename_property && result.request.new_property_name.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_property_command_requires_option(
            catalog,
            localized_property_command_rename(catalog),
            "--new-property-name")};
    }
    return std::nullopt;
}

}  // namespace copperfin::studio
