// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/sqlite_federation_connector.h"

#include "copperfin/platform/json.h"

namespace copperfin::platform {

bool sqlite_federation_connector_available() noexcept {
    return false;
}

SqliteFederationResult execute_sqlite_federation_plan_read_only(
    const FederationExecutionPlan&,
    const SqliteFederationLimits&) {
    SqliteFederationResult result;
    result.error_code = "federation.sqlite.connector_unavailable";
    result.error_detail = "SQLite support is unavailable in this build.";
    return result;
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

std::string serialize_sqlite_federation_result_json(const SqliteFederationResult& result) {
    return "{\"schemaVersion\":1,\"ok\":" + std::string(result.ok ? "true" : "false") +
        ",\"columns\":[],\"rows\":[],\"errorCode\":\"" + json_escape_string(result.error_code) + "\"}";
}

}  // namespace copperfin::platform
