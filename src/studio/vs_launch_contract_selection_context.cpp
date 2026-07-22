// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_selection_context(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    [[maybe_unused]] std::string& parsed_argument_error) {
if (argument == "--clear-parent") {
            result.request.clear_parent = true;
            return {true, false};
        }

if (argument == "--selection-context") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_selection_context_error(catalog)}; return {true, true};
            }
            const auto selection_context = parse_selection_context_token(args[++index]);
            if (!selection_context.has_value()) {
                result = {.ok = false, .error = localized_selection_context_error(catalog)}; return {true, true};
            }
            result.request.designer_selection_contexts.push_back(*selection_context);
            return {true, false};
        }

if (argument == "--new-property-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--new-property-name")}; return {true, true};
            }
            result.request.new_property_name = args[++index];
            return {true, false};
        }

if (argument == "--new-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--new-object-name")}; return {true, true};
            }
            result.request.new_object_name = args[++index];
            return {true, false};
        }

if (argument == "--new-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--new-name")}; return {true, true};
            }
            result.request.new_name = args[++index];
            return {true, false};
        }

if (argument == "--new-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--new-unique-id")}; return {true, true};
            }
            result.request.new_unique_id = args[++index];
            return {true, false};
        }

if (argument == "--parent-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--parent-name")}; return {true, true};
            }
            result.request.parent_name = args[++index];
            return {true, false};
        }

if (argument == "--parent-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--parent-unique-id")}; return {true, true};
            }
            result.request.parent_unique_id = args[++index];
            return {true, false};
        }

if (argument == "--target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--target-object-name")}; return {true, true};
            }
            result.request.target_object_name = args[++index];
            return {true, false};
        }

if (argument == "--target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--target-unique-id")}; return {true, true};
            }
            result.request.target_unique_id = args[++index];
            return {true, false};
        }

if (argument == "--group-child-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--group-child-object-name")}; return {true, true};
            }
            result.request.group_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--group-child-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--group-child-unique-id")}; return {true, true};
            }
            result.request.group_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--object-name")}; return {true, true};
            }
            result.request.object_name = args[++index];
            return {true, false};
        }

if (argument == "--unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--unique-id")}; return {true, true};
            }
            result.request.unique_id = args[++index];
            return {true, false};
        }

if (argument == "--line") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--line")}; return {true, true};
            }
            std::size_t line = 0;
            if (!parse_size_value(args[++index], line)) {
                result = {.ok = false, .error = localized_unsigned_integer_value_required(catalog, "--line")}; return {true, true};
            }
            result.request.line = line;
            return {true, false};
        }

if (argument == "--column") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column")}; return {true, true};
            }
            std::size_t column = 0;
            if (!parse_size_value(args[++index], column)) {
                result = {.ok = false, .error = localized_unsigned_integer_value_required(catalog, "--column")}; return {true, true};
            }
            result.request.column = column;
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio
