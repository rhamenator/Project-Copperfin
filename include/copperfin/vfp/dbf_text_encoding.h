// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace copperfin::vfp {

enum class DbfTextEncodingError {
    none,
    unsupported_code_page,
    invalid_input,
    unrepresentable_character
};

struct DbfTextConversionResult {
    bool ok = false;
    std::string text;
    DbfTextEncodingError error = DbfTextEncodingError::invalid_input;
};

// DBF tables without a code-page mark use Copperfin's established UTF-8
// compatibility mode. Marked tables are decoded using their DBF/VFP code page.
DbfTextConversionResult decode_dbf_text(
    std::uint8_t code_page_mark,
    std::string_view bytes);

// Converts UTF-8 editor text to the table's original code page. Conversion is
// lossless: invalid UTF-8 and characters unavailable in the target page fail.
DbfTextConversionResult encode_dbf_text(
    std::uint8_t code_page_mark,
    std::string_view utf8_text);

}  // namespace copperfin::vfp
