// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_import.h"
#include "copperfin/vfp/dbf_table.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    const std::uint32_t seed = argc > 1 ? static_cast<std::uint32_t>(std::stoul(argv[1])) : 1U;
    const std::size_t rows = argc > 2 ? std::stoul(argv[2]) : 300U;
    if (rows < 1U || rows > 50000U) {
        std::cerr << "rows must be between 1 and 50000\n";
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
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "ROWKEY", .type = 'C', .length = 20U},
        {.name = "AMOUNT", .type = 'N', .length = 10U}
    };
    std::mt19937 generator(seed);
    std::vector<std::vector<std::string>> expected;
    expected.reserve(rows);
    for (std::size_t index = 0; index < rows; ++index) {
        expected.push_back({"R" + std::to_string(index) + "X" +
                                std::to_string(generator() % 1000000U),
                            std::to_string(generator() % 100000000U)});
    }
    const auto written = copperfin::vfp::create_dbase_iii_table_file(
        source.string(), fields, expected);
    if (!written.ok || written.record_count != rows) {
        std::cerr << "source creation failed: " << written.error << " seed=" << seed << " rows=" << rows << '\n';
        return 1;
    }
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
    for (std::size_t index = 0; index < rows; ++index) {
        const auto& record = parsed.table.records[index];
        if (record.deleted || record.values.size() != 2U ||
            record.values[0].display_value != expected[index][0] ||
            record.values[1].display_value != expected[index][1]) {
            std::cerr << "round trip mismatch at row " << index << " seed=" << seed << " rows=" << rows << '\n';
            return 1;
        }
    }
    std::cout << "Migration round trip passed: seed=" << seed << " rows=" << rows
              << " source_bytes=" << std::filesystem::file_size(source)
              << " imported_bytes=" << std::filesystem::file_size(destination) << '\n';
}
