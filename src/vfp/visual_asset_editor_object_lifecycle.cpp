// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "visual_asset_editor_support.h"

#include "copperfin/platform/path.h"

namespace copperfin::vfp {
namespace {

struct VisualAssetRollbackSnapshot {
    std::vector<std::uint8_t> table_bytes;
    std::string memo_path;
    std::vector<std::uint8_t> memo_bytes;
};

VisualAssetEditResult capture_visual_asset_rollback_snapshot(
    const std::string& path,
    VisualAssetRollbackSnapshot& snapshot) {
    const auto recovery_result = recover_visual_asset_storage_transaction(path);
    if (!recovery_result.ok) {
        return recovery_result;
    }

    snapshot.table_bytes = read_binary_file(path);
    if (snapshot.table_bytes.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }

    const auto memo_resolution_result = resolve_visual_asset_storage_memo_path(
        path,
        snapshot.memo_path);
    if (!memo_resolution_result.ok) {
        return memo_resolution_result;
    }
    if (!snapshot.memo_path.empty()) {
        snapshot.memo_bytes = read_binary_file(snapshot.memo_path);
        if (snapshot.memo_bytes.empty()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.MemoSidecarOpenFailed")};
        }
    }
    return {.ok = true, .error = {}};
}

VisualAssetEditResult restore_visual_asset_rollback_snapshot(
    const std::string& path,
    const VisualAssetRollbackSnapshot& snapshot) {
    if (snapshot.memo_path.empty()) {
        return write_visual_asset_table_transaction(path, snapshot.table_bytes);
    }
    return write_visual_asset_file_transaction(
        path,
        snapshot.table_bytes,
        snapshot.memo_path,
        snapshot.memo_bytes);
}

}  // namespace

VisualObjectListResult list_visual_objects(const std::string& path) {
    if (path.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"),
            .objects = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .objects = {}
        };
    }

    std::vector<VisualObjectSnapshot> objects;
    objects.reserve(table_result.table.records.size());
    for (const auto& record : table_result.table.records) {
        objects.push_back(build_visual_object_snapshot(record, table_result.table));
    }

    return {
        .ok = true,
        .error = {},
        .objects = std::move(objects)
    };
}

VisualObjectChildrenListResult list_visual_object_children(const VisualObjectChildrenListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"),
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }

    std::size_t parent_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, parent_record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }
    if (parent_record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Object.ParentRecordUnavailable"),
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }

    const std::string parent_name = visual_object_record_name(table_result.table.records[parent_record_index]);
    if (parent_name.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Object.ParentNameMissing"),
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }

    std::vector<VisualObjectSnapshot> children;
    const std::string normalized_parent_name = normalize_visual_object_name(parent_name);
    for (const auto& record : table_result.table.records) {
        const auto* record_parent = find_record_value(record, "PARENT");
        if (record_parent == nullptr) {
            continue;
        }
        if (normalize_visual_object_name(record_parent->display_value) == normalized_parent_name) {
            children.push_back(build_visual_object_snapshot(record, table_result.table));
        }
    }

    return {
        .ok = true,
        .error = {},
        .parent_record_index = parent_record_index,
        .parent_name = parent_name,
        .children = std::move(children)
    };
}

VisualObjectDescendantsListResult list_visual_object_descendants(const VisualObjectDescendantsListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"),
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }

    std::size_t parent_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, parent_record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }
    if (parent_record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Object.ParentRecordUnavailable"),
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }

    const auto& table = table_result.table;
    const std::string parent_name = visual_object_record_name(table.records[parent_record_index]);
    if (parent_name.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Object.ParentNameMissing"),
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }

    std::vector<VisualObjectDescendantSnapshot> descendants;
    std::vector<bool> visited(table.records.size(), false);
    visited[parent_record_index] = true;

    std::function<void(const std::string&, std::size_t)> append_descendants =
        [&](const std::string& current_parent_name, std::size_t depth) {
            const std::string normalized_parent_name = normalize_visual_object_name(current_parent_name);
            if (normalized_parent_name.empty()) {
                return;
            }

            for (std::size_t record_index = 0U; record_index < table.records.size(); ++record_index) {
                if (visited[record_index]) {
                    continue;
                }
                const auto* record_parent = find_record_value(table.records[record_index], "PARENT");
                if (record_parent == nullptr ||
                    normalize_visual_object_name(record_parent->display_value) != normalized_parent_name) {
                    continue;
                }

                visited[record_index] = true;
                VisualObjectSnapshot snapshot = build_visual_object_snapshot(table.records[record_index], table);
                descendants.push_back({
                    .object = snapshot,
                    .depth = depth
                });
                append_descendants(snapshot.object_name, depth + 1U);
            }
        };

    append_descendants(parent_name, 1U);

    return {
        .ok = true,
        .error = {},
        .parent_record_index = parent_record_index,
        .parent_name = parent_name,
        .descendants = std::move(descendants)
    };
}

VisualObjectAncestorsListResult list_visual_object_ancestors(const VisualObjectAncestorsListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"),
            .record_index = 0U,
            .ancestors = {}
        };
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .record_index = 0U,
            .ancestors = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .record_index = 0U,
            .ancestors = {}
        };
    }
    const auto& table = table_result.table;
    if (record_index >= table.records.size()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable"),
            .record_index = 0U,
            .ancestors = {}
        };
    }

    std::vector<VisualObjectAncestorSnapshot> ancestors;
    std::vector<bool> visited(table.records.size(), false);
    std::size_t current_record_index = record_index;
    visited[current_record_index] = true;

    for (std::size_t depth = 1U; depth <= table.records.size(); ++depth) {
        const auto* parent_value = find_record_value(table.records[current_record_index], "PARENT");
        const std::string parent_name = parent_value == nullptr ? std::string{} : trim_both(parent_value->display_value);
        if (parent_name.empty()) {
            break;
        }

        std::vector<std::size_t> parent_matches;
        const std::string normalized_parent_name = normalize_visual_object_name(parent_name);
        for (const auto& record : table.records) {
            if (normalize_visual_object_name(visual_object_record_name(record)) == normalized_parent_name) {
                parent_matches.push_back(record.record_index);
            }
        }
        if (parent_matches.empty()) {
            break;
        }
        if (parent_matches.size() > 1U) {
            return {
                .ok = false,
                .error = visual_asset_text("VisualAssetEditor.Object.ParentNameAmbiguous"),
                .record_index = 0U,
                .ancestors = {}
            };
        }
        const std::size_t parent_record_index = parent_matches.front();
        if (parent_record_index >= visited.size() || visited[parent_record_index]) {
            return {
                .ok = false,
                .error = visual_asset_text("VisualAssetEditor.Object.ParentChainCycle"),
                .record_index = 0U,
                .ancestors = {}
            };
        }

        visited[parent_record_index] = true;
        ancestors.push_back({
            .object = build_visual_object_snapshot(table.records[parent_record_index], table),
            .depth = depth
        });
        current_record_index = parent_record_index;
    }

    return {
        .ok = true,
        .error = {},
        .record_index = record_index,
        .ancestors = std::move(ancestors)
    };
}

VisualObjectMethodListResult list_visual_object_methods(const VisualObjectMethodListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"),
            .record_index = 0U,
            .record_deleted = false,
            .methods = {}
        };
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .record_index = 0U,
            .record_deleted = false,
            .methods = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .record_index = 0U,
            .record_deleted = false,
            .methods = {}
        };
    }
    if (record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable"),
            .record_index = 0U,
            .record_deleted = false,
            .methods = {}
        };
    }

    const auto& record = table_result.table.records[record_index];
    const auto* methods_field = find_record_value(record, "METHODS");
    std::vector<VisualObjectMethodSnapshot> methods;
    if (methods_field != nullptr && !trim_both(methods_field->display_value).empty()) {
        methods = parse_visual_methods_blob(
            methods_field->display_value,
            methods_field->memo_block_number);
    }

    return {
        .ok = true,
        .error = {},
        .record_index = record_index,
        .record_deleted = record.deleted,
        .methods = std::move(methods)
    };
}

VisualObjectMethodQueryResult query_visual_object_method(const VisualObjectMethodQueryRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"),
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }
    if (trim_both(request.method_name).empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Method.NameRequired"),
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }
    if (record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable"),
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }

    const auto& record = table_result.table.records[record_index];
    const auto* methods_field = find_record_value(record, "METHODS");
    if (methods_field == nullptr) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Object.SelectedMemoFieldMissing", {{"fieldName", "METHODS"}}),
            .exists = false,
            .record_index = record_index,
            .record_deleted = record.deleted,
            .method = {}
        };
    }

    const std::vector<VisualObjectMethodSnapshot> methods = parse_visual_methods_blob(
        methods_field->display_value,
        methods_field->memo_block_number);
    const std::string normalized_method_name = normalize_visual_object_name(request.method_name);
    std::vector<VisualObjectMethodSnapshot> matches;
    for (const auto& method : methods) {
        if (normalize_visual_object_name(method.method_name) == normalized_method_name) {
            matches.push_back(method);
        }
    }
    if (matches.size() > 1U) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Method.Ambiguous"),
            .exists = false,
            .record_index = record_index,
            .record_deleted = record.deleted,
            .method = {}
        };
    }
    if (matches.empty()) {
        return {
            .ok = true,
            .error = {},
            .exists = false,
            .record_index = record_index,
            .record_deleted = record.deleted,
            .method = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .exists = true,
        .record_index = record_index,
        .record_deleted = record.deleted,
        .method = matches.front()
    };
}

VisualAssetEditResult update_visual_object_method(const VisualObjectMethodEditRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (trim_both(request.method_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    const auto* methods_field = find_record_value(table_result.table.records[record_index], "METHODS");
    if (methods_field == nullptr) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedMemoFieldMissing", {{"fieldName", "METHODS"}})};
    }

    const std::string updated_blob = update_visual_methods_blob(
        methods_field->display_value,
        request.method_name,
        request.method_kind,
        request.source_text);

    auto update_result = update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "METHODS",
        .property_value = updated_blob
    });
    if (update_result.ok) {
        update_result.affected_object_count = 1U;
    }
    return update_result;
}

VisualAssetEditResult delete_visual_object_method(const VisualObjectMethodDeleteRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (trim_both(request.method_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    const auto* methods_field = find_record_value(table_result.table.records[record_index], "METHODS");
    if (methods_field == nullptr) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedMemoFieldMissing", {{"fieldName", "METHODS"}})};
    }

    const std::string normalized_method_name = normalize_visual_object_name(request.method_name);
    const auto methods = parse_visual_methods_blob(methods_field->display_value, 0U);
    const auto matching_count = std::count_if(methods.begin(), methods.end(), [&](const VisualObjectMethodSnapshot& method) {
        return normalize_visual_object_name(method.method_name) == normalized_method_name;
    });
    if (matching_count == 0) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NotFound")};
    }
    if (matching_count > 1) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.Ambiguous")};
    }

    const auto [deleted, updated_blob] = delete_visual_method_from_blob(
        methods_field->display_value,
        request.method_name);
    if (!deleted) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NotFound")};
    }

    auto update_result = update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "METHODS",
        .property_value = updated_blob
    });
    if (update_result.ok) {
        update_result.affected_object_count = 1U;
    }
    return update_result;
}

VisualAssetEditResult delete_visual_object_methods(const VisualObjectMethodDeleteBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.DeleteBatchRequired")};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_deletes = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.method_name).empty()) {
            const auto rollback_result = rollback_batch_deletes();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Method.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
        }

        const auto result = delete_visual_object_method({
            .path = request.path,
            .record_index = method.record_index,
            .object_name = method.object_name,
            .unique_id = method.unique_id,
            .method_name = method.method_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_deletes();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.methods.size()};
}

VisualAssetEditResult rename_visual_object_method(const VisualObjectMethodRenameRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (trim_both(request.method_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
    }
    if (trim_both(request.new_method_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetNameRequired")};
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    const auto* methods_field = find_record_value(table_result.table.records[record_index], "METHODS");
    if (methods_field == nullptr) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedMemoFieldMissing", {{"fieldName", "METHODS"}})};
    }

    std::string updated_blob;
    const auto rename_result = rename_visual_method_in_blob(
        methods_field->display_value,
        request.method_name,
        request.new_method_name,
        updated_blob);
    if (!rename_result.ok) {
        return rename_result;
    }

    auto update_result = update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "METHODS",
        .property_value = updated_blob
    });
    if (update_result.ok) {
        update_result.affected_object_count = 1U;
    }
    return update_result;
}

VisualAssetEditResult rename_visual_object_methods(const VisualObjectMethodRenameBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.RenameBatchRequired")};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_renames = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.method_name).empty()) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Method.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
        }
        if (trim_both(method.new_method_name).empty()) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Method.TargetNameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetNameRequired")};
        }

        const auto result = rename_visual_object_method({
            .path = request.path,
            .record_index = method.record_index,
            .object_name = method.object_name,
            .unique_id = method.unique_id,
            .method_name = method.method_name,
            .new_method_name = method.new_method_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.methods.size()};
}

VisualAssetEditResult copy_visual_object_method(const VisualObjectMethodCopyRequest& request) {
    if (!request.target_method_name.empty() && trim_both(request.target_method_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetNameRequired")};
    }

    const auto source_method = query_visual_object_method({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .method_name = request.source_method_name
    });
    if (!source_method.ok) {
        return {.ok = false, .error = source_method.error};
    }
    if (!source_method.exists) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.SourceNotFound")};
    }

    const std::string target_method_name = request.target_method_name.empty()
        ? source_method.method.method_name
        : trim_both(request.target_method_name);
    const auto target_method = query_visual_object_method({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .method_name = target_method_name
    });
    if (!target_method.ok) {
        return {.ok = false, .error = target_method.error};
    }
    if (target_method.exists && !request.replace_existing) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetObjectAlreadyHasMethod")};
    }

    return update_visual_object_method({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .method_name = target_method_name,
        .method_kind = target_method.exists ? target_method.method.kind : source_method.method.kind,
        .source_text = source_method.method.source_text
    });
}

VisualAssetEditResult copy_visual_object_methods(const VisualObjectMethodCopyBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.CopyBatchRequired")};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_copies = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.source_method_name).empty()) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Method.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
        }
        if (!method.target_method_name.empty() && trim_both(method.target_method_name).empty()) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Method.TargetNameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetNameRequired")};
        }

        const auto result = copy_visual_object_method({
            .path = request.path,
            .source_record_index = method.source_record_index,
            .source_object_name = method.source_object_name,
            .source_unique_id = method.source_unique_id,
            .source_method_name = method.source_method_name,
            .target_record_index = method.target_record_index,
            .target_object_name = method.target_object_name,
            .target_unique_id = method.target_unique_id,
            .target_method_name = method.target_method_name,
            .replace_existing = method.replace_existing
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.methods.size()};
}

VisualAssetEditResult move_visual_object_method(const VisualObjectMethodMoveRequest& request) {
    if (!request.target_method_name.empty() && trim_both(request.target_method_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetNameRequired")};
    }

    const auto source_method = query_visual_object_method({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .method_name = request.source_method_name
    });
    if (!source_method.ok) {
        return {.ok = false, .error = source_method.error};
    }
    if (!source_method.exists) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.SourceNotFound")};
    }

    const std::string target_method_name = request.target_method_name.empty()
        ? source_method.method.method_name
        : trim_both(request.target_method_name);
    const auto target_method = query_visual_object_method({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .method_name = target_method_name
    });
    if (!target_method.ok) {
        return {.ok = false, .error = target_method.error};
    }
    if (target_method.record_index == source_method.record_index &&
        normalize_visual_object_name(target_method_name) == normalize_visual_object_name(source_method.method.method_name)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.SourceMoveToSelf")};
    }
    if (target_method.exists && !request.replace_existing) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetObjectAlreadyHasMethod")};
    }

    const auto copy_result = copy_visual_object_method({
        .path = request.path,
        .source_record_index = request.source_record_index,
        .source_object_name = request.source_object_name,
        .source_unique_id = request.source_unique_id,
        .source_method_name = request.source_method_name,
        .target_record_index = request.target_record_index,
        .target_object_name = request.target_object_name,
        .target_unique_id = request.target_unique_id,
        .target_method_name = request.target_method_name,
        .replace_existing = request.replace_existing
    });
    if (!copy_result.ok) {
        return copy_result;
    }

    const auto delete_result = delete_visual_object_method({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .method_name = request.source_method_name
    });
    if (!delete_result.ok) {
        const auto rollback_result = undo_visual_object_property(request.path);
        if (!rollback_result.ok) {
            return {.ok = false, .error = visual_asset_target_rollback_failed_text(delete_result.error, rollback_result.error)};
        }
        return {.ok = false, .error = delete_result.error};
    }

    return {.ok = true, .error = {}, .affected_object_count = 1U};
}

VisualAssetEditResult move_visual_object_methods(const VisualObjectMethodMoveBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.MoveBatchRequired")};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_moves = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.source_method_name).empty()) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Method.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
        }
        if (!method.target_method_name.empty() && trim_both(method.target_method_name).empty()) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Method.TargetNameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetNameRequired")};
        }

        const auto result = move_visual_object_method({
            .path = request.path,
            .source_record_index = method.source_record_index,
            .source_object_name = method.source_object_name,
            .source_unique_id = method.source_unique_id,
            .source_method_name = method.source_method_name,
            .target_record_index = method.target_record_index,
            .target_object_name = method.target_object_name,
            .target_unique_id = method.target_unique_id,
            .target_method_name = method.target_method_name,
            .replace_existing = method.replace_existing
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.methods.size()};
}

VisualAssetEditResult reorder_visual_object_method(const VisualObjectMethodReorderRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (trim_both(request.method_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    const auto* methods_field = find_record_value(table_result.table.records[record_index], "METHODS");
    if (methods_field == nullptr) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedMemoFieldMissing", {{"fieldName", "METHODS"}})};
    }

    std::string updated_blob;
    const auto reorder_result = reorder_visual_methods_blob(
        methods_field->display_value,
        request.method_name,
        request.placement,
        request.relative_method_name,
        updated_blob);
    if (!reorder_result.ok) {
        return reorder_result;
    }

    auto update_result = update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "METHODS",
        .property_value = updated_blob
    });
    if (update_result.ok) {
        update_result.affected_object_count = 1U;
    }
    return update_result;
}

VisualAssetEditResult reorder_visual_object_methods(const VisualObjectMethodReorderBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.ReorderBatchRequired")};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_reorders = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.method_name).empty()) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Method.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NameRequired")};
        }

        const auto result = reorder_visual_object_method({
            .path = request.path,
            .record_index = method.record_index,
            .object_name = method.object_name,
            .unique_id = method.unique_id,
            .method_name = method.method_name,
            .placement = method.placement,
            .relative_method_name = method.relative_method_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.methods.size()};
}

VisualObjectDuplicateResult duplicate_visual_object(const VisualObjectDuplicateRequest& request) {
    if (request.path.empty()) {
        return failed_visual_object_duplicate_result(visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"));
    }

    std::size_t source_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, source_record_index);
    if (!resolution.ok) {
        return failed_visual_object_duplicate_result(resolution.error);
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return failed_visual_object_duplicate_result(table_result.error);
    }
    const auto& table = table_result.table;
    if (source_record_index >= table.records.size()) {
        return failed_visual_object_duplicate_result(
            visual_asset_text("VisualAssetEditor.Object.RecordUnavailable"));
    }

    const auto reject_missing_replacement_field = [&](const std::string& field_name, const std::string& value) -> VisualObjectDuplicateResult {
        if (value.empty() || find_field_index(table, field_name).has_value()) {
            return {
                .ok = true,
                .error = {},
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .parent_name = {}
            };
        }
        return failed_visual_object_duplicate_result(
            visual_asset_text("VisualAssetEditor.Identity.ReplacementFieldMissing"));
    };
    for (const auto& check : {
             reject_missing_replacement_field("OBJNAME", request.new_object_name),
             reject_missing_replacement_field("NAME", request.new_name),
             reject_missing_replacement_field("UNIQUEID", request.new_unique_id)
         }) {
        if (!check.ok) {
            return check;
        }
    }

    std::vector<std::string> duplicate_values;
    duplicate_values.reserve(table.fields.size());
    for (const auto& field : table.fields) {
        const auto* value = find_record_value(table.records[source_record_index], field.name);
        duplicate_values.push_back(value == nullptr ? std::string{} : value->display_value);
    }
    replace_duplicate_field_value(table, duplicate_values, "OBJNAME", request.new_object_name);
    replace_duplicate_field_value(table, duplicate_values, "NAME", request.new_name);
    replace_duplicate_field_value(table, duplicate_values, "UNIQUEID", request.new_unique_id);

    for (const auto& identity_field : {"OBJNAME", "NAME", "UNIQUEID"}) {
        const std::string final_value = duplicate_field_value(table, duplicate_values, identity_field);
        if (normalize_visual_object_name(final_value).empty()) {
            continue;
        }
        const auto collision = reject_identity_collision(table, identity_field, final_value);
        if (!collision.ok) {
            return collision;
        }
    }

    std::vector<VisualObjectPropertyChange> replacement_values;
    for (const auto& [field_name, value] : {
             std::pair{"OBJNAME", request.new_object_name},
             std::pair{"NAME", request.new_name},
             std::pair{"UNIQUEID", request.new_unique_id}
         }) {
        if (!value.empty()) {
            replacement_values.push_back({.property_name = field_name, .property_value = value});
        }
    }

    const std::size_t duplicate_record_index = table.records.size();
    const auto append_result = append_visual_asset_records_preserving_raw(
        request.path,
        {{.source_record_index = source_record_index, .field_values = std::move(replacement_values)}});
    if (!append_result.ok) {
        return failed_visual_object_duplicate_result(append_result.error);
    }

    const auto duplicated_table_result = parse_dbf_table_from_file(request.path, duplicate_record_index + 1U);
    if (!duplicated_table_result.ok || duplicate_record_index >= duplicated_table_result.table.records.size()) {
        return failed_visual_object_duplicate_result(
            duplicated_table_result.ok ? visual_asset_text("VisualAssetEditor.Object.DuplicatedRecordUnavailable") :
                                         duplicated_table_result.error);
    }
    const auto duplicated_object = created_visual_object_from_record(
        duplicated_table_result.table.records[duplicate_record_index],
        duplicate_record_index);

    return {
        .ok = true,
        .error = {},
        .record_index = duplicate_record_index,
        .object_name = duplicated_object.object_name,
        .unique_id = duplicated_object.unique_id,
        .parent_name = duplicated_object.parent_name
    };
}

VisualObjectDuplicateBatchResult duplicate_visual_objects(const VisualObjectDuplicateBatchRequest& request) {
    if (request.path.empty()) {
        return failed_visual_object_duplicate_batch_result(visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"));
    }
    if (request.objects.empty()) {
        return failed_visual_object_duplicate_batch_result(visual_asset_text("VisualAssetEditor.Object.DuplicateBatchRequired"));
    }

    VisualAssetRollbackSnapshot original_asset;
    const auto snapshot_result = capture_visual_asset_rollback_snapshot(
        request.path,
        original_asset);
    if (!snapshot_result.ok) {
        return failed_visual_object_duplicate_batch_result(snapshot_result.error);
    }

    const auto restore_original_asset = [&]() -> VisualAssetEditResult {
        return restore_visual_asset_rollback_snapshot(request.path, original_asset);
    };

    std::vector<std::size_t> duplicated_record_indexes;
    duplicated_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectCreatedObject> duplicated_objects;
    duplicated_objects.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto duplicate_result = duplicate_visual_object({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .new_object_name = object.new_object_name,
            .new_name = object.new_name,
            .new_unique_id = object.new_unique_id
        });
        if (!duplicate_result.ok) {
            const auto rollback_result = restore_original_asset();
            if (!rollback_result.ok) {
                return failed_visual_object_duplicate_batch_result(
                    visual_asset_rollback_failed_text(duplicate_result.error, rollback_result.error));
            }
            return failed_visual_object_duplicate_batch_result(duplicate_result.error);
        }
        duplicated_record_indexes.push_back(duplicate_result.record_index);
        duplicated_objects.push_back({
            .record_index = duplicate_result.record_index,
            .object_name = duplicate_result.object_name,
            .unique_id = duplicate_result.unique_id,
            .parent_name = duplicate_result.parent_name
        });
    }

    return {
        .ok = true,
        .error = {},
        .record_indexes = duplicated_record_indexes,
        .duplicated_objects = std::move(duplicated_objects)
    };
}

VisualObjectSubtreeDuplicateResult failed_visual_object_subtree_duplicate_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .root_record_index = 0U,
        .copied_count = 0U,
        .root_object_name = {},
        .root_unique_id = {},
        .root_parent_name = {}
    };
}

VisualObjectSubtreeDuplicateResult empty_visual_object_subtree_duplicate_result() {
    return {
        .ok = true,
        .error = {},
        .root_record_index = 0U,
        .copied_count = 0U,
        .root_object_name = {},
        .root_unique_id = {},
        .root_parent_name = {}
    };
}

VisualObjectSubtreeDuplicateResult duplicate_visual_object_subtree(const VisualObjectSubtreeDuplicateRequest& request) {
    if (request.path.empty()) {
        return failed_visual_object_subtree_duplicate_result(visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"));
    }
    if (request.replacements.empty()) {
        return failed_visual_object_subtree_duplicate_result(visual_asset_text("VisualAssetEditor.Identity.SubtreeReplacementBatchRequired"));
    }

    std::size_t root_record_index = 0U;
    const auto root_resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, root_record_index);
    if (!root_resolution.ok) {
        return failed_visual_object_subtree_duplicate_result(root_resolution.error);
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return failed_visual_object_subtree_duplicate_result(table_result.error);
    }
    const auto& table = table_result.table;
    if (root_record_index >= table.records.size()) {
        return failed_visual_object_subtree_duplicate_result(
            visual_asset_text("VisualAssetEditor.Object.RecordUnavailable"));
    }

    const auto require_field = [&](const std::string& field_name) -> VisualObjectSubtreeDuplicateResult {
        if (find_field_index(table, field_name).has_value()) {
            return empty_visual_object_subtree_duplicate_result();
        }
        return failed_visual_object_subtree_duplicate_result(
            visual_asset_text("VisualAssetEditor.Identity.ReplacementFieldMissing"));
    };
    for (const auto& check : {require_field("OBJNAME"), require_field("NAME"), require_field("UNIQUEID"), require_field("PARENT")}) {
        if (!check.ok) {
            return check;
        }
    }

    const auto descendants_result = list_visual_object_descendants({
        .path = request.path,
        .record_index = root_record_index,
        .object_name = {},
        .unique_id = {}
    });
    if (!descendants_result.ok) {
        return failed_visual_object_subtree_duplicate_result(descendants_result.error);
    }

    std::vector<std::size_t> copy_record_indexes;
    copy_record_indexes.reserve(descendants_result.descendants.size() + 1U);
    copy_record_indexes.push_back(root_record_index);
    for (const auto& descendant : descendants_result.descendants) {
        copy_record_indexes.push_back(descendant.object.record_index);
    }

    struct CopyPlan {
        std::size_t record_index = 0;
        std::string source_unique_id;
        const VisualObjectSubtreeDuplicateReplacement* replacement = nullptr;
        std::string original_object_name;
        std::string original_parent_name;
        std::string copied_parent_name;
    };
    std::vector<CopyPlan> copy_plan;
    copy_plan.reserve(copy_record_indexes.size());

    const auto unique_in_replacements = [&](const std::string& source_unique_id) {
        const std::string normalized_source_unique_id = normalize_visual_object_name(source_unique_id);
        return std::count_if(
            request.replacements.begin(),
            request.replacements.end(),
            [&](const VisualObjectSubtreeDuplicateReplacement& replacement) {
                return normalize_visual_object_name(replacement.source_unique_id) == normalized_source_unique_id;
            });
    };

    for (const auto record_index : copy_record_indexes) {
        const auto* unique_id = find_record_value(table.records[record_index], "UNIQUEID");
        const std::string source_unique_id = unique_id == nullptr ? std::string{} : trim_both(unique_id->display_value);
        if (source_unique_id.empty()) {
            return failed_visual_object_subtree_duplicate_result(visual_asset_text("VisualAssetEditor.Identity.CopiedRowFieldRequired", {{"fieldName", "UNIQUEID"}}));
        }
        if (unique_in_replacements(source_unique_id) != 1) {
            return failed_visual_object_subtree_duplicate_result(
                visual_asset_text("VisualAssetEditor.Identity.SubtreeReplacementMissingOrAmbiguous"));
        }

        const auto* replacement = find_subtree_duplicate_replacement(request.replacements, source_unique_id);
        if (replacement == nullptr ||
            trim_both(replacement->new_object_name).empty() ||
            trim_both(replacement->new_name).empty() ||
            trim_both(replacement->new_unique_id).empty()) {
            return failed_visual_object_subtree_duplicate_result(
                visual_asset_text("VisualAssetEditor.Identity.SubtreeReplacementDataMissing"));
        }

        const auto* parent = find_record_value(table.records[record_index], "PARENT");
        copy_plan.push_back({
            .record_index = record_index,
            .source_unique_id = source_unique_id,
            .replacement = replacement,
            .original_object_name = visual_object_record_name(table.records[record_index]),
            .original_parent_name = parent == nullptr ? std::string{} : trim_both(parent->display_value),
            .copied_parent_name = parent == nullptr ? std::string{} : trim_both(parent->display_value)
        });
    }

    for (auto& plan : copy_plan) {
        const std::string normalized_parent_name = normalize_visual_object_name(plan.original_parent_name);
        if (normalized_parent_name.empty()) {
            continue;
        }
        const auto parent_plan = std::find_if(copy_plan.begin(), copy_plan.end(), [&](const CopyPlan& candidate) {
            return normalize_visual_object_name(candidate.original_object_name) == normalized_parent_name;
        });
        if (parent_plan != copy_plan.end()) {
            plan.copied_parent_name = parent_plan->replacement->new_object_name;
        }
    }

    const auto reject_duplicate_identity_values = [&](const std::string& field_name, const std::vector<std::string>& values) -> VisualObjectSubtreeDuplicateResult {
        std::vector<std::string> normalized_values;
        for (const auto& value : values) {
            const std::string normalized_value = normalize_visual_object_name(value);
            if (normalized_value.empty()) {
                continue;
            }
            if (!find_matching_record_indexes(table, field_name, normalized_value).empty()) {
                return failed_visual_object_subtree_duplicate_result(
                    visual_asset_text("VisualAssetEditor.Identity.ReplacementExists"));
            }
            if (std::find(normalized_values.begin(), normalized_values.end(), normalized_value) != normalized_values.end()) {
                return failed_visual_object_subtree_duplicate_result(
                    visual_asset_text("VisualAssetEditor.Identity.ReplacementDuplicatedInSubtree"));
            }
            normalized_values.push_back(normalized_value);
        }
        return empty_visual_object_subtree_duplicate_result();
    };

    std::vector<std::string> new_objnames;
    std::vector<std::string> new_names;
    std::vector<std::string> new_unique_ids;
    new_objnames.reserve(copy_plan.size());
    new_names.reserve(copy_plan.size());
    new_unique_ids.reserve(copy_plan.size());
    for (const auto& plan : copy_plan) {
        new_objnames.push_back(plan.replacement->new_object_name);
        new_names.push_back(plan.replacement->new_name);
        new_unique_ids.push_back(plan.replacement->new_unique_id);
    }
    for (const auto& check : {
             reject_duplicate_identity_values("OBJNAME", new_objnames),
             reject_duplicate_identity_values("NAME", new_names),
             reject_duplicate_identity_values("UNIQUEID", new_unique_ids)
         }) {
        if (!check.ok) {
            return check;
        }
    }

    const std::size_t copied_root_record_index = table.records.size();
    std::vector<VisualAssetRawRecordAppend> appends;
    appends.reserve(copy_plan.size());
    for (const auto& plan : copy_plan) {
        std::vector<VisualObjectPropertyChange> field_values{
            {.property_name = "OBJNAME", .property_value = plan.replacement->new_object_name},
            {.property_name = "NAME", .property_value = plan.replacement->new_name},
            {.property_name = "UNIQUEID", .property_value = plan.replacement->new_unique_id}
        };
        if (!plan.copied_parent_name.empty()) {
            field_values.push_back({.property_name = "PARENT", .property_value = plan.copied_parent_name});
        }
        appends.push_back({
            .source_record_index = plan.record_index,
            .field_values = std::move(field_values)
        });
    }

    const auto append_result = append_visual_asset_records_preserving_raw(request.path, appends);
    if (!append_result.ok) {
        return failed_visual_object_subtree_duplicate_result(append_result.error);
    }

    const auto copied_table_result = parse_dbf_table_from_file(request.path, copied_root_record_index + 1U);
    if (!copied_table_result.ok || copied_root_record_index >= copied_table_result.table.records.size()) {
        return failed_visual_object_subtree_duplicate_result(
            copied_table_result.ok ? visual_asset_text("VisualAssetEditor.Object.CopiedRootRecordUnavailable") :
                                     copied_table_result.error);
    }
    const auto copied_root = created_visual_object_from_record(
        copied_table_result.table.records[copied_root_record_index],
        copied_root_record_index);

    return {
        .ok = true,
        .error = {},
        .root_record_index = copied_root_record_index,
        .copied_count = copy_plan.size(),
        .root_object_name = copied_root.object_name,
        .root_unique_id = copied_root.unique_id,
        .root_parent_name = copied_root.parent_name
    };
}

VisualObjectCreateResult failed_visual_object_create_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .parent_name = {}
    };
}

VisualObjectCreateBatchResult failed_visual_object_create_batch_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .record_indexes = {},
        .created_objects = {}
    };
}

VisualObjectCreateResult create_visual_object(const VisualObjectCreateRequest& request) {
    if (request.path.empty()) {
        return failed_visual_object_create_result(visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"));
    }
    if (request.field_values.empty()) {
        return failed_visual_object_create_result(visual_asset_text("VisualAssetEditor.Object.FieldValuesRequired"));
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return failed_visual_object_create_result(table_result.error);
    }
    const auto& table = table_result.table;

    std::vector<std::string> created_values(table.fields.size());
    for (const auto& field_value : request.field_values) {
        if (trim_both(field_value.property_name).empty()) {
            return failed_visual_object_create_result(visual_asset_text("VisualAssetEditor.Field.NameRequired"));
        }
        const auto field_index = find_field_index(table, field_value.property_name);
        if (!field_index.has_value()) {
            return failed_visual_object_create_result(visual_asset_text("VisualAssetEditor.Field.NotFound"));
        }
        created_values[*field_index] = field_value.property_value;
    }

    for (const auto& identity_field : {"OBJNAME", "NAME", "UNIQUEID"}) {
        const std::string final_value = duplicate_field_value(table, created_values, identity_field);
        if (normalize_visual_object_name(final_value).empty()) {
            continue;
        }
        const auto collision = reject_identity_collision(table, identity_field, final_value);
        if (!collision.ok) {
            return failed_visual_object_create_result(collision.error);
        }
    }

    const std::size_t created_record_index = table.records.size();
    const auto append_result = append_visual_asset_records_preserving_raw(
        request.path,
        {{.source_record_index = std::nullopt, .field_values = request.field_values}});
    if (!append_result.ok) {
        return failed_visual_object_create_result(append_result.error);
    }

    const auto created_table_result = parse_dbf_table_from_file(request.path, created_record_index + 1U);
    if (!created_table_result.ok || created_record_index >= created_table_result.table.records.size()) {
        return failed_visual_object_create_result(
            created_table_result.ok ? visual_asset_text("VisualAssetEditor.Object.CreatedRecordUnavailable") :
                                      created_table_result.error);
    }
    const auto& created_record = created_table_result.table.records[created_record_index];
    const auto* created_unique_id = find_record_value(created_record, "UNIQUEID");
    const auto* created_parent_name = find_record_value(created_record, "PARENT");

    return {
        .ok = true,
        .error = {},
        .record_index = created_record_index,
        .object_name = visual_object_record_name(created_record),
        .unique_id = created_unique_id == nullptr ? std::string{} : trim_both(created_unique_id->display_value),
        .parent_name = created_parent_name == nullptr ? std::string{} : trim_both(created_parent_name->display_value)
    };
}

VisualObjectCreateBatchResult create_visual_objects(const VisualObjectCreateBatchRequest& request) {
    if (request.path.empty()) {
        return failed_visual_object_create_batch_result(visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"));
    }
    if (request.objects.empty()) {
        return failed_visual_object_create_batch_result(visual_asset_text("VisualAssetEditor.Object.CreateBatchRequired"));
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return failed_visual_object_create_batch_result(table_result.error);
    }
    const auto& table = table_result.table;

    std::vector<std::vector<std::string>> records = visual_record_values_for_write(table.fields, table.records);
    std::vector<std::size_t> created_record_indexes;
    created_record_indexes.reserve(request.objects.size());
    std::vector<VisualAssetRawRecordAppend> appends;
    appends.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        if (object.field_values.empty()) {
            return failed_visual_object_create_batch_result(visual_asset_text("VisualAssetEditor.Object.FieldValuesRequired"));
        }

        std::vector<std::string> created_values(table.fields.size());
        for (const auto& field_value : object.field_values) {
            if (trim_both(field_value.property_name).empty()) {
                return failed_visual_object_create_batch_result(visual_asset_text("VisualAssetEditor.Field.NameRequired"));
            }
            const auto field_index = find_field_index(table, field_value.property_name);
            if (!field_index.has_value()) {
                return failed_visual_object_create_batch_result(visual_asset_text("VisualAssetEditor.Field.NotFound"));
            }
            created_values[*field_index] = field_value.property_value;
        }

        for (const auto& identity_field : {"OBJNAME", "NAME", "UNIQUEID"}) {
            const auto identity_field_index = find_field_index(table, identity_field);
            if (!identity_field_index.has_value() || *identity_field_index >= created_values.size()) {
                continue;
            }
            const std::string final_value = created_values[*identity_field_index];
            const std::string normalized_final_value = normalize_visual_object_name(final_value);
            if (normalized_final_value.empty()) {
                continue;
            }
            const auto collision = std::find_if(records.begin(), records.end(), [&](const std::vector<std::string>& record_values) {
                if (*identity_field_index >= record_values.size()) {
                    return false;
                }
                return normalize_visual_object_name(record_values[*identity_field_index]) == normalized_final_value;
            });
            if (collision != records.end()) {
                return failed_visual_object_create_batch_result(
                    visual_asset_text("VisualAssetEditor.Identity.ReplacementExists"));
            }
        }

        created_record_indexes.push_back(records.size());
        records.push_back(std::move(created_values));
        appends.push_back({
            .source_record_index = std::nullopt,
            .field_values = object.field_values
        });
    }

    const auto append_result = append_visual_asset_records_preserving_raw(request.path, appends);
    if (!append_result.ok) {
        return failed_visual_object_create_batch_result(append_result.error);
    }

    const auto created_table_result = parse_dbf_table_from_file(request.path, records.size());
    if (!created_table_result.ok) {
        return failed_visual_object_create_batch_result(created_table_result.error);
    }

    std::vector<VisualObjectCreatedObject> created_objects;
    created_objects.reserve(created_record_indexes.size());
    for (const auto created_record_index : created_record_indexes) {
        if (created_record_index >= created_table_result.table.records.size()) {
            return failed_visual_object_create_batch_result(
                visual_asset_text("VisualAssetEditor.Object.CreatedBatchRecordUnavailable"));
        }
        created_objects.push_back(
            created_visual_object_from_record(created_table_result.table.records[created_record_index], created_record_index));
    }

    return {
        .ok = true,
        .error = {},
        .record_indexes = created_record_indexes,
        .created_objects = std::move(created_objects)
    };
}

VisualAssetEditResult align_visual_objects(const VisualObjectAlignmentRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.AlignmentTargetsRequired")};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    if (mode != "left" &&
        mode != "right" &&
        mode != "top" &&
        mode != "bottom" &&
        mode != "horizontal-center" &&
        mode != "vertical-center") {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.AlignmentModeUnsupported")};
    }

    VisualObjectGeometry anchor_geometry;
    const auto anchor_result = read_visual_object_geometry(
        request.path,
        request.anchor_record_index,
        request.anchor_object_name,
        request.anchor_unique_id,
        anchor_geometry);
    if (!anchor_result.ok) {
        return anchor_result;
    }

    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        VisualObjectGeometry object_geometry;
        const auto object_result = read_visual_object_geometry(
            request.path,
            object.record_index,
            object.object_name,
            object.unique_id,
            object_geometry);
        if (!object_result.ok) {
            return object_result;
        }

        std::string property_name;
        double aligned_value = 0.0;
        if (mode == "left") {
            property_name = "HPOS";
            aligned_value = anchor_geometry.hpos;
        } else if (mode == "right") {
            property_name = "HPOS";
            aligned_value = anchor_geometry.hpos + anchor_geometry.width - object_geometry.width;
        } else if (mode == "top") {
            property_name = "VPOS";
            aligned_value = anchor_geometry.vpos;
        } else if (mode == "bottom") {
            property_name = "VPOS";
            aligned_value = anchor_geometry.vpos + anchor_geometry.height - object_geometry.height;
        } else if (mode == "horizontal-center") {
            property_name = "HPOS";
            aligned_value = anchor_geometry.hpos + ((anchor_geometry.width - object_geometry.width) / 2.0);
        } else {
            property_name = "VPOS";
            aligned_value = anchor_geometry.vpos + ((anchor_geometry.height - object_geometry.height) / 2.0);
        }

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {.property_name = property_name, .property_value = format_visual_geometry_number(aligned_value)}
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult resize_visual_objects(const VisualObjectResizeRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ResizeTargetsRequired")};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    if (mode != "width" && mode != "height" && mode != "size") {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ResizeModeUnsupported")};
    }

    VisualObjectGeometry anchor_geometry;
    const auto anchor_result = read_visual_object_geometry(
        request.path,
        request.anchor_record_index,
        request.anchor_object_name,
        request.anchor_unique_id,
        anchor_geometry);
    if (!anchor_result.ok) {
        return anchor_result;
    }

    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        VisualObjectGeometry object_geometry;
        const auto object_result = read_visual_object_geometry(
            request.path,
            object.record_index,
            object.object_name,
            object.unique_id,
            object_geometry);
        if (!object_result.ok) {
            return object_result;
        }

        std::vector<VisualObjectPropertyChange> properties;
        if (mode == "width" || mode == "size") {
            properties.push_back({
                .property_name = "WIDTH",
                .property_value = format_visual_geometry_number(anchor_geometry.width)
            });
        }
        if (mode == "height" || mode == "size") {
            properties.push_back({
                .property_name = "HEIGHT",
                .property_value = format_visual_geometry_number(anchor_geometry.height)
            });
        }

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = std::move(properties)
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualObjectGroupResult failed_visual_object_group_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .container_record_index = 0U,
        .child_count = 0U,
        .container_object_name = {},
        .container_unique_id = {},
        .container_parent_name = {}
    };
}

VisualObjectGroupResult group_visual_objects(const VisualObjectGroupRequest& request) {
    if (request.path.empty()) {
        return failed_visual_object_group_result(visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"));
    }
    if (request.container_field_values.empty()) {
        return failed_visual_object_group_result(visual_asset_text("VisualAssetEditor.Object.GroupContainerFieldsRequired"));
    }
    if (request.objects.empty()) {
        return failed_visual_object_group_result(visual_asset_text("VisualAssetEditor.Object.GroupSelectionRequired"));
    }

    VisualAssetRollbackSnapshot original_asset;
    const auto snapshot_result = capture_visual_asset_rollback_snapshot(
        request.path,
        original_asset);
    if (!snapshot_result.ok) {
        return failed_visual_object_group_result(snapshot_result.error);
    }

    const auto fail_with_rollback = [&](std::string error) {
        const auto rollback_result = restore_visual_asset_rollback_snapshot(
            request.path,
            original_asset);
        if (!rollback_result.ok) {
            error = visual_asset_rollback_failed_text(
                std::move(error),
                rollback_result.error);
        }
        return failed_visual_object_group_result(std::move(error));
    };

    const auto create_result = create_visual_object({
        .path = request.path,
        .field_values = request.container_field_values
    });
    if (!create_result.ok) {
        return fail_with_rollback(create_result.error);
    }

    const auto table_result = parse_dbf_table_from_file(request.path, create_result.record_index + 1U);
    if (!table_result.ok || create_result.record_index >= table_result.table.records.size()) {
        return fail_with_rollback(
            table_result.ok ? visual_asset_text("VisualAssetEditor.Object.GroupContainerUnavailable") : table_result.error);
    }

    const std::string container_name = visual_object_record_name(table_result.table.records[create_result.record_index]);
    if (container_name.empty()) {
        return fail_with_rollback(visual_asset_text("VisualAssetEditor.Object.GroupContainerNameMissing"));
    }

    std::vector<VisualObjectReparentBatchItem> reparent_items;
    reparent_items.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        reparent_items.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .parent_object_name = container_name,
            .parent_unique_id = {},
            .clear_parent = false
        });
    }

    const auto reparent_result = reparent_visual_objects({
        .path = request.path,
        .objects = reparent_items
    });
    if (!reparent_result.ok) {
        return fail_with_rollback(reparent_result.error);
    }

    return {
        .ok = true,
        .error = {},
        .container_record_index = create_result.record_index,
        .child_count = request.objects.size(),
        .container_object_name = create_result.object_name,
        .container_unique_id = create_result.unique_id,
        .container_parent_name = create_result.parent_name
    };
}

VisualObjectUngroupResult failed_visual_object_ungroup_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .container_record_index = 0U,
        .child_count = 0U,
        .container_object_name = {},
        .container_unique_id = {},
        .container_parent_name = {},
        .parent_name = {},
        .parent_record_available = false,
        .parent_record_index = 0U
    };
}

namespace {

// Shared by the public single-object deleted-state setter and by internal
// callers (e.g. ungroup_visual_object) that manage their own undo contract
// and must not have this step register an additional journal entry.
struct DeletedStateChangeResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
    bool prior_deleted = false;
    bool changed = false;
};

DeletedStateChangeResult apply_visual_object_deleted_state_raw(
    const std::string& path,
    std::size_t requested_record_index,
    const std::string& object_name,
    const std::string& unique_id,
    bool deleted) {
    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = path,
        .record_index = requested_record_index,
        .object_name = object_name,
        .unique_id = unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return {.ok = false, .error = resolution.error};
    }

    const auto table_result = parse_dbf_table_from_file(path, record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = table_result.ok
            ? visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")
            : table_result.error};
    }
    const bool prior_deleted = table_result.table.records[record_index].deleted;
    if (prior_deleted == deleted) {
        return {.ok = true, .error = {}, .record_index = record_index, .prior_deleted = prior_deleted, .changed = false};
    }

    const auto result = set_record_deleted_flag(path, record_index, deleted);
    if (!result.ok) {
        return {.ok = false, .error = result.error};
    }

    return {.ok = true, .error = {}, .record_index = record_index, .prior_deleted = prior_deleted, .changed = true};
}

}  // namespace

VisualObjectUngroupResult ungroup_visual_object(const VisualObjectUngroupRequest& request) {
    if (request.path.empty()) {
        return failed_visual_object_ungroup_result(visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"));
    }

    VisualAssetRollbackSnapshot original_asset;
    const auto snapshot_result = capture_visual_asset_rollback_snapshot(
        request.path,
        original_asset);
    if (!snapshot_result.ok) {
        return failed_visual_object_ungroup_result(snapshot_result.error);
    }

    std::size_t container_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, container_record_index);
    if (!resolution.ok) {
        return failed_visual_object_ungroup_result(resolution.error);
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return failed_visual_object_ungroup_result(table_result.error);
    }
    if (container_record_index >= table_result.table.records.size()) {
        return failed_visual_object_ungroup_result(
            visual_asset_text("VisualAssetEditor.Object.ContainerRecordUnavailable"));
    }

    const std::string container_name = visual_object_record_name(table_result.table.records[container_record_index]);
    const auto container_identity = created_visual_object_from_record(
        table_result.table.records[container_record_index],
        container_record_index);
    if (container_name.empty()) {
        return failed_visual_object_ungroup_result(visual_asset_text("VisualAssetEditor.Object.SelectedContainerNameMissing"));
    }
    const auto* parent_value = find_record_value(table_result.table.records[container_record_index], "PARENT");
    const std::string container_parent_name = parent_value == nullptr ? std::string{} : trim_both(parent_value->display_value);
    const auto* container_parent = find_visual_object_record_by_name(table_result.table, container_parent_name);
    const bool container_parent_available = container_parent != nullptr;
    const std::size_t container_parent_record_index = container_parent_available ? container_parent->record_index : 0U;

    const auto children_result = list_visual_object_children({
        .path = request.path,
        .record_index = container_record_index,
        .object_name = {},
        .unique_id = {}
    });
    if (!children_result.ok) {
        return failed_visual_object_ungroup_result(children_result.error);
    }
    if (children_result.children.empty()) {
        return failed_visual_object_ungroup_result(visual_asset_text("VisualAssetEditor.Object.SelectedContainerChildrenRequired"));
    }

    std::vector<VisualObjectReparentBatchItem> reparent_items;
    reparent_items.reserve(children_result.children.size());
    for (const auto& child : children_result.children) {
        reparent_items.push_back({
            .record_index = child.record_index,
            .object_name = {},
            .unique_id = {},
            .parent_object_name = container_parent_name,
            .parent_unique_id = {},
            .clear_parent = container_parent_name.empty()
        });
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_reparents = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    const auto reparent_result = reparent_visual_objects({
        .path = request.path,
        .objects = reparent_items
    });
    if (!reparent_result.ok) {
        const auto restore_result = restore_visual_asset_rollback_snapshot(
            request.path,
            original_asset);
        if (!restore_result.ok) {
            return failed_visual_object_ungroup_result(
                visual_asset_rollback_failed_text(reparent_result.error, restore_result.error));
        }
        return failed_visual_object_ungroup_result(reparent_result.error);
    }

    const auto delete_result = apply_visual_object_deleted_state_raw(
        request.path, container_record_index, {}, {}, true);
    if (!delete_result.ok) {
        const auto rollback_result = rollback_reparents();
        const auto restore_result = restore_visual_asset_rollback_snapshot(
            request.path,
            original_asset);
        if (!rollback_result.ok || !restore_result.ok) {
            std::string rollback_error = rollback_result.ok
                ? restore_result.error
                : rollback_result.error;
            if (!rollback_result.ok && !restore_result.ok) {
                rollback_error = visual_asset_rollback_failed_text(
                    rollback_result.error,
                    restore_result.error);
            }
            return failed_visual_object_ungroup_result(
                visual_asset_rollback_failed_text(delete_result.error, std::move(rollback_error)));
        }
        return failed_visual_object_ungroup_result(delete_result.error);
    }

    return {
        .ok = true,
        .error = {},
        .container_record_index = container_record_index,
        .child_count = children_result.children.size(),
        .container_object_name = container_identity.object_name,
        .container_unique_id = container_identity.unique_id,
        .container_parent_name = container_identity.parent_name,
        .parent_name = container_parent_name,
        .parent_record_available = container_parent_available,
        .parent_record_index = container_parent_record_index
    };
}

VisualAssetEditResult distribute_visual_objects(const VisualObjectDistributeRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.size() < 3U) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.DistributionTargetCountRequired")};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    std::string property_name;
    if (mode == "horizontal") {
        property_name = "HPOS";
    } else if (mode == "vertical") {
        property_name = "VPOS";
    } else {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.DistributionModeUnsupported")};
    }

    struct DistributionItem {
        VisualObjectAlignmentTarget object;
        double coordinate = 0.0;
        std::size_t original_index = 0;
    };
    std::vector<DistributionItem> items;
    items.reserve(request.objects.size());
    for (std::size_t index = 0U; index < request.objects.size(); ++index) {
        double coordinate = 0.0;
        const auto coordinate_result = read_visual_object_geometry_coordinate(
            request.path,
            request.objects[index],
            property_name,
            coordinate);
        if (!coordinate_result.ok) {
            return coordinate_result;
        }
        items.push_back({
            .object = request.objects[index],
            .coordinate = coordinate,
            .original_index = index
        });
    }

    std::stable_sort(items.begin(), items.end(), [](const DistributionItem& left, const DistributionItem& right) {
        if (left.coordinate == right.coordinate) {
            return left.original_index < right.original_index;
        }
        return left.coordinate < right.coordinate;
    });

    const double first_coordinate = items.front().coordinate;
    const double last_coordinate = items.back().coordinate;
    if (std::abs(last_coordinate - first_coordinate) < 0.0005) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.DistributionDistinctEndpointsRequired")};
    }

    const double step = (last_coordinate - first_coordinate) / static_cast<double>(items.size() - 1U);
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(items.size() - 2U);
    for (std::size_t index = 1U; index + 1U < items.size(); ++index) {
        const double distributed_coordinate = first_coordinate + (step * static_cast<double>(index));
        edits.push_back({
            .record_index = items[index].object.record_index,
            .object_name = items[index].object.object_name,
            .unique_id = items[index].object.unique_id,
            .properties = {
                {
                    .property_name = property_name,
                    .property_value = format_visual_geometry_number(distributed_coordinate)
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult snap_visual_objects_to_grid(const VisualObjectSnapToGridRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SnapSelectionRequired")};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    const bool snap_horizontal = mode == "horizontal" || mode == "both";
    const bool snap_vertical = mode == "vertical" || mode == "both";
    if (!snap_horizontal && !snap_vertical) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.GridSnappingModeUnsupported")};
    }
    if (snap_horizontal && request.grid_width <= 0.0) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.GridWidthPositiveRequired")};
    }
    if (snap_vertical && request.grid_height <= 0.0) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.GridHeightPositiveRequired")};
    }

    const auto snap_coordinate = [](double coordinate, double grid_size) {
        return std::round(coordinate / grid_size) * grid_size;
    };

    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        std::vector<VisualObjectPropertyChange> properties;
        properties.reserve((snap_horizontal ? 1U : 0U) + (snap_vertical ? 1U : 0U));
        if (snap_horizontal) {
            double coordinate = 0.0;
            const auto coordinate_result = read_visual_object_geometry_coordinate(
                request.path,
                object,
                "HPOS",
                coordinate);
            if (!coordinate_result.ok) {
                return coordinate_result;
            }
            properties.push_back({
                .property_name = "HPOS",
                .property_value = format_visual_geometry_number(snap_coordinate(coordinate, request.grid_width))
            });
        }
        if (snap_vertical) {
            double coordinate = 0.0;
            const auto coordinate_result = read_visual_object_geometry_coordinate(
                request.path,
                object,
                "VPOS",
                coordinate);
            if (!coordinate_result.ok) {
                return coordinate_result;
            }
            properties.push_back({
                .property_name = "VPOS",
                .property_value = format_visual_geometry_number(snap_coordinate(coordinate, request.grid_height))
            });
        }
        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = std::move(properties)
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult nudge_visual_objects(const VisualObjectNudgeRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.NudgeSelectionRequired")};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    const bool nudge_horizontal = mode == "horizontal" || mode == "both";
    const bool nudge_vertical = mode == "vertical" || mode == "both";
    if (!nudge_horizontal && !nudge_vertical) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.NudgeModeUnsupported")};
    }
    if (nudge_horizontal && std::abs(request.delta_hpos) < 0.0000001) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.HorizontalNudgeDeltaRequired")};
    }
    if (nudge_vertical && std::abs(request.delta_vpos) < 0.0000001) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.VerticalNudgeDeltaRequired")};
    }

    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        std::vector<VisualObjectPropertyChange> properties;
        properties.reserve((nudge_horizontal ? 1U : 0U) + (nudge_vertical ? 1U : 0U));
        if (nudge_horizontal) {
            double coordinate = 0.0;
            const auto coordinate_result = read_visual_object_geometry_coordinate(
                request.path,
                object,
                "HPOS",
                coordinate);
            if (!coordinate_result.ok) {
                return coordinate_result;
            }
            properties.push_back({
                .property_name = "HPOS",
                .property_value = format_visual_geometry_number(coordinate + request.delta_hpos)
            });
        }
        if (nudge_vertical) {
            double coordinate = 0.0;
            const auto coordinate_result = read_visual_object_geometry_coordinate(
                request.path,
                object,
                "VPOS",
                coordinate);
            if (!coordinate_result.ok) {
                return coordinate_result;
            }
            properties.push_back({
                .property_name = "VPOS",
                .property_value = format_visual_geometry_number(coordinate + request.delta_vpos)
            });
        }
        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = std::move(properties)
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult reparent_visual_object(const VisualObjectReparentRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }

    std::size_t source_record_index = 0U;
    const auto source_resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, source_record_index);
    if (!source_resolution.ok) {
        return source_resolution;
    }

    std::string parent_name;
    if (!request.clear_parent) {
        if (trim_both(request.parent_object_name).empty() && trim_both(request.parent_unique_id).empty()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ParentSelectorRequired")};
        }

        std::size_t parent_record_index = 0U;
        const auto parent_resolution = resolve_visual_object_record_index({
            .path = request.path,
            .record_index = 0U,
            .object_name = request.parent_object_name,
            .unique_id = request.parent_unique_id,
            .property_name = {},
            .property_value = {}
        }, parent_record_index);
        if (!parent_resolution.ok) {
            return parent_resolution;
        }
        if (parent_record_index == source_record_index) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ReparentSelfUnsupported")};
        }

        const auto table_result = parse_dbf_table_from_file(
            request.path,
            std::max(source_record_index, parent_record_index) + 1U);
        if (!table_result.ok) {
            return {.ok = false, .error = table_result.error};
        }
        if (parent_record_index >= table_result.table.records.size()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ParentRecordUnavailable")};
        }

        std::vector<std::size_t> visited_parent_record_indexes;
        std::size_t parent_chain_record_index = parent_record_index;
        while (parent_chain_record_index < table_result.table.records.size()) {
            if (std::find(
                    visited_parent_record_indexes.begin(),
                    visited_parent_record_indexes.end(),
                    parent_chain_record_index) != visited_parent_record_indexes.end()) {
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ParentObjectChainCycle")};
            }
            visited_parent_record_indexes.push_back(parent_chain_record_index);
            if (parent_chain_record_index == source_record_index) {
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ReparentDescendantUnsupported")};
            }

            const auto* parent_chain_record = find_visual_object_record_by_record_index(
                table_result.table,
                parent_chain_record_index);
            if (parent_chain_record == nullptr) {
                break;
            }
            const auto* parent_chain_value = find_record_value(*parent_chain_record, "PARENT");
            const std::string parent_chain_name = parent_chain_value == nullptr
                ? std::string{}
                : trim_both(parent_chain_value->display_value);
            if (parent_chain_name.empty()) {
                break;
            }
            const auto* next_parent_record = find_visual_object_record_by_name(
                table_result.table,
                parent_chain_name);
            if (next_parent_record == nullptr) {
                break;
            }
            parent_chain_record_index = next_parent_record->record_index;
        }

        parent_name = visual_object_record_name(table_result.table.records[parent_record_index]);
        if (parent_name.empty()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ParentNameMissing")};
        }
    }

    auto reparent_result = update_visual_object_property({
        .path = request.path,
        .record_index = source_record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "PARENT",
        .property_value = parent_name
    });
    if (reparent_result.ok) {
        reparent_result.affected_object_count = 1U;
    }
    return reparent_result;
}

VisualAssetEditResult reparent_visual_objects(const VisualObjectReparentBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ReparentBatchRequired")};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_reparents = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& object : request.objects) {
        const auto result = reparent_visual_object({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .parent_object_name = object.parent_object_name,
            .parent_unique_id = object.parent_unique_id,
            .clear_parent = object.clear_parent
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_reparents();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.objects.size()};
}

VisualAssetEditResult rename_visual_object(const VisualObjectRenameRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (!request.update_object_name && !request.update_name && !request.update_unique_id) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Identity.FieldsRequired")};
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    const auto& table = table_result.table;
    if (record_index >= table.records.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    const auto require_field = [&](const std::string& field_name) -> VisualAssetEditResult {
        if (find_field_index(table, field_name).has_value()) {
            return {.ok = true, .error = {}};
        }
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Identity.FieldMissing")};
    };
    for (const auto& check : {
             request.update_object_name ? require_field("OBJNAME") : VisualAssetEditResult{.ok = true, .error = {}},
             request.update_name ? require_field("NAME") : VisualAssetEditResult{.ok = true, .error = {}},
             request.update_unique_id ? require_field("UNIQUEID") : VisualAssetEditResult{.ok = true, .error = {}}
         }) {
        if (!check.ok) {
            return check;
        }
    }

    const auto* current_objname = find_record_value(table.records[record_index], "OBJNAME");
    const auto* current_name = find_record_value(table.records[record_index], "NAME");
    const auto* current_unique_id = find_record_value(table.records[record_index], "UNIQUEID");
    const std::string final_objname = request.update_object_name
        ? request.new_object_name
        : (current_objname == nullptr ? std::string{} : current_objname->display_value);
    const std::string final_name = request.update_name
        ? request.new_name
        : (current_name == nullptr ? std::string{} : current_name->display_value);
    const std::string final_unique_id = request.update_unique_id
        ? request.new_unique_id
        : (current_unique_id == nullptr ? std::string{} : current_unique_id->display_value);

    for (const auto& check : {
             reject_identity_collision_excluding_record(table, "OBJNAME", final_objname, record_index),
             reject_identity_collision_excluding_record(table, "NAME", final_name, record_index),
             reject_identity_collision_excluding_record(table, "UNIQUEID", final_unique_id, record_index)
         }) {
        if (!check.ok) {
            return check;
        }
    }

    std::vector<VisualObjectPropertyChange> changes;
    if (request.update_object_name) {
        changes.push_back({.property_name = "OBJNAME", .property_value = request.new_object_name});
    }
    if (request.update_name) {
        changes.push_back({.property_name = "NAME", .property_value = request.new_name});
    }
    if (request.update_unique_id) {
        changes.push_back({.property_name = "UNIQUEID", .property_value = request.new_unique_id});
    }

    auto rename_result = update_visual_object_properties({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .properties = changes
    });
    if (rename_result.ok) {
        rename_result.affected_object_count = 1U;
    }
    return rename_result;
}

VisualAssetEditResult rename_visual_objects(const VisualObjectRenameBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RenameBatchRequired")};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_renames = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& object : request.objects) {
        const auto result = rename_visual_object({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .update_object_name = object.update_object_name,
            .new_object_name = object.new_object_name,
            .update_name = object.update_name,
            .new_name = object.new_name,
            .update_unique_id = object.update_unique_id,
            .new_unique_id = object.new_unique_id
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.objects.size()};
}

VisualAssetEditResult reorder_visual_object(const VisualObjectReorderRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }

    const std::string placement = normalize_visual_property_name(request.placement);
    if (placement != "front" && placement != "back" && placement != "before" && placement != "after") {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.PlacementUnsupported")};
    }

    std::size_t source_record_index = 0U;
    const auto source_resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, source_record_index);
    if (!source_resolution.ok) {
        return source_resolution;
    }

    std::size_t target_record_index = 0U;
    if (placement == "before" || placement == "after") {
        if (trim_both(request.target_object_name).empty() && trim_both(request.target_unique_id).empty()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TargetSelectorRequired")};
        }
        const auto target_resolution = resolve_visual_object_record_index({
            .path = request.path,
            .record_index = 0U,
            .object_name = request.target_object_name,
            .unique_id = request.target_unique_id,
            .property_name = {},
            .property_value = {}
        }, target_record_index);
        if (!target_resolution.ok) {
            return target_resolution;
        }
        if (target_record_index == source_record_index) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ReorderRelativeToSelf")};
        }
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    const auto& table = table_result.table;
    if (source_record_index >= table.records.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }
    if ((placement == "before" || placement == "after") && target_record_index >= table.records.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TargetRecordUnavailable")};
    }

    std::vector<std::size_t> order;
    order.reserve(table.records.size());
    for (std::size_t index = 0U; index < table.records.size(); ++index) {
        if (index != source_record_index) {
            order.push_back(index);
        }
    }

    std::size_t insert_position = 0U;
    if (placement == "back") {
        insert_position = order.size();
    } else if (placement == "before" || placement == "after") {
        const auto target = std::find(order.begin(), order.end(), target_record_index);
        if (target == order.end()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TargetRecordUnavailable")};
        }
        insert_position = static_cast<std::size_t>(std::distance(order.begin(), target));
        if (placement == "after") {
            ++insert_position;
        }
    }
    order.insert(order.begin() + static_cast<std::ptrdiff_t>(insert_position), source_record_index);

    const auto reorder_result = reorder_visual_asset_records_preserving_raw(request.path, order);
    if (!reorder_result.ok) {
        return reorder_result;
    }

    return {.ok = true, .error = {}, .affected_object_count = 1U};
}

VisualAssetEditResult reorder_visual_objects(const VisualObjectReorderBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ReorderBatchRequired")};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    std::vector<DbfRecord> reordered_records = table_result.table.records;
    for (const auto& object : request.objects) {
        const auto result = apply_visual_object_reorder_to_records(reordered_records, object);
        if (!result.ok) {
            return result;
        }
    }

    std::vector<std::size_t> record_order;
    record_order.reserve(reordered_records.size());
    for (const auto& record : reordered_records) {
        record_order.push_back(record.record_index);
    }
    const auto reorder_result = reorder_visual_asset_records_preserving_raw(
        request.path,
        record_order);
    if (!reorder_result.ok) {
        return reorder_result;
    }

    return {.ok = true, .error = {}, .affected_object_count = request.objects.size()};
}

VisualAssetEditResult set_visual_object_deleted_state(const VisualObjectDeletedStateRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }

    const auto change = apply_visual_object_deleted_state_raw(
        request.path, request.record_index, request.object_name, request.unique_id, request.deleted);
    if (!change.ok) {
        return {.ok = false, .error = change.error};
    }
    if (!change.changed) {
        return {.ok = true, .error = {}, .affected_object_count = 1U};
    }

    std::string undo_error;
    if (!record_visual_asset_undo_entry(request.path, {
            .record_index = change.record_index,
            .property_name = kVisualAssetDeletedStateUndoPropertyName,
            .prior_value = change.prior_deleted ? "1" : "0",
            .prior_value_exists = true,
            .label = visual_asset_text("VisualAssetEditor.Undo.DeletedStateLabel"),
            .grouped_changes = {}
        }, undo_error)) {
        set_record_deleted_flag(request.path, change.record_index, change.prior_deleted);
        return {.ok = false, .error = undo_error};
    }

    return {.ok = true, .error = {}, .affected_object_count = 1U};
}

VisualAssetEditResult set_visual_object_deleted_states(const VisualObjectDeletedStateBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.DeletedStateBatchRequired")};
    }

    struct AppliedDeletedState {
        std::size_t record_index = 0;
        bool prior_deleted = false;
    };
    std::vector<AppliedDeletedState> applied;

    const auto rollback_applied_states = [&]() -> VisualAssetEditResult {
        for (auto item = applied.rbegin(); item != applied.rend(); ++item) {
            const auto rollback_result = set_record_deleted_flag(request.path, item->record_index, item->prior_deleted);
            if (!rollback_result.ok) {
                return {.ok = false, .error = rollback_result.error};
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& object : request.objects) {
        std::size_t record_index = 0U;
        const auto resolution = resolve_visual_object_record_index({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = {},
            .property_value = {}
        }, record_index);
        if (!resolution.ok) {
            const auto rollback_result = rollback_applied_states();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(resolution.error, rollback_result.error)
                };
            }
            return resolution;
        }

        const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
        if (!table_result.ok || record_index >= table_result.table.records.size()) {
            const std::string error = table_result.ok
                ? visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")
                : table_result.error;
            const auto rollback_result = rollback_applied_states();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(error, rollback_result.error)
                };
            }
            return {.ok = false, .error = error};
        }

        const bool prior_deleted = table_result.table.records[record_index].deleted;
        if (prior_deleted == object.deleted) {
            continue;
        }

        const auto result = set_record_deleted_flag(request.path, record_index, object.deleted);
        if (!result.ok) {
            const auto rollback_result = rollback_applied_states();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return {.ok = false, .error = result.error};
        }
        applied.push_back({.record_index = record_index, .prior_deleted = prior_deleted});
    }

    if (applied.empty()) {
        return {.ok = true, .error = {}, .affected_object_count = 0U};
    }

    std::vector<VisualAssetUndoEntry> grouped_changes;
    grouped_changes.reserve(applied.size());
    for (const auto& item : applied) {
        grouped_changes.push_back({
            .record_index = item.record_index,
            .property_name = kVisualAssetDeletedStateUndoPropertyName,
            .prior_value = item.prior_deleted ? "1" : "0",
            .prior_value_exists = true,
            .label = {},
            .grouped_changes = {}
        });
    }

    std::string undo_error;
    if (!record_visual_asset_undo_entry(request.path, {
            .record_index = applied.back().record_index,
            .property_name = kVisualAssetDeletedStateUndoPropertyName,
            .prior_value = applied.back().prior_deleted ? "1" : "0",
            .prior_value_exists = true,
            .label = visual_asset_text("VisualAssetEditor.Undo.DeletedStateLabel"),
            .grouped_changes = grouped_changes
        }, undo_error)) {
        const auto rollback_result = rollback_applied_states();
        if (!rollback_result.ok) {
            return {.ok = false, .error = visual_asset_rollback_failed_text(undo_error, rollback_result.error)};
        }
        return {.ok = false, .error = undo_error};
    }

    return {.ok = true, .error = {}, .affected_object_count = applied.size()};
}

VisualAssetEditResult set_visual_object_subtree_deleted_state(const VisualObjectSubtreeDeletedStateRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }

    std::size_t root_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, root_record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto descendants_result = list_visual_object_descendants({
        .path = request.path,
        .record_index = root_record_index,
        .object_name = {},
        .unique_id = {}
    });
    if (!descendants_result.ok) {
        return {.ok = false, .error = descendants_result.error};
    }

    std::vector<VisualObjectDeletedStateBatchItem> objects;
    objects.reserve(descendants_result.descendants.size() + 1U);
    objects.push_back({
        .record_index = root_record_index,
        .object_name = {},
        .unique_id = {},
        .deleted = request.deleted
    });
    for (const auto& descendant : descendants_result.descendants) {
        objects.push_back({
            .record_index = descendant.object.record_index,
            .object_name = {},
            .unique_id = {},
            .deleted = request.deleted
        });
    }

    return set_visual_object_deleted_states({
        .path = request.path,
        .objects = std::move(objects)
    });
}

namespace {

bool should_group_report_batch_undo(const std::string& path) {
    auto extension = copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(path).extension());
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return extension == ".frx" || extension == ".lbx";
}

bool visual_property_states_match(const VisualPropertyState& left, const VisualPropertyState& right) {
    return left.exists == right.exists &&
           left.direct_field == right.direct_field &&
           left.property_name == right.property_name &&
           left.value == right.value &&
           left.record_deleted == right.record_deleted;
}

VisualAssetEditResult rollback_visual_object_batch_changes(
    const std::string& path,
    const std::vector<VisualAssetUndoEntry>& applied_changes) {
    for (auto it = applied_changes.rbegin(); it != applied_changes.rend(); ++it) {
        const auto rollback_result = apply_visual_object_property_change(
            {
                .path = path,
                .record_index = it->record_index,
                .object_name = {},
                .unique_id = {},
                .property_name = it->property_name,
                .property_value = it->prior_value
            },
            false,
            !it->prior_value_exists);
        if (!rollback_result.ok) {
            return rollback_result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = 1U};
}

VisualAssetEditResult apply_visual_object_batch_update(
    const std::string& path,
    const std::vector<VisualObjectBatchEditItem>& objects,
    std::size_t affected_object_count) {
    if (path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.EditBatchRequired")};
    }

    std::vector<VisualAssetUndoEntry> applied_changes;
    std::string latest_label;

    for (const auto& object : objects) {
        if (object.properties.empty()) {
            const auto rollback_result = rollback_visual_object_batch_changes(path, applied_changes);
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(
                        visual_asset_text("VisualAssetEditor.Property.ChangeBatchRequired"),
                        rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.ChangeBatchRequired")};
        }

        for (const auto& property : object.properties) {
            std::size_t resolved_record_index = 0U;
            const auto resolution = resolve_visual_object_record_index({
                .path = path,
                .record_index = object.record_index,
                .object_name = object.object_name,
                .unique_id = object.unique_id,
                .property_name = property.property_name,
                .property_value = property.property_value
            }, resolved_record_index);
            if (!resolution.ok) {
                const auto rollback_result = rollback_visual_object_batch_changes(path, applied_changes);
                if (!rollback_result.ok) {
                    return {
                        .ok = false,
                        .error = visual_asset_rollback_failed_text(resolution.error, rollback_result.error)
                    };
                }
                return resolution;
            }

            const auto prior_state = read_current_visual_property_state(path, resolved_record_index, property.property_name);
            if (!prior_state.has_value()) {
                const auto rollback_result = rollback_visual_object_batch_changes(path, applied_changes);
                if (!rollback_result.ok) {
                    return {
                        .ok = false,
                        .error = visual_asset_rollback_failed_text(
                            visual_asset_text("VisualAssetEditor.Undo.CurrentPropertyReadFailed"),
                            rollback_result.error)
                    };
                }
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Undo.CurrentPropertyReadFailed")};
            }

            const auto result = apply_visual_object_property_change({
                .path = path,
                .record_index = object.record_index,
                .object_name = object.object_name,
                .unique_id = object.unique_id,
                .property_name = property.property_name,
                .property_value = property.property_value
            }, false, false);
            if (!result.ok) {
                const auto rollback_result = rollback_visual_object_batch_changes(path, applied_changes);
                if (!rollback_result.ok) {
                    return {
                        .ok = false,
                        .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                    };
                }
                return result;
            }

            const auto current_state = read_current_visual_property_state(path, resolved_record_index, property.property_name);
            if (!current_state.has_value()) {
                const auto rollback_result = rollback_visual_object_batch_changes(path, applied_changes);
                if (!rollback_result.ok) {
                    return {
                        .ok = false,
                        .error = visual_asset_rollback_failed_text(
                            visual_asset_text("VisualAssetEditor.Undo.CurrentPropertyReadFailed"),
                            rollback_result.error)
                    };
                }
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Undo.CurrentPropertyReadFailed")};
            }

            if (!visual_property_states_match(*prior_state, *current_state)) {
                applied_changes.push_back({
                    .record_index = resolved_record_index,
                    .property_name = property.property_name,
                    .prior_value = prior_state->value,
                    .prior_value_exists = prior_state->exists,
                    .label = {},
                    .grouped_changes = {}
                });
                latest_label = visual_asset_text(
                    "VisualAssetEditor.Undo.PropertyLabel",
                    {{"propertyName", property.property_name}});
            }
        }
    }

    if (applied_changes.empty()) {
        return {.ok = true, .error = {}, .affected_object_count = affected_object_count};
    }

    std::string error;
    const auto& latest_change = applied_changes.back();
    if (!record_visual_asset_undo_entry(path, {
            .record_index = latest_change.record_index,
            .property_name = latest_change.property_name,
            .prior_value = latest_change.prior_value,
            .prior_value_exists = latest_change.prior_value_exists,
            .label = latest_label,
            .grouped_changes = applied_changes
        }, error)) {
        const auto rollback_result = rollback_visual_object_batch_changes(path, applied_changes);
        if (!rollback_result.ok) {
            return {.ok = false, .error = visual_asset_rollback_failed_text(error, rollback_result.error)};
        }
        return {.ok = false, .error = error};
    }

    return {.ok = true, .error = {}, .affected_object_count = affected_object_count};
}

}  // namespace

VisualAssetEditResult update_visual_object_properties(const VisualObjectMultiEditRequest& request) {
    if (request.properties.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.ChangeBatchRequired")};
    }

    if (!should_group_report_batch_undo(request.path)) {
        const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
        for (const auto& property : request.properties) {
            const auto result = update_visual_object_property({
                .path = request.path,
                .record_index = request.record_index,
                .object_name = request.object_name,
                .unique_id = request.unique_id,
                .property_name = property.property_name,
                .property_value = property.property_value
            });
            if (!result.ok) {
                while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
                    const auto rollback_result = undo_visual_object_property(request.path);
                    if (!rollback_result.ok) {
                        return {
                            .ok = false,
                            .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                        };
                    }
                }
                return result;
            }
        }

        return {.ok = true, .error = {}, .affected_object_count = 1U};
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = {
            {
                .record_index = request.record_index,
                .object_name = request.object_name,
                .unique_id = request.unique_id,
                .properties = request.properties
            }
        }
    });
}

VisualAssetEditResult update_visual_object_batch(const VisualObjectBatchEditRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.EditBatchRequired")};
    }

    if (!should_group_report_batch_undo(request.path)) {
        const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
        for (const auto& object : request.objects) {
            if (object.properties.empty()) {
                while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
                    const auto rollback_result = undo_visual_object_property(request.path);
                    if (!rollback_result.ok) {
                        return {
                            .ok = false,
                            .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.ChangeBatchRequired"), rollback_result.error)
                        };
                    }
                }
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.ChangeBatchRequired")};
            }

            const auto result = update_visual_object_properties({
                .path = request.path,
                .record_index = object.record_index,
                .object_name = object.object_name,
                .unique_id = object.unique_id,
                .properties = object.properties
            });
            if (!result.ok) {
                while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
                    const auto rollback_result = undo_visual_object_property(request.path);
                    if (!rollback_result.ok) {
                        return {
                            .ok = false,
                            .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                        };
                    }
                }
                return result;
            }
        }

        return {.ok = true, .error = {}, .affected_object_count = request.objects.size()};
    }

    std::vector<VisualObjectBatchEditItem> expanded_objects;
    const auto expansion_result = expand_report_section_top_batch_updates(
        request.path,
        request.objects,
        expanded_objects);
    if (!expansion_result.ok) {
        return expansion_result;
    }

    return apply_visual_object_batch_update(
        request.path,
        expanded_objects,
        request.objects.size());
}

}  // namespace copperfin::vfp
