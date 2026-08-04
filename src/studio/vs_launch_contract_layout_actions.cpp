// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_layout_actions(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    [[maybe_unused]] std::string& parsed_argument_error) {
if (argument == "--align-object") {
            result.request.align_object = true;
            return {true, false};
        }

if (argument == "--resize-object") {
            result.request.resize_object = true;
            return {true, false};
        }

if (argument == "--distribute-object") {
            result.request.distribute_object = true;
            return {true, false};
        }

if (argument == "--snap-object") {
            result.request.snap_object = true;
            return {true, false};
        }

if (argument == "--nudge-object") {
            result.request.nudge_object = true;
            return {true, false};
        }

if (argument == "--alignment-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--alignment-mode")}; return {true, true};
            }
            result.request.alignment_mode = args[++index];
            return {true, false};
        }

if (argument == "--resize-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--resize-mode")}; return {true, true};
            }
            result.request.resize_mode = args[++index];
            return {true, false};
        }

if (argument == "--distribution-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--distribution-mode")}; return {true, true};
            }
            result.request.distribution_mode = args[++index];
            return {true, false};
        }

if (argument == "--snap-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--snap-mode")}; return {true, true};
            }
            result.request.snap_mode = args[++index];
            return {true, false};
        }

if (argument == "--nudge-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--nudge-mode")}; return {true, true};
            }
            result.request.nudge_mode = args[++index];
            return {true, false};
        }

if (argument == "--grid-width") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-width")}; return {true, true};
            }
            double grid_width = 0.0;
            if (!parse_double_value(args[++index], grid_width)) {
                result = {.ok = false, .error = localized_numeric_value_required(catalog, "--grid-width")}; return {true, true};
            }
            result.request.grid_width = grid_width;
            return {true, false};
        }

if (argument == "--grid-height") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-height")}; return {true, true};
            }
            double grid_height = 0.0;
            if (!parse_double_value(args[++index], grid_height)) {
                result = {.ok = false, .error = localized_numeric_value_required(catalog, "--grid-height")}; return {true, true};
            }
            result.request.grid_height = grid_height;
            return {true, false};
        }

if (argument == "--delta-hpos") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--delta-hpos")}; return {true, true};
            }
            double delta_hpos = 0.0;
            if (!parse_double_value(args[++index], delta_hpos)) {
                result = {.ok = false, .error = localized_numeric_value_required(catalog, "--delta-hpos")}; return {true, true};
            }
            result.request.delta_hpos = delta_hpos;
            return {true, false};
        }

if (argument == "--delta-vpos") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--delta-vpos")}; return {true, true};
            }
            double delta_vpos = 0.0;
            if (!parse_double_value(args[++index], delta_vpos)) {
                result = {.ok = false, .error = localized_numeric_value_required(catalog, "--delta-vpos")}; return {true, true};
            }
            result.request.delta_vpos = delta_vpos;
            return {true, false};
        }

if (argument == "--starting-tab-index") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--starting-tab-index")}; return {true, true};
            }
            int starting_tab_index = 0;
            if (!parse_int_value(args[++index], starting_tab_index)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--starting-tab-index")}; return {true, true};
            }
            result.request.starting_tab_index = starting_tab_index;
            result.request.starting_tab_index_available = true;
            return {true, false};
        }

if (argument == "--align-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--align-target-object-name")}; return {true, true};
            }
            result.request.align_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--align-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--align-target-unique-id")}; return {true, true};
            }
            result.request.align_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--resize-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--resize-target-object-name")}; return {true, true};
            }
            result.request.resize_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--resize-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--resize-target-unique-id")}; return {true, true};
            }
            result.request.resize_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--distribute-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--distribute-target-object-name")}; return {true, true};
            }
            result.request.distribute_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--distribute-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--distribute-target-unique-id")}; return {true, true};
            }
            result.request.distribute_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--snap-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--snap-target-object-name")}; return {true, true};
            }
            result.request.snap_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--snap-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--snap-target-unique-id")}; return {true, true};
            }
            result.request.snap_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--nudge-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--nudge-target-object-name")}; return {true, true};
            }
            result.request.nudge_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--nudge-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--nudge-target-unique-id")}; return {true, true};
            }
            result.request.nudge_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio
