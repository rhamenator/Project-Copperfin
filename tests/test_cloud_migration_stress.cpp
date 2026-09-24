// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_import.h"
#include "copperfin/vfp/dbf_table.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

namespace {

// #6496: reads the header fields this file's own dBASE III writer
// produces (record count/header_length/record_length, all little-endian
// per the documented dBASE header layout) so every_nth_deleted() below
// can locate each record's deletion-marker byte directly, without a
// per-record set_record_deleted_flag() call -- that API's own
// documentation notes it re-reads/rewrites the *entire* file per call,
// which would make marking many rows deleted at 300000-row scale
// Theta(d*S) instead of the single O(S) whole-file pass used here.
std::uint16_t read_le_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U);
}

// Marks every nth source record deleted (0x2A) directly in the already-
// written dBASE III file, in one read-modify-write pass. Returns the
// number of records actually marked deleted, for the caller to verify
// against on round trip.
std::size_t mark_every_nth_record_deleted(const std::filesystem::path& path, std::size_t n) {
    std::ifstream input(path, std::ios::binary);
    std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    input.close();
    if (bytes.size() < 12U) {
        return 0U;
    }
    const std::uint16_t header_length = read_le_u16(bytes, 8U);
    const std::uint16_t record_length = read_le_u16(bytes, 10U);
    if (record_length == 0U) {
        return 0U;
    }
    std::size_t marked = 0U;
    for (std::size_t offset = header_length, index = 0U;
         offset + record_length <= bytes.size();
         offset += record_length, ++index) {
        if (n != 0U && index % n == 0U) {
            bytes[offset] = 0x2AU;
            ++marked;
        }
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return marked;
}

}  // namespace

int main(int argc, char** argv) {
    const std::uint32_t seed = argc > 1 ? static_cast<std::uint32_t>(std::stoul(argv[1])) : 1U;
    const std::size_t rows = argc > 2 ? std::stoul(argv[2]) : 300U;
    if (rows < 1U || rows > 300000U) {
        std::cerr << "rows must be between 1 and 300000\n";
        return 2;
    }
    const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() /
                      ("copperfin-cloud-migration-" + std::to_string(nonce));
    std::filesystem::create_directories(root);
    struct Cleanup {
        std::filesystem::path path;
        ~Cleanup() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    } cleanup{root};

    const auto source = root / "source.dbf";
    const auto destination = root / "imported.dbf";
    // #6496: ACTIVE broadens this scaled round trip beyond pure C/N
    // fidelity to also cover NULL-vs-false Logical fidelity (#5631) and,
    // via mark_every_nth_record_deleted() below, deleted-row fidelity
    // (#5567) at the same scale the existing C/N content already runs at.
    // "?" is normalize_logical_value()'s own blank/unknown sentinel.
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "ROWKEY", .type = 'C', .length = 200U},
        {.name = "AMOUNT", .type = 'N', .length = 10U},
        {.name = "ACTIVE", .type = 'L', .length = 1U}
    };
    std::mt19937 generator(seed);
    std::vector<std::vector<std::string>> expected;
    expected.reserve(rows);
    for (std::size_t index = 0; index < rows; ++index) {
        const std::uint32_t active_choice = generator() % 3U;
        expected.push_back({"R" + std::to_string(index) + "X" +
                                std::to_string(generator() % 1000000U),
                            std::to_string(generator() % 100000000U),
                            active_choice == 0U ? "T" : active_choice == 1U ? "F" : "?"});
    }
    const auto written = copperfin::vfp::create_dbase_iii_table_file(
        source.string(), fields, expected);
    if (!written.ok || written.record_count != rows) {
        std::cerr << "source creation failed: " << written.error << " seed=" << seed << " rows=" << rows << '\n';
        return 1;
    }
    // Every 97th row (an arbitrary, non-round period so it does not
    // coincide with any obvious stride in the generated content) is
    // marked deleted directly in the source file.
    constexpr std::size_t deletion_period = 97U;
    const std::size_t expected_deleted_count = mark_every_nth_record_deleted(source, deletion_period);

    const auto imported = copperfin::vfp::import_xbase_table_to_vfp_native(
        source.string(), destination.string());
    if (!imported.ok || imported.record_count != rows) {
        std::cerr << "import failed: " << imported.error << " seed=" << seed << " rows=" << rows << '\n';
        return 1;
    }
    const auto parsed = copperfin::vfp::parse_dbf_table_from_file(destination.string(), rows);
    if (!parsed.ok || parsed.table.records.size() != rows) {
        std::cerr << "round trip parse failed: " << parsed.error << " seed=" << seed << " rows=" << rows << '\n';
        return 1;
    }
    std::size_t actual_deleted_count = 0U;
    for (std::size_t index = 0; index < rows; ++index) {
        const auto& record = parsed.table.records[index];
        const bool should_be_deleted = (index % deletion_period == 0U);
        if (record.deleted != should_be_deleted) {
            std::cerr << "deleted-flag mismatch at row " << index << " expected=" << should_be_deleted
                       << " actual=" << record.deleted << " seed=" << seed << " rows=" << rows << '\n';
            return 1;
        }
        if (record.deleted) {
            ++actual_deleted_count;
        }
        if (record.values.size() != 3U ||
            record.values[0].display_value != expected[index][0] ||
            record.values[1].display_value != expected[index][1]) {
            std::cerr << "round trip mismatch at row " << index << " seed=" << seed << " rows=" << rows << '\n';
            return 1;
        }
        const std::string& expected_active = expected[index][2];
        const auto& active_value = record.values[2];
        if (expected_active == "?") {
            if (!active_value.is_null) {
                std::cerr << "NULL-fidelity mismatch at row " << index
                           << ": expected NULL/unknown ACTIVE, got is_null=false display_value='"
                           << active_value.display_value << "' seed=" << seed << " rows=" << rows << '\n';
                return 1;
            }
        } else if (active_value.is_null || active_value.display_value != (expected_active == "T" ? "true" : "false")) {
            std::cerr << "Logical-fidelity mismatch at row " << index << " seed=" << seed << " rows=" << rows << '\n';
            return 1;
        }
    }
    if (actual_deleted_count != expected_deleted_count) {
        std::cerr << "deleted-row count mismatch: expected=" << expected_deleted_count
                   << " actual=" << actual_deleted_count << " seed=" << seed << " rows=" << rows << '\n';
        return 1;
    }
    std::cout << "Migration round trip passed: seed=" << seed << " rows=" << rows
              << " deleted_rows=" << actual_deleted_count
              << " source_bytes=" << std::filesystem::file_size(source)
              << " imported_bytes=" << std::filesystem::file_size(destination) << '\n';
}
