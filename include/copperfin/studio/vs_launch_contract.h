// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
    StudioOpenRequest request{};
    std::string error;
};

LaunchParseResult parse_launch_arguments(const std::vector<std::string>& args);
LaunchParseResult parse_launch_arguments(
    const std::vector<std::string>& args,
    const localization::LocalizedCatalog& catalog);

}  // namespace studio

}  // namespace copperfin
