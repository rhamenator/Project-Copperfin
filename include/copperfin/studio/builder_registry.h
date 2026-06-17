#pragma once

#include <string_view>
#include <vector>

namespace copperfin::studio {

enum class StudioBuilderKind {
    builder,
    wizard
};

enum class StudioBuilderContext {
    form,
    class_designer,
    control,
    report,
    label,
    project,
    data_environment
};

struct StudioBuilderDescriptor {
    std::string_view id;
    std::string_view title;
    StudioBuilderKind kind = StudioBuilderKind::builder;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string_view vfp9_equivalent;
    std::string_view copperfin_component;
    std::string_view entry_point;
    std::string_view description;
};

[[nodiscard]] const char* studio_builder_kind_name(StudioBuilderKind kind);
[[nodiscard]] const char* studio_builder_context_name(StudioBuilderContext context);
[[nodiscard]] const std::vector<StudioBuilderDescriptor>& studio_builder_registry();
[[nodiscard]] std::vector<StudioBuilderDescriptor> studio_builders_for_context(StudioBuilderContext context);

}  // namespace copperfin::studio
