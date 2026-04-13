#pragma once

#include <string>

namespace copperfin::platform {

enum class FederationBackend {
    sqlite,
    postgresql,
    sqlserver,
    oracle
};

struct QueryTranslationResult {
    bool ok = false;
    std::string translated_sql;
    std::string error;
};

[[nodiscard]] QueryTranslationResult translate_fox_sql_to_backend(
    FederationBackend backend,
    const std::string& fox_sql);

}  // namespace copperfin::platform
