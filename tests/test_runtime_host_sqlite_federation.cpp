// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/path.h"
#include "test_environment_support.h"
#include "test_process_capture_support.h"

#if COPPERFIN_SQLITE_CONNECTOR_AVAILABLE
#include "copperfin/platform/sqlite_api.h"
#endif

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::filesystem::path unique_temp_root() {
    static std::atomic_uint sequence{0U};
#if defined(_WIN32)
    const unsigned long process_id = static_cast<unsigned long>(::_getpid());
#else
    const unsigned long process_id = static_cast<unsigned long>(::getpid());
#endif
    return std::filesystem::temp_directory_path() /
        ("copperfin_runtime_sqlite_federation_" + std::to_string(process_id) + "_" +
         std::to_string(sequence.fetch_add(1U, std::memory_order_relaxed)));
}

#if COPPERFIN_SQLITE_CONNECTOR_AVAILABLE
bool create_database(const std::filesystem::path& path) {
    sqlite3* database = nullptr;
    const std::string path_text = copperfin::platform::path_to_utf8_string(path);
    const int open_result = sqlite3_open_v2(
        path_text.c_str(),
        &database,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE,
        nullptr);
    if (open_result != SQLITE_OK || database == nullptr) {
        if (database != nullptr) {
            sqlite3_close_v2(database);
        }
        return false;
    }
    static constexpr const char* sql =
        "CREATE TABLE customer(id INTEGER PRIMARY KEY, name TEXT NOT NULL, active INTEGER NOT NULL);"
        "INSERT INTO customer VALUES(1, 'Ada', 1);"
        "INSERT INTO customer VALUES(2, 'Grace', 0);";
    char* error = nullptr;
    const bool ok = sqlite3_exec(database, sql, nullptr, nullptr, &error) == SQLITE_OK;
    sqlite3_free(error);
    sqlite3_close_v2(database);
    return ok;
}
#endif

copperfin::test_support::CapturedProcessResult run_host(
    const std::filesystem::path& runtime_host,
    const std::filesystem::path& working_directory,
    const std::vector<std::string>& arguments) {
    return copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(runtime_host, arguments, working_directory));
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_runtime_host_sqlite_federation <copperfin_runtime_host>\n";
        return 2;
    }

    const std::filesystem::path runtime_host = argv[1];
    const std::filesystem::path temp_root = unique_temp_root();
    const std::filesystem::path database_path = temp_root / "customers.db";
    std::error_code error;
    std::filesystem::create_directories(temp_root, error);
    expect(!error, "runtime-host SQLite test should create its unique fixture root");

#if COPPERFIN_SQLITE_CONNECTOR_AVAILABLE
    expect(create_database(database_path), "runtime-host SQLite test should create its database fixture");
    const std::string database_text = copperfin::platform::path_to_utf8_string(database_path);
    const std::string query =
        "SELECT id, ALLTRIM(name) AS customer_name FROM customer WHERE active = .T.";

    const auto success = run_host(
        runtime_host,
        temp_root,
        {
            "--federation-backend", "sqlite",
            "--federation-query", query,
            "--federation-target", database_text,
            "--federation-execute-read-only", "true"
        });
    expect(success.started && success.exit_code == 0,
           "runtime host should execute a translated SQLite query successfully");
    expect(success.stdout_text.find("runtime.mode: federation-query-execute\n") != std::string::npos,
           "runtime host should identify the live execution mode");
    expect(success.stdout_text.find(
               "federation.result_json: {\"schemaVersion\":1,\"ok\":true") != std::string::npos,
           "runtime host should emit the versioned result contract");
    expect(success.stdout_text.find("customer_name") != std::string::npos &&
               success.stdout_text.find("Ada") != std::string::npos,
           "runtime host result should preserve translated projection data");
    expect(success.stderr_text.find("audit.event: federation.sqlite_read outcome=success") != std::string::npos,
           "successful live execution should emit a content-free audit event");
    expect(success.stderr_text.find(database_text) == std::string::npos &&
               success.stderr_text.find("SELECT") == std::string::npos &&
               success.stderr_text.find("Ada") == std::string::npos,
           "live execution audit output should omit target, query, and result data");

    const auto planning_only = run_host(
        runtime_host,
        temp_root,
        {
            "--federation-backend", "sqlite",
            "--federation-query", query,
            "--federation-target", database_text,
            "--federation-execute-read-only", "false"
        });
    expect(planning_only.exit_code == 0 &&
               planning_only.stdout_text.find("runtime.mode: federation-query-plan") != std::string::npos,
           "explicit false should preserve the existing plan-only behavior");
    expect(planning_only.stderr_text.find("federation.sqlite_read") == std::string::npos,
           "plan-only behavior should not emit a data-access audit event");

    const auto missing_target = run_host(
        runtime_host,
        temp_root,
        {
            "--federation-backend", "sqlite",
            "--federation-query", query,
            "--federation-execute-read-only", "true"
        });
    expect(missing_target.exit_code == 2 &&
               missing_target.stdout_text.find("runtime.mode: federation-query-execute") != std::string::npos,
           "live execution should fail before access when an explicit database target is absent");

    const auto wrong_backend = run_host(
        runtime_host,
        temp_root,
        {
            "--federation-backend", "postgresql",
            "--federation-query", "SELECT id FROM customer",
            "--federation-target", "provider-session",
            "--federation-execute-read-only", "true"
        });
    expect(wrong_backend.exit_code == 2,
           "the first live connector slice should reject non-SQLite backends explicitly");

    {
        copperfin::test_support::ScopedEnvironmentValue role("COPPERFIN_SECURITY_ROLE", "unknown-role");
        const auto denied = run_host(
            runtime_host,
            temp_root,
            {
                "--federation-backend", "sqlite",
                "--federation-query", query,
                "--federation-target", database_text,
                "--federation-execute-read-only", "true"
            });
        expect(denied.exit_code == 7 &&
                   denied.stdout_text.find("project.open") != std::string::npos,
               "unknown roles should fail closed before SQLite data access");
        expect(denied.stderr_text.find("federation.sqlite_read") == std::string::npos,
               "permission denial should occur before the connector is invoked");
    }

    const auto missing_database = run_host(
        runtime_host,
        temp_root,
        {
            "--federation-backend", "sqlite",
            "--federation-query", "SELECT id FROM customer",
            "--federation-target", copperfin::platform::path_to_utf8_string(temp_root / "missing.db"),
            "--federation-execute-read-only", "true"
        });
    expect(missing_database.exit_code == 8 &&
               missing_database.stdout_text.find("federation.sqlite.target_rejected") != std::string::npos,
           "missing database targets should produce the stable connector error code");
    expect(missing_database.stderr_text.find("outcome=rejected") != std::string::npos,
           "failed connector attempts should emit a content-free rejected audit event");
#else
    const auto unavailable = run_host(
        runtime_host,
        temp_root,
        {
            "--federation-backend", "sqlite",
            "--federation-query", "SELECT id FROM customer",
            "--federation-target", "missing.db",
            "--federation-execute-read-only", "true"
        });
    expect(unavailable.exit_code == 8 &&
               unavailable.stdout_text.find("federation.sqlite.connector_unavailable") != std::string::npos,
           "builds without SQLite should fail closed with an explicit capability error");
#endif

    std::filesystem::remove_all(temp_root, error);
    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
