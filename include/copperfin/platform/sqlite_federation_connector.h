// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/federation_execution.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::platform {

enum class SqliteFederationValueKind {
    null_value,
    integer,
    real,
    text,
    blob
};

struct SqliteFederationValue final {
    SqliteFederationValueKind kind = SqliteFederationValueKind::null_value;
    std::string value;
};

struct SqliteFederationRow final {
    std::vector<SqliteFederationValue> values;
};

struct SqliteFederationLimits final {
    std::size_t max_database_bytes = 64U * 1024U * 1024U;
    std::size_t max_rows = 1000U;
    std::size_t max_columns = 256U;
    std::size_t max_cell_bytes = 1024U * 1024U;
    std::size_t max_result_bytes = 8U * 1024U * 1024U;
    std::uint64_t max_virtual_machine_steps = 10'000'000U;
};

struct SqliteFederationResult final {
    bool ok = false;
    std::vector<std::string> columns;
    std::vector<SqliteFederationRow> rows;
    std::string error_code;
    std::string error_detail;
};

[[nodiscard]] bool sqlite_federation_connector_available() noexcept;

[[nodiscard]] SqliteFederationResult execute_sqlite_federation_plan_read_only(
    const FederationExecutionPlan& plan,
    const SqliteFederationLimits& limits = {});

[[nodiscard]] std::string serialize_sqlite_federation_result_json(
    const SqliteFederationResult& result);

[[nodiscard]] const char* sqlite_federation_value_kind_name(
    SqliteFederationValueKind kind) noexcept;

}  // namespace copperfin::platform
