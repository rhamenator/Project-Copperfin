// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once

#include <bit>
#include <cstdint>

namespace copperfin::test {
// RQ-CF-PRG-CAST-BYTE-NUMERIC-001 / RQ-CF-PRG-CAST-UINT64-NUMERIC-001:
// independent binary64 predecessor of -1 toward zero truncates to zero.
// RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001 native profile:50 proves
// the PRG spelling -0.9999999999999999 is instead EXACT -1. Do not conflate
// C++ direct-input construction with native source-decimal conversion.
static_assert(std::bit_cast<std::uint64_t>(-0.9999999999999999)==0xBFEFFFFFFFFFFFFFULL);
inline constexpr const char cast_inner_negative_neighbor[] =
    "IIF(BINTOC(CTOBIN(CHR(191)+CHR(239)+REPLICATE(CHR(255),6),'BS'),'BRS')=="
    "CHR(191)+CHR(239)+REPLICATE(CHR(255),6),"
    "CTOBIN(CHR(191)+CHR(239)+REPLICATE(CHR(255),6),'BS'),-1)";
// The byte-roundtrip guard makes a wrong reconstructed bit pattern select -1,
// which MUST reject, not accidentally satisfy the preserved zero expectation.
// BS decodes big-endian IEEE bytes; BRS emits them. This is not source-number
// special handling in production, a native unsupported-CAST claim or a tolerance.
} // namespace copperfin::test
