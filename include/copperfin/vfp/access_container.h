// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

// Container-level classification only, derived from the file's leading
// signature bytes -- not a claim of page/table/row read support. See
// docs/68-access-mdb-jet-physical-page-layout-notes.md for the format
// evidence and its provenance.
enum class AccessContainerFamily {
    unknown,
    jet,
    ace
};

// Coarse engine-generation bucket, derived from the single generation byte
// at offset 0x14. Only the low two values are well-corroborated across
// independent community sources; anything at or above 0x02 is bucketed as
// "later" rather than mapped to a specific Access edition/year, since
// sources disagree on that finer mapping (see docs/68 for the detail).
enum class AccessContainerGeneration {
    unknown,
    jet3,
    jet4,
    later
};

struct AccessContainerHeader {
    AccessContainerFamily family = AccessContainerFamily::unknown;
    AccessContainerGeneration generation = AccessContainerGeneration::unknown;
    std::uint8_t generation_byte = 0;

    [[nodiscard]] bool looks_like_access_container() const;
};

[[nodiscard]] const char* access_container_family_name(AccessContainerFamily family);
[[nodiscard]] const char* access_container_generation_name(AccessContainerGeneration generation);

struct AccessContainerParseResult {
    bool ok = false;
    AccessContainerHeader header{};
    std::string error;
};

AccessContainerParseResult parse_access_container_header(const std::vector<std::uint8_t>& bytes);
AccessContainerParseResult parse_access_container_header_from_file(const std::string& path);

}  // namespace copperfin::vfp
