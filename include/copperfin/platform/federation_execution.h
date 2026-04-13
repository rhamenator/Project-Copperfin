#pragma once

#include "copperfin/platform/query_translator.h"

#include <optional>
#include <string>

namespace copperfin::platform {

struct FederationExecutionRequest {
    FederationBackend backend = FederationBackend::sqlite;
    std::string fox_sql;
    std::string target;
};

struct FederationExecutionPlan {
    bool ok = false;
    FederationBackend backend = FederationBackend::sqlite;
    std::string connector;
    std::string target;
    std::string translated_sql;
    std::string execution_command;
    std::string error;
};

[[nodiscard]] const char* federation_backend_name(FederationBackend backend);

[[nodiscard]] std::optional<FederationBackend> federation_backend_from_string(const std::string& value);

[[nodiscard]] FederationExecutionPlan build_federation_execution_plan(
    const FederationExecutionRequest& request);

}  // namespace copperfin::platform