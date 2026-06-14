#include "copperfin/platform/query_translator.h"

#include <algorithm>
#include <cctype>
#include <regex>

namespace copperfin::platform {

namespace {

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

bool is_word_char(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

std::size_t find_keyword(const std::string& upper_sql, const std::string& keyword) {
    std::size_t index = 0;
    while ((index = upper_sql.find(keyword, index)) != std::string::npos) {
        const std::size_t next = index + keyword.size();
        const bool before_ok = (index == 0) || !is_word_char(upper_sql[index - 1U]);
        const bool after_ok = (next >= upper_sql.size()) || !is_word_char(upper_sql[next]);
        if (before_ok && after_ok) {
            return index;
        }
        ++index;
    }
    return std::string::npos;
}

void replace_case_insensitive(std::string& text, const std::string& pattern, const std::string& replacement) {
    const std::regex token_regex(pattern, std::regex_constants::icase);
    text = std::regex_replace(text, token_regex, replacement);
}

void replace_function(std::string& text, const std::string& function_name, const std::string& replacement) {
    replace_case_insensitive(text, "\\b" + function_name + "\\s*\\(", replacement + "(");
}

void replace_all_literals(std::string& text) {
    replace_case_insensitive(text, "\\.T\\.", "TRUE");
    replace_case_insensitive(text, "\\.F\\.", "FALSE");
    replace_function(text, "ALLTRIM", "TRIM");
}

void replace_fox_sql_dialect(std::string& text, FederationBackend backend) {
    switch (backend) {
        case FederationBackend::sqlite:
            replace_function(text, "IIF", "CASE WHEN ");
            break;
        case FederationBackend::postgresql:
            replace_function(text, "NVL", "COALESCE");
            break;
        case FederationBackend::sqlserver:
            replace_function(text, "SUBSTR", "SUBSTRING");
            break;
        case FederationBackend::oracle:
            replace_function(text, "IFNULL", "NVL");
            break;
    }
}

bool looks_like_select(const std::string& sql) {
    const std::string upper = uppercase_copy(sql);
    const std::size_t select_pos = find_keyword(upper, "SELECT");
    if (select_pos == std::string::npos) {
        return false;
    }
    const std::size_t from_pos = find_keyword(upper, "FROM");
    return from_pos != std::string::npos && from_pos > select_pos;
}

}  // namespace

QueryTranslationResult translate_fox_sql_to_backend(
    FederationBackend backend,
    const std::string& fox_sql) {
    if (!looks_like_select(fox_sql)) {
        return {.ok = false, .translated_sql = {}, .error = "Only first-pass SELECT...FROM SQL translation is supported."};
    }

    std::string translated = fox_sql;
    replace_all_literals(translated);
    replace_fox_sql_dialect(translated, backend);

    return {.ok = true, .translated_sql = translated, .error = {}};
}

}  // namespace copperfin::platform
