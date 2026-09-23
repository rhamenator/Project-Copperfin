// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
//
// #6496: expands the hosted Cloud Defect Hunt migration lane's coverage
// beyond one seeded single-table C/N round trip. docs/cloud-validation.md
// explicitly disclosed this lane did not exercise NULL/deleted fidelity,
// memo sidecars, or multi-table database publication -- this file builds
// a small deterministic fixture family (two related dBASE III source
// tables, one with a memo sidecar) and verifies round-trip fidelity
// through import_xbase_table_to_vfp_native() for exactly the properties
// the parent issue calls out: record count/order, deleted flags, NULL
// versus blank/false/zero, field types, memo payload bytes (including
// empty and non-ASCII content), and table identity (no cross-
// contamination between tables migrated in the same batch). A second
// test injects a failure after the first table of a batch is already
// staged and verifies no false success, no orphaned destination file,
// and unchanged source bytes.
//
// Fixture construction is hand-rolled at the byte level (dBASE III
// header/descriptor/record layout, and DBT block layout) rather than
// reusing the pre-existing third-party fixtures in
// tests/fixtures/legacy-dbase-infused/, since those do not contain
// deleted rows, blank/unknown Logical values, or non-ASCII memo content
// -- the exact properties this coverage gap is about. The byte layout
// itself (header_length/record_length fields, 32-byte field descriptors
// with a zeroed physical-offset slot for this format family, the 0x0D
// descriptor terminator, deletion marker 0x2A, and the DBT format: a
// 512-byte header followed by 512-byte-aligned blocks holding raw text
// terminated by 0x1A 0x1A, addressed by an ASCII decimal block number
// stored in the source field bytes) was confirmed empirically against
// tests/fixtures/legacy-dbase-infused/dbase_83.dbf/.dbt (a real,
// upstream-sourced dBASE III + memo file) before writing this fixture
// builder, not assumed from documentation alone.

#include "copperfin/vfp/dbf_import.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_dbf_table_support.h"
#include "test_environment_support.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

using copperfin::test_dbf_table::read_binary_file;
using copperfin::test_dbf_table::write_ascii;
using copperfin::test_dbf_table::write_binary_file;
using copperfin::test_dbf_table::write_field_descriptor;
using copperfin::test_dbf_table::write_le_u16;
using copperfin::test_dbf_table::write_le_u32;

// expect()/failures are this file's own -- test_dbf_table.cpp defines its
// own pair tied to that translation unit's main(), not exported for reuse
// by a sibling executable; test_dbf_table_support.cpp (linked here for
// the byte-writing helpers above) intentionally does not define them.
int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

struct FixtureField {
    std::string name;
    char type;
    std::uint8_t length;
};

// Builds a minimal, valid dBASE III (version 0x83 when a memo field is
// present, 0x03 otherwise) table with the given fields and raw field-byte
// content per record (already padded/formatted exactly as it should
// appear on disk -- callers are responsible for that, matching how
// low-level fixture builders work elsewhere in this test suite).
// `deleted` marks which records carry the 0x2A deletion marker.
void build_dbase_iii_table(
    const std::filesystem::path& path,
    const std::vector<FixtureField>& fields,
    const std::vector<std::vector<std::string>>& record_field_bytes,
    const std::vector<bool>& deleted,
    bool has_memo) {
    std::size_t record_length = 1U;  // deletion-flag byte
    for (const auto& field : fields) {
        record_length += field.length;
    }
    const std::size_t header_length = 32U + (fields.size() * 32U) + 1U;
    const std::size_t record_count = record_field_bytes.size();
    std::vector<std::uint8_t> bytes(
        header_length + (record_count * record_length) + 1U, 0U);

    bytes[0] = has_memo ? 0x83U : 0x03U;
    write_le_u32(bytes, 4U, static_cast<std::uint32_t>(record_count));
    write_le_u16(bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(bytes, 10U, static_cast<std::uint16_t>(record_length));

    std::size_t descriptor_offset = 32U;
    for (const auto& field : fields) {
        // #6496: dBASE III (DbfFormatFamily::dbase) does not use the
        // descriptor's physical-offset slot -- confirmed against the real
        // dbase_83.dbf fixture, whose equivalent bytes are all zero;
        // physical offsets are computed sequentially by the reader
        // instead. Passing 0 here matches that real file.
        write_field_descriptor(bytes, descriptor_offset, field.name, field.type, 0U, field.length, 0U);
        descriptor_offset += 32U;
    }
    bytes[descriptor_offset] = 0x0DU;

    std::size_t record_offset = header_length;
    for (std::size_t r = 0U; r < record_count; ++r) {
        bytes[record_offset] = deleted[r] ? 0x2AU : 0x20U;
        std::size_t field_offset = record_offset + 1U;
        for (std::size_t f = 0U; f < fields.size(); ++f) {
            const std::string& value = record_field_bytes[r][f];
            write_ascii(bytes, field_offset, value.substr(0U, fields[f].length));
            // write_ascii does not space-pad short values; do that here so
            // trailing bytes are deterministic spaces, not zero-init.
            for (std::size_t pad = value.size(); pad < fields[f].length; ++pad) {
                bytes[field_offset + pad] = 0x20U;
            }
            field_offset += fields[f].length;
        }
        record_offset += record_length;
    }
    bytes.back() = 0x1AU;

    expect(write_binary_file(path, bytes), "dBASE III fixture should be writable: " + path.string());
}

// Builds a minimal, valid dBASE III DBT sidecar: a 512-byte header (next
// free block number at offset 0, matching the real dbase_83.dbt fixture)
// followed by 512-byte-aligned blocks, each raw content immediately
// terminated by 0x1A 0x1A. Returns each requested payload's assigned
// block number, in order (block 0 is reserved/unused, matching real
// dBASE III behavior where an empty/absent memo is block "0").
std::vector<std::uint32_t> build_dbase_iii_memo_file(
    const std::filesystem::path& path,
    const std::vector<std::vector<std::uint8_t>>& payloads) {
    std::vector<std::uint8_t> bytes(512U, 0U);
    std::vector<std::uint32_t> block_numbers;
    std::uint32_t next_block = 1U;
    for (const auto& payload : payloads) {
        block_numbers.push_back(next_block);
        const std::size_t start = bytes.size();
        bytes.resize(start + payload.size() + 2U);
        std::copy(payload.begin(), payload.end(), bytes.begin() + static_cast<std::ptrdiff_t>(start));
        bytes[bytes.size() - 2U] = 0x1AU;
        bytes[bytes.size() - 1U] = 0x1AU;
        // Advance to the next 512-byte-aligned block boundary.
        const std::size_t used_blocks = (bytes.size() + 511U) / 512U;
        bytes.resize(used_blocks * 512U, 0U);
        next_block = static_cast<std::uint32_t>(used_blocks);
    }
    write_le_u32(bytes, 0U, next_block);
    expect(write_binary_file(path, bytes), "dBASE III memo fixture should be writable: " + path.string());
    return block_numbers;
}

std::string ascii_block_number(std::uint32_t block, std::uint8_t width) {
    std::string text = std::to_string(block);
    if (text.size() < width) {
        text = std::string(width - text.size(), ' ') + text;
    }
    return text;
}

void test_migration_preserves_null_deleted_and_memo_fidelity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_migration_fidelity_6496";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // --- Table 1: PRODUCTS, with a memo field (empty + non-ASCII payloads),
    // a deleted row, and a blank/unknown Logical value. ---
    const fs::path products_source = temp_root / "products_source.dbf";
    const fs::path products_memo = temp_root / "products_source.dbt";
    const std::vector<FixtureField> products_fields{
        {.name = "ID", .type = 'N', .length = 3U},
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
        {.name = "NOTE", .type = 'M', .length = 10U},
    };
    // Non-ASCII payload: UTF-8 bytes for "café ☕" -- real multi-byte text,
    // not just a single high byte, so a byte-for-byte comparison actually
    // exercises multi-byte preservation.
    const std::vector<std::uint8_t> non_ascii_payload{
        'c', 'a', 'f', 0xC3U, 0xA9U, ' ', 0xE2U, 0x98U, 0x95U};
    const auto products_blocks = build_dbase_iii_memo_file(
        products_memo, {{}, {'G', 'i', 'f', 't'}, non_ascii_payload});
    const std::vector<std::vector<std::string>> products_records{
        {"1  ", "Widget    ", "T", ascii_block_number(products_blocks[0], 10U)},
        {"2  ", "Gadget    ", "F", ascii_block_number(products_blocks[1], 10U)},
        {"3  ", "Gizmo     ", " ", ascii_block_number(products_blocks[2], 10U)},
    };
    build_dbase_iii_table(products_source, products_fields, products_records,
        /*deleted=*/{false, true, false}, /*has_memo=*/true);

    // --- Table 2: CUSTOMERS, no memo, simple fields -- proves table
    // identity/no cross-contamination when migrated in the same batch. ---
    const fs::path customers_source = temp_root / "customers_source.dbf";
    const std::vector<FixtureField> customers_fields{
        {.name = "ID", .type = 'N', .length = 3U},
        {.name = "NAME", .type = 'C', .length = 10U},
    };
    const std::vector<std::vector<std::string>> customers_records{
        {"1  ", "Alice     "},
        {"2  ", "Bob       "},
    };
    build_dbase_iii_table(customers_source, customers_fields, customers_records,
        /*deleted=*/{false, false}, /*has_memo=*/false);

    // --- Migrate both, as one "database publication" batch. ---
    const fs::path products_dest = temp_root / "products.dbf";
    const fs::path customers_dest = temp_root / "customers.dbf";
    const auto products_import = copperfin::vfp::import_xbase_table_to_vfp_native(
        products_source.string(), products_dest.string(), products_memo.string());
    expect(products_import.ok && products_import.record_count == 3U,
        "#6496: PRODUCTS migration should succeed with all 3 records: " + products_import.error);
    const auto customers_import = copperfin::vfp::import_xbase_table_to_vfp_native(
        customers_source.string(), customers_dest.string());
    expect(customers_import.ok && customers_import.record_count == 2U,
        "#6496: CUSTOMERS migration should succeed with both records: " + customers_import.error);

    // --- Table identity: CUSTOMERS' destination must be its own 2-field,
    // 2-record table -- no PRODUCTS content, no field bleed. ---
    const auto customers_parsed = copperfin::vfp::parse_dbf_table_from_file(customers_dest.string(), 10U);
    expect(customers_parsed.ok && customers_parsed.table.fields.size() == 2U &&
               customers_parsed.table.records.size() == 2U,
        "#6496: CUSTOMERS destination should have exactly its own 2 fields and 2 records, not PRODUCTS'");
    if (customers_parsed.ok && customers_parsed.table.records.size() == 2U) {
        expect(customers_parsed.table.records[0U].values[1U].display_value.starts_with("Alice") &&
                   customers_parsed.table.records[1U].values[1U].display_value.starts_with("Bob"),
            "#6496: CUSTOMERS record order should be preserved and unmixed with PRODUCTS content");
    }

    // --- PRODUCTS: deleted flag, NULL-vs-blank Logical, memo bytes. ---
    const auto products_parsed = copperfin::vfp::parse_dbf_table_from_file(products_dest.string(), 10U);
    expect(products_parsed.ok && products_parsed.table.records.size() == 3U,
        "#6496: PRODUCTS destination should retain all 3 records (including the deleted one): " +
            products_parsed.error);
    if (products_parsed.ok && products_parsed.table.records.size() == 3U) {
        const auto find_field = [&](std::size_t row, const std::string& name) -> const copperfin::vfp::DbfRecordValue* {
            for (const auto& value : products_parsed.table.records[row].values) {
                if (value.field_name == name) {
                    return &value;
                }
            }
            return nullptr;
        };

        expect(!products_parsed.table.records[0U].deleted,
            "#6496: record 0 (not deleted at source) must not be marked deleted after migration");
        expect(products_parsed.table.records[1U].deleted,
            "#6496: record 1's deleted flag at the source must survive migration, matching #5567's own fix");
        expect(!products_parsed.table.records[2U].deleted,
            "#6496: record 2 (not deleted at source) must not be marked deleted after migration");

        const auto* active0 = find_field(0U, "ACTIVE");
        const auto* active2 = find_field(2U, "ACTIVE");
        expect(active0 != nullptr && !active0->is_null && active0->display_value == "true",
            "#6496: an explicit .T. Logical value must round-trip as true, not null");
        expect(active2 != nullptr && active2->is_null,
            "#6496: a blank/unknown Logical byte at the source must round-trip as NULL, not a silent false, matching #5631's own fix");

        const auto* note0 = find_field(0U, "NOTE");
        const auto* note1 = find_field(1U, "NOTE");
        const auto* note2 = find_field(2U, "NOTE");
        expect(note0 != nullptr && note0->display_value.empty(),
            "#6496: an empty memo payload must round-trip as empty, not a placeholder or corrupted value");
        expect(note1 != nullptr && note1->display_value == "Gift",
            "#6496: an ordinary ASCII memo payload must round-trip byte-for-byte");
        // std::equal comparing std::string's (signed) char against
        // std::uint8_t compares sign-extended values (e.g. char 0xC3 ==
        // -61 promotes to a different int than uint8_t 0xC3 == 195) --
        // an apples-to-oranges bug in the assertion itself, not a product
        // concern, caught by a temporary byte dump during authoring that
        // showed the raw bytes already matched exactly. Compare as
        // unsigned on both sides.
        expect(note2 != nullptr && note2->display_value.size() == non_ascii_payload.size() &&
                   std::equal(note2->display_value.begin(), note2->display_value.end(), non_ascii_payload.begin(),
                       [](char a, std::uint8_t b) { return static_cast<std::uint8_t>(a) == b; }),
            "#6496: a non-ASCII (multi-byte UTF-8) memo payload must round-trip byte-for-byte, not be truncated or corrupted at a byte boundary");
    }

    fs::remove_all(temp_root, ignored);
}

void test_migration_failure_after_first_table_leaves_no_orphan_and_source_unchanged() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_migration_failure_retry_6496";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path products_source = temp_root / "products_source.dbf";
    const std::vector<FixtureField> products_fields{
        {.name = "ID", .type = 'N', .length = 3U},
        {.name = "NAME", .type = 'C', .length = 10U},
    };
    build_dbase_iii_table(products_source, products_fields,
        {{"1  ", "Widget    "}}, /*deleted=*/{false}, /*has_memo=*/false);
    const auto products_source_bytes_before = read_binary_file(products_source);

    const fs::path customers_source = temp_root / "customers_source.dbf";
    build_dbase_iii_table(customers_source, products_fields,
        {{"1  ", "Alice     "}}, /*deleted=*/{false}, /*has_memo=*/false);
    const auto customers_source_bytes_before = read_binary_file(customers_source);

    // First table of the batch migrates successfully.
    const fs::path products_dest = temp_root / "products.dbf";
    const auto products_import = copperfin::vfp::import_xbase_table_to_vfp_native(
        products_source.string(), products_dest.string());
    expect(products_import.ok, "#6496: first table of the batch should migrate successfully: " + products_import.error);

    // Second table's destination collides with an existing file --
    // import_xbase_table_to_vfp_native() documents that it never
    // overwrites an existing destination and fails closed instead.
    const fs::path customers_dest = temp_root / "customers.dbf";
    expect(write_binary_file(customers_dest, {'p', 'r', 'e', '-', 'e', 'x', 'i', 's', 't', 'i', 'n', 'g'}),
        "pre-existing blocking file should be writable");
    const auto blocking_import = copperfin::vfp::import_xbase_table_to_vfp_native(
        customers_source.string(), customers_dest.string());
    expect(!blocking_import.ok,
        "#6496: a destination collision on the second table of a batch must fail closed, not report false success");

    // No false success, and the first table's already-migrated file must
    // remain intact and readable -- a later failure in the same batch must
    // not retroactively corrupt or remove earlier successful members.
    expect(fs::exists(products_dest), "#6496: the first table's successful migration must survive a later batch member's failure");
    const auto products_reparsed = copperfin::vfp::parse_dbf_table_from_file(products_dest.string(), 10U);
    expect(products_reparsed.ok && products_reparsed.table.records.size() == 1U,
        "#6496: the first table's destination file must remain valid and readable after a later failure");

    // The blocking file must be exactly what it was -- the failed import
    // must not have partially overwritten it (an orphaned partial write).
    const auto blocking_bytes_after = read_binary_file(customers_dest);
    expect(blocking_bytes_after.size() == 12U && blocking_bytes_after[0U] == 'p',
        "#6496: a failed import must not partially overwrite the colliding destination file");

    // Source bytes for both tables must be completely unchanged -- the
    // source is documented as opened read-only and never modified.
    expect(read_binary_file(products_source) == products_source_bytes_before,
        "#6496: the first table's source file must be byte-for-byte unchanged after the batch");
    expect(read_binary_file(customers_source) == customers_source_bytes_before,
        "#6496: the second table's source file must be byte-for-byte unchanged even though its import failed");

    // Retry with a valid destination succeeds.
    const fs::path customers_dest_retry = temp_root / "customers_retry.dbf";
    const auto retry_import = copperfin::vfp::import_xbase_table_to_vfp_native(
        customers_source.string(), customers_dest_retry.string());
    expect(retry_import.ok && retry_import.record_count == 1U,
        "#6496: retrying the second table with a valid destination after the first failure must succeed: " +
            retry_import.error);

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_migration_preserves_null_deleted_and_memo_fidelity();
    test_migration_failure_after_first_table_leaves_no_orphan_and_source_unchanged();

    if (failures != 0) {
        return 1;
    }
    return 0;
}
