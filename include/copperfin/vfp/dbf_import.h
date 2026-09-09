// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::vfp {

struct DbfImportFieldMapping {
    std::string field_name;
    char source_type = '\0';
    char target_type = '\0';
};

struct DbfImportResult {
    bool ok = false;
    std::string error;
    std::size_t record_count = 0;
    std::vector<DbfImportFieldMapping> field_mappings;
};

// One field this source cannot losslessly import, per #5532's dry-run
// preview -- see docs/69-dbase-import-field-mapping.md for the full
// unsupported-type table these reasons come from.
struct DbfImportFieldIssue {
    std::string field_name;
    char source_type = '\0';
    std::string reason;
};

struct DbfImportPreviewResult {
    // True only if the source parsed, its format family and code page are
    // supported, and every field mapped successfully -- i.e. a real import
    // of this exact source would currently succeed (up to record-level
    // memo-payload issues, which the preview does not check; see the
    // function comment below).
    bool ok = false;
    // Set only for a table-level failure (source parse failure,
    // unsupported format family, unsupported code page) that stops the
    // preview before it can even enumerate fields. Empty otherwise, even
    // when field_issues is non-empty -- unmappable fields are reported
    // there, not here.
    std::string error;
    std::size_t record_count = 0;
    // Every field that *would* map successfully.
    std::vector<DbfImportFieldMapping> field_mappings;
    // Every field that would not -- unlike the committing path, which
    // fails closed on the first one it finds, this collects all of them
    // so a caller can see the complete picture before deciding whether to
    // proceed.
    std::vector<DbfImportFieldIssue> field_issues;
};

// #5523/#5525: first slices of #5517's IMPORT DATABASE ... TYPE XBASE
// wizard. Reads one legacy dBASE/FoxBASE/FoxPro-family table (source_path,
// DbfFormatFamily::dbase/foxbase/foxpro -- see #5482/#5483's readers) and
// writes a brand-new VFP-native table at destination_path with equivalent
// field/record content, reusing the existing family readers and VFP-native
// writer. FoxBASE and FoxPro use a strict subset of dBASE's type system (no
// dBASE Level 7-only I/+/O types are possible in those older formats), so
// this same function and mapping cover all three families.
//
// Source is opened read-only and never modified. destination_path is never
// overwritten: an existing entry there fails the import closed before any
// write is attempted (a non-atomic existence check, matching this table
// creation family's existing behavior elsewhere in the codebase).
//
// Only C/N/F/L/D/M/I/+/O source field types are supported in this slice;
// any other source type (dBASE B binary DBT payloads, @ timestamps, and
// anything else) fails the whole import closed before any destination
// file is created -- see docs/32-recovered-requirements-traceability.md's
// RQ-CF-MIGRATION-003 row for the written field-type mapping and the
// reasoning for excluding those types from this first slice.
DbfImportResult import_xbase_table_to_vfp_native(
    const std::string& source_path,
    const std::string& destination_path,
    const std::string& source_memo_sidecar_path = {});

// #5532: dry-run/report mode. Performs the same source-family, code-page,
// and per-field mapping checks as import_xbase_table_to_vfp_native()
// without ever writing anything -- no destination_path is needed, since
// nothing is created. Unlike the committing path, which fails closed on
// the first unmappable field it finds (a safety property: never write a
// partial or silently-lossy table), this collects every unmappable field
// so a caller can see the complete picture before deciding whether to
// import at all.
//
// Does not check for unresolved memo payloads (the reader's "<memo block
// N>" diagnostic placeholder) or a destination-side memo-sidecar
// conflict -- both are properties of a specific destination/write
// attempt, not of the source table's importability in the abstract, so
// they remain checks the committing path performs, not this preview.
DbfImportPreviewResult preview_xbase_table_import(
    const std::string& source_path,
    const std::string& source_memo_sidecar_path = {});

}  // namespace copperfin::vfp
