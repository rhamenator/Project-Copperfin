// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace copperfin::localization {
struct LocalizedCatalog;
}

namespace copperfin::studio {

enum class ProductHostKind {
    native_ide,
    visual_studio_shell,
    shared_service
};

struct ProductSubsystemDescriptor {
    std::string id;
    std::string title;
    std::string vfp9_equivalent;
    std::string vfp9_equivalent_display;
    std::string copperfin_component;
    ProductHostKind host_kind = ProductHostKind::native_ide;
    std::string current_status;
    std::string parity_scope;
    std::string modern_editor_direction;
};

[[nodiscard]] const char* product_host_kind_name(ProductHostKind kind);
[[nodiscard]] std::vector<ProductSubsystemDescriptor> product_subsystems();
[[nodiscard]] std::vector<ProductSubsystemDescriptor> product_subsystems_for_catalog(
    const copperfin::localization::LocalizedCatalog& catalog);

}  // namespace copperfin::studio
