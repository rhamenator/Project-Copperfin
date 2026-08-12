// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if defined(_WIN32) && __has_include(<winsqlite3.h>)
#include <winsqlite3.h>
#elif defined(_WIN32)

// Windows provides winsqlite3.dll and winsqlite3.lib even on SDK layouts that
// omit the optional winsqlite3.h header. These private connector declarations
// cover only the public-domain SQLite C ABI used by Copperfin.
extern "C" {

struct sqlite3;
struct sqlite3_stmt;
using sqlite3_int64 = __int64;

__declspec(dllimport) int sqlite3_open_v2(const char*, sqlite3**, int, const char*);
__declspec(dllimport) int sqlite3_close_v2(sqlite3*);
__declspec(dllimport) const char* sqlite3_errmsg(sqlite3*);
__declspec(dllimport) int sqlite3_db_readonly(sqlite3*, const char*);
__declspec(dllimport) int sqlite3_extended_result_codes(sqlite3*, int);
__declspec(dllimport) int sqlite3_busy_timeout(sqlite3*, int);
__declspec(dllimport) int sqlite3_db_config(sqlite3*, int, ...);
__declspec(dllimport) int sqlite3_limit(sqlite3*, int, int);
__declspec(dllimport) int sqlite3_set_authorizer(
    sqlite3*,
    int (*)(void*, int, const char*, const char*, const char*, const char*),
    void*);
__declspec(dllimport) void sqlite3_progress_handler(sqlite3*, int, int (*)(void*), void*);
__declspec(dllimport) int sqlite3_prepare_v2(
    sqlite3*, const char*, int, sqlite3_stmt**, const char**);
__declspec(dllimport) int sqlite3_stmt_readonly(sqlite3_stmt*);
__declspec(dllimport) int sqlite3_column_count(sqlite3_stmt*);
__declspec(dllimport) const char* sqlite3_column_name(sqlite3_stmt*, int);
__declspec(dllimport) int sqlite3_column_type(sqlite3_stmt*, int);
__declspec(dllimport) sqlite3_int64 sqlite3_column_int64(sqlite3_stmt*, int);
__declspec(dllimport) double sqlite3_column_double(sqlite3_stmt*, int);
__declspec(dllimport) int sqlite3_column_bytes(sqlite3_stmt*, int);
__declspec(dllimport) const unsigned char* sqlite3_column_text(sqlite3_stmt*, int);
__declspec(dllimport) const void* sqlite3_column_blob(sqlite3_stmt*, int);
__declspec(dllimport) int sqlite3_step(sqlite3_stmt*);
__declspec(dllimport) int sqlite3_finalize(sqlite3_stmt*);
__declspec(dllimport) int sqlite3_exec(
    sqlite3*, const char*, int (*)(void*, int, char**, char**), void*, char**);
__declspec(dllimport) void sqlite3_free(void*);

}  // extern "C"

#define SQLITE_OK 0
#define SQLITE_INTERRUPT 9
#define SQLITE_ROW 100
#define SQLITE_DONE 101
#define SQLITE_INTEGER 1
#define SQLITE_FLOAT 2
#define SQLITE_TEXT 3
#define SQLITE_BLOB 4
#define SQLITE_NULL 5
#define SQLITE_DENY 1
#define SQLITE_READ 20
#define SQLITE_SELECT 21
#define SQLITE_FUNCTION 31
#define SQLITE_RECURSIVE 33
#define SQLITE_OPEN_READONLY 0x00000001
#define SQLITE_OPEN_READWRITE 0x00000002
#define SQLITE_OPEN_CREATE 0x00000004
#define SQLITE_OPEN_EXCLUSIVE 0x00000010
#define SQLITE_OPEN_NOMUTEX 0x00008000
#define SQLITE_LIMIT_LENGTH 0
#define SQLITE_LIMIT_SQL_LENGTH 1
#define SQLITE_LIMIT_COLUMN 2
#define SQLITE_LIMIT_ATTACHED 7
#define SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION 1005
#define SQLITE_DBCONFIG_DEFENSIVE 1010
#define SQLITE_DBCONFIG_TRUSTED_SCHEMA 1017

#else
#include <sqlite3.h>
#endif
