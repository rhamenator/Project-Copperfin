#include "copperfin/platform/query_translator.h"

#include <algorithm>

namespace copperfin::platform {

namespace {

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

void replace_all(std::string& text, const std::string& from, const std::string& to) {
    std::size_t index = 0;
    while ((index = text.find(from, index)) != std::string::npos) {
        text.replace(index, from.size(), to);
        index += to.size();
    }
}

bool looks_like_select(const std::string& sql) {
    const std::string upper = uppercase_copy(sql);
    return upper.find("SELECT") != std::string::npos && upper.find("FROM") != std::string::npos;
}

}  // namespace

QueryTranslationResult translate_fox_sql_to_backend(
    FederationBackend backend,
    const std::string& fox_sql) {
    if (!looks_like_select(fox_sql)) {
        return {.ok = false, .error = "Only first-pass SELECT...FROM SQL translation is supported."};
    }

    std::string translated = fox_sql;

    replace_all(translated, ".T.", "TRUE");
    replace_all(translated, ".F.", "FALSE");
    replace_all(translated, " ALLTRIM(", " TRIM(");

    switch (backend) {
        case FederationBackend::sqlite:
            replace_all(translated, " IIF(", " CASE WHEN ");
            break;
        case FederationBackend::postgresql:
            replace_all(translated, " NVL(", " COALESCE(");
            break;
        case FederationBackend::sqlserver:
            replace_all(translated, " SUBSTR(", " SUBSTRING(");
            break;
        case FederationBackend::oracle:
            replace_all(translated, " IFNULL(", " NVL(");
            break;
    }

    return {.ok = true, .translated_sql = translated};
}

}  // namespace copperfin::platform
