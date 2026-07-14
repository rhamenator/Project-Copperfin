// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

namespace copperfin::vfp {

inline bool is_report_settings_root_record(int objtype) {
    return objtype == 1;
}

} // namespace copperfin::vfp
