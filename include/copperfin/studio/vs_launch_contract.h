// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/studio/document_model.h"

#include <string>
#include <vector>

namespace copperfin {

namespace localization {
struct LocalizedCatalog;
}  // namespace localization

namespace studio {

struct LaunchParseResult {
    bool ok = false;
    bool show_help = false;
    bool output_json = false;
    bool mutates_asset = false;
    StudioOpenRequest request{};
    std::string error;
};

LaunchParseResult parse_launch_arguments(const std::vector<std::string>& args);
LaunchParseResult parse_launch_arguments(
    const std::vector<std::string>& args,
    const localization::LocalizedCatalog& catalog);

}  // namespace studio

}  // namespace copperfin
