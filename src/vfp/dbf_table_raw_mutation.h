// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#ifndef COPPERFIN_DBF_TABLE_RAW_MUTATION_H
#define COPPERFIN_DBF_TABLE_RAW_MUTATION_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace copperfin::vfp {

struct DbfRawRecordAppend {
    std::optional<std::size_t> source_record_index;
    std::vector<std::pair<std::string, std::string>> field_values;
};

struct DbfRawRecordMutationResult {
    bool ok = false;
    std::string error;
    std::vector<std::uint8_t> table_bytes;
    bool has_memo_sidecar = false;
    std::string memo_path;
    std::vector<std::uint8_t> memo_bytes;
    std::size_t record_count = 0;
};

DbfRawRecordMutationResult stage_dbf_raw_record_appends(
    const std::string& path,
    const std::vector<DbfRawRecordAppend>& appends);

DbfRawRecordMutationResult stage_dbf_raw_record_reorder(
    const std::string& path,
    const std::vector<std::size_t>& record_order);

}  // namespace copperfin::vfp

#endif
