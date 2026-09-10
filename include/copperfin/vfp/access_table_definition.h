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

// #5476 (parent #141, related #5474/#66, #68): read-only Access container
// schema inspection. Grounded in docs/68-access-mdb-jet-physical-page-
// layout-notes.md, whose primary source is the mdbtools project's own
// HACKING.md -- community reverse-engineering documentation, not an
// official Microsoft specification (docs/66-access-container-format-notes.md
// established none exists for the physical byte layout). Independently
// cross-checked against real Jet3 and Jet4 .mdb fixtures during this
// slice's development: the decoded column names/types for each fixture's
// own MSysObjects table (page 2) matched Access's well-known real system-
// catalog schema exactly, and every column's internal consistency (fixed-
// length columns landing at sane sequential byte offsets; variable-length
// columns showing uninitialized/garbage bytes in their unused fixed-offset
// field, matching mdbtools' own documented warning that Jet does not zero
// pages before writing) corroborated the field-offset layout used here.
// This is real-fixture verification, not merely trusting the community
// source -- but still not verification against an official specification
// or a licensed Access installation actually writing the tested bytes.

// Jet page size is fixed per container generation: 2048 bytes for Jet3,
// 4096 for Jet4. The ACE engine (.accdb, AccessContainerGeneration::later)
// is assumed to share Jet4's 4096-byte page size -- this is an
// extrapolation, not independently verified against a real .accdb fixture
// (none was available during this slice's development); a future slice
// with real ACCDB fixtures should confirm or correct this.
[[nodiscard]] std::size_t access_container_page_size(AccessContainerGeneration generation);

// The best-effort column-type classification documented in mdbtools'
// HACKING.md. Values 0x0D/0x0E are explicitly undocumented there ("not
// complete"); this codebase surfaces the raw byte (AccessColumnDefinition
// ::raw_type) alongside this enum so a caller is never blocked by an
// unrecognized value the way a hard failure would block it.
enum class AccessColumnType : std::uint8_t {
    unknown = 0x00,
    boolean = 0x01,
    byte_value = 0x02,
    integer = 0x03,
    long_integer = 0x04,
    currency = 0x05,
    single_float = 0x06,
    double_float = 0x07,
    date_time = 0x08,
    binary = 0x09,
    text = 0x0A,
    ole = 0x0B,
    memo = 0x0C,
    guid = 0x0F,
    numeric = 0x10
};

[[nodiscard]] const char* access_column_type_name(AccessColumnType type);

struct AccessColumnDefinition {
    std::string name;
    AccessColumnType type = AccessColumnType::unknown;
    std::uint8_t raw_type = 0;
    std::uint16_t column_number = 0;
    // Fixed byte width for a fixed-length column; 0 for a long-value type
    // (memo/OLE), matching mdbtools' own "0 if memo" note -- not a claim
    // that the value is unbounded in some other numeric sense.
    std::uint16_t length = 0;
    // From the low bit of the column-descriptor bitmask byte
    // (0x01 = "fixed length column"). Cross-validated during this slice's
    // development: every column this bit was set for had a plausible,
    // sequential fixed-byte-offset value in the field mdbtools' docs call
    // offset_F, and every column without it showed uninitialized garbage
    // there instead -- exactly the behavior expected if the bit is what
    // mdbtools' notes say it is.
    bool fixed_length = false;
    // The raw column-descriptor bitmask byte, for a caller that wants bits
    // this type doesn't surface a named accessor for -- e.g. bit 0x02,
    // which mdbtools' own notes mark as uncertain ("can be null, possibly
    // related to joins?"), so this codebase does not expose a confident
    // `nullable` field for it.
    std::uint8_t raw_bitmask = 0;
    // The raw offset_F field: for a fixed_length column, the column's
    // byte offset within a row's fixed-column region (used by #5539's
    // row decoder, access_msysobjects.cpp, to locate the value). For a
    // non-fixed column this field is documented -- and cross-validated
    // against real fixtures, see this struct's own fixed_length comment
    // -- to hold uninitialized garbage; callers must not read it unless
    // fixed_length is true.
    std::uint16_t offset_f = 0;
};

struct AccessTableDefinition {
    bool ok = false;
    std::string error;
    // The page this table's TDEF was read from. Always populated;
    // `name` below is the table's real name, populated only when
    // scan_access_container_schema() could resolve one (see that
    // field's own comment).
    std::uint32_t page_number = 0;
    // The table's real name, resolved by scan_access_container_schema()
    // from a decoded MSysObjects catalog row (#5539,
    // access_msysobjects.cpp's scan_access_msysobjects_catalog())
    // whose candidate TDEF page number (its Id, masked per mdbtools'
    // own mdb_read_catalog() precedent) matches this table's own
    // page_number -- i.e. a name is attached only when that mapping is
    // independently corroborated by an actually-discovered TDEF page,
    // not trusted from the catalog row alone (see #5541's own real-
    // fixture-based caution about treating MSysObjects.Id as
    // unconditionally reliable, and docs/72 for how that caution was
    // subsequently resolved via independent-reader cross-verification).
    // Left unset (nullopt) when parse_access_table_definition_page()
    // was called directly (not through scan_access_container_schema()),
    // when MSysObjects's own rows could not be decoded, or when no
    // catalog row's candidate page matched this one.
    std::optional<std::string> name;
    // From the table_type byte: true for 0x53 ('S', system table -- e.g.
    // MSysObjects itself), false for 0x4E ('N', user table).
    bool is_system_table = false;
    std::uint32_t row_count = 0;
    std::vector<AccessColumnDefinition> columns;
};

// Parses one Table Definition (TDEF) page already loaded into memory.
// page_bytes must be exactly access_container_page_size(generation) bytes;
// anything else, or a page whose leading byte is not 0x02 (TDEF), fails
// closed with a structured error rather than reading past the buffer.
//
// A TDEF that spans multiple pages (its next_pg field is nonzero -- used
// for tables with enough columns/indexes that one page is not enough) is
// explicitly out of scope for this slice: it fails closed with a distinct
// error rather than silently parsing only the first page's partial data,
// which real fixtures show correctly for a simple table (a handful of
// columns) but which this slice has not implemented multi-page reassembly
// for.
[[nodiscard]] AccessTableDefinition parse_access_table_definition_page(
    const std::vector<std::uint8_t>& page_bytes,
    AccessContainerGeneration generation,
    std::uint32_t page_number);

struct AccessContainerSchemaResult {
    bool ok = false;
    std::string error;
    // Every table whose TDEF page was discovered and successfully parsed.
    // A page that looks like a TDEF continuation (pointed to by another
    // TDEF's next_pg) is not treated as its own table. A discovered TDEF
    // page this slice cannot parse (a multi-page TDEF, or a page failing
    // its own bounds/consistency checks) is recorded in `skipped` instead
    // of silently dropped or aborting the whole scan.
    std::vector<AccessTableDefinition> tables;
    struct SkippedTable {
        std::uint32_t page_number = 0;
        std::string reason;
    };
    std::vector<SkippedTable> skipped;
};

// Scans every page in the container at path for TDEF pages (page_type
// 0x02) that are not a continuation page of another TDEF chain, and
// parses each one. See AccessTableDefinition::page_number's comment for
// why discovered tables are identified by page number, not name, in this
// slice.
[[nodiscard]] AccessContainerSchemaResult scan_access_container_schema(const std::string& path);

}  // namespace copperfin::vfp
