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

}  // namespace copperfin::vfp
