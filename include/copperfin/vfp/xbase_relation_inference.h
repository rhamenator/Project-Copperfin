// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>
#include <vector>

namespace copperfin::vfp {

// #5534 (parent #5517's IMPORT DATABASE ... TYPE XBASE wizard): first
// slice of "import the source table's index files ... and use the
// recovered key expressions to infer relationships between imported
// tables ... then present those inferred relations to the user." This
// slice covers only the read-only relation-*inference* half; the
// companion index-*rebuild* half (translating recovered key expressions
// into freshly-built VFP-native CDX tags on the destination table) is a
// separate, materially larger undertaking -- a real CDX B-tree writer,
// for which this codebase has no existing precedent (every index_probe.cpp
// reader is header-probe-only, "no B-tree materialization," per
// docs/13-index-format-notes.md) -- and is deliberately left to its own
// follow-up rather than attempted here.
//
// Like import_xbase_table_to_vfp_native()/preview_xbase_table_import()
// (dbf_import.h) themselves, this is a standalone library function, not
// yet wired to any PRG-level IMPORT DATABASE command surface: #5517's own
// wizard command syntax does not exist yet either (no dispatch code
// references the single-table import functions), so this slice ships the
// underlying capability for a later orchestration layer to call, matching
// this epic's own established incremental pattern.
struct XbaseImportedTable {
    std::string name;      // display name for the inferred-relation report
    std::string dbf_path;  // resolved on-disk path to the table's .dbf
};

// One best-effort candidate relation between two imported tables' indexed
// columns. This is a *suggestion* only, per #5534's own explicit framing
// ("necessarily best-effort ... should be presented as suggestions, not
// silently applied") -- xBase index files carry key expressions, not
// declared foreign-key metadata, so there is no way to confirm this is a
// real relationship, only that both tables happen to index a
// same-named column. Deliberately non-directional (neither side is
// asserted to be the "parent"/primary-key table): no signal available to
// this codebase (index_probe.cpp's IndexTagProbe/IndexProbe expose no
// per-tag uniqueness or primary-key marker) can reliably tell which side
// is which, so asserting a direction would be a guess this project's own
// discipline avoids making.
struct InferredTableRelation {
    std::string table_a;
    std::string table_b;
    // The shared column name (identical on both sides by construction --
    // this is exactly what makes the relation a *candidate* in the first
    // place; see this struct's own comment for why no direction is
    // asserted).
    std::string column;
};

// Scans each table's companion index files (CDX/IDX/NDX/MDX/NTX, all of
// which already have working header probes via parse_*_index_probe() in
// index_probe.cpp) for tags/keys that are a plain column reference (a
// composite/expression key, e.g. a concatenation or function call, is not
// translatable to a single-column relation and is silently excluded from
// consideration -- not reported as an error, since this is a best-effort
// suggestion pass, not a validation pass), then reports a candidate
// relation for every pair of *different* tables that both index a
// column sharing the same name (case-insensitively). A table whose .dbf
// cannot be read, or which has no companion index file at all, simply
// contributes no candidate relations -- not a hard failure for the whole
// scan, matching this codebase's established skip-and-continue precedent
// for other best-effort multi-item scans (e.g.
// AccessContainerSchemaResult::skipped).
[[nodiscard]] std::vector<InferredTableRelation> infer_xbase_index_relations(
    const std::vector<XbaseImportedTable>& tables);

}  // namespace copperfin::vfp
