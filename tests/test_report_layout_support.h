// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/studio/report_layout.h"
#include "copperfin/localization/localization.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace cf_test_report_layout {

extern int failures;

void expect(bool condition, const std::string& message);

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys);

copperfin::vfp::DbfRecordValue value(
    std::string name,
    std::string display_value,
    std::uint32_t memo_block_number = 0U);

}  // namespace cf_test_report_layout
