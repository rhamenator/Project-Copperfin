// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/vfp/access_container.h"

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

// #5539 (parent #141, related #5476, #5474, #68, #5540): decodes rows from
// the MSysObjects system catalog table -- the general Jet3/Jet4 data-row
// decoding algorithm applied to the one well-known, well-documented system
// table, per this issue's own explicitly scoped acceptance criteria (see
// docs/72-access-msysobjects-row-decoding.md for the full evidence basis
// and worked byte-level walkthroughs this implementation is grounded in).
//
// Grounded in the mdbtools project's own HACKING.md (GPL, community
// reverse-engineering documentation -- see docs/68's provenance discussion)
// and independently cross-checked against real Jet3 and Jet4 .mdb fixtures
// during this slice's development, including using the (locally installed)
// mdbtools `mdb-sql` tool itself as an independent-reader ground truth for
// the decoded Id/ParentId/Name/Type values -- not merely trusting the
// community documentation the way the earlier TDEF-page slice (#5476) also
// went beyond. See docs/72 for the full record of what was checked and how.

struct AccessCatalogEntry {
    // The catalog row's own Id field, exactly as stored (not masked).
    std::uint32_t id = 0;
    std::uint32_t parent_id = 0;
    std::string name;
    // MSysObjects.Type: 1 = table, other values cover queries/forms/
    // reports/relationships/etc. This slice only surfaces Type == 1 rows
    // in scan_access_msysobjects_catalog()'s result, per its own scope
    // (table-name enumeration), but the field is exposed on every decoded
    // entry regardless in case a future caller needs the raw catalog.
    std::int16_t type = 0;
    // id & 0x00FFFFFF -- the candidate TDEF page number, per mdbtools'
    // own mdb_read_catalog() precedent (cited during this issue's review
    // discussion; see docs/72). Only meaningful for Type == 1 rows: it is
    // NOT independently verified to be a reliable page pointer on its
    // own -- scan_access_container_schema() only attaches a name to a
    // discovered table when this candidate page number matches an
    // actually-discovered TDEF page, rather than trusting it blind.
    std::uint32_t candidate_page_number = 0;
};

struct AccessMSysObjectsScanResult {
    bool ok = false;
    std::string error;
    // Every successfully decoded MSysObjects row (all Type values, not
    // just tables) -- callers that only want tables should filter on
    // type == 1 themselves, matching AccessCatalogEntry::type's own
    // documented scope note.
    std::vector<AccessCatalogEntry> entries;
    struct SkippedRow {
        std::uint32_t page_number = 0;
        std::uint32_t row_index = 0;
        std::string reason;
    };
    // A row this slice could not decode (out-of-bounds structure, a
    // >= 256-byte Jet3 row requiring jump-table offset decoding -- see
    // this header's own documented non-goal below -- or a lookup-overflow
    // row pointing at another page) is recorded here instead of silently
    // dropped or aborting the whole scan, matching
    // AccessContainerSchemaResult::skipped's own precedent.
    std::vector<SkippedRow> skipped;
};

// Scans every data page belonging to MSysObjects (page 2's own TDEF,
// itself read via parse_access_table_definition_page()) and decodes each
// row using the general Jet3/Jet4 row-decoding algorithm.
//
// Deliberately out of scope for this slice (see docs/72 for the full
// reasoning, mirroring #5476's own documented non-goals):
// - Jet3 rows >= 256 bytes, which require jump-table offset decoding.
//   docs/68 already flagged the row-decode algorithm generally as having
//   "no independently-checkable invariant"; the jump-table sub-path in
//   particular could not be verified against any real fixture available
//   during this slice's development (every real MSysObjects row observed
//   was well under 256 bytes). Rather than ship an interpretation of the
//   documented algorithm that has never been checked against real bytes,
//   such a row is recorded in AccessMSysObjectsScanResult::skipped with a
//   clear reason instead of silently guessed at.
// - Jet4 "compressed unicode" TEXT values (a 0xFF 0xFE-prefixed encoding).
//   Every real Jet4 Name value observed during this slice's development
//   was plain (uncompressed) UCS-2LE; a compressed value is detected (by
//   its leading marker bytes) and recorded in `skipped` rather than
//   decoded, for the same real-fixture-verification reason as above.
// - A lookup-overflow row (offset table entry's 0x4000 flag set), which
//   points at a Data Pointer to another page rather than storing the row
//   inline -- recorded in `skipped` rather than followed.
[[nodiscard]] AccessMSysObjectsScanResult scan_access_msysobjects_catalog(const std::string& path);

}  // namespace copperfin::vfp
