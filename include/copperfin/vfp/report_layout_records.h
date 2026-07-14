// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <cmath>
#include <limits>

namespace copperfin::vfp {

inline bool is_report_settings_root_record(int objtype) {
    return objtype == 1;
}

inline int truncate_report_layout_geometry(double value, int fallback = 0) {
    if (!std::isfinite(value) ||
        value < static_cast<double>(std::numeric_limits<int>::min()) ||
        value > static_cast<double>(std::numeric_limits<int>::max())) {
        return fallback;
    }
    return static_cast<int>(value);
}

} // namespace copperfin::vfp
