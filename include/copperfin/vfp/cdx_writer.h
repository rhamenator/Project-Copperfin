// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

// #5534 (index-rebuild half, the sibling of the already-shipped
// relation-inference half -- #5534/docs/74): writes a new, real-VFP9-
// verified single-tag compound index (.cdx) file.
//
// Every existing CDX-consuming module in this codebase (index_probe.cpp,
// cdx_header.cpp) is explicitly "header-probe-only, no B-tree
// materialization" (docs/13); no community reverse-engineering source
// documents VFP's CDX B-tree leaf-node key-storage format the way
// mdbtools documents Jet/ACE. This was determined from scratch this
// session against real VFP9 (9.0.00.7423, the local copperfin-vfp9-win11
// VM): two real fixtures were hand-analyzed to derive the format, and a
// hand-built file (built independently from that derived understanding,
// for data VFP9 never itself indexed) was confirmed to open, traverse,
// and SEEK() correctly under real VFP9 -- the strongest verification
// available. See docs/77-cdx-index-write-format-notes.md for the full
// evidence trail, byte-level format tables, and documented gaps.
//
// Deliberately scoped to the single most tractable, fully-verified case:
//   - A single tag (no compound multi-tag CDX).
//   - Ascending order only (no DESCENDING).
//   - Character-type keys only (no numeric/date/logical).
//   - No UNIQUE, no FOR clause.
//   - Record numbers 0-255 only -- real evidence (a 260-record fixture)
//     shows a wider entry encoding exists for larger record numbers, but
//     its exact byte layout was not independently derived or
//     write-side-verified this session (see docs/77).
//   - Individual keys of 15 bytes or fewer only -- the front-compression
//     control byte packs a key's own trimmed length into a 4-bit
//     nibble, so longer keys cannot be represented and fail closed
//     (Vfp.CdxWriter.Error.KeyLengthUnsupported) rather than
//     truncating into a corrupt leaf page (see docs/77).
//   - All entries must fit in a single 512-byte leaf page (no B-tree
//     splitting/branch nodes) -- real evidence shows VFP9 introduces an
//     internal-node structure once a tag's data exceeds one leaf page,
//     but that structure's exact format was not derived (see docs/77).
// Each of these is a real, explicitly out-of-scope gap, not an
// oversight -- fails closed with a structured error rather than
// producing a file whose correctness for that case was never verified.
struct CdxIndexEntry {
    // Deliberately wider than the single byte the leaf entry format can
    // actually hold (0-255, see the "record numbers beyond 255" gap in
    // docs/77): a caller-side narrowing straight to std::uint8_t would
    // silently wrap a record number of, say, 256 down to 0 before this
    // API could ever see the out-of-range value. create_vfp_cdx_single_
    // tag_index_file() itself validates this and fails closed
    // (Vfp.CdxWriter.Error.RecordNumberOutOfRange) rather than narrowing
    // an unchecked value.
    std::uint32_t record_number = 0;
    // The field's own raw value (space-padded or not); trailing spaces
    // are trimmed internally before front-compression, matching VFP's
    // own observed character-key storage convention.
    std::string key_field_value;
};

struct CdxWriteResult {
    bool ok = false;
    std::string error;
};

// Writes a new single-tag CDX file at `cdx_path` (overwriting any
// existing file at that path). `entries` need not be pre-sorted --
// they are sorted internally by the trimmed key value (ascending,
// byte-wise). `key_length` is the tag's declared key length (e.g. 10
// for a `C(10)` field); every entry's own trimmed key must not exceed
// it.
//
// Also sets the associated table's own DBF header `has_production_index`
// bit (dbf_header.h's existing DbfHeader::has_production_index(), byte
// 28 bit 0x01) via `dbf_path` -- real VFP9 was found this session to
// refuse SET ORDER TO TAG ... OF <cdx file> against a table lacking this
// bit, even when the CDX file is named explicitly rather than relying on
// automatic same-basename production-index association (see docs/77).
// This codebase's own DBF writers (dbf_table.cpp) do not currently set
// this bit, so any table this function indexes needs it set here.
[[nodiscard]] CdxWriteResult create_vfp_cdx_single_tag_index_file(
    const std::string& cdx_path,
    const std::string& dbf_path,
    const std::string& tag_name,
    const std::string& key_expression,
    std::uint16_t key_length,
    const std::vector<CdxIndexEntry>& entries);

}  // namespace copperfin::vfp
