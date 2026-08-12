// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <string>

namespace copperfin::platform {

struct FileVersionMetadata {
    std::string full_version = "0.0.0.0";
    std::string file_description;
    std::string company_name;
    std::string file_version = "0.0.0.0";
    std::string product_name;
    std::string product_version = "0.0.0.0";
    std::string trademark_or_copyright;
};

// Reads Windows version-resource metadata without exposing host SDK types.
// Files without readable metadata retain stable fallback values.
[[nodiscard]] FileVersionMetadata read_file_version_metadata(
    const std::filesystem::path& path);

}  // namespace copperfin::platform
