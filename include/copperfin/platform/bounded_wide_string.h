// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <cwchar>
#include <string>

namespace copperfin::platform {

// Constructs a std::wstring from a buffer whose only known bound is a
// declared length in wchar_t units -- e.g. Win32's VerQueryValueW puLen for
// a StringFileInfo query -- NOT a guarantee that a null terminator exists
// within that length. Some resource-writing tools declare a length that
// includes no terminator, or lies about it. Using the
// null-terminator-scanning std::wstring(const wchar_t*) constructor
// directly on such a buffer reads past the declared length looking for a
// zero wherever one happens to occur in memory: an out-of-bounds read once
// the buffer's true allocation ends at declared_length. wcsnlen() bounds
// the scan to at most declared_length characters regardless of what the
// data actually contains, while still stopping early at an embedded null
// terminator when the buffer is well-formed.
inline std::wstring bounded_wide_string(
    const wchar_t* text, std::size_t declared_length) {
    return std::wstring(text, wcsnlen(text, declared_length));
}

}  // namespace copperfin::platform
