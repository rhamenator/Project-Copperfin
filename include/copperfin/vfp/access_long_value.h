// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/vfp/access_container.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::vfp {

// #5549 (parent #141, related #5476, #5539, #5477, #5478, #5479): general
// Jet3/Jet4 long-value (memo/OLE) column reading -- the mechanism Access
// uses for a column value too large to fit inline in a row. This is the
// common prerequisite #5477 (forms/reports), #5478 (VBA extraction), and
// #5479 (saved queries) each need to retrieve their own large blob from
// some system object's row, rather than each reinventing it.
//
// Grounded in mdbtools' HACKING.md ("Each memo column ..." and "LVAL
// (Long Value) Pages" sections -- the same community reverse-engineering
// documentation already established as this codebase's evidence source
// for #5476/#5539, see docs/68) and independently real-fixture-verified
// this session against two real local Jet3 .mdb files:
//
//   - The 12-byte in-row field descriptor (3-byte declared_length, 1-byte
//     bitmask, 4-byte lval_dp "data pointer", 4 reserved/unknown bytes)
//     matches every real example exercised.
//   - Bitmask 0x80 (inline): the value is appended directly after the
//     12-byte header, declared_length bytes long.
//   - Bitmask 0x40 (single LVAL page, "record type 1"): lval_dp addresses
//     one row on one LVAL page (identified by the literal ASCII marker
//     "LVAL" at the page's tdef_pg field position, replacing the numeric
//     pointer a regular data page has there); that row's entire byte
//     span -- via the same page/row-directory format regular data pages
//     use, which access_msysobjects.cpp's row decoder already relies on
//     -- IS the value, declared_length bytes long.
//   - Bitmask 0x00 (chained LVAL pages, "record type 2"): the addressed
//     row begins with its own 4-byte data pointer to the NEXT row in the
//     chain (0 terminates it), followed by that hop's partial value
//     bytes; concatenating every hop's partial bytes in declaration order
//     recovers the full declared_length-byte value. Verified against a
//     real 15-hop chain (29013 bytes across 15 LVAL pages) that
//     reassembles to exactly its declared length.
//   - The data pointer's own bit layout: the row_id is the LOW byte, the
//     page number is the upper 3 bytes (value >> 8) -- the OPPOSITE
//     convention from MSysObjects.Id's own page-number masking
//     (id & 0x00FFFFFF, see access_msysobjects.h). This was confirmed by
//     locating a real fixture's literal "LVAL" page marker at the page
//     number this convention predicts, and NOT at the (out-of-file-range)
//     page number the other convention would have predicted.
//
// Deliberately out of scope for this slice:
//   - Writing/mutation of any kind.
//   - Interpreting what is *inside* a retrieved long value (e.g.
//     MSysObjects.LvProp's own documented inner chunk structure, VBA
//     project decompression, form/report structural parsing) -- those
//     are each their own separate, already-tracked issues (#5477, #5478,
//     #5479) that consume this capability's raw-bytes output.
//   - Jet4 "compressed unicode" text decoding for a *text*-typed memo
//     value (its raw bytes are still returned unchanged; only
//     interpreting them as text is out of scope, matching
//     access_msysobjects.cpp's own existing, separately-tracked gap for
//     the same encoding).
struct AccessLongValueFieldDescriptor {
    std::uint32_t declared_length = 0;  // 3-byte memo_len field.
    std::uint8_t bitmask = 0;
    std::uint32_t lval_dp = 0;  // Raw 4-byte data pointer (meaningless when bitmask == 0x80).
};

// Parses the 12-byte in-row field descriptor from the start of a
// long-value column's own raw bytes, as already extracted from a decoded
// row (e.g. the variable-column span this codebase's row decoders already
// resolve). Returns no value if fewer than 12 bytes are given.
[[nodiscard]] std::optional<AccessLongValueFieldDescriptor> parse_access_long_value_field_descriptor(
    const std::vector<std::uint8_t>& column_bytes);

struct AccessLongValueResult {
    bool ok = false;
    std::vector<std::uint8_t> value;
    std::string error;
};

// Resolves a long-value (memo/OLE) column's actual bytes.
//
// `column_bytes` is the column's own raw bytes as already extracted from
// a decoded row (at least 12 bytes: the field descriptor, plus -- for the
// inline case only -- the value itself appended directly after it).
// `path`/`generation` are consulted only for the LVAL-page-resident cases
// (bitmask 0x40/0x00), to read the container's overflow pages; the page
// size is derived from `generation` via access_container_page_size().
//
// Fails closed (a structured error, not a guessed/partial value) for: a
// column shorter than the 12-byte descriptor; an inline value whose
// declared length does not match the bytes actually present; a bitmask
// other than the three documented values; an LVAL-page read that cannot
// be resolved (out-of-range page, missing "LVAL" marker, malformed row
// directory, row_id past the page's own declared row count); or a chain
// exceeding a bounded hop count (guards against a corrupt or malicious
// cyclic chain rather than looping indefinitely).
[[nodiscard]] AccessLongValueResult read_access_long_value_column(
    const std::string& path,
    AccessContainerGeneration generation,
    const std::vector<std::uint8_t>& column_bytes);

}  // namespace copperfin::vfp
