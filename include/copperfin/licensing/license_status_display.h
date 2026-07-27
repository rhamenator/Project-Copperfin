// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <string>

#include "copperfin/licensing/license_status.h"
#include "copperfin/localization/localization.h"

namespace copperfin::licensing {

// Presentation-only helper. The licensing core keeps raw diagnostics so it
// remains dependency-free; native CLI/host surfaces use this helper to route
// known human-facing messages through the active catalog.
inline std::string localized_license_diagnostic(
    const LicenseStatus& status,
    const localization::LocalizedCatalog& catalog) {
    if (status.diagnostic_key.empty()) {
        return status.diagnostic;
    }

    const std::string translated = catalog.translate(
        status.diagnostic_key,
        status.diagnostic_argument.empty()
            ? localization::PlaceholderMap{}
            : localization::PlaceholderMap{{"argument", status.diagnostic_argument}});
    return translated == status.diagnostic_key ? status.diagnostic : translated;
}

}  // namespace copperfin::licensing
