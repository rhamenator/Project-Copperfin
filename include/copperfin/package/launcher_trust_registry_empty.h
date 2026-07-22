// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/package/launcher_inventory_trust.h"

#include <array>

namespace copperfin::package_trust {

// Release builds may configure an out-of-tree replacement for this header.
// The default stays empty so development packages retain their documented
// unsigned fallback until an approved release signer is provisioned.
inline constexpr std::array<LauncherInventoryTrustedKey, 0>
    kKnownLauncherInventoryTrustedKeys{};

}  // namespace copperfin::package_trust
