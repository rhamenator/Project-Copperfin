#pragma once

#include <cstddef>
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
    std::vector<VisualObjectPropertySnapshot> properties;
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
VisualAssetEditResult update_visual_object_property(const VisualObjectEditRequest& request);
VisualAssetEditResult update_visual_object_properties(const VisualObjectMultiEditRequest& request);
[[nodiscard]] VisualAssetUndoStatus query_visual_object_undo(const std::string& path);
VisualAssetEditResult undo_visual_object_property(const std::string& path);

}  // namespace copperfin::vfp
