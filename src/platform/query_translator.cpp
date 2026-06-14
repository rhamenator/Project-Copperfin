#include "copperfin/platform/query_translator.h"

#include <array>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <regex>
#include <string_view>
#include <vector>

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

std::string trim_copy(std::string_view text) {
    const auto is_space = [](unsigned char ch) { return std::isspace(ch) != 0; };

    const auto start = std::find_if_not(text.begin(), text.end(), is_space);
    if (start == text.end())
    {
        return {};
    }
    const auto end = std::find_if_not(text.rbegin(), text.rend(), is_space).base();
    return std::string(start, end);
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

std::string to_case_when(std::string_view condition, std::string_view true_value, std::string_view false_value) {
    std::ostringstream stream;
    stream << "CASE WHEN " << condition << " THEN " << true_value << " ELSE " << false_value << " END";
    return stream.str();
}

bool parse_iif_call(const std::string& text, const std::size_t open_paren, std::size_t& close_paren, std::array<std::string, 3>& iif_args) {
    int parentheses_depth = 1;
    bool in_single = false;
    bool in_double = false;

    std::size_t current_arg_start = open_paren + 1U;
    std::vector<std::string> args;
    std::size_t i = current_arg_start;
    for (; i < text.size(); ++i)
    {
        const char ch = text[i];
        if (in_single)
        {
            if (ch == '\'' && (i + 1U) < text.size() && text[i + 1U] == '\'')
            {
                ++i;
                continue;
            }
            if (ch == '\'')
            {
                in_single = false;
            }
            continue;
        }
        if (in_double)
        {
            if (ch == '"')
            {
                in_double = false;
            }
            continue;
        }

        if (ch == '\'')
        {
            in_single = true;
            continue;
        }
        if (ch == '"')
        {
            in_double = true;
            continue;
        }

        if (ch == '(')
        {
            ++parentheses_depth;
            continue;
        }
        if (ch == ')')
        {
            if (parentheses_depth > 1)
            {
                --parentheses_depth;
                continue;
            }

            args.emplace_back(trim_copy(std::string_view{text.data() + current_arg_start, i - current_arg_start}));
            close_paren = i;
            break;
        }
        if (ch == ',' && parentheses_depth == 1)
        {
            args.emplace_back(trim_copy(std::string_view{text.data() + current_arg_start, i - current_arg_start}));
            current_arg_start = i + 1U;
        }
    }

    if (args.empty() || args.size() != 3U || close_paren == 0U)
    {
        return false;
    }

    iif_args = {args[0], args[1], args[2]};
    return true;
}

bool replace_iif_with_case(std::string& text) {
    const std::string upper = uppercase_copy(text);
    bool replaced = false;
    bool in_single = false;
    bool in_double = false;

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        const char ch = text[i];
        if (in_single)
        {
            if (ch == '\'' && (i + 1U) < text.size() && text[i + 1U] == '\'')
            {
                ++i;
                continue;
            }
            if (ch == '\'')
            {
                in_single = false;
            }
            continue;
        }
        if (in_double)
        {
            if (ch == '"')
            {
                in_double = false;
            }
            continue;
        }

        if (ch == '\'')
        {
            in_single = true;
            continue;
        }
        if (ch == '"')
        {
            in_double = true;
            continue;
        }

        if (i + 3U >= text.size() || upper.substr(i, 3U) != "IIF")
        {
            continue;
        }

        if ((i != 0U) && is_word_char(upper[i - 1U]))
        {
            continue;
        }
        if (i + 3U < text.size() && is_word_char(upper[i + 3U]))
        {
            continue;
        }

        std::size_t open_paren = i + 3U;
        while (open_paren < text.size() && std::isspace(static_cast<unsigned char>(text[open_paren])))
        {
            ++open_paren;
        }
        if (open_paren >= text.size() || text[open_paren] != '(')
        {
            continue;
        }

        std::array<std::string, 3> iif_args;
        std::size_t close_paren = 0U;
        if (!parse_iif_call(text, open_paren, close_paren, iif_args))
        {
            continue;
        }

        const auto replacement = to_case_when(iif_args[0], iif_args[1], iif_args[2]);
        text.replace(i, close_paren - i + 1U, replacement);
        replaced = true;
        return true;
    }

    return replaced;
}

void replace_iif_calls(std::string& text) {
    while (replace_iif_with_case(text)) {
    }
}

void replace_all_literals(std::string& text) {
    replace_case_insensitive(text, "\\.T\\.", "TRUE");
    replace_case_insensitive(text, "\\.F\\.", "FALSE");
    replace_function(text, "ALLTRIM", "TRIM");
}

void replace_fox_sql_dialect(std::string& text, FederationBackend backend) {
    replace_iif_calls(text);
    switch (backend) {
        case FederationBackend::sqlite:
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
