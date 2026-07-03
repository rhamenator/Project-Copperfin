// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <vector>
#include <string>

namespace copperfin::platform {

enum class FederationBackend {
    sqlite,
    postgresql,
    sqlserver,
    oracle
};

struct QueryProjectionField {
    std::string expression;
    std::string alias;
    bool wildcard = false;
};

struct QueryTranslationResult {
    bool ok = false;
    std::string translated_sql;
    std::vector<QueryProjectionField> projection_fields;
    std::string error;
};

[[nodiscard]] QueryTranslationResult translate_fox_sql_to_backend(
    FederationBackend backend,
    const std::string& fox_sql);

}  // namespace copperfin::platform
