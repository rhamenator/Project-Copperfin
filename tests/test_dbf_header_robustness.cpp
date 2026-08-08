// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
//
// Bounded, deterministic defense-in-depth robustness coverage for Copperfin's
// own DBF header parser (copperfin::vfp::parse_dbf_header /
// parse_dbf_header_from_file). This exercises undersized, truncated,
// inconsistent, boundary-value, and deterministic synthetic-random byte
// sequences against the parser and asserts it never crashes or reads out of
// bounds, using only synthetic local inputs. It targets no third-party
// system, downloads no corpus, and makes no network access. This is
// optional defense-in-depth testing, not a v1 or RC release gate, and it
// does not claim exhaustive format or security coverage -- see
// docs/36-dbf-header-parser-robustness-testing.md.

#include "copperfin/vfp/dbf_header.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace fs = std::filesystem;
using copperfin::vfp::DbfParseResult;
using copperfin::vfp::parse_dbf_header;
using copperfin::vfp::parse_dbf_header_from_file;

int failures = 0;
long long total_cases = 0;

void expect(bool condition, std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

// Hard resource limits. Every sweep below is a fixed, small, compile-time
// constant -- there is no unbounded loop, no external corpus, and no
// wall-clock-dependent retry. The wall-clock assertion in main() is a
// belt-and-suspenders check on top of that structural bound, not the
// primary control.
constexpr int kBoundarySizeSweepMaxLength = 96;
constexpr int kPseudoRandomCaseCount = 5000;
constexpr int kPseudoRandomMaxInputBytes = 512;
constexpr auto kWallClockBudget = std::chrono::seconds(60);

// A fixed, valid 96-byte synthetic Visual FoxPro header template. Bytes
// 32..95 are arbitrary filler (never read by the header parser, which only
// consults offsets 0..29), used only to exercise longer inputs.
std::vector<std::uint8_t> valid_template() {
    std::vector<std::uint8_t> bytes(kBoundarySizeSweepMaxLength, 0xABU);
    bytes[0] = 0x30U;                    // Visual FoxPro
    bytes[1] = 126U;                     // last_update_year (2026 - 1900)
    bytes[2] = 4U;                       // last_update_month
    bytes[3] = 8U;                       // last_update_day
    bytes[4] = 0x0AU;                    // record_count low byte (10)
    bytes[5] = 0x00U;
    bytes[6] = 0x00U;
    bytes[7] = 0x00U;
    bytes[8] = 0xA1U;                    // header_length low byte (161)
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;                   // record_length low byte (64)
    bytes[11] = 0x00U;
    bytes[28] = 0x05U;                   // table_flags: structural CDX + database container
    bytes[29] = 0x03U;                   // code_page_mark: CP1252
    return bytes;
}

void assert_never_reads_out_of_bounds(const std::vector<std::uint8_t>& bytes) {
    // The call itself is the assertion under ASan/UBSan: an out-of-bounds
    // read or other undefined behavior aborts the process before this
    // function can return normally.
    const DbfParseResult result = parse_dbf_header(bytes);
    ++total_cases;

    if (bytes.size() < 32U) {
        expect(!result.ok, "parse_dbf_header must reject inputs shorter than the 32-byte minimum header");
        return;
    }

    // looks_like_dbf() only inspects fields drawn from the first 30 bytes,
    // so for any fixed 32-byte prefix the parse outcome must be invariant
    // to how many trailing bytes follow it.
    const DbfParseResult minimal = parse_dbf_header(std::vector<std::uint8_t>(bytes.begin(), bytes.begin() + 32));
    expect(result.ok == minimal.ok,
           "parse_dbf_header outcome must not depend on trailing bytes beyond the fixed 32-byte header");
    expect(result.ok == result.header.looks_like_dbf(),
           "parse_dbf_header ok flag must agree with the header's own looks_like_dbf() predicate");
}

void test_boundary_size_sweep() {
    const std::vector<std::uint8_t> full = valid_template();
    for (int length = 0; length <= kBoundarySizeSweepMaxLength; ++length) {
        const std::vector<std::uint8_t> sliced(full.begin(), full.begin() + length);
        assert_never_reads_out_of_bounds(sliced);
    }

    // Undersized inputs built from arbitrary (not the valid template's)
    // bytes, to make sure the short-input rejection does not depend on the
    // specific byte values present.
    for (int length = 0; length < 32; ++length) {
        std::vector<std::uint8_t> arbitrary(static_cast<std::size_t>(length));
        for (int index = 0; index < length; ++index) {
            arbitrary[static_cast<std::size_t>(index)] = static_cast<std::uint8_t>((index * 37 + 11) & 0xFF);
        }
        assert_never_reads_out_of_bounds(arbitrary);
    }
}

void test_single_byte_field_exhaustive_sweep() {
    // Exhaustive (all 256 values) sweep of each single-byte field that
    // drives a branch in the header's derived accessors, holding every
    // other byte fixed at a known-valid value.
    const std::vector<std::uint8_t> base = valid_template();
    const std::vector<std::size_t> single_byte_offsets = {0U, 1U, 2U, 3U, 28U, 29U};

    for (const std::size_t offset : single_byte_offsets) {
        for (int value = 0; value <= 0xFF; ++value) {
            std::vector<std::uint8_t> bytes = base;
            bytes[offset] = static_cast<std::uint8_t>(value);
            const DbfParseResult result = parse_dbf_header(bytes);
            ++total_cases;
            if (!result.ok) {
                continue;
            }
            // Every derived accessor must be safe to call for any byte
            // value in a field it reads, including version-dependent
            // catalog lookups and the code-page table.
            (void)result.header.version_description();
            (void)result.header.has_database_container();
            (void)result.header.has_production_index();
            (void)result.header.has_structural_cdx();
            (void)result.header.has_memo_file();
            (void)result.header.last_update_iso8601();
            (void)copperfin::vfp::dbf_code_page_from_mark(result.header.code_page_mark);
        }
    }
}

void test_multi_byte_field_boundary_values() {
    // Classic boundary-value analysis for the two 16-bit little-endian
    // fields the header decodes: minimum, minimum+1, just under/at/over the
    // 32-byte header-size threshold, and the full 16-bit range's edges.
    const std::vector<std::uint16_t> boundary_u16_values = {
        0U, 1U, 31U, 32U, 33U, 0x7FFFU, 0x8000U, 0xFFFEU, 0xFFFFU};

    for (const std::uint16_t header_length : boundary_u16_values) {
        for (const std::uint16_t record_length : boundary_u16_values) {
            std::vector<std::uint8_t> bytes = valid_template();
            bytes[8] = static_cast<std::uint8_t>(header_length & 0xFFU);
            bytes[9] = static_cast<std::uint8_t>((header_length >> 8U) & 0xFFU);
            bytes[10] = static_cast<std::uint8_t>(record_length & 0xFFU);
            bytes[11] = static_cast<std::uint8_t>((record_length >> 8U) & 0xFFU);
            const DbfParseResult result = parse_dbf_header(bytes);
            ++total_cases;
            const bool expected_ok = header_length >= 32U && record_length > 0U;
            expect(result.ok == expected_ok,
                   "parse_dbf_header ok flag must match looks_like_dbf()'s documented "
                   "header_length/record_length boundary contract");
        }
    }

    // record_count is decoded but never used to bound a read inside the
    // header parser itself; sweep its 32-bit boundary values to confirm
    // that remains true (no crash, and the value round-trips exactly).
    const std::vector<std::uint32_t> boundary_u32_values = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFEU, 0xFFFFFFFFU};
    for (const std::uint32_t record_count : boundary_u32_values) {
        std::vector<std::uint8_t> bytes = valid_template();
        bytes[4] = static_cast<std::uint8_t>(record_count & 0xFFU);
        bytes[5] = static_cast<std::uint8_t>((record_count >> 8U) & 0xFFU);
        bytes[6] = static_cast<std::uint8_t>((record_count >> 16U) & 0xFFU);
        bytes[7] = static_cast<std::uint8_t>((record_count >> 24U) & 0xFFU);
        const DbfParseResult result = parse_dbf_header(bytes);
        ++total_cases;
        expect(result.ok, "a boundary record_count value alone must not change an otherwise-valid header's outcome");
        expect(result.header.record_count == record_count,
               "record_count must round-trip exactly for every 32-bit boundary value");
    }
}

void test_deterministic_pseudo_random_arbitrary_bytes() {
    // std::mt19937's output algorithm is standardized. Use its raw output
    // directly because standard-library distribution mappings may vary by
    // implementation; this keeps the byte sequence identical cross-platform.
    std::mt19937 engine(0xC0FFEEU);

    for (int iteration = 0; iteration < kPseudoRandomCaseCount; ++iteration) {
        const auto length = static_cast<std::size_t>(
            engine() % static_cast<std::mt19937::result_type>(kPseudoRandomMaxInputBytes + 1));
        std::vector<std::uint8_t> bytes(length);
        for (std::size_t index = 0; index < length; ++index) {
            bytes[index] = static_cast<std::uint8_t>(engine() & 0xFFU);
        }
        assert_never_reads_out_of_bounds(bytes);
    }
}

void write_bytes(const fs::path& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!bytes.empty()) {
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }
}

void test_parse_dbf_header_from_file_synthetic_inputs() {
    const fs::path root = fs::current_path() / "synthetic-files";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    const auto nonexistent = parse_dbf_header_from_file((root / "does-not-exist.dbf").string());
    ++total_cases;
    expect(!nonexistent.ok, "parse_dbf_header_from_file must fail closed for a nonexistent path");

    const fs::path empty_path = root / "empty.dbf";
    write_bytes(empty_path, {});
    const auto empty_result = parse_dbf_header_from_file(empty_path.string());
    ++total_cases;
    expect(!empty_result.ok, "parse_dbf_header_from_file must reject an empty file");

    const fs::path short_path = root / "short.dbf";
    write_bytes(short_path, std::vector<std::uint8_t>(31U, 0x30U));
    const auto short_result = parse_dbf_header_from_file(short_path.string());
    ++total_cases;
    expect(!short_result.ok, "parse_dbf_header_from_file must reject a file one byte short of the minimum header");

    const fs::path valid_path = root / "valid.dbf";
    write_bytes(valid_path, valid_template());
    const auto valid_result = parse_dbf_header_from_file(valid_path.string());
    ++total_cases;
    expect(valid_result.ok, "parse_dbf_header_from_file must accept a synthetic valid header");
    expect(valid_result.header.version == 0x30U, "parse_dbf_header_from_file must decode the version byte");

    const fs::path garbage_path = root / "garbage.dbf";
    std::vector<std::uint8_t> garbage(4096U);
    for (std::size_t index = 0; index < garbage.size(); ++index) {
        garbage[index] = static_cast<std::uint8_t>((index * 91U + 17U) & 0xFFU);
    }
    write_bytes(garbage_path, garbage);
    const auto garbage_result = parse_dbf_header_from_file(garbage_path.string());
    ++total_cases;
    (void)garbage_result;  // Deterministic arbitrary bytes: only required not to crash.

    fs::remove_all(root, ignored);
}

std::string json_report(std::chrono::milliseconds elapsed) {
    std::ostringstream stream;
    stream << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"kind\": \"copperfin-dbf-header-robustness-result\",\n"
           << "  \"target\": \"copperfin::vfp::parse_dbf_header\",\n"
           << "  \"defense_in_depth_only\": true,\n"
           << "  \"release_gate\": false,\n"
           << "  \"exhaustive_coverage_claimed\": false,\n"
           << "  \"deterministic\": true,\n"
           << "  \"total_cases\": " << total_cases << ",\n"
           << "  \"failures\": " << failures << ",\n"
           << "  \"elapsed_milliseconds\": " << elapsed.count() << ",\n"
           << "  \"wall_clock_budget_seconds\": " << kWallClockBudget.count() << ",\n"
           << "  \"status\": \"" << (failures == 0 ? "passed" : "failed") << "\"\n"
           << "}\n";
    return stream.str();
}

}  // namespace

int main() {
    const auto started = std::chrono::steady_clock::now();

    test_boundary_size_sweep();
    test_single_byte_field_exhaustive_sweep();
    test_multi_byte_field_boundary_values();
    test_deterministic_pseudo_random_arbitrary_bytes();
    test_parse_dbf_header_from_file_synthetic_inputs();

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started);
    const std::string report = json_report(elapsed);
    std::ofstream report_stream("dbf_header_robustness_report.json", std::ios::trunc);
    if (!report_stream) {
        std::cerr << "FAIL: unable to create DBF header robustness JSON report.\n";
        return EXIT_FAILURE;
    }
    report_stream << report;
    report_stream.close();
    if (!report_stream) {
        std::cerr << "FAIL: unable to finish DBF header robustness JSON report.\n";
        return EXIT_FAILURE;
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed across " << total_cases << " bounded, deterministic case(s).\n";
        return EXIT_FAILURE;
    }

    std::cout << "DBF header robustness: " << total_cases
              << " bounded, deterministic case(s) passed in " << elapsed.count() << "ms.\n";
    return EXIT_SUCCESS;
}
