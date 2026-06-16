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
    std::string property_name;
    std::string property_value;
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
VisualAssetEditResult update_visual_object_property(const VisualObjectEditRequest& request);
[[nodiscard]] VisualAssetUndoStatus query_visual_object_undo(const std::string& path);
VisualAssetEditResult undo_visual_object_property(const std::string& path);

}  // namespace copperfin::vfp
