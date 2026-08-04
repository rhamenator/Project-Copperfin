// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <optional>
#include <string>

namespace copperfin::runtime::detail {

std::optional<int> parse_posix_locale_code_page(std::string locale_or_codeset);

int resolve_posix_host_code_page(
    const std::optional<std::string>& nl_codeset,
    const std::array<std::optional<std::string>, 3U>& locale_candidates);

int default_host_code_page();
int default_host_oem_code_page();
bool is_supported_vfp_code_page(int code_page);
bool is_lead_byte_for_code_page(int code_page, unsigned char byte);

}  // namespace copperfin::runtime::detail
