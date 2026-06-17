#pragma once

#include "copperfin/vfp/visual_asset_editor.h"

#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioToolboxObjectCreateRequest {
    std::string path;
    std::string toolbox_item_id;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

[[nodiscard]] vfp::VisualObjectCreateResult create_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request);

}  // namespace copperfin::studio
