#include "copperfin/platform/federation_execution.h"

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
    if (!translation.ok) {
        return {
            .ok = false,
            .backend = request.backend,
            .error = translation.error
        };
    }

    const std::string effective_target =
        trim_copy(request.target).empty() ? default_target_for_backend(request.backend) : trim_copy(request.target);
    const std::string connector = connector_for_backend(request.backend);
    const std::string translated_sql = translation.translated_sql;

    return {
        .ok = true,
        .backend = request.backend,
        .connector = connector,
        .target = effective_target,
        .translated_sql = translated_sql,
        .execution_command =
            "connector.execute_query(connector=\"" + escape_for_command(connector) +
            "\", target=\"" + escape_for_command(effective_target) +
            "\", sql=\"" + escape_for_command(translated_sql) + "\")"
    };
}

}  // namespace copperfin::platform