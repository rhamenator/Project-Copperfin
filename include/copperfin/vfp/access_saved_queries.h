// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

// #5479 (parent #138, related #5476, #5539, #5549): read-only extraction
// of Access saved-query definitions, reconstructed as SQL text.
//
// Unlike #5477 (forms/reports) and #5478 (VBA extraction) -- both of
// which this session investigated and found genuinely undocumented by
// every currently-allowed evidence source (see the investigation
// comments on those issues) -- saved queries turned out to need no new
// physical-format reverse-engineering at all: a query's definition is
// stored as ordinary rows in the `MSysQueries` system table (a regular
// Jet table, read via this codebase's existing TDEF/row-decoding
// machinery), not as some separate opaque binary object.
//
// Grounded in `mdbtools`' own GPL-licensed source code (not just its
// `HACKING.md` prose, which does not cover query storage at all) --
// specifically `src/util/mdb-queries.c`'s `MdbCatalogEntry`/`MSysQueries`
// column bindings and its per-`Attribute`-value SQL-clause
// reconstruction switch -- and `include/mdbtools.h.in`'s `MDB_QUERY`/
// `MDB_FORM`/`MDB_REPORT`/`MDB_MODULE` object-type enum, cross-checked
// against `mdb-queries`' own real-fixture output (installed locally) as
// independent-reader ground truth. `HACKING.md` itself already
// establishes this codebase's own precedent for treating `mdbtools`
// source files as legitimate evidence ("See props.c for an example").
//
// The MSysObjects.Type column (already decoded by
// scan_access_msysobjects_catalog(), #5539) holds a raw type byte whose
// low 7 bits (`type & 0x7F`) select an object category -- mdbtools'
// enum: MDB_FORM=0, MDB_TABLE=1, MDB_MACRO=2, MDB_SYSTEM_TABLE=3,
// MDB_REPORT=4, MDB_QUERY=5, MDB_LINKED_TABLE=6, MDB_MODULE=7,
// MDB_RELATIONSHIP=8. This codebase's own `Type == 1` (table)
// convention (docs/72) already matches this scheme's `MDB_TABLE`
// exactly, corroborating the mapping. `MDB_QUERY` (5) selects the rows
// this slice cares about.
//
// A query's own SQL is reconstructed from every `MSysQueries` row whose
// `ObjectId` matches that query's catalog `Id`, interpreting each row's
// `Attribute` byte (matching `mdb-queries.c`'s own switch exactly, not
// exceeding its scope for this first slice):
//   - 3: predicate (TOP n / TOP n PERCENT / DISTINCT / DISTINCTROW),
//        derived from the row's `Flag` bits and `Name1`.
//   - 5: a table name (`Name1`), appended to the FROM clause.
//   - 6: a selected column expression (`Expression`), appended to the
//        column list.
//   - 7: a join/relationship clause -- read but not incorporated into
//        the reconstructed SQL (matching mdb-queries.c's own documented
//        scope, which only comments the values without using them).
//   - 8: the WHERE clause expression (`Expression`).
//   - 11: an ORDER BY expression (`Expression`), with `Name1 == "D"`
//        appending DESCENDING.
//
// This is a best-effort SQL reconstruction (matching mdb-queries.c's own
// scope, not a claim of full Access SQL/QBE fidelity): non-SELECT query
// types (action queries: INSERT/UPDATE/DELETE/crosstab/union/pass-
// through/data-definition) are not positively identified, since no
// currently-allowed evidence source (neither mdb-queries.c nor any real
// fixture available during this slice's development) documents how
// MSysQueries's own row structure distinguishes them. A query with
// clause rows but no `Attribute == 5` (table) row at all -- which no
// real SELECT-shaped query this slice's development observed ever
// lacked -- is treated as unreconstructable and recorded in `skipped`
// rather than returned as a fabricated `SELECT ... FROM ` with an empty
// FROM clause. This is a partial mitigation, not full query-type
// detection: a non-SELECT query that references tables the same way a
// SELECT would (plausible for UPDATE/DELETE) is not caught by this check
// and may still produce SQL text that does not reflect its real
// semantics.
struct AccessSavedQuery {
    std::string name;
    std::string sql;
};

struct AccessSavedQueriesScanResult {
    bool ok = false;
    std::string error;
    std::vector<AccessSavedQuery> queries;
    struct SkippedQuery {
        std::string name;
        std::string reason;
    };
    std::vector<SkippedQuery> skipped;
};

// Scans the container's MSysObjects catalog for every Type&0x7F==MDB_QUERY
// (5) entry, then reconstructs each one's SQL from its own MSysQueries
// rows. A query whose reconstruction cannot proceed (e.g. no MSysQueries
// rows at all for its ObjectId, or an Expression column this codebase's
// long-value reader cannot resolve) is recorded in `skipped` with a
// reason rather than silently producing empty/wrong SQL.
[[nodiscard]] AccessSavedQueriesScanResult scan_access_saved_queries(const std::string& path);

}  // namespace copperfin::vfp
