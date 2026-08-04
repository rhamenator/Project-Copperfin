// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/platform/query_translator.h"

#include "localized_text.h"

#include <array>
#include <algorithm>
#include <sstream>
#include <string_view>
#include <vector>

namespace copperfin::platform {

namespace {

bool is_ascii_alpha(unsigned char ch) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

bool is_ascii_digit(unsigned char ch) {
    return ch >= '0' && ch <= '9';
}

bool is_ascii_alnum(unsigned char ch) {
    return is_ascii_alpha(ch) || is_ascii_digit(ch);
}

bool is_identifier_byte(unsigned char ch) {
    return is_ascii_alnum(ch) || ch == '_' || ch >= 0x80U;
}

bool is_ascii_space(unsigned char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v';
}

char uppercase_ascii_byte(unsigned char ch) {
    return ch >= 'a' && ch <= 'z'
        ? static_cast<char>(ch - 'a' + 'A')
        : static_cast<char>(ch);
}

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return uppercase_ascii_byte(ch);
    });
    return value;
}

bool is_word_char(char ch) {
    return is_identifier_byte(static_cast<unsigned char>(ch));
}

std::string trim_copy(std::string_view text) {
    const auto is_space = [](unsigned char ch) { return is_ascii_space(ch); };

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

std::size_t find_top_level_keyword(
    const std::string& upper_sql,
    std::size_t start_pos,
    const std::string& keyword) {
    bool in_single_quote = false;
    bool in_double_quote = false;
    int parentheses_depth = 0;

    for (std::size_t index = start_pos; index < upper_sql.size(); ++index) {
        const char ch = upper_sql[index];
        if (in_single_quote)
        {
            if (ch == '\'' && (index + 1U) < upper_sql.size() && upper_sql[index + 1U] == '\'')
            {
                ++index;
                continue;
            }
            if (ch == '\'')
            {
                in_single_quote = false;
            }
            continue;
        }
        if (in_double_quote)
        {
            if (ch == '"')
            {
                in_double_quote = false;
            }
            continue;
        }
        if (ch == '\'')
        {
            in_single_quote = true;
            continue;
        }
        if (ch == '"')
        {
            in_double_quote = true;
            continue;
        }

        if (ch == '(')
        {
            ++parentheses_depth;
            continue;
        }
        if (ch == ')')
        {
            if (parentheses_depth > 0)
            {
                --parentheses_depth;
            }
            continue;
        }
        if (parentheses_depth != 0)
        {
            continue;
        }
        if (index + keyword.size() <= upper_sql.size() &&
            upper_sql.compare(index, keyword.size(), keyword) == 0)
        {
            const std::size_t keyword_end = index + keyword.size();
            const bool before_ok = (index == 0U) || !is_word_char(upper_sql[index - 1U]);
            const bool after_ok = (keyword_end >= upper_sql.size()) || !is_word_char(upper_sql[keyword_end]);
            if (before_ok && after_ok)
            {
                return index;
            }
        }
    }
    return std::string::npos;
}

bool is_projection_identifier(std::string_view value) {
    if (value.empty())
    {
        return false;
    }
    const auto first = static_cast<unsigned char>(value.front());
    if (!(is_ascii_alpha(first) || first == '_' || first >= 0x80U))
    {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](char ch) {
        return is_identifier_byte(static_cast<unsigned char>(ch));
    });
}

std::vector<QueryProjectionField> extract_projection_fields_from_sql(std::string_view translated_sql) {
    std::vector<QueryProjectionField> projection_fields;

    const std::string upper_sql = uppercase_copy(std::string(translated_sql));
    const std::size_t select_pos = find_keyword(upper_sql, "SELECT");
    if (select_pos == std::string::npos)
    {
        return projection_fields;
    }

    const std::size_t projection_start = select_pos + 6U;
    const std::size_t from_pos = find_top_level_keyword(upper_sql, projection_start, "FROM");
    const std::size_t projection_end = (from_pos == std::string::npos) ? translated_sql.size() : from_pos;

    if (projection_end <= projection_start)
    {
        return projection_fields;
    }

    std::string_view projection_clause = translated_sql.substr(projection_start, projection_end - projection_start);
    bool in_single_quote = false;
    bool in_double_quote = false;
    int parentheses_depth = 0;
    std::size_t field_start = 0U;
    for (std::size_t index = 0U; index <= projection_clause.size(); ++index)
    {
        const bool at_end = (index >= projection_clause.size());
        const char ch = at_end ? ',' : projection_clause[index];
        if (in_single_quote)
        {
            if (!at_end && ch == '\'' && (index + 1U) < projection_clause.size() && projection_clause[index + 1U] == '\'')
            {
                ++index;
                continue;
            }
            if (!at_end && ch == '\'')
            {
                in_single_quote = false;
            }
            continue;
        }
        if (in_double_quote)
        {
            if (!at_end && ch == '"')
            {
                in_double_quote = false;
            }
            continue;
        }
        if (!at_end && ch == '\'')
        {
            in_single_quote = true;
            continue;
        }
        if (!at_end && ch == '"')
        {
            in_double_quote = true;
            continue;
        }
        if (ch == '(')
        {
            ++parentheses_depth;
            continue;
        }
        if (ch == ')')
        {
            if (parentheses_depth > 0)
            {
                --parentheses_depth;
            }
            continue;
        }

        if ((at_end || ch == ',') && parentheses_depth == 0) {
            const std::string_view raw_field = projection_clause.substr(field_start, index - field_start);
            std::string field_text = trim_copy(raw_field);
            field_start = index + 1U;
            if (field_text.empty())
            {
                continue;
            }

            if (field_text == "*")
            {
                projection_fields.push_back({.expression = "*", .alias = "", .wildcard = true});
                continue;
            }

            const std::string upper_field = uppercase_copy(field_text);
            const std::size_t as_pos = find_top_level_keyword(upper_field, 0U, "AS");
            QueryProjectionField field;
            if (as_pos != std::string::npos)
            {
                field.expression = trim_copy(field_text.substr(0U, as_pos));
                field.alias = trim_copy(field_text.substr(as_pos + 2U));
            }
            else
            {
                std::size_t alias_end = field_text.size();
                while (alias_end > 0U && is_ascii_space(static_cast<unsigned char>(field_text[alias_end - 1U])))
                {
                    --alias_end;
                }
                std::size_t alias_start = alias_end;
                while (alias_start > 0U &&
                       is_identifier_byte(static_cast<unsigned char>(field_text[alias_start - 1U])))
                {
                    --alias_start;
                }

                if (alias_start > 0U && alias_end > alias_start &&
                    is_ascii_space(static_cast<unsigned char>(field_text[alias_start - 1U])) &&
                    is_projection_identifier(field_text.substr(alias_start, alias_end - alias_start)))
                {
                    field.alias = trim_copy(field_text.substr(alias_start, alias_end - alias_start));
                    field.expression = trim_copy(field_text.substr(0U, alias_start));
                }
                else
                {
                    field.expression = field_text;
                }
            }

            projection_fields.push_back(std::move(field));
        }
    }

    return projection_fields;
}

bool equal_ignore_case(std::string_view left, std::string_view right) {
    return left.size() == right.size() &&
           std::equal(left.begin(), left.end(), right.begin(), right.end(), [](char lhs, char rhs) {
               return uppercase_ascii_byte(static_cast<unsigned char>(lhs)) ==
                   uppercase_ascii_byte(static_cast<unsigned char>(rhs));
           });
}

bool starts_with_ignore_case(std::string_view text, std::size_t offset, std::string_view needle) {
    return equal_ignore_case(text.substr(offset, needle.size()), needle);
}

bool is_identifier_boundary(char ch) {
    return !is_identifier_byte(static_cast<unsigned char>(ch)) && ch != '.';
}

bool match_identifier_at(std::string_view text, std::size_t offset, std::string_view identifier) {
    if (offset + identifier.size() > text.size())
    {
        return false;
    }
    if (!equal_ignore_case(text.substr(offset, identifier.size()), identifier))
    {
        return false;
    }
    if (offset != 0U && !is_identifier_boundary(text[offset - 1U]))
    {
        return false;
    }
    const std::size_t after = offset + identifier.size();
    if (after != text.size() && !is_identifier_boundary(text[after]))
    {
        return false;
    }
    return true;
}

void replace_keyword_in_sql(std::string& text, std::string_view keyword, std::string_view replacement, bool strict_word) {
    std::string output;
    output.reserve(text.size());

    bool in_single_quote = false;
    bool in_double_quote = false;

    for (std::size_t index = 0U; index < text.size();) {
        const char ch = text[index];
        if (in_single_quote)
        {
            output.push_back(ch);
            if (ch == '\'' && (index + 1U) < text.size() && text[index + 1U] == '\'')
            {
                output.push_back('\'');
                ++index;
            }
            else if (ch == '\'')
            {
                in_single_quote = false;
            }
            ++index;
            continue;
        }
        if (in_double_quote)
        {
            output.push_back(ch);
            if (ch == '"')
            {
                in_double_quote = false;
            }
            ++index;
            continue;
        }

        if (ch == '\'')
        {
            in_single_quote = true;
            output.push_back(ch);
            ++index;
            continue;
        }
        if (ch == '"')
        {
            in_double_quote = true;
            output.push_back(ch);
            ++index;
            continue;
        }

        if ((strict_word && match_identifier_at(text, index, keyword)) ||
            (!strict_word && starts_with_ignore_case(text, index, keyword)))
        {
            output.append(replacement);
            index += keyword.size();
            continue;
        }

        output.push_back(ch);
        ++index;
    }

    text = std::move(output);
}

void replace_keyword_function_in_sql(std::string& text, std::string_view function_name, std::string_view replacement) {
    std::string output;
    output.reserve(text.size());

    bool in_single_quote = false;
    bool in_double_quote = false;

    for (std::size_t index = 0U; index < text.size();) {
        const char ch = text[index];
        if (in_single_quote)
        {
            output.push_back(ch);
            if (ch == '\'' && (index + 1U) < text.size() && text[index + 1U] == '\'')
            {
                output.push_back('\'');
                ++index;
            }
            else if (ch == '\'')
            {
                in_single_quote = false;
            }
            ++index;
            continue;
        }
        if (in_double_quote)
        {
            output.push_back(ch);
            if (ch == '"')
            {
                in_double_quote = false;
            }
            ++index;
            continue;
        }

        if (ch == '\'')
        {
            in_single_quote = true;
            output.push_back(ch);
            ++index;
            continue;
        }
        if (ch == '"')
        {
            in_double_quote = true;
            output.push_back(ch);
            ++index;
            continue;
        }

        if (match_identifier_at(text, index, function_name))
        {
            std::size_t open = index + function_name.size();
            while (open < text.size() && is_ascii_space(static_cast<unsigned char>(text[open])))
            {
                ++open;
            }
            if (open < text.size() && text[open] == '(')
            {
                output.append(replacement);
                output.push_back('(');
                index = open + 1U;
                continue;
            }
        }

        output.push_back(ch);
        ++index;
    }

    text = std::move(output);
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
        while (open_paren < text.size() && is_ascii_space(static_cast<unsigned char>(text[open_paren])))
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

void replace_all_literals(std::string& text, FederationBackend backend) {
    if (backend == FederationBackend::sqlserver) {
        replace_keyword_in_sql(text, ".T.", "1", false);
        replace_keyword_in_sql(text, ".F.", "0", false);
        return;
    }

    replace_keyword_in_sql(text, ".T.", "TRUE", false);
    replace_keyword_in_sql(text, ".F.", "FALSE", false);
    replace_keyword_function_in_sql(text, "ALLTRIM", "TRIM");
}

void replace_fox_sql_dialect(std::string& text, FederationBackend backend) {
    replace_iif_calls(text);
    switch (backend) {
        case FederationBackend::sqlite:
            replace_keyword_function_in_sql(text, "LEN", "LENGTH");
            replace_keyword_function_in_sql(text, "IFNULL", "COALESCE");
            break;
        case FederationBackend::postgresql:
            replace_keyword_function_in_sql(text, "NVL", "COALESCE");
            replace_keyword_function_in_sql(text, "IFNULL", "COALESCE");
            replace_keyword_function_in_sql(text, "ISNULL", "COALESCE");
            replace_keyword_function_in_sql(text, "LEN", "LENGTH");
            break;
        case FederationBackend::sqlserver:
            replace_keyword_function_in_sql(text, "SUBSTR", "SUBSTRING");
            replace_keyword_function_in_sql(text, "NVL", "ISNULL");
            replace_keyword_function_in_sql(text, "IFNULL", "ISNULL");
            break;
        case FederationBackend::oracle:
            replace_keyword_function_in_sql(text, "IFNULL", "NVL");
            replace_keyword_function_in_sql(text, "LEN", "LENGTH");
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
        return {.ok = false,
                .translated_sql = {},
                .projection_fields = {},
                .error = platform_text("Platform.QueryTranslator.Error.SelectFromOnly")};
    }

    std::string translated = fox_sql;
    replace_all_literals(translated, backend);
    replace_fox_sql_dialect(translated, backend);

    QueryTranslationResult result;
    result.ok = true;
    result.translated_sql = std::move(translated);
    result.projection_fields = extract_projection_fields_from_sql(result.translated_sql);
    return result;
}

}  // namespace copperfin::platform
