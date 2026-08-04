// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
