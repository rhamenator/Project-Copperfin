#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

struct VisualPropertyAssignment {
    std::string name;
    std::string value;
    std::size_t source_line_index = static_cast<std::size_t>(-1);
};

struct VisualObjectEditRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
    std::string property_value;
};

struct VisualObjectPropertyChange {
    std::string property_name;
    std::string property_value;
};

struct VisualObjectMultiEditRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::vector<VisualObjectPropertyChange> properties;
};

struct VisualObjectBatchEditItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::vector<VisualObjectPropertyChange> properties;
};

struct VisualObjectBatchEditRequest {
    std::string path;
    std::vector<VisualObjectBatchEditItem> objects;
};

struct VisualObjectPropertyQueryRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
};

struct VisualObjectPropertyQueryResult {
    bool ok = false;
    std::string error;
    bool exists = false;
    bool direct_field = false;
    std::size_t record_index = 0;
    bool record_deleted = false;
    std::string property_name;
    std::string value;
};

struct VisualObjectPropertyListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectPropertySnapshot {
    std::string property_name;
    std::string value;
    bool direct_field = false;
    char field_type = '\0';
    std::size_t source_line_index = static_cast<std::size_t>(-1);
};

struct VisualObjectPropertyListResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
    bool record_deleted = false;
    std::vector<VisualObjectPropertySnapshot> properties;
};

struct VisualObjectDeletedStateRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool deleted = false;
};

struct VisualObjectDeletedStateBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool deleted = false;
};

struct VisualObjectDeletedStateBatchRequest {
    std::string path;
    std::vector<VisualObjectDeletedStateBatchItem> objects;
};

struct VisualObjectSubtreeDeletedStateRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool deleted = false;
};

struct VisualObjectSnapshot {
    std::size_t record_index = 0;
    bool deleted = false;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::string class_name;
    std::string baseclass_name;
    std::string caption;
};

struct VisualObjectListResult {
    bool ok = false;
    std::string error;
    std::vector<VisualObjectSnapshot> objects;
};

struct VisualObjectChildrenListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectChildrenListResult {
    bool ok = false;
    std::string error;
    std::size_t parent_record_index = 0;
    std::string parent_name;
    std::vector<VisualObjectSnapshot> children;
};

struct VisualObjectDescendantSnapshot {
    VisualObjectSnapshot object;
    std::size_t depth = 0;
};

struct VisualObjectDescendantsListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectDescendantsListResult {
    bool ok = false;
    std::string error;
    std::size_t parent_record_index = 0;
    std::string parent_name;
    std::vector<VisualObjectDescendantSnapshot> descendants;
};

struct VisualObjectAncestorSnapshot {
    VisualObjectSnapshot object;
    std::size_t depth = 0;
};

struct VisualObjectAncestorsListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectAncestorsListResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
    std::vector<VisualObjectAncestorSnapshot> ancestors;
};

struct VisualObjectMethodListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectMethodSnapshot {
    std::string method_name;
    std::string kind;
    std::string source_text;
    std::size_t source_line_index = static_cast<std::size_t>(-1);
    std::uint32_t source_memo_block_number = 0;
};

struct VisualObjectMethodListResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
    bool record_deleted = false;
    std::vector<VisualObjectMethodSnapshot> methods;
};

struct VisualObjectMethodEditRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
    std::string method_kind = "procedure";
    std::string source_text;
};

struct VisualObjectMethodDeleteRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
};

struct VisualObjectDuplicateRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string new_object_name;
    std::string new_name;
    std::string new_unique_id;
};

struct VisualObjectDuplicateResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
};

struct VisualObjectCreateRequest {
    std::string path;
    std::vector<VisualObjectPropertyChange> field_values;
};

struct VisualObjectCreateResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
};

struct VisualObjectReparentRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string parent_object_name;
    std::string parent_unique_id;
    bool clear_parent = false;
};

struct VisualObjectRenameRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool update_object_name = false;
    std::string new_object_name;
    bool update_name = false;
    std::string new_name;
    bool update_unique_id = false;
    std::string new_unique_id;
};

struct VisualObjectReorderRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string placement;
    std::string target_object_name;
    std::string target_unique_id;
};

struct VisualAssetEditResult {
    bool ok = false;
    std::string error;
};

struct VisualAssetUndoStatus {
    bool available = false;
    std::string label;
};

[[nodiscard]] std::vector<VisualPropertyAssignment> parse_visual_property_blob(const std::string& text);
[[nodiscard]] std::string serialize_visual_property_blob(const std::vector<VisualPropertyAssignment>& properties);
[[nodiscard]] bool is_property_blob_asset_path(const std::string& path);
[[nodiscard]] VisualObjectPropertyQueryResult query_visual_object_property(const VisualObjectPropertyQueryRequest& request);
[[nodiscard]] VisualObjectPropertyListResult list_visual_object_properties(const VisualObjectPropertyListRequest& request);
[[nodiscard]] VisualObjectListResult list_visual_objects(const std::string& path);
[[nodiscard]] VisualObjectChildrenListResult list_visual_object_children(const VisualObjectChildrenListRequest& request);
[[nodiscard]] VisualObjectDescendantsListResult list_visual_object_descendants(const VisualObjectDescendantsListRequest& request);
[[nodiscard]] VisualObjectAncestorsListResult list_visual_object_ancestors(const VisualObjectAncestorsListRequest& request);
[[nodiscard]] VisualObjectMethodListResult list_visual_object_methods(const VisualObjectMethodListRequest& request);
VisualAssetEditResult update_visual_object_method(const VisualObjectMethodEditRequest& request);
VisualAssetEditResult delete_visual_object_method(const VisualObjectMethodDeleteRequest& request);
VisualObjectDuplicateResult duplicate_visual_object(const VisualObjectDuplicateRequest& request);
VisualObjectCreateResult create_visual_object(const VisualObjectCreateRequest& request);
VisualAssetEditResult reparent_visual_object(const VisualObjectReparentRequest& request);
VisualAssetEditResult rename_visual_object(const VisualObjectRenameRequest& request);
VisualAssetEditResult reorder_visual_object(const VisualObjectReorderRequest& request);
VisualAssetEditResult set_visual_object_deleted_state(const VisualObjectDeletedStateRequest& request);
VisualAssetEditResult set_visual_object_deleted_states(const VisualObjectDeletedStateBatchRequest& request);
VisualAssetEditResult set_visual_object_subtree_deleted_state(const VisualObjectSubtreeDeletedStateRequest& request);
VisualAssetEditResult update_visual_object_property(const VisualObjectEditRequest& request);
VisualAssetEditResult update_visual_object_properties(const VisualObjectMultiEditRequest& request);
VisualAssetEditResult update_visual_object_batch(const VisualObjectBatchEditRequest& request);
[[nodiscard]] VisualAssetUndoStatus query_visual_object_undo(const std::string& path);
VisualAssetEditResult undo_visual_object_property(const std::string& path);

}  // namespace copperfin::vfp
