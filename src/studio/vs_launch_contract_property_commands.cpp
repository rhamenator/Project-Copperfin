// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_property_commands(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    [[maybe_unused]] std::string& parsed_argument_error) {
if (argument == "--set-property") {
            result.request.apply_property_update = true;
            return {true, false};
        }

if (argument == "--clear-property") {
            result.request.clear_property = true;
            return {true, false};
        }

if (argument == "--rename-property") {
            result.request.rename_property = true;
            return {true, false};
        }

if (argument == "--property-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--property-name")}; return {true, true};
            }
            result.request.property_name = args[++index];
            return {true, false};
        }

if (argument == "--property-value") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--property-value")}; return {true, true};
            }
            result.request.property_value = args[++index];
            return {true, false};
        }

if (argument == "--field-value") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--field-value")}; return {true, true};
            }
            const std::string assignment = args[++index];
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                result = {.ok = false, .error = localized_field_value_name_value_syntax_required(catalog)}; return {true, true};
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
            return {true, false};
        }

if (argument == "--undo-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--undo-mode")}; return {true, true};
            }
            std::string mode = lowercase_copy(args[++index]);
            if (mode == "edit") {
                result.request.undo_mode = StudioUndoMode::edit;
                return {true, false};
            }
            if (mode == "command") {
                result.request.undo_mode = StudioUndoMode::command;
                return {true, false};
            }
            result = {.ok = false, .error = localized_undo_mode_value_required(catalog)}; return {true, true};
        }

if (argument == "--undo-label") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--undo-label")}; return {true, true};
            }
            result.request.undo_label = args[++index];
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio
