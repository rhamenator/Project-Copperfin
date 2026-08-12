// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <array>
#include <optional>
#include <string>

namespace copperfin::platform {

[[nodiscard]] std::optional<int> parse_posix_locale_code_page(
    std::string locale_or_codeset);

[[nodiscard]] int resolve_posix_host_code_page(
    const std::optional<std::string>& nl_codeset,
    const std::array<std::optional<std::string>, 3U>& locale_candidates);

[[nodiscard]] int host_code_page();
[[nodiscard]] int host_oem_code_page();

[[nodiscard]] std::optional<std::string> convert_code_page_bytes(
    int source_code_page,
    int target_code_page,
    const std::string& input);

}  // namespace copperfin::platform
