// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_object_lifecycle(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    [[maybe_unused]] std::string& parsed_argument_error) {
if (argument == "--delete-object") {
            result.request.delete_object = true;
            return {true, false};
        }

if (argument == "--restore-object") {
            result.request.restore_object = true;
            return {true, false};
        }

if (argument == "--deleted-states") {
            result.request.deleted_states = true;
            return {true, false};
        }

if (argument == "--subtree-deleted-state") {
            result.request.subtree_deleted_state = true;
            return {true, false};
        }

if (argument == "--duplicate-object") {
            result.request.duplicate_object = true;
            return {true, false};
        }

if (argument == "--rename-object") {
            result.request.rename_object = true;
            return {true, false};
        }

if (argument == "--reparent-object") {
            result.request.reparent_object = true;
            return {true, false};
        }

if (argument == "--reorder-object") {
            result.request.reorder_object = true;
            return {true, false};
        }

if (argument == "--group-object") {
            result.request.group_object = true;
            return {true, false};
        }

if (argument == "--ungroup-object") {
            result.request.ungroup_object = true;
            return {true, false};
        }

if (argument == "--placement") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--placement")}; return {true, true};
            }
            result.request.placement = args[++index];
            return {true, false};
        }

if (argument == "--deleted-state-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--deleted-state-target-object-name")}; return {true, true};
            }
            result.request.deleted_state_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {},
                .deleted = false,
                .deleted_available = false
            });
            return {true, false};
        }

if (argument == "--deleted-state-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--deleted-state-target-unique-id")}; return {true, true};
            }
            result.request.deleted_state_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index],
                .deleted = false,
                .deleted_available = false
            });
            return {true, false};
        }

if (argument == "--deleted-state") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--deleted-state")}; return {true, true};
            }
            const auto deleted_state = parse_bool_value(args[++index]);
            if (!deleted_state.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--deleted-state")}; return {true, true};
            }
            auto pending = std::find_if(
                result.request.deleted_state_objects.rbegin(),
                result.request.deleted_state_objects.rend(),
                [](const StudioDeletedStateSelector& object) {
                    return !object.deleted_available;
                });
            if (pending == result.request.deleted_state_objects.rend()) {
                result = {.ok = false, .error = localized_deleted_state_requires_target_selector(catalog)}; return {true, true};
            }
            pending->deleted = *deleted_state;
            pending->deleted_available = true;
            return {true, false};
        }

if (argument == "--subtree-deleted") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--subtree-deleted")}; return {true, true};
            }
            const auto subtree_deleted = parse_bool_value(args[++index]);
            if (!subtree_deleted.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--subtree-deleted")}; return {true, true};
            }
            result.request.subtree_deleted = *subtree_deleted;
            result.request.subtree_deleted_available = true;
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio
