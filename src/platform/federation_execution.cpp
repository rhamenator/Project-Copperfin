// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/federation_execution.h"

#include "localized_text.h"

#include <algorithm>
#include <cctype>

namespace copperfin::platform {

namespace {

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string escape_for_command(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (const char ch : value) {
        if (ch == '"' || ch == '\\') {
            result.push_back('\\');
        }
        result.push_back(ch);
    }
    return result;
}

std::string default_target_for_backend(FederationBackend backend) {
    switch (backend) {
        case FederationBackend::sqlite:
            return "sqlite-default";
        case FederationBackend::postgresql:
            return "postgresql-default";
        case FederationBackend::sqlserver:
            return "sqlserver-default";
        case FederationBackend::oracle:
            return "oracle-default";
    }
    return "federation-default";
}

std::string connector_for_backend(FederationBackend backend) {
    switch (backend) {
        case FederationBackend::sqlite:
            return "sqlite";
        case FederationBackend::postgresql:
            return "postgresql";
        case FederationBackend::sqlserver:
            return "sqlserver";
        case FederationBackend::oracle:
            return "oracle";
    }
    return "unknown";
}

std::string format_federation_command(
    const std::string& connector,
    const std::string& target,
    const std::string& translated_sql) {
    return "connector.execute_query(connector=\"" + escape_for_command(connector) +
        "\", target=\"" + escape_for_command(target) +
        "\", sql=\"" + escape_for_command(translated_sql) + "\")";
}

std::string format_ai_fallback_command(
    const std::string& connector,
    const std::string& target,
    const std::string& fox_sql) {
    return "connector.plan_query(connector=\"" + escape_for_command(connector) +
        "\", target=\"" + escape_for_command(target) +
        "\", fox_sql=\"" + escape_for_command(fox_sql) + "\")";
}

void populate_fallback_plan_fields(FederationExecutionPlan& plan, const FederationExecutionRequest& request) {
    const bool ai_requested =
        request.planning_policy.enable_ai_assistance || request.planning_policy.require_ai_assistance;

    plan.planning_policy_allows_ai = ai_requested;
    plan.deterministic_translation_succeeded = false;
    plan.ai_assisted = ai_requested;
    plan.planning_mode = request.planning_policy.require_ai_assistance ? "ai_required_fallback" : "ai_optional_fallback";
}

}  // namespace

const char* federation_backend_name(FederationBackend backend) {
    switch (backend) {
        case FederationBackend::sqlite:
            return "sqlite";
        case FederationBackend::postgresql:
            return "postgresql";
        case FederationBackend::sqlserver:
            return "sqlserver";
        case FederationBackend::oracle:
            return "oracle";
    }
    return "unknown";
}

std::optional<FederationBackend> federation_backend_from_string(const std::string& value) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    if (normalized == "sqlite") {
        return FederationBackend::sqlite;
    }
    if (normalized == "postgresql" || normalized == "postgres") {
        return FederationBackend::postgresql;
    }
    if (normalized == "sqlserver" || normalized == "mssql") {
        return FederationBackend::sqlserver;
    }
    if (normalized == "oracle") {
        return FederationBackend::oracle;
    }
    return std::nullopt;
}

FederationExecutionPlan build_federation_execution_plan(const FederationExecutionRequest& request) {
    const auto translation = translate_fox_sql_to_backend(request.backend, request.fox_sql);
    FederationExecutionPlan plan{};
    plan.backend = request.backend;
    plan.connector = connector_for_backend(request.backend);
    plan.target = trim_copy(request.target).empty() ? default_target_for_backend(request.backend) : trim_copy(request.target);
    plan.planning_policy_allows_ai = request.planning_policy.enable_ai_assistance ||
                                     request.planning_policy.require_ai_assistance;
    plan.planning_policy_audit_enabled = request.planning_policy.policy_audit_enabled;

    if (!translation.ok) {
        populate_fallback_plan_fields(plan, request);
        if (plan.planning_policy_allows_ai) {
            const auto* plan_mode = request.planning_policy.require_ai_assistance ? "required" : "optional";
            plan.ok = false;
            plan.error = platform_text(
                "Platform.FederationExecution.Error.AiPlannerNotImplemented",
                {{"planMode", plan_mode}, {"translationError", translation.error}});
            plan.execution_command = format_ai_fallback_command(plan.connector, plan.target, request.fox_sql);
        } else {
            plan.ok = false;
            plan.error = translation.error;
            plan.planning_mode = "deterministic_rejected";
            plan.execution_command.clear();
            plan.deterministic_translation_succeeded = false;
            plan.ai_assisted = false;
        }
        return plan;
    }

    const std::string translated_sql = translation.translated_sql;

    plan.ok = true;
    plan.planning_mode = "deterministic";
    plan.ai_assisted = false;
    plan.deterministic_translation_succeeded = true;
    plan.translated_sql = translated_sql;
    plan.projection_fields = translation.projection_fields;
    plan.execution_command = format_federation_command(plan.connector, plan.target, translated_sql);
    return plan;
}

}  // namespace copperfin::platform
