// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/query_translator.h"

#include <optional>
#include <string>

namespace copperfin::platform {

struct FederationPlanningPolicy {
    bool enable_ai_assistance = false;
    bool require_ai_assistance = false;
    bool policy_audit_enabled = true;
};

struct FederationExecutionRequest {
    FederationBackend backend = FederationBackend::sqlite;
    std::string fox_sql;
    std::string target;
    FederationPlanningPolicy planning_policy{};
};

struct FederationExecutionPlan {
    bool ok = false;
    FederationBackend backend = FederationBackend::sqlite;
    std::string connector;
    std::string target;
    std::string translated_sql;
    std::vector<QueryProjectionField> projection_fields;
    std::string planning_mode;
    std::string execution_command;
    bool ai_assisted = false;
    bool deterministic_translation_succeeded = false;
    bool planning_policy_allows_ai = false;
    bool planning_policy_audit_enabled = true;
    std::string error;
};

[[nodiscard]] const char* federation_backend_name(FederationBackend backend);

[[nodiscard]] std::optional<FederationBackend> federation_backend_from_string(const std::string& value);

[[nodiscard]] FederationExecutionPlan build_federation_execution_plan(
    const FederationExecutionRequest& request);

}  // namespace copperfin::platform
