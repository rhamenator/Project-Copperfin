// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/sqlite_federation_connector.h"

#include "copperfin/platform/json.h"
#include "copperfin/platform/path.h"
#include "copperfin/platform/sqlite_api.h"
#include "copperfin/security/physical_path_containment.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

namespace copperfin::platform {

namespace {

struct DatabaseCloser final {
    void operator()(sqlite3* database) const noexcept {
        if (database != nullptr) {
            sqlite3_close_v2(database);
        }
    }
};

struct StatementFinalizer final {
    void operator()(sqlite3_stmt* statement) const noexcept {
        if (statement != nullptr) {
            sqlite3_finalize(statement);
        }
    }
};

using DatabaseHandle = std::unique_ptr<sqlite3, DatabaseCloser>;
using StatementHandle = std::unique_ptr<sqlite3_stmt, StatementFinalizer>;

struct ProgressState final {
    std::uint64_t callbacks = 0U;
    std::uint64_t maximum_callbacks = 0U;
};

SqliteFederationResult failure(std::string code, std::string detail = {}) {
    SqliteFederationResult result;
    result.error_code = std::move(code);
    result.error_detail = std::move(detail);
    return result;
}

bool add_bounded_size(std::size_t& total, std::size_t increment, std::size_t maximum) {
    if (increment > maximum || total > maximum - increment) {
        return false;
    }
    total += increment;
    return true;
}

bool valid_utf8(std::string_view value) {
    std::size_t index = 0U;
    while (index < value.size()) {
        const auto lead = static_cast<unsigned char>(value[index]);
        std::size_t continuation_count = 0U;
        std::uint32_t code_point = 0U;
        if (lead <= 0x7FU) {
            ++index;
            continue;
        }
        if (lead >= 0xC2U && lead <= 0xDFU) {
            continuation_count = 1U;
            code_point = lead & 0x1FU;
        } else if (lead >= 0xE0U && lead <= 0xEFU) {
            continuation_count = 2U;
            code_point = lead & 0x0FU;
        } else if (lead >= 0xF0U && lead <= 0xF4U) {
            continuation_count = 3U;
            code_point = lead & 0x07U;
        } else {
            return false;
        }
        if (index + continuation_count >= value.size()) {
            return false;
        }
        for (std::size_t offset = 1U; offset <= continuation_count; ++offset) {
            const auto continuation = static_cast<unsigned char>(value[index + offset]);
            if ((continuation & 0xC0U) != 0x80U) {
                return false;
            }
            code_point = (code_point << 6U) | (continuation & 0x3FU);
        }
        if ((continuation_count == 2U && code_point < 0x800U) ||
            (continuation_count == 3U && code_point < 0x10000U) ||
            (code_point >= 0xD800U && code_point <= 0xDFFFU) ||
            code_point > 0x10FFFFU) {
            return false;
        }
        index += continuation_count + 1U;
    }
    return true;
}

std::string lowercase_hex(const void* bytes, std::size_t size) {
    static constexpr char digits[] = "0123456789abcdef";
    const auto* data = static_cast<const unsigned char*>(bytes);
    std::string result(size * 2U, '0');
    for (std::size_t index = 0U; index < size; ++index) {
        result[index * 2U] = digits[(data[index] >> 4U) & 0x0FU];
        result[index * 2U + 1U] = digits[data[index] & 0x0FU];
    }
    return result;
}

std::string invariant_integer(sqlite3_int64 value) {
    char buffer[32]{};
    const auto converted = std::to_chars(std::begin(buffer), std::end(buffer), value);
    if (converted.ec != std::errc{}) {
        return {};
    }
    return std::string(buffer, converted.ptr);
}

std::string invariant_real(double value) {
    if (!std::isfinite(value)) {
        return {};
    }
    char buffer[64]{};
    const auto converted = std::to_chars(
        std::begin(buffer),
        std::end(buffer),
        value,
        std::chars_format::general,
        std::numeric_limits<double>::max_digits10);
    if (converted.ec != std::errc{}) {
        return {};
    }
    return std::string(buffer, converted.ptr);
}

bool only_trailing_space(std::string_view tail) {
    return std::all_of(tail.begin(), tail.end(), [](unsigned char ch) {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    });
}

bool ascii_case_insensitive_equal(std::string_view left, std::string_view right) {
    return left.size() == right.size() &&
        std::equal(left.begin(), left.end(), right.begin(), [](unsigned char lhs, unsigned char rhs) {
            return std::tolower(lhs) == std::tolower(rhs);
        });
}

int read_only_authorizer(
    void*,
    int action,
    const char* first,
    const char* second,
    const char*,
    const char*) {
    switch (action) {
        case SQLITE_SELECT:
        case SQLITE_READ:
            return SQLITE_OK;
        case SQLITE_FUNCTION: {
            (void)first;
            const std::string_view function_name = second == nullptr ? std::string_view{} : second;
            if (ascii_case_insensitive_equal(function_name, "load_extension") ||
                ascii_case_insensitive_equal(function_name, "readfile") ||
                ascii_case_insensitive_equal(function_name, "writefile") ||
                ascii_case_insensitive_equal(function_name, "fts3_tokenizer")) {
                return SQLITE_DENY;
            }
            return SQLITE_OK;
        }
#if defined(SQLITE_RECURSIVE)
        case SQLITE_RECURSIVE:
            return SQLITE_OK;
#endif
        default:
            return SQLITE_DENY;
    }
}

int progress_handler(void* context) {
    auto* state = static_cast<ProgressState*>(context);
    ++state->callbacks;
    return state->callbacks >= state->maximum_callbacks ? 1 : 0;
}

bool limits_are_valid(const SqliteFederationLimits& limits) {
    return limits.max_database_bytes > 0U &&
        limits.max_rows > 0U &&
        limits.max_columns > 0U &&
        limits.max_columns <= static_cast<std::size_t>(std::numeric_limits<int>::max()) &&
        limits.max_cell_bytes > 0U &&
        limits.max_cell_bytes <= static_cast<std::size_t>(std::numeric_limits<int>::max()) &&
        limits.max_result_bytes > 0U &&
        limits.max_virtual_machine_steps >= 1000U;
}

bool append_column_value(
    sqlite3_stmt* statement,
    int column,
    const SqliteFederationLimits& limits,
    std::size_t& result_bytes,
    SqliteFederationValue& output,
    std::string& error_code) {
    const int value_type = sqlite3_column_type(statement, column);
    switch (value_type) {
        case SQLITE_NULL:
            output.kind = SqliteFederationValueKind::null_value;
            return true;
        case SQLITE_INTEGER:
            output.kind = SqliteFederationValueKind::integer;
            output.value = invariant_integer(sqlite3_column_int64(statement, column));
            break;
        case SQLITE_FLOAT:
            output.kind = SqliteFederationValueKind::real;
            output.value = invariant_real(sqlite3_column_double(statement, column));
            if (output.value.empty()) {
                error_code = "federation.sqlite.non_finite_real";
                return false;
            }
            break;
        case SQLITE_TEXT: {
            output.kind = SqliteFederationValueKind::text;
            const int byte_count = sqlite3_column_bytes(statement, column);
            if (byte_count < 0 || static_cast<std::size_t>(byte_count) > limits.max_cell_bytes) {
                error_code = "federation.sqlite.cell_limit_exceeded";
                return false;
            }
            const auto* bytes = reinterpret_cast<const char*>(sqlite3_column_text(statement, column));
            if (byte_count > 0 && bytes == nullptr) {
                error_code = "federation.sqlite.value_read_failed";
                return false;
            }
            output.value.assign(bytes == nullptr ? "" : bytes, static_cast<std::size_t>(byte_count));
            if (!valid_utf8(output.value)) {
                error_code = "federation.sqlite.invalid_utf8";
                return false;
            }
            break;
        }
        case SQLITE_BLOB: {
            output.kind = SqliteFederationValueKind::blob;
            const int byte_count = sqlite3_column_bytes(statement, column);
            if (byte_count < 0 || static_cast<std::size_t>(byte_count) > limits.max_cell_bytes ||
                static_cast<std::size_t>(byte_count) > limits.max_result_bytes / 2U) {
                error_code = "federation.sqlite.cell_limit_exceeded";
                return false;
            }
            const void* bytes = sqlite3_column_blob(statement, column);
            if (byte_count > 0 && bytes == nullptr) {
                error_code = "federation.sqlite.value_read_failed";
                return false;
            }
            output.value = lowercase_hex(bytes, static_cast<std::size_t>(byte_count));
            break;
        }
        default:
            error_code = "federation.sqlite.value_type_rejected";
            return false;
    }

    if (output.value.size() > limits.max_cell_bytes * 2U ||
        !add_bounded_size(result_bytes, output.value.size(), limits.max_result_bytes)) {
        error_code = "federation.sqlite.result_limit_exceeded";
        return false;
    }
    return true;
}

}  // namespace

bool sqlite_federation_connector_available() noexcept {
    return true;
}

const char* sqlite_federation_value_kind_name(SqliteFederationValueKind kind) noexcept {
    switch (kind) {
        case SqliteFederationValueKind::null_value:
            return "null";
        case SqliteFederationValueKind::integer:
            return "integer";
        case SqliteFederationValueKind::real:
            return "real";
        case SqliteFederationValueKind::text:
            return "text";
        case SqliteFederationValueKind::blob:
            return "blob";
    }
    return "null";
}

SqliteFederationResult execute_sqlite_federation_plan_read_only(
    const FederationExecutionPlan& plan,
    const SqliteFederationLimits& limits) {
    if (!limits_are_valid(limits)) {
        return failure("federation.sqlite.invalid_limits");
    }
    if (!plan.ok || plan.backend != FederationBackend::sqlite || plan.connector != "sqlite" ||
        plan.planning_mode != "deterministic" || !plan.deterministic_translation_succeeded ||
        plan.ai_assisted || plan.translated_sql.empty()) {
        return failure("federation.sqlite.plan_rejected");
    }
    if (plan.target.empty() || plan.target == "sqlite-default" ||
        plan.target.find('\0') != std::string::npos || plan.target.starts_with("file:")) {
        return failure("federation.sqlite.target_required");
    }

    std::error_code path_error;
    const std::filesystem::path requested_path = std::filesystem::absolute(
        path_from_utf8_string(plan.target),
        path_error);
    if (path_error || requested_path.empty() || requested_path.parent_path().empty()) {
        return failure("federation.sqlite.target_rejected");
    }
    const auto inspected = copperfin::security::inspect_physical_path_containment(
        requested_path,
        requested_path.parent_path());
    if (!inspected.allowed || inspected.identity.file_size > limits.max_database_bytes) {
        return failure(
            inspected.identity.file_size > limits.max_database_bytes
                ? "federation.sqlite.database_limit_exceeded"
                : "federation.sqlite.target_rejected");
    }

    sqlite3* raw_database = nullptr;
    const std::string database_path = path_to_utf8_string(inspected.canonical_path);
    int open_flags = SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
#if defined(SQLITE_OPEN_NOFOLLOW)
    open_flags |= SQLITE_OPEN_NOFOLLOW;
#endif
    const int open_result = sqlite3_open_v2(
        database_path.c_str(),
        &raw_database,
        open_flags,
        nullptr);
    DatabaseHandle database(raw_database);
    if (open_result != SQLITE_OK || database == nullptr) {
        return failure(
            "federation.sqlite.open_failed",
            raw_database == nullptr ? std::string{} : sqlite3_errmsg(raw_database));
    }
    if (sqlite3_db_readonly(database.get(), "main") != 1) {
        return failure("federation.sqlite.read_only_required");
    }

    sqlite3_extended_result_codes(database.get(), 1);
    sqlite3_busy_timeout(database.get(), 0);
#if defined(SQLITE_DBCONFIG_DEFENSIVE)
    sqlite3_db_config(database.get(), SQLITE_DBCONFIG_DEFENSIVE, 1, nullptr);
#endif
#if defined(SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION)
    sqlite3_db_config(database.get(), SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 0, nullptr);
#endif
#if defined(SQLITE_DBCONFIG_TRUSTED_SCHEMA)
    sqlite3_db_config(database.get(), SQLITE_DBCONFIG_TRUSTED_SCHEMA, 0, nullptr);
#endif
    sqlite3_limit(database.get(), SQLITE_LIMIT_ATTACHED, 0);
    sqlite3_limit(database.get(), SQLITE_LIMIT_COLUMN, static_cast<int>(limits.max_columns));
    sqlite3_limit(database.get(), SQLITE_LIMIT_LENGTH, static_cast<int>(std::max<std::size_t>(
        limits.max_cell_bytes,
        4096U)));
    constexpr std::size_t maximum_sql_bytes = 1024U * 1024U;
    if (plan.translated_sql.size() > maximum_sql_bytes) {
        return failure("federation.sqlite.sql_limit_exceeded");
    }
    sqlite3_limit(database.get(), SQLITE_LIMIT_SQL_LENGTH, static_cast<int>(maximum_sql_bytes));
    if (sqlite3_set_authorizer(database.get(), read_only_authorizer, nullptr) != SQLITE_OK) {
        return failure("federation.sqlite.authorizer_failed");
    }

    ProgressState progress{
        .callbacks = 0U,
        .maximum_callbacks = limits.max_virtual_machine_steps / 1000U
    };
    sqlite3_progress_handler(database.get(), 1000, progress_handler, &progress);

    sqlite3_stmt* raw_statement = nullptr;
    const char* tail = nullptr;
    const int prepare_result = sqlite3_prepare_v2(
        database.get(),
        plan.translated_sql.c_str(),
        static_cast<int>(plan.translated_sql.size()),
        &raw_statement,
        &tail);
    StatementHandle statement(raw_statement);
    if (prepare_result != SQLITE_OK || statement == nullptr) {
        return failure("federation.sqlite.prepare_failed", sqlite3_errmsg(database.get()));
    }
    if (tail == nullptr || !only_trailing_space(tail) || sqlite3_stmt_readonly(statement.get()) != 1) {
        return failure("federation.sqlite.statement_rejected");
    }

    const int column_count = sqlite3_column_count(statement.get());
    if (column_count <= 0 || static_cast<std::size_t>(column_count) > limits.max_columns) {
        return failure("federation.sqlite.column_limit_exceeded");
    }

    SqliteFederationResult result;
    result.ok = true;
    result.columns.reserve(static_cast<std::size_t>(column_count));
    std::size_t result_bytes = 0U;
    for (int column = 0; column < column_count; ++column) {
        const char* name = sqlite3_column_name(statement.get(), column);
        const std::string column_name = name == nullptr ? std::string{} : std::string(name);
        if (column_name.size() > limits.max_cell_bytes || !valid_utf8(column_name) ||
            !add_bounded_size(result_bytes, column_name.size(), limits.max_result_bytes)) {
            return failure("federation.sqlite.column_name_rejected");
        }
        result.columns.push_back(column_name);
    }

    while (true) {
        const int step_result = sqlite3_step(statement.get());
        if (step_result == SQLITE_DONE) {
            break;
        }
        if (step_result != SQLITE_ROW) {
            return failure(
                step_result == SQLITE_INTERRUPT
                    ? "federation.sqlite.step_limit_exceeded"
                    : "federation.sqlite.execution_failed",
                sqlite3_errmsg(database.get()));
        }
        if (result.rows.size() >= limits.max_rows) {
            return failure("federation.sqlite.row_limit_exceeded");
        }

        SqliteFederationRow row;
        row.values.resize(static_cast<std::size_t>(column_count));
        for (int column = 0; column < column_count; ++column) {
            std::string value_error;
            if (!append_column_value(
                    statement.get(),
                    column,
                    limits,
                    result_bytes,
                    row.values[static_cast<std::size_t>(column)],
                    value_error)) {
                return failure(std::move(value_error));
            }
        }
        result.rows.push_back(std::move(row));
    }

    const auto reinspected = copperfin::security::inspect_physical_path_containment(
        inspected.canonical_path,
        inspected.canonical_path.parent_path());
    if (!reinspected.allowed || reinspected.identity != inspected.identity) {
        return failure("federation.sqlite.target_identity_changed");
    }
    return result;
}

std::string serialize_sqlite_federation_result_json(const SqliteFederationResult& result) {
    std::string json = "{\"schemaVersion\":1,\"ok\":";
    json += result.ok ? "true" : "false";
    json += ",\"columns\":[";
    for (std::size_t index = 0U; index < result.columns.size(); ++index) {
        if (index > 0U) {
            json.push_back(',');
        }
        json += "\"" + json_escape_string(result.columns[index]) + "\"";
    }
    json += "],\"rows\":[";
    for (std::size_t row_index = 0U; row_index < result.rows.size(); ++row_index) {
        if (row_index > 0U) {
            json.push_back(',');
        }
        json.push_back('[');
        const auto& row = result.rows[row_index];
        for (std::size_t column = 0U; column < row.values.size(); ++column) {
            if (column > 0U) {
                json.push_back(',');
            }
            const auto& value = row.values[column];
            json += "{\"kind\":\"";
            json += sqlite_federation_value_kind_name(value.kind);
            json += "\",\"value\":\"" + json_escape_string(value.value) + "\"}";
        }
        json.push_back(']');
    }
    json += "],\"errorCode\":\"" + json_escape_string(result.error_code) + "\"}";
    return json;
}

}  // namespace copperfin::platform
