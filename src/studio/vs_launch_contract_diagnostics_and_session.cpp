// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_diagnostics_and_session(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    [[maybe_unused]] std::string& parsed_argument_error) {
if (argument == "--help" || argument == "-h" || argument == "/?") {
            result.ok = true;
            result.show_help = true;
            return {true, true};
        }

if (argument == "--from-vs") {
            result.request.launched_from_visual_studio = true;
            return {true, false};
        }

if (argument == "--read-only") {
            result.request.read_only = true;
            return {true, false};
        }

if (argument == "--json") {
            result.output_json = true;
            return {true, false};
        }

if (argument == "--path") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--path")}; return {true, true};
            }
            result.request.path = args[++index];
            return {true, false};
        }

if (argument == "--symbol") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--symbol")}; return {true, true};
            }
            result.request.symbol = args[++index];
            return {true, false};
        }

if (argument == "--record") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record")}; return {true, true};
            }
            std::size_t record_index = 0;
            if (!parse_size_value(args[++index], record_index)) {
                result = {.ok = false, .error = localized_unsigned_integer_value_required(catalog, "--record")}; return {true, true};
            }
            result.request.record_index = record_index;
            result.request.selection_record_available = true;
            return {true, false};
        }

if (argument == "--visible") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--visible")}; return {true, true};
            }
            const auto visible = parse_bool_value(args[++index]);
            if (!visible.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--visible")}; return {true, true};
            }
            result.request.visible = *visible;
            result.request.visible_available = true;
            return {true, false};
        }

if (argument == "--enabled") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--enabled")}; return {true, true};
            }
            const auto enabled = parse_bool_value(args[++index]);
            if (!enabled.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--enabled")}; return {true, true};
            }
            result.request.enabled = *enabled;
            result.request.enabled_available = true;
            return {true, false};
        }

if (argument == "--object-read-only") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--object-read-only")}; return {true, true};
            }
            const auto object_read_only = parse_bool_value(args[++index]);
            if (!object_read_only.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--object-read-only")}; return {true, true};
            }
            result.request.object_read_only = *object_read_only;
            result.request.object_read_only_available = true;
            return {true, false};
        }

if (argument == "--locked") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--locked")}; return {true, true};
            }
            const auto locked = parse_bool_value(args[++index]);
            if (!locked.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--locked")}; return {true, true};
            }
            result.request.locked = *locked;
            result.request.locked_available = true;
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio
