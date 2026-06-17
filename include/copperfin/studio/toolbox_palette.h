#pragma once

#include <string_view>
#include <vector>

namespace copperfin::studio {

enum class StudioToolboxContext {
    form,
    class_designer,
    container,
    report
};

struct StudioToolboxItemDescriptor {
    std::string_view id;
    std::string_view title;
    std::string_view category;
    std::string_view vfp_class;
    std::string_view base_class;
    std::string_view default_name_prefix;
    std::vector<StudioToolboxContext> contexts;
    bool container = false;
    std::string_view description;
};

[[nodiscard]] const char* studio_toolbox_context_name(StudioToolboxContext context);
[[nodiscard]] const std::vector<StudioToolboxItemDescriptor>& studio_toolbox_palette();
[[nodiscard]] std::vector<StudioToolboxItemDescriptor> studio_toolbox_items_for_context(StudioToolboxContext context);

}  // namespace copperfin::studio
