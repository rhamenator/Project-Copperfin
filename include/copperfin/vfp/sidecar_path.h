// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COPPERFIN_VFP_SIDECAR_PATH_H
#define COPPERFIN_VFP_SIDECAR_PATH_H

#include <filesystem>
#include <optional>
#include <string_view>

namespace copperfin::vfp {

struct SidecarPathResolution {
    std::filesystem::path requested_path{};
    std::optional<std::filesystem::path> path{};
    bool ambiguous = false;
};

[[nodiscard]] SidecarPathResolution resolve_unique_casefold_path(
    const std::filesystem::path& candidate,
    bool require_regular_file = true);

[[nodiscard]] SidecarPathResolution resolve_vfp_sidecar_path(
    const std::filesystem::path& primary_path,
    std::string_view sidecar_extension);

[[nodiscard]] SidecarPathResolution resolve_vfp_memo_sidecar_path(
    const std::filesystem::path& primary_path);

}  // namespace copperfin::vfp

#endif
