// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/json.h"
#include "copperfin/platform/sqlite_federation_connector.h"

#if COPPERFIN_SQLITE_CONNECTOR_AVAILABLE
#include "copperfin/platform/sqlite_api.h"
#endif

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

#if COPPERFIN_SQLITE_CONNECTOR_AVAILABLE

class TemporaryDatabase final {
public:
    TemporaryDatabase() {
        const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
            ("copperfin_sqlite_federation_" + std::to_string(nonce) + ".db");

        sqlite3* database = nullptr;
        const int open_result = sqlite3_open_v2(
            path_.string().c_str(),
            &database,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE,
            nullptr);
        if (open_result != SQLITE_OK || database == nullptr) {
            if (database != nullptr) {
                sqlite3_close_v2(database);
            }
            return;
        }
        static constexpr const char* schema =
            "CREATE TABLE customer("
            "id INTEGER PRIMARY KEY, name TEXT NOT NULL, active INTEGER NOT NULL, "
            "score REAL NOT NULL, payload BLOB NOT NULL, nullable TEXT);"
            "INSERT INTO customer VALUES(1, ' Ada ', 1, 12.5, X'00ff10', NULL);"
            "INSERT INTO customer VALUES(2, 'Grace', 0, 9.25, X'aa', 'present');";
        char* error = nullptr;
        valid_ = sqlite3_exec(database, schema, nullptr, nullptr, &error) == SQLITE_OK;
        sqlite3_free(error);
        sqlite3_close_v2(database);
    }

    ~TemporaryDatabase() {
        std::error_code ignored;
        std::filesystem::remove(path_, ignored);
    }

    TemporaryDatabase(const TemporaryDatabase&) = delete;
    TemporaryDatabase& operator=(const TemporaryDatabase&) = delete;

    [[nodiscard]] bool valid() const noexcept {
        return valid_;
    }

    [[nodiscard]] const std::filesystem::path& path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
    bool valid_ = false;
};

copperfin::platform::FederationExecutionPlan direct_plan(
    const std::filesystem::path& path,
    std::string sql) {
    copperfin::platform::FederationExecutionPlan plan;
    plan.ok = true;
    plan.backend = copperfin::platform::FederationBackend::sqlite;
    plan.connector = "sqlite";
    plan.target = path.string();
    plan.translated_sql = std::move(sql);
    plan.planning_mode = "deterministic";
    plan.deterministic_translation_succeeded = true;
    return plan;
}

void test_translated_query_executes_with_typed_bounded_results(const TemporaryDatabase& database) {
    const auto plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::sqlite,
        .fox_sql =
            "SELECT id, ALLTRIM(name) AS customer_name, score, payload, nullable "
            "FROM customer WHERE active = .T. ORDER BY id",
        .target = database.path().string()
    });
    expect(plan.ok, "SQLite execution fixture query should translate deterministically");

    const auto result = copperfin::platform::execute_sqlite_federation_plan_read_only(plan);
    expect(result.ok, "translated SQLite query should execute against the read-only connector");
    if (!result.ok) {
        std::cerr << "connector error: " << result.error_code << " " << result.error_detail << "\n";
        return;
    }

    expect(result.columns.size() == 5U, "result should preserve five projected column names");
    expect(result.rows.size() == 1U, "WHERE translation should retain exactly one active row");
    if (result.rows.size() == 1U && result.rows[0].values.size() == 5U) {
        const auto& values = result.rows[0].values;
        expect(values[0].kind == copperfin::platform::SqliteFederationValueKind::integer &&
                   values[0].value == "1",
               "INTEGER results should retain exact invariant text");
        expect(values[1].kind == copperfin::platform::SqliteFederationValueKind::text &&
                   values[1].value == "Ada",
               "translated ALLTRIM should produce the expected text value");
        expect(values[2].kind == copperfin::platform::SqliteFederationValueKind::real &&
                   values[2].value == "12.5",
               "REAL results should retain deterministic invariant text");
        expect(values[3].kind == copperfin::platform::SqliteFederationValueKind::blob &&
                   values[3].value == "00ff10",
               "BLOB results should use lowercase hexadecimal text");
        expect(values[4].kind == copperfin::platform::SqliteFederationValueKind::null_value &&
                   values[4].value.empty(),
               "NULL results should retain an explicit null kind and empty payload");
    }

    const std::string json = copperfin::platform::serialize_sqlite_federation_result_json(result);
    const auto parsed = copperfin::platform::select_json_value(json);
    expect(parsed.ok() && parsed.kind == copperfin::platform::JsonValueKind::object,
           "serialized connector results should be valid JSON objects");
    const auto first_kind = copperfin::platform::select_json_value(json, "/rows/0/0/kind");
    expect(first_kind.ok() && first_kind.decoded_string == "integer",
           "serialized result should preserve typed cell metadata");
}

void test_rejects_non_read_only_and_multi_statement_inputs(const TemporaryDatabase& database) {
    const auto update = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(database.path(), "UPDATE customer SET name = 'changed'"));
    expect(!update.ok && (update.error_code == "federation.sqlite.prepare_failed" ||
                          update.error_code == "federation.sqlite.statement_rejected"),
           "authorizer and statement checks should reject mutation SQL");

    const auto multiple = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(database.path(), "SELECT id FROM customer; SELECT name FROM customer"));
    expect(!multiple.ok && multiple.error_code == "federation.sqlite.statement_rejected",
           "connector should reject trailing SQL statements");

    const auto pragma = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(database.path(), "PRAGMA schema_version"));
    expect(!pragma.ok,
           "connector should reject PRAGMA statements even when SQLite classifies them as read-only");

    const auto extension = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(database.path(), "SELECT LOAD_EXTENSION('not-present')"));
    expect(!extension.ok && extension.error_code == "federation.sqlite.prepare_failed",
           "function authorization should case-insensitively reject extension loading before execution");
}

void test_enforces_result_and_execution_limits(const TemporaryDatabase& database) {
    auto row_limits = copperfin::platform::SqliteFederationLimits{};
    row_limits.max_rows = 1U;
    const auto rows = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(database.path(), "SELECT id FROM customer ORDER BY id"),
        row_limits);
    expect(!rows.ok && rows.error_code == "federation.sqlite.row_limit_exceeded",
           "connector should fail closed instead of returning a truncated row set");

    auto cell_limits = copperfin::platform::SqliteFederationLimits{};
    cell_limits.max_cell_bytes = 3U;
    const auto cell = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(database.path(), "SELECT name AS x FROM customer WHERE id = 2"),
        cell_limits);
    expect(!cell.ok && cell.error_code == "federation.sqlite.cell_limit_exceeded",
           "connector should reject cells beyond the explicit byte limit");

    auto step_limits = copperfin::platform::SqliteFederationLimits{};
    step_limits.max_virtual_machine_steps = 1000U;
    const auto steps = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(
            database.path(),
            "WITH RECURSIVE count(x) AS (VALUES(1) UNION ALL SELECT x+1 FROM count WHERE x<100000) "
            "SELECT max(x) FROM count"),
        step_limits);
    expect(!steps.ok && steps.error_code == "federation.sqlite.step_limit_exceeded",
           "connector should interrupt queries that exceed the virtual-machine step budget");
}

void test_rejects_invalid_plan_target_and_database_size(const TemporaryDatabase& database) {
    auto missing_target = direct_plan(database.path(), "SELECT 1");
    missing_target.target = "sqlite-default";
    const auto target = copperfin::platform::execute_sqlite_federation_plan_read_only(missing_target);
    expect(!target.ok && target.error_code == "federation.sqlite.target_required",
           "live execution should require an explicit SQLite database target");

    auto database_limits = copperfin::platform::SqliteFederationLimits{};
    database_limits.max_database_bytes = 1U;
    const auto size = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(database.path(), "SELECT 1"),
        database_limits);
    expect(!size.ok && size.error_code == "federation.sqlite.database_limit_exceeded",
           "connector should reject database files above the configured byte ceiling");

    auto invalid_limits = copperfin::platform::SqliteFederationLimits{};
    invalid_limits.max_rows = 0U;
    const auto invalid = copperfin::platform::execute_sqlite_federation_plan_read_only(
        direct_plan(database.path(), "SELECT 1"),
        invalid_limits);
    expect(!invalid.ok && invalid.error_code == "federation.sqlite.invalid_limits",
           "zero-valued resource ceilings should be rejected");
}

#endif

}  // namespace

int main() {
    expect(
        copperfin::platform::sqlite_federation_connector_available() ==
            (COPPERFIN_SQLITE_CONNECTOR_AVAILABLE != 0),
        "runtime connector availability should match the configured build contract");

#if COPPERFIN_SQLITE_CONNECTOR_AVAILABLE
    TemporaryDatabase database;
    expect(database.valid(), "SQLite test fixture should be created successfully");
    if (database.valid()) {
        test_translated_query_executes_with_typed_bounded_results(database);
        test_rejects_non_read_only_and_multi_statement_inputs(database);
        test_enforces_result_and_execution_limits(database);
        test_rejects_invalid_plan_target_and_database_size(database);
    }
#endif

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
