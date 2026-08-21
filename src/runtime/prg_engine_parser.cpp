// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_internal.h"

#include "copperfin/platform/path.h"
#include "copperfin/vfp/sidecar_path.h"
#include "localized_text.h"
#include "prg_engine_command_helpers.h"
#include "prg_engine_helpers.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <locale>
#include <optional>
#include <sstream>
#include <set>
#include <stdexcept>

namespace copperfin::runtime {

namespace {

namespace fs = std::filesystem;

struct LogicalLine {
    std::size_t line_number = 0;
    std::string text{};
    bool is_text_block = false;
    std::string block_text{};
};

using PreprocessorDefineMap = std::map<std::string, std::string>;

struct PreprocessorState {
    PreprocessorDefineMap defines;
    std::set<std::string> include_stack;
    struct ConditionalFrame {
        bool parent_active = true;
        bool current_active = true;
        bool branch_taken = false;
        bool else_seen = false;
    };
    std::vector<ConditionalFrame> conditionals;
};

std::string strip_inline_comment(const std::string& line) {
    char quote_delimiter = '\0';
    std::size_t bracket_depth = 0U;
    std::size_t brace_depth = 0U;
    for (std::size_t index = 0; index < line.size(); ++index) {
        const char ch = line[index];
        if (quote_delimiter != '\0') {
            if (ch == quote_delimiter) {
                if ((index + 1U) < line.size() && line[index + 1U] == quote_delimiter) {
                    ++index;
                    continue;
                }
                quote_delimiter = '\0';
            }
            continue;
        }
        if (ch == '\'' || ch == '"') {
            quote_delimiter = ch;
            continue;
        }
        if (ch == '[') {
            ++bracket_depth;
            continue;
        }
        if (ch == ']' && bracket_depth > 0U) {
            --bracket_depth;
            continue;
        }
        if (ch == '{') {
            ++brace_depth;
            continue;
        }
        if (ch == '}' && brace_depth > 0U) {
            --brace_depth;
            continue;
        }
        if (bracket_depth == 0U && brace_depth == 0U &&
            ch == '&' && (index + 1U) < line.size() && line[index + 1U] == '&') {
            return line.substr(0U, index);
        }
    }
    return line;
}

std::size_t find_matching_parenthesis_in_text(const std::string& text, std::size_t open) {
    int depth = 0;
    char quote = '\0';
    for (std::size_t index = open; index < text.size(); ++index) {
        const char current = text[index];
        if (quote != '\0') {
            if (current == quote) {
                if (index + 1U < text.size() && text[index + 1U] == quote) {
                    ++index;
                } else {
                    quote = '\0';
                }
            }
            continue;
        }
        if (current == '\'' || current == '"') {
            quote = current;
        } else if (current == '(') {
            ++depth;
        } else if (current == ')' && --depth == 0) {
            return index;
        }
    }
    return std::string::npos;
}

bool looks_like_array_declaration_body(const std::string& body) {
    const std::string trimmed = trim_copy(body);
    for (const char ch : trimmed) {
        if (ch == '(' || ch == '[') {
            return true;
        }
        if (std::isspace(static_cast<unsigned char>(ch)) != 0 || ch == ',') {
            return false;
        }
    }
    return false;
}

std::string local_declaration_name(const std::string& declaration) {
    const std::size_t as_position = find_keyword_top_level(declaration, "AS");
    return trim_copy(as_position == std::string::npos
                         ? declaration
                         : declaration.substr(0U, as_position));
}

std::pair<std::string, std::string> split_declare_library_expression(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.empty()) {
        return {};
    }

    std::size_t end = 0U;
    if (trimmed.front() == '\'' || trimmed.front() == '"') {
        const char delimiter = trimmed.front();
        end = 1U;
        while (end < trimmed.size()) {
            if (trimmed[end] == delimiter) {
                if ((end + 1U) < trimmed.size() && trimmed[end + 1U] == delimiter) {
                    end += 2U;
                    continue;
                }
                ++end;
                break;
            }
            ++end;
        }
    } else if (trimmed.front() == '[') {
        const std::size_t close = trimmed.find(']', 1U);
        end = close == std::string::npos ? trimmed.size() : close + 1U;
    } else if (trimmed.front() == '(') {
        char quote_delimiter = '\0';
        std::size_t depth = 0U;
        for (; end < trimmed.size(); ++end) {
            const char ch = trimmed[end];
            if (quote_delimiter != '\0') {
                if (ch == quote_delimiter) {
                    if ((end + 1U) < trimmed.size() && trimmed[end + 1U] == quote_delimiter) {
                        ++end;
                    } else {
                        quote_delimiter = '\0';
                    }
                }
                continue;
            }
            if (ch == '\'' || ch == '"') {
                quote_delimiter = ch;
            } else if (ch == '(') {
                ++depth;
            } else if (ch == ')' && depth > 0U) {
                --depth;
                if (depth == 0U) {
                    ++end;
                    break;
                }
            }
        }
    } else {
        while (end < trimmed.size() &&
               std::isspace(static_cast<unsigned char>(trimmed[end])) == 0) {
            ++end;
        }
    }

    return {
        trim_copy(trimmed.substr(0U, end)),
        trim_copy(trimmed.substr(end))
    };
}

std::string extract_scatter_name_target_clause(const std::string& body) {
    std::size_t search_index = 0U;
    while (search_index < body.size()) {
        const std::size_t position = find_keyword_top_level_from(body, "NAME", search_index);
        if (position == std::string::npos) {
            break;
        }

        const std::size_t value_start = position + 4U;
        const std::size_t value_end = find_first_keyword_top_level(
            body,
            {"FIELDS", "TO", "MEMVAR", "BLANK", "MEMO", "ADDITIVE"},
            value_start);
        const std::string candidate = trim_copy(
            value_end == std::string::npos
                ? body.substr(value_start)
                : body.substr(value_start, value_end - value_start));
        const std::string first_word = normalize_identifier(split_first_word(candidate).first);
        if (!candidate.empty() &&
            first_word != "name" &&
            first_word != "memvar" &&
            first_word != "to" &&
            first_word != "fields" &&
            first_word != "blank" &&
            first_word != "memo" &&
            first_word != "additive") {
            return candidate;
        }

        search_index = position + 1U;
    }

    return {};
}

std::vector<LogicalLine> load_logical_lines(std::istream& input) {
    std::vector<LogicalLine> lines;

    std::string raw_line;
    std::size_t line_number = 0;
    std::size_t current_start = 0;
    std::string current_text;
    bool continuing = false;

    while (std::getline(input, raw_line)) {
        ++line_number;
        if (!raw_line.empty() && raw_line.back() == '\r') {
            raw_line.pop_back();
        }

        const std::string text_probe = trim_copy(strip_inline_comment(raw_line));
        if (!continuing && (uppercase_copy(text_probe) == "TEXT" || starts_with_insensitive(text_probe, "TEXT "))) {
            const std::size_t block_start = line_number;
            std::string block_text;
            while (std::getline(input, raw_line)) {
                ++line_number;
                if (!raw_line.empty() && raw_line.back() == '\r') {
                    raw_line.pop_back();
                }

                if (uppercase_copy(trim_copy(strip_inline_comment(raw_line))) == "ENDTEXT") {
                    break;
                }

                block_text += raw_line;
                block_text += "\n";
            }

            lines.push_back({
                .line_number = block_start,
                .text = text_probe,
                .is_text_block = true,
                .block_text = std::move(block_text)
            });
            continuing = false;
            current_text.clear();
            continue;
        }

        std::string line = strip_inline_comment(raw_line);
        if (!continuing) {
            current_start = line_number;
            current_text.clear();
        }

        const std::string trimmed = trim_copy(line);
        if (!current_text.empty() && !trimmed.empty()) {
            current_text += " ";
        }
        current_text += trimmed;

        if (!trimmed.empty() && trimmed.back() == ';') {
            current_text.pop_back();
            continuing = true;
            continue;
        }

        continuing = false;
        lines.push_back({
            .line_number = current_start,
            .text = trim_copy(current_text)
        });
    }

    if (continuing && !current_text.empty()) {
        lines.push_back({
            .line_number = current_start,
            .text = trim_copy(current_text)
        });
    }

    return lines;
}

std::vector<LogicalLine> load_logical_lines(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return input ? load_logical_lines(input) : std::vector<LogicalLine>{};
}

std::vector<LogicalLine> load_logical_lines_from_text(const std::string& source_text) {
    std::istringstream input(source_text);
    return load_logical_lines(input);
}

std::vector<std::string> load_source_lines(std::istream& input) {
    std::vector<std::string> lines;

    std::string raw_line;
    while (std::getline(input, raw_line)) {
        if (!raw_line.empty() && raw_line.back() == '\r') {
            raw_line.pop_back();
        }
        lines.push_back(std::move(raw_line));
    }

    return lines;
}

std::vector<std::string> load_source_lines(const std::string& path) {
    std::ifstream input(copperfin::platform::path_from_utf8_string(path), std::ios::binary);
    return input ? load_source_lines(input) : std::vector<std::string>{};
}

std::vector<std::string> load_source_lines_from_text(const std::string& source_text) {
    std::istringstream input(source_text);
    return load_source_lines(input);
}

bool is_preprocessor_identifier_start(const char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

bool is_preprocessor_identifier_char(const char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

std::string substitute_preprocessor_constants(
    const std::string& text,
    const PreprocessorDefineMap& defines,
    std::set<std::string>& expansion_stack);

bool has_wrapping_parentheses(const std::string& text);

std::string expand_preprocessor_identifier(
    const std::string& identifier,
    const PreprocessorDefineMap& defines,
    std::set<std::string>& expansion_stack) {
    const auto define = defines.find(normalize_identifier(identifier));
    if (define == defines.end()) {
        return identifier;
    }
    if (!expansion_stack.insert(define->first).second) {
        return define->second;
    }

    const std::string expanded = substitute_preprocessor_constants(define->second, defines, expansion_stack);
    expansion_stack.erase(define->first);
    return expanded;
}

std::string substitute_preprocessor_constants(
    const std::string& text,
    const PreprocessorDefineMap& defines,
    std::set<std::string>& expansion_stack) {
    std::string expanded;
    expanded.reserve(text.size());

    char quote_delimiter = '\0';
    std::size_t bracket_depth = 0U;
    std::size_t brace_depth = 0U;
    std::vector<bool> bracket_constant_context;
    for (std::size_t index = 0; index < text.size();) {
        const char ch = text[index];
        if (quote_delimiter != '\0') {
            expanded += ch;
            if (ch == quote_delimiter) {
                if ((index + 1U) < text.size() && text[index + 1U] == quote_delimiter) {
                    expanded += text[index + 1U];
                    index += 2U;
                    continue;
                }
                quote_delimiter = '\0';
            }
            ++index;
            continue;
        }

        if (ch == '\'' || ch == '"') {
            quote_delimiter = ch;
            expanded += ch;
            ++index;
            continue;
        }
        if (ch == '[') {
            bool expands_constants = false;
            if (bracket_depth == 0U) {
                std::size_t previous = index;
                while (previous > 0U && std::isspace(static_cast<unsigned char>(text[previous - 1U])) != 0) {
                    --previous;
                }
                if (previous > 0U) {
                    const char previous_character = text[previous - 1U];
                    expands_constants = is_preprocessor_identifier_char(previous_character) ||
                        previous_character == ')' || previous_character == ']';
                }
            } else if (!bracket_constant_context.empty()) {
                expands_constants = bracket_constant_context.back();
            }
            bracket_constant_context.push_back(expands_constants);
            ++bracket_depth;
            expanded += ch;
            ++index;
            continue;
        }
        if (ch == ']' && bracket_depth > 0U) {
            --bracket_depth;
            if (!bracket_constant_context.empty()) {
                bracket_constant_context.pop_back();
            }
            expanded += ch;
            ++index;
            continue;
        }
        if (ch == '{') {
            ++brace_depth;
            expanded += ch;
            ++index;
            continue;
        }
        if (ch == '}' && brace_depth > 0U) {
            --brace_depth;
            expanded += ch;
            ++index;
            continue;
        }

        const bool expands_bracket_constants = !bracket_constant_context.empty() &&
            bracket_constant_context.back();
        if ((bracket_depth == 0U || expands_bracket_constants) &&
            brace_depth == 0U &&
            is_preprocessor_identifier_start(ch)) {
            std::size_t token_end = index + 1U;
            while (token_end < text.size() && is_preprocessor_identifier_char(text[token_end])) {
                ++token_end;
            }
            expanded += expand_preprocessor_identifier(text.substr(index, token_end - index), defines, expansion_stack);
            index = token_end;
            continue;
        }

        expanded += ch;
        ++index;
    }

    return expanded;
}

std::string substitute_preprocessor_constants(const std::string& text, const PreprocessorDefineMap& defines) {
    std::set<std::string> expansion_stack;
    return substitute_preprocessor_constants(text, defines, expansion_stack);
}

std::string expand_indirect_store_target_macros(
    const std::string& line,
    const PreprocessorDefineMap& defines) {
    const std::string trimmed_line = trim_copy(line);
    if (!starts_with_insensitive(trimmed_line, "STORE ")) {
        return line;
    }

    const std::string body = trim_copy(trimmed_line.substr(6U));
    const std::size_t to_position = find_keyword_top_level(body, "TO");
    if (to_position == std::string::npos) {
        return line;
    }

    const std::string expression = trim_copy(body.substr(0U, to_position));
    const std::vector<std::string> raw_targets =
        split_csv_like(trim_copy(body.substr(to_position + 2U)));
    std::vector<std::string> targets;
    targets.reserve(raw_targets.size());
    bool changed = false;
    for (const std::string& raw_target : raw_targets) {
        std::string target = trim_copy(raw_target);
        if (target.size() >= 4U && target.front() == '(' && target.back() == ')') {
            const std::string indirect_name = trim_copy(target.substr(1U, target.size() - 2U));
            if (indirect_name.size() >= 2U &&
                indirect_name.front() == '[' &&
                indirect_name.back() == ']') {
                const std::string macro_name =
                    trim_copy(indirect_name.substr(1U, indirect_name.size() - 2U));
                if (is_bare_identifier_text(macro_name) &&
                    defines.contains(normalize_identifier(macro_name))) {
                    std::set<std::string> expansion_stack;
                    target = expand_preprocessor_identifier(
                        macro_name,
                        defines,
                        expansion_stack);
                    changed = true;
                }
            }
        }
        targets.push_back(std::move(target));
    }

    if (!changed) {
        return line;
    }

    std::ostringstream expanded;
    expanded << "STORE " << expression << " TO ";
    for (std::size_t index = 0U; index < targets.size(); ++index) {
        if (index != 0U) {
            expanded << ", ";
        }
        expanded << targets[index];
    }
    return expanded.str();
}

bool is_preprocessor_active(const PreprocessorState& state) {
    return state.conditionals.empty() || state.conditionals.back().current_active;
}

struct PreprocessorScalarValue {
    enum class Kind {
        unknown,
        logical,
        numeric,
        string
    };

    Kind kind = Kind::unknown;
    bool logical_value = false;
    double numeric_value = 0.0;
    std::string string_value{};
};

std::string decode_preprocessor_string_literal(const std::string& text) {
    if (text.size() >= 2U &&
        ((text.front() == '\'' && text.back() == '\'') ||
         (text.front() == '"' && text.back() == '"'))) {
        const char delimiter = text.front();
        std::string decoded;
        decoded.reserve(text.size() - 2U);
        for (std::size_t index = 1U; index + 1U < text.size(); ++index) {
            const char ch = text[index];
            if (ch == delimiter && index + 2U < text.size() && text[index + 1U] == delimiter) {
                decoded += delimiter;
                ++index;
                continue;
            }
            decoded += ch;
        }
        return decoded;
    }
    if (text.size() >= 2U && text.front() == '[' && text.back() == ']') {
        return text.substr(1U, text.size() - 2U);
    }
    return text;
}

std::optional<double> try_parse_preprocessor_numeric_literal(const std::string& text) {
    if (text.empty()) {
        return std::nullopt;
    }
    return try_parse_invariant_double(text);
}

PreprocessorScalarValue parse_preprocessor_scalar_value(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.empty()) {
        return {};
    }

    const std::string upper = uppercase_copy(trimmed);
    if (upper == ".T.") {
        return {.kind = PreprocessorScalarValue::Kind::logical, .logical_value = true};
    }
    if (upper == ".F.") {
        return {.kind = PreprocessorScalarValue::Kind::logical, .logical_value = false};
    }
    if ((trimmed.size() >= 2U &&
         ((trimmed.front() == '\'' && trimmed.back() == '\'') ||
          (trimmed.front() == '"' && trimmed.back() == '"'))) ||
        (trimmed.size() >= 2U && trimmed.front() == '[' && trimmed.back() == ']')) {
        return {
            .kind = PreprocessorScalarValue::Kind::string,
            .string_value = decode_preprocessor_string_literal(trimmed)
        };
    }
    if (const auto numeric = try_parse_preprocessor_numeric_literal(trimmed); numeric.has_value()) {
        return {
            .kind = PreprocessorScalarValue::Kind::numeric,
            .numeric_value = *numeric
        };
    }

    return {
        .kind = PreprocessorScalarValue::Kind::unknown,
        .string_value = trimmed
    };
}

std::string stringify_preprocessor_scalar_value(const PreprocessorScalarValue& value) {
    switch (value.kind) {
        case PreprocessorScalarValue::Kind::logical:
            return value.logical_value ? ".T." : ".F.";
        case PreprocessorScalarValue::Kind::numeric: {
            std::ostringstream buffer;
            buffer.imbue(std::locale::classic());
            buffer << value.numeric_value;
            return buffer.str();
        }
        case PreprocessorScalarValue::Kind::string:
        case PreprocessorScalarValue::Kind::unknown:
            return value.string_value;
    }
    return {};
}

bool evaluate_preprocessor_truthiness(const PreprocessorScalarValue& value) {
    switch (value.kind) {
        case PreprocessorScalarValue::Kind::logical:
            return value.logical_value;
        case PreprocessorScalarValue::Kind::numeric:
            return value.numeric_value != 0.0;
        case PreprocessorScalarValue::Kind::string:
            return !value.string_value.empty();
        case PreprocessorScalarValue::Kind::unknown:
            return false;
    }
    return false;
}

std::size_t find_preprocessor_comparison_operator(
    const std::string& expression,
    std::string& operator_text) {
    char quote_delimiter = '\0';
    std::size_t bracket_depth = 0U;
    std::size_t brace_depth = 0U;
    std::size_t paren_depth = 0U;
    for (std::size_t index = 0; index < expression.size(); ++index) {
        const char ch = expression[index];
        if (quote_delimiter != '\0') {
            if (ch == quote_delimiter) {
                if ((index + 1U) < expression.size() && expression[index + 1U] == quote_delimiter) {
                    ++index;
                    continue;
                }
                quote_delimiter = '\0';
            }
            continue;
        }
        if (ch == '\'' || ch == '"') {
            quote_delimiter = ch;
            continue;
        }
        if (ch == '[') {
            ++bracket_depth;
            continue;
        }
        if (ch == ']' && bracket_depth > 0U) {
            --bracket_depth;
            continue;
        }
        if (ch == '{') {
            ++brace_depth;
            continue;
        }
        if (ch == '}' && brace_depth > 0U) {
            --brace_depth;
            continue;
        }
        if (ch == '(') {
            ++paren_depth;
            continue;
        }
        if (ch == ')' && paren_depth > 0U) {
            --paren_depth;
            continue;
        }
        if (bracket_depth > 0U || brace_depth > 0U || paren_depth > 0U) {
            continue;
        }

        if ((index + 1U) < expression.size()) {
            const std::string_view pair(expression.data() + index, 2U);
            if (pair == "==" || pair == "<=" || pair == ">=" || pair == "<>") {
                operator_text.assign(pair.begin(), pair.end());
                return index;
            }
        }
        if (ch == '=' || ch == '#' || ch == '<' || ch == '>' || ch == '$') {
            operator_text.assign(1U, ch);
            return index;
        }
    }
    return std::string::npos;
}

bool evaluate_preprocessor_condition_expression(
    const std::string& expression,
    const PreprocessorDefineMap& defines) {
    std::string expanded = trim_copy(substitute_preprocessor_constants(expression, defines));
    while (has_wrapping_parentheses(expanded)) {
        expanded = trim_copy(expanded.substr(1U, expanded.size() - 2U));
    }
    if (expanded.empty()) {
        return false;
    }

    std::string operator_text;
    if (const std::size_t operator_position =
            find_preprocessor_comparison_operator(expanded, operator_text);
        operator_position != std::string::npos) {
        const PreprocessorScalarValue left = parse_preprocessor_scalar_value(
            expanded.substr(0U, operator_position));
        const PreprocessorScalarValue right = parse_preprocessor_scalar_value(
            expanded.substr(operator_position + operator_text.size()));

        if (operator_text == "$") {
            const std::string left_text = uppercase_copy(stringify_preprocessor_scalar_value(left));
            const std::string right_text = uppercase_copy(stringify_preprocessor_scalar_value(right));
            return !left_text.empty() && right_text.find(left_text) != std::string::npos;
        }

        const bool numeric_comparison =
            (left.kind == PreprocessorScalarValue::Kind::numeric ||
             left.kind == PreprocessorScalarValue::Kind::logical) &&
            (right.kind == PreprocessorScalarValue::Kind::numeric ||
             right.kind == PreprocessorScalarValue::Kind::logical);
        if (numeric_comparison) {
            const double lhs = left.kind == PreprocessorScalarValue::Kind::logical
                ? (left.logical_value ? 1.0 : 0.0)
                : left.numeric_value;
            const double rhs = right.kind == PreprocessorScalarValue::Kind::logical
                ? (right.logical_value ? 1.0 : 0.0)
                : right.numeric_value;
            if (operator_text == "=" || operator_text == "==") {
                return lhs == rhs;
            }
            if (operator_text == "#" || operator_text == "<>") {
                return lhs != rhs;
            }
            if (operator_text == "<") {
                return lhs < rhs;
            }
            if (operator_text == "<=") {
                return lhs <= rhs;
            }
            if (operator_text == ">") {
                return lhs > rhs;
            }
            if (operator_text == ">=") {
                return lhs >= rhs;
            }
        }

        const std::string lhs = uppercase_copy(stringify_preprocessor_scalar_value(left));
        const std::string rhs = uppercase_copy(stringify_preprocessor_scalar_value(right));
        if (operator_text == "=" || operator_text == "==") {
            return lhs == rhs;
        }
        if (operator_text == "#" || operator_text == "<>") {
            return lhs != rhs;
        }
        if (operator_text == "<") {
            return lhs < rhs;
        }
        if (operator_text == "<=") {
            return lhs <= rhs;
        }
        if (operator_text == ">") {
            return lhs > rhs;
        }
        if (operator_text == ">=") {
            return lhs >= rhs;
        }
        return false;
    }

    return evaluate_preprocessor_truthiness(parse_preprocessor_scalar_value(expanded));
}

bool try_parse_ifdef_directive(const std::string& line, std::string& identifier) {
    const std::string trimmed = trim_copy(line);
    if (!starts_with_insensitive(trimmed, "#IFDEF")) {
        return false;
    }
    identifier = trim_copy(trimmed.substr(6U));
    return !identifier.empty();
}

bool try_parse_ifndef_directive(const std::string& line, std::string& identifier) {
    const std::string trimmed = trim_copy(line);
    if (!starts_with_insensitive(trimmed, "#IFNDEF")) {
        return false;
    }
    identifier = trim_copy(trimmed.substr(7U));
    return !identifier.empty();
}

bool try_parse_if_directive(const std::string& line, std::string& expression) {
    const std::string trimmed = trim_copy(line);
    if (!starts_with_insensitive(trimmed, "#IF ")) {
        return false;
    }
    expression = trim_copy(trimmed.substr(3U));
    return !expression.empty();
}

bool is_else_directive(const std::string& line) {
    return uppercase_copy(trim_copy(line)) == "#ELSE";
}

bool is_endif_directive(const std::string& line) {
    return uppercase_copy(trim_copy(line)) == "#ENDIF";
}

void push_preprocessor_conditional(PreprocessorState& state, const bool condition_value) {
    const bool parent_active = is_preprocessor_active(state);
    state.conditionals.push_back({
        .parent_active = parent_active,
        .current_active = parent_active && condition_value,
        .branch_taken = condition_value,
        .else_seen = false
    });
}

void handle_preprocessor_else(PreprocessorState& state) {
    if (state.conditionals.empty()) {
        return;
    }
    auto& frame = state.conditionals.back();
    if (frame.else_seen) {
        frame.current_active = false;
        return;
    }

    frame.current_active = frame.parent_active && !frame.branch_taken;
    frame.branch_taken = true;
    frame.else_seen = true;
}

void handle_preprocessor_endif(PreprocessorState& state) {
    if (!state.conditionals.empty()) {
        state.conditionals.pop_back();
    }
}

bool try_parse_include_directive(const std::string& line, std::string& include_path_text) {
    const std::string trimmed = trim_copy(line);
    if (!starts_with_insensitive(trimmed, "#INCLUDE")) {
        return false;
    }

    const std::string body = trim_copy(trimmed.substr(8U));
    if (body.size() >= 2U &&
        ((body.front() == '"' && body.back() == '"') ||
         (body.front() == '\'' && body.back() == '\'') ||
         (body.front() == '<' && body.back() == '>'))) {
        include_path_text = body.substr(1U, body.size() - 2U);
        return !include_path_text.empty();
    }
    if (body.empty() ||
        std::any_of(
            body.begin(),
            body.end(),
            [](const char character) {
                return std::isspace(static_cast<unsigned char>(character)) != 0;
            })) {
        return false;
    }
    // VFP accepts the common unquoted form: #include frxBuilder.h.
    include_path_text = body;
    return true;
}

bool try_parse_define_directive(
    const std::string& line,
    std::string& identifier,
    std::string& expression) {
    const std::string trimmed = trim_copy(line);
    if (!starts_with_insensitive(trimmed, "#DEFINE")) {
        return false;
    }

    const std::string body = trim_copy(trimmed.substr(7U));
    if (body.empty()) {
        return false;
    }

    const auto [name_token, remainder] = split_first_word(body);
    identifier = trim_copy(name_token);
    expression = trim_copy(remainder);
    return !identifier.empty();
}

fs::path resolve_include_path(const fs::path& owning_path, const std::string& include_path_text) {
    std::string normalized = include_path_text;
    for (char& ch : normalized) {
        if (ch == '\\') {
            ch = '/';
        }
    }
    const fs::path direct = (
        owning_path.parent_path() /
        copperfin::platform::path_from_utf8_string(normalized)).lexically_normal();
    std::error_code direct_error;
    if (fs::exists(direct, direct_error) && !direct_error) {
        return direct;
    }

    const auto resolve_casefold = [](const fs::path& candidate) -> std::optional<fs::path> {
        const copperfin::vfp::SidecarPathResolution resolution =
            copperfin::vfp::resolve_unique_casefold_path(candidate);
        if (!resolution.ambiguous && resolution.path.has_value()) {
            return resolution.path;
        }
        return std::nullopt;
    };
    if (const auto resolved = resolve_casefold(direct); resolved.has_value()) {
        return *resolved;
    }

    const fs::path fallback = (
        owning_path.parent_path() /
        copperfin::platform::path_from_utf8_string(normalized).filename()).lexically_normal();
    std::error_code fallback_error;
    if (fs::exists(fallback, fallback_error) && !fallback_error) {
        return fallback;
    }
    if (const auto resolved = resolve_casefold(fallback); resolved.has_value()) {
        return *resolved;
    }
    return direct;
}

const std::string* find_source_text_override(
    const std::map<std::string, std::string>* source_text_overrides,
    const std::string& path,
    const bool fail_on_ambiguity,
    bool* ambiguous) {
    if (ambiguous != nullptr) {
        *ambiguous = false;
    }
    if (source_text_overrides == nullptr) {
        return nullptr;
    }
    if (const auto exact = source_text_overrides->find(path);
        exact != source_text_overrides->end()) {
        return &exact->second;
    }
    const std::string* folded_match = nullptr;
    for (const auto& candidate : *source_text_overrides) {
        if (!paths_equal_insensitive(candidate.first, path)) {
            continue;
        }
        if (folded_match != nullptr) {
            if (fail_on_ambiguity) {
                if (ambiguous != nullptr) {
                    *ambiguous = true;
                }
                return nullptr;
            }
            return folded_match;
        }
        folded_match = &candidate.second;
    }
    return folded_match;
}

void append_preprocessed_logical_lines(
    const fs::path& path,
    PreprocessorState& state,
    const bool emit_non_directive_lines,
    std::vector<LogicalLine>& output_lines,
    const std::string* source_override = nullptr,
    const std::map<std::string, std::string>* source_text_overrides = nullptr,
    const bool require_source_text_overrides = false) {
    const std::vector<LogicalLine> source_lines = source_override == nullptr
        ? load_logical_lines(path)
        : load_logical_lines_from_text(*source_override);
    for (const auto& logical_line : source_lines) {
        const std::string trimmed = trim_copy(logical_line.text);
        if (trimmed.empty()) {
            if (emit_non_directive_lines && is_preprocessor_active(state)) {
                output_lines.push_back(logical_line);
            }
            continue;
        }

        std::string conditional_identifier;
        if (try_parse_ifdef_directive(trimmed, conditional_identifier)) {
            push_preprocessor_conditional(
                state,
                state.defines.contains(normalize_identifier(conditional_identifier)));
            continue;
        }
        if (try_parse_ifndef_directive(trimmed, conditional_identifier)) {
            push_preprocessor_conditional(
                state,
                !state.defines.contains(normalize_identifier(conditional_identifier)));
            continue;
        }
        std::string conditional_expression;
        if (try_parse_if_directive(trimmed, conditional_expression)) {
            push_preprocessor_conditional(
                state,
                evaluate_preprocessor_condition_expression(conditional_expression, state.defines));
            continue;
        }
        if (is_else_directive(trimmed)) {
            handle_preprocessor_else(state);
            continue;
        }
        if (is_endif_directive(trimmed)) {
            handle_preprocessor_endif(state);
            continue;
        }

        if (!is_preprocessor_active(state)) {
            continue;
        }

        std::string include_path_text;
        if (try_parse_include_directive(trimmed, include_path_text)) {
            const fs::path include_path = resolve_include_path(path, include_path_text);
            const std::string include_key = normalize_path(
                copperfin::platform::path_to_utf8_string(include_path));
            const std::string* include_source = find_source_text_override(
                source_text_overrides,
                include_key,
                require_source_text_overrides,
                nullptr);
            std::error_code exists_error;
            const bool include_exists = fs::exists(include_path, exists_error);
            if (include_source != nullptr || (!require_source_text_overrides && include_exists)) {
                if (state.include_stack.insert(include_key).second) {
                    append_preprocessed_logical_lines(
                        include_path,
                        state,
                        false,
                        output_lines,
                        include_source,
                        source_text_overrides,
                        require_source_text_overrides);
                    state.include_stack.erase(include_key);
                }
            } else if (require_source_text_overrides) {
                throw std::runtime_error(runtime_text(
                    "Runtime.Prg.Parser.Error.VerifiedIncludeSourceUnavailable",
                    {{"path", include_key}}));
            }
            continue;
        }

        std::string identifier;
        std::string expression;
        if (try_parse_define_directive(trimmed, identifier, expression)) {
            state.defines[normalize_identifier(identifier)] = expression;
            continue;
        }

        if (!emit_non_directive_lines) {
            continue;
        }

        LogicalLine expanded_line = logical_line;
        expanded_line.text = substitute_preprocessor_constants(logical_line.text, state.defines);
        expanded_line.text = expand_indirect_store_target_macros(expanded_line.text, state.defines);
        output_lines.push_back(std::move(expanded_line));
    }
}

std::vector<LogicalLine> load_preprocessed_logical_lines(const std::string& path) {
    std::vector<LogicalLine> output_lines;
    PreprocessorState state;
    append_preprocessed_logical_lines(
        copperfin::platform::path_from_utf8_string(path), state, true, output_lines);
    return output_lines;
}

std::vector<LogicalLine> load_preprocessed_logical_lines_from_text(
    const std::string& path,
    const std::string& source_text,
    const std::map<std::string, std::string>& source_text_overrides,
    const bool require_source_text_overrides) {
    std::vector<LogicalLine> output_lines;
    PreprocessorState state;
    append_preprocessed_logical_lines(
        copperfin::platform::path_from_utf8_string(path),
        state,
        true,
        output_lines,
        &source_text,
        &source_text_overrides,
        require_source_text_overrides);
    return output_lines;
}

Statement make_statement(StatementKind kind, const std::string& path, std::size_t line, const std::string& text) {
    Statement statement;
    statement.kind = kind;
    statement.location = {.file_path = normalize_path(path), .line = line};
    statement.text = text;
    return statement;
}

void parse_default_statement(const std::string& line, Statement& statement);

std::vector<Statement> parse_child_object_property_clauses(
    const std::string& path,
    std::size_t line_number,
    const std::string& clauses_text) {
    std::vector<Statement> property_statements;
    for (const std::string& raw_clause : split_csv_like(clauses_text)) {
        const std::string clause = trim_copy(raw_clause);
        if (clause.empty()) {
            continue;
        }

        Statement statement = make_statement(StatementKind::no_op, path, line_number, clause);
        parse_default_statement(clause, statement);
        if (statement.kind == StatementKind::assignment &&
            !trim_copy(statement.identifier).empty() &&
            !trim_copy(statement.expression).empty()) {
            property_statements.push_back(std::move(statement));
        }
    }
    return property_statements;
}

bool has_wrapping_parentheses(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.size() < 2U || trimmed.front() != '(' || trimmed.back() != ')') {
        return false;
    }

    int nesting = 0;
    char quote_delimiter = '\0';
    for (std::size_t index = 0; index < trimmed.size(); ++index) {
        const char ch = trimmed[index];
        if ((ch == '\'' || ch == '"') && (quote_delimiter == '\0' || quote_delimiter == ch)) {
            quote_delimiter = quote_delimiter == '\0' ? ch : '\0';
            continue;
        }
        if (quote_delimiter != '\0') {
            continue;
        }

        if (ch == '(') {
            ++nesting;
            continue;
        }
        if (ch == ')') {
            --nesting;
            if (nesting == 0 && index + 1U < trimmed.size()) {
                return false;
            }
        }
    }
    return nesting == 0;
}

std::string parse_dialog_command_body(const std::string& line, const std::string& keyword) {
    if (uppercase_copy(trim_copy(line)) == keyword) {
        return {};
    }

    const std::string function_prefix = keyword + "(";
    const std::string command_prefix = keyword + " ";
    if (starts_with_insensitive(line, function_prefix)) {
        return trim_copy(line.substr(keyword.size()));
    }
    if (starts_with_insensitive(line, command_prefix)) {
        return trim_copy(line.substr(keyword.size() + 1U));
    }
    return {};
}

std::string strip_dialog_argument_wrapping(std::string body) {
    body = trim_copy(std::move(body));
    while (has_wrapping_parentheses(body)) {
        body = trim_copy(body.substr(1U, body.size() - 2U));
    }
    return body;
}

std::string extract_dialog_target_clause(std::string& body) {
    const std::size_t to_pos = find_keyword_top_level(body, "TO");
    if (to_pos == std::string::npos) {
        return {};
    }

    std::string target = trim_copy(body.substr(to_pos + 2U));
    body = trim_copy(body.substr(0U, to_pos));
    target = trim_copy(target);
    return target;
}

void assign_dialog_positional_if_empty(std::string& field, const std::vector<std::string>& arguments, std::size_t index) {
    if (!field.empty() || index >= arguments.size()) {
        return;
    }
    field = trim_copy(arguments[index]);
}

bool parse_file_storage_statement(const std::string& line, Statement& statement) {
    if (starts_with_insensitive(line, "ERASE ") || starts_with_insensitive(line, "DELETE FILE ")) {
        statement.kind = StatementKind::erase_command;
        const bool starts_delete = starts_with_insensitive(line, "DELETE FILE ");
        statement.expression = trim_copy(line.substr(starts_delete ? 12U : 6U));
        return true;
    }
    if (starts_with_insensitive(line, "COPY FILE ")) {
        statement.kind = StatementKind::copy_file_command;
        const std::string body = trim_copy(line.substr(10U));
        const std::size_t to_pos = find_keyword_top_level(body, "TO");
        if (to_pos != std::string::npos) {
            statement.expression = trim_copy(body.substr(0U, to_pos));
            statement.secondary_expression = trim_copy(body.substr(to_pos + 2U));
        } else {
            statement.expression = body;
        }
        return true;
    }
    if (starts_with_insensitive(line, "RENAME ")) {
        statement.kind = StatementKind::rename_file_command;
        const std::string body = trim_copy(line.substr(7U));
        const std::size_t to_pos = find_keyword_top_level(body, "TO");
        if (to_pos != std::string::npos) {
            statement.expression = trim_copy(body.substr(0U, to_pos));
            statement.secondary_expression = trim_copy(body.substr(to_pos + 2U));
        } else {
            statement.expression = body;
        }
        return true;
    }
    if (!line.empty() && (line[0] == '?' || (line.size() >= 2U && line[0] == '?' && line[1] == '?'))) {
        statement.kind = StatementKind::print_command;
        std::size_t start = 1U;
        while (start < line.size() && line[start] == '?') {
            ++start;
        }
        statement.expression = trim_copy(line.substr(start));
        return true;
    }
    return false;
}

bool parse_table_definition_statement(const std::string& line, Statement& statement) {
    if (starts_with_insensitive(line, "ALTER TABLE ")) {
        statement.kind = StatementKind::alter_table_command;
        const std::string body = trim_copy(line.substr(12U));
        const std::size_t action_position = find_first_keyword_top_level(body, {"ADD", "DROP", "ALTER"});
        if (action_position == std::string::npos) {
            statement.identifier = body;
        } else {
            statement.identifier = trim_copy(body.substr(0U, action_position));
            std::string action_clause = trim_copy(body.substr(action_position));
            const auto [action, remainder] = split_first_word(action_clause);
            action_clause = trim_copy(remainder);
            if (starts_with_insensitive(action_clause, "COLUMN ")) {
                action_clause = trim_copy(action_clause.substr(7U));
            }
            statement.expression = action_clause;
            statement.secondary_expression = lowercase_copy(action);
        }
        return true;
    }
    if (starts_with_insensitive(line, "CREATE TABLE ")) {
        statement.kind = StatementKind::create_table_command;
        const std::string body = trim_copy(line.substr(13U));
        const std::size_t from_position = find_keyword_top_level(body, "FROM");
        if (from_position != std::string::npos) {
            const std::string target = trim_copy(body.substr(0U, from_position));
            const std::string source_clause = trim_copy(body.substr(from_position + 4U));
            if (starts_with_insensitive(source_clause, "ARRAY") &&
                (source_clause.size() == 5U ||
                 std::isspace(static_cast<unsigned char>(source_clause[5U])) != 0)) {
                statement.identifier = target;
                statement.secondary_expression = trim_copy(source_clause.substr(5U));
                statement.tertiary_expression = "array";
                return true;
            }
        }
        const auto find_matching_parenthesis = [&](std::size_t open) -> std::size_t
        {
            int depth = 0;
            char quote = '\0';
            for (std::size_t index = open; index < body.size(); ++index)
            {
                const char current = body[index];
                if (quote != '\0')
                {
                    if (current == quote)
                    {
                        if (index + 1U < body.size() && body[index + 1U] == quote)
                        {
                            ++index;
                        }
                        else
                        {
                            quote = '\0';
                        }
                    }
                    continue;
                }
                if (current == '\'' || current == '"')
                {
                    quote = current;
                }
                else if (current == '(')
                {
                    ++depth;
                }
                else if (current == ')' && --depth == 0)
                {
                    return index;
                }
            }
            return std::string::npos;
        };

        const auto paren_open = body.find('(');
        if (paren_open == 0U)
        {
            const std::size_t target_close = find_matching_parenthesis(paren_open);
            std::size_t fields_open = target_close == std::string::npos ? std::string::npos : target_close + 1U;
            while (fields_open < body.size() && std::isspace(static_cast<unsigned char>(body[fields_open])) != 0)
            {
                ++fields_open;
            }
            if (fields_open != std::string::npos && starts_with_insensitive(body.substr(fields_open), "FREE") &&
                (body.size() == fields_open + 4U ||
                 std::isspace(static_cast<unsigned char>(body[fields_open + 4U])) != 0))
            {
                fields_open += 4U;
                while (fields_open < body.size() && std::isspace(static_cast<unsigned char>(body[fields_open])) != 0)
                {
                    ++fields_open;
                }
            }
            const std::size_t fields_close = fields_open == std::string::npos
                                                 ? std::string::npos
                                                 : find_matching_parenthesis(fields_open);
            if (target_close != std::string::npos && fields_open != std::string::npos &&
                fields_close == body.size() - 1U && target_close < fields_open)
            {
                statement.identifier = trim_copy(body.substr(0U, target_close + 1U));
                statement.expression = body.substr(fields_open + 1U, fields_close - fields_open - 1U);
                return true;
            }
        }

        if (paren_open != std::string::npos && body.back() == ')') {
            statement.identifier = trim_copy(body.substr(0U, paren_open));
            statement.expression = body.substr(paren_open + 1U, body.size() - paren_open - 2U);
        } else {
            statement.identifier = body;
        }
        return true;
    }
    if (starts_with_insensitive(line, "CREATE CURSOR ")) {
        statement.kind = StatementKind::create_cursor_command;
        const std::string body = trim_copy(line.substr(14U));
        const auto paren_open = body.find('(');
        if (paren_open != std::string::npos && body.back() == ')') {
            const std::string declaration = trim_copy(body.substr(0U, paren_open));
            const std::size_t name_position = find_keyword_top_level(declaration, "NAME");
            if (name_position == std::string::npos) {
                statement.identifier = declaration;
            } else {
                statement.identifier = trim_copy(declaration.substr(0U, name_position));
                statement.secondary_expression = trim_copy(declaration.substr(name_position + 4U));
            }
            statement.expression = body.substr(paren_open + 1U, body.size() - paren_open - 2U);
        } else {
            statement.identifier = body;
        }
        return true;
    }
    return false;
}

bool parse_memory_transfer_statement(const std::string& line, Statement& statement) {
    if (starts_with_insensitive(line, "SAVE SCREEN")) {
        statement.kind = StatementKind::no_op;
        return true;
    }
    if (starts_with_insensitive(line, "SAVE TO ")) {
        statement.kind = StatementKind::save_memvars_command;
        const std::string body = trim_copy(line.substr(8U));
        const auto tail_start = find_first_keyword_top_level(body, {"ALL", "LIKE", "EXCEPT"});
        statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
        const std::string like_pattern = extract_command_clause(body, "LIKE", {"EXCEPT"});
        const std::string except_pattern = extract_command_clause(body, "EXCEPT", {"LIKE"});
        if (!like_pattern.empty()) {
            statement.identifier = "LIKE:" + like_pattern;
        } else if (!except_pattern.empty()) {
            statement.identifier = "EXCEPT:" + except_pattern;
        }
        return true;
    }
    if (starts_with_insensitive(line, "RESTORE FROM ")) {
        statement.kind = StatementKind::restore_memvars_command;
        const std::string body = trim_copy(line.substr(13U));
        const auto tail_start = find_first_keyword_top_level(body, {"ADDITIVE"});
        statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
        if (has_keyword(body, "ADDITIVE")) {
            statement.identifier = "ADDITIVE";
        }
        return true;
    }
    return false;
}

bool parse_table_transfer_statement(const std::string& line, Statement& statement) {
    // Copperfin modernization extension (not a Visual FoxPro compatibility
    // spelling): EXPORT DATABASE <dbc> TO <json> TYPE JSON.
    if (starts_with_insensitive(line, "EXPORT DATABASE ")) {
        statement.kind = StatementKind::export_database_command;
        const std::string body = trim_copy(line.substr(16U));
        const std::size_t to_position = find_first_keyword_top_level(body, {"TO"});
        statement.expression = to_position == std::string::npos
            ? body
            : trim_copy(body.substr(0U, to_position));
        statement.secondary_expression = extract_command_clause(body, "TO", {"TYPE"});
        statement.tertiary_expression = extract_command_clause(body, "TYPE", {});
        return true;
    }
    if (starts_with_insensitive(line, "COPY TO ARRAY ")) {
        statement.kind = StatementKind::copy_to_command;
        statement.identifier = "array";
        const std::string body = trim_copy(line.substr(14U));
        const auto tail_start = find_first_keyword_top_level(body, {"FIELDS", "FOR", "WHILE"});
        statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
        statement.tertiary_expression = extract_fields_command_clause(body, {"FOR", "WHILE"});
        statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE"});
        return true;
    }
    if (starts_with_insensitive(line, "COPY STRUCTURE EXTENDED TO ")) {
        statement.kind = StatementKind::copy_to_command;
        statement.identifier = "structure_extended";
        const std::string body = trim_copy(line.substr(27U));
        const auto tail_start = find_first_keyword_top_level(body, {"FIELDS"});
        statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
        statement.tertiary_expression = extract_fields_command_clause(body, {});
        return true;
    }
    if (starts_with_insensitive(line, "COPY TO ") || starts_with_insensitive(line, "COPY STRUCTURE TO ")) {
        statement.kind = StatementKind::copy_to_command;
        const bool is_structure = starts_with_insensitive(line, "COPY STRUCTURE TO ");
        const std::string body = trim_copy(line.substr(is_structure ? 18U : 8U));
        statement.identifier = is_structure ? "structure" : std::string{};
        const auto tail_start = find_first_keyword_top_level(body, {"TYPE", "DELIMITED", "FIELDS", "FOR", "WHILE", "IN"});
        statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
        statement.secondary_expression = extract_command_clause(body, "TYPE", {"WITH", "FIELDS", "FOR", "WHILE", "IN"});
        if (statement.secondary_expression.empty() && has_keyword(body, "DELIMITED")) {
            statement.secondary_expression = "DELIMITED";
        }
        statement.tertiary_expression = extract_fields_command_clause(body, {"TYPE", "FOR", "WHILE", "IN"});
        statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
        const std::string with_clause = extract_command_clause(body, "WITH", {"FIELDS", "FOR", "WHILE", "IN"});
        if (!with_clause.empty()) {
            statement.names.push_back(with_clause);
        }
        return true;
    }
    if (starts_with_insensitive(line, "APPEND FROM ARRAY ")) {
        statement.kind = StatementKind::append_from_command;
        statement.identifier = "array";
        const std::string body = trim_copy(line.substr(18U));
        const auto tail_start = find_first_keyword_top_level(body, {"FIELDS", "FOR", "WHILE"});
        statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
        statement.tertiary_expression = extract_fields_command_clause(body, {"FOR", "WHILE"});
        statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE"});
        return true;
    }
    if (starts_with_insensitive(line, "APPEND FROM ")) {
        statement.kind = StatementKind::append_from_command;
        const std::string body = trim_copy(line.substr(12U));
        const auto tail_start = find_first_keyword_top_level(body, {"TYPE", "DELIMITED", "FIELDS", "FOR", "WHILE"});
        statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
        statement.secondary_expression = extract_command_clause(body, "TYPE", {"WITH", "FIELDS", "FOR", "WHILE"});
        if (statement.secondary_expression.empty() && has_keyword(body, "DELIMITED")) {
            statement.secondary_expression = "DELIMITED";
        }
        statement.tertiary_expression = extract_fields_command_clause(body, {"TYPE", "FOR", "WHILE"});
        statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE"});
        const std::string with_clause = extract_command_clause(body, "WITH", {"FIELDS", "FOR", "WHILE"});
        if (!with_clause.empty()) {
            statement.names.push_back(with_clause);
        }
        return true;
    }
    if (starts_with_insensitive(line, "SCATTER ")) {
        statement.kind = StatementKind::scatter_command;
        const std::string body = trim_copy(line.substr(8U));
        statement.expression = extract_command_clause(body, "TO", {"FIELDS", "MEMVAR", "NAME", "BLANK", "MEMO", "ADDITIVE"});
        statement.secondary_expression = extract_fields_command_clause(body, {"TO", "MEMVAR", "NAME", "BLANK", "MEMO", "ADDITIVE"});
        const std::string name_target = extract_scatter_name_target_clause(body);
        if (!name_target.empty()) {
            statement.identifier = "name";
            statement.expression = name_target;
        } else {
            statement.identifier = has_keyword(body, "MEMVAR") ? "memvar" : std::string{};
        }
        statement.tertiary_expression = has_keyword(body, "BLANK") ? "blank" : std::string{};
        statement.quaternary_expression = has_keyword(body, "MEMO") ? "memo" : std::string{};
        if (has_keyword(body, "ADDITIVE")) {
            statement.names.push_back("additive");
        }
        return true;
    }
    return false;
}

bool parse_storage_statement(const std::string& line, Statement& statement) {
    return parse_file_storage_statement(line, statement) ||
        parse_table_definition_statement(line, statement) ||
        parse_memory_transfer_statement(line, statement) ||
        parse_table_transfer_statement(line, statement);
}

bool parse_control_flow_statement(
    const std::string& line,
    const std::string& upper,
    Statement& statement) {
    const auto is_block_terminator = [&](const std::string_view keyword) {
        const std::string trimmed = trim_copy(line);
        if (trimmed.size() < keyword.size() ||
            !std::equal(
                keyword.begin(),
                keyword.end(),
                trimmed.begin(),
                [](const char left, const char right) {
                    return std::toupper(static_cast<unsigned char>(left)) ==
                        std::toupper(static_cast<unsigned char>(right));
                }))
        {
            return false;
        }
        return trimmed.size() == keyword.size() ||
            std::isspace(static_cast<unsigned char>(trimmed[keyword.size()])) != 0;
    };

    if (starts_with_insensitive(line, "IF ")) {
        statement.kind = StatementKind::if_statement;
        statement.expression = trim_copy(line.substr(3U));
    } else if (starts_with_insensitive(line, "ELSEIF ")) {
        statement.kind = StatementKind::else_statement;
        statement.expression = trim_copy(line.substr(7U));
    } else if (upper == "DO CASE") {
        statement.kind = StatementKind::do_case_statement;
    } else if (starts_with_insensitive(line, "CASE ")) {
        statement.kind = StatementKind::case_statement;
        statement.expression = trim_copy(line.substr(5U));
    } else if (upper == "OTHERWISE") {
        statement.kind = StatementKind::otherwise_statement;
    } else if (upper == "ELSE") {
        statement.kind = StatementKind::else_statement;
    } else if (is_block_terminator("ENDIF")) {
        statement.kind = StatementKind::endif_statement;
    } else if (starts_with_insensitive(line, "FOR EACH ")) {
        // FOR EACH <element> IN <collection>
        statement.kind = StatementKind::for_each_statement;
        const std::string body = trim_copy(line.substr(9U));
        const std::size_t in_pos = find_keyword_top_level(body, "IN");
        if (in_pos != std::string::npos) {
            statement.identifier = trim_copy(body.substr(0U, in_pos));
            statement.expression = trim_copy(body.substr(in_pos + 2U));
            const std::string upper_expression = uppercase_copy(statement.expression);
            static constexpr std::string_view foxobject_suffix = " FOXOBJECT";
            if (upper_expression.size() > foxobject_suffix.size() &&
                upper_expression.compare(
                    upper_expression.size() - foxobject_suffix.size(),
                    foxobject_suffix.size(),
                    foxobject_suffix) == 0)
            {
                statement.expression = trim_copy(
                    statement.expression.substr(
                        0U,
                        statement.expression.size() - foxobject_suffix.size()));
            }
        }
    } else if (starts_with_insensitive(line, "FOR ")) {
        statement.kind = StatementKind::for_statement;
        const std::string body = trim_copy(line.substr(4U));
        const auto equals = body.find('=');
        const auto to_position = uppercase_copy(body).find(" TO ");
        if (equals != std::string::npos && to_position != std::string::npos && to_position > equals) {
            statement.identifier = trim_copy(body.substr(0U, equals));
            statement.expression = trim_copy(body.substr(equals + 1U, to_position - equals - 1U));
            const auto step_position = uppercase_copy(body).find(" STEP ", to_position + 4U);
            if (step_position == std::string::npos) {
                statement.secondary_expression = trim_copy(body.substr(to_position + 4U));
            } else {
                statement.secondary_expression = trim_copy(body.substr(to_position + 4U, step_position - to_position - 4U));
                statement.tertiary_expression = trim_copy(body.substr(step_position + 6U));
            }
        }
    } else if (starts_with_insensitive(line, "DO WHILE ")) {
        statement.kind = StatementKind::do_while_statement;
        statement.expression = trim_copy(line.substr(9U));
    } else if (is_block_terminator("ENDFOR")) {
        statement.kind = StatementKind::endfor_statement;
    } else if (is_block_terminator("ENDDO")) {
        statement.kind = StatementKind::enddo_statement;
    } else if (is_block_terminator("ENDCASE")) {
        statement.kind = StatementKind::endcase_statement;
    } else if (upper == "LOOP") {
        statement.kind = StatementKind::loop_statement;
    } else if (upper == "CONTINUE") {
        statement.kind = StatementKind::continue_command;
    } else if (upper == "EXIT") {
        statement.kind = StatementKind::exit_statement;
    } else if (starts_with_insensitive(line, "WITH ")) {
        statement.kind = StatementKind::with_statement;
        statement.expression = trim_copy(line.substr(5U));
    } else if (is_block_terminator("ENDWITH")) {
        statement.kind = StatementKind::endwith_statement;
    } else if (upper == "TRY") {
        statement.kind = StatementKind::try_statement;
    } else if (starts_with_insensitive(line, "CATCH")) {
        statement.kind = StatementKind::catch_statement;
        const std::string catch_tail = trim_copy(line.substr(5U));
        const std::size_t when_position = find_keyword_top_level(catch_tail, "WHEN");
        const std::string catch_head = trim_copy(
            when_position == std::string::npos
                ? catch_tail
                : catch_tail.substr(0U, when_position));
        if (when_position != std::string::npos)
        {
            statement.secondary_expression = trim_copy(catch_tail.substr(when_position + 4U));
        }
        statement.identifier = catch_head;
        if (starts_with_insensitive(statement.identifier, "TO "))
        {
            statement.identifier = trim_copy(statement.identifier.substr(3U));
        }
    } else if (upper == "FINALLY") {
        statement.kind = StatementKind::finally_statement;
    } else if (is_block_terminator("ENDTRY")) {
        statement.kind = StatementKind::endtry_statement;
    } else {
        return false;
    }
    return true;
}

bool parse_popup_statement(const std::string& line, Statement& statement) {
    const std::string on_bar_prefix = "ON BAR ";
    if (starts_with_insensitive(line, on_bar_prefix)) {
        const std::string body = trim_copy(line.substr(on_bar_prefix.size()));
        const auto [bar_number, bar_tail] = split_first_word(body);
        const std::size_t of_position = find_keyword_top_level(bar_tail, "OF");
        const std::size_t activate_position = of_position == std::string::npos
            ? std::string::npos
            : find_keyword_top_level_from(bar_tail, "ACTIVATE", of_position + 2U);
        if (of_position == std::string::npos || activate_position == std::string::npos) {
            return false;
        }

        const std::string popup_name = unquote_identifier(trim_copy(
            bar_tail.substr(of_position + 2U, activate_position - of_position - 2U)));
        const std::string activation_text = trim_copy(bar_tail.substr(activate_position + 8U));
        if (!starts_with_insensitive(activation_text, "POPUP ")) {
            return false;
        }
        const std::string submenu_text = trim_copy(activation_text.substr(6U));
        const std::string submenu_name = unquote_identifier(take_first_token(submenu_text));
        if (bar_number.empty() || popup_name.empty() || submenu_name.empty() ||
            submenu_name != submenu_text) {
            return false;
        }

        statement.kind = StatementKind::on_bar_activate_popup_command;
        statement.secondary_expression = trim_copy(bar_number);
        statement.identifier = popup_name;
        statement.expression = submenu_name;
        return true;
    }
    const std::string selection_popup_prefix = "ON SELECTION POPUP ";
    if (starts_with_insensitive(line, selection_popup_prefix)) {
        const std::string body = trim_copy(line.substr(selection_popup_prefix.size()));
        const auto [popup_name, command_text] = split_first_word(body);
        if (popup_name.empty()) {
            return false;
        }

        if (command_text.empty()) {
            statement.kind = StatementKind::on_selection_popup_command;
            statement.identifier = unquote_identifier(popup_name);
            return true;
        }

        if (!starts_with_insensitive(command_text, "DO ")) {
            return false;
        }
        const std::string routine_text = trim_copy(command_text.substr(3U));
        const std::string routine_name = unquote_identifier(take_first_token(routine_text));
        if (routine_name.empty() || routine_name != routine_text) {
            return false;
        }

        statement.kind = StatementKind::on_selection_popup_command;
        statement.identifier = unquote_identifier(popup_name);
        statement.expression = routine_name;
        return true;
    }
    const std::string selection_bar_prefix = "ON SELECTION BAR ";
    if (starts_with_insensitive(line, selection_bar_prefix)) {
        const std::string body = trim_copy(line.substr(selection_bar_prefix.size()));
        const auto [bar_number, bar_tail] = split_first_word(body);
        const std::size_t of_position = find_keyword_top_level(bar_tail, "OF");
        const std::size_t do_position = of_position == std::string::npos
            ? std::string::npos
            : find_keyword_top_level_from(bar_tail, "DO", of_position + 2U);
        if (of_position == std::string::npos) {
            return false;
        }

        if (do_position != std::string::npos) {
            const std::string popup_name = unquote_identifier(trim_copy(
                bar_tail.substr(of_position + 2U, do_position - of_position - 2U)));
            const std::string routine_text = trim_copy(bar_tail.substr(do_position + 2U));
            const std::string routine_name = unquote_identifier(take_first_token(routine_text));
            if (bar_number.empty() || popup_name.empty() || routine_name.empty() ||
                routine_name != routine_text) {
                return false;
            }

            statement.kind = StatementKind::on_selection_bar_command;
            statement.secondary_expression = trim_copy(bar_number);
            statement.identifier = popup_name;
            statement.expression = routine_name;
            return true;
        }

        const auto [popup_token, action_text] = split_first_word(
            trim_copy(bar_tail.substr(of_position + 2U)));
        const std::string popup_name = unquote_identifier(popup_token);
        if (bar_number.empty() || popup_name.empty() || action_text.empty() ||
            action_text.find('&') != std::string::npos ||
            action_text.find('\n') != std::string::npos ||
            action_text.find('\r') != std::string::npos) {
            return false;
        }

        statement.kind = StatementKind::on_selection_bar_action_command;
        statement.secondary_expression = trim_copy(bar_number);
        statement.identifier = popup_name;
        statement.expression = action_text;
        return true;
    }
    if (starts_with_insensitive(line, "DEFINE MENU ")) {
        statement.kind = StatementKind::define_menu_command;
        const std::string body = trim_copy(line.substr(12U));
        statement.identifier = unquote_identifier(take_first_token(body));
        return true;
    }
    if (starts_with_insensitive(line, "DEFINE POPUP ")) {
        statement.kind = StatementKind::define_popup_command;
        const std::string body = trim_copy(line.substr(13U));
        statement.identifier = unquote_identifier(take_first_token(body));
        return true;
    }
    if (!starts_with_insensitive(line, "DEFINE BAR ")) {
        return false;
    }

    const std::string body = trim_copy(line.substr(11U));
    const auto [bar_number, bar_tail] = split_first_word(body);
    const std::size_t of_position = find_keyword_top_level(bar_tail, "OF");
    const std::size_t prompt_position = of_position == std::string::npos
        ? std::string::npos
        : find_keyword_top_level_from(bar_tail, "PROMPT", of_position + 2U);
    if (of_position == std::string::npos || prompt_position == std::string::npos) {
        return false;
    }

    statement.kind = StatementKind::define_bar_command;
    statement.secondary_expression = trim_copy(bar_number);
    statement.identifier = unquote_identifier(trim_copy(
        bar_tail.substr(of_position + 2U, prompt_position - of_position - 2U)));
    statement.expression = trim_copy(bar_tail.substr(prompt_position + 6U));
    return true;
}

void parse_default_statement(const std::string& line, Statement& statement) {
    if (parse_popup_statement(line, statement)) {
        return;
    }
    if (starts_with_insensitive(line, "DECLARE ") &&
        !looks_like_array_declaration_body(line.substr(8U))) {
        statement.kind = StatementKind::declare_dll;
        const std::string body = trim_copy(line.substr(8U));
        const std::size_t in_pos = find_keyword_top_level(body, "IN");
        if (in_pos != std::string::npos) {
            const std::string lhs = trim_copy(body.substr(0U, in_pos));
            const std::string rhs = trim_copy(body.substr(in_pos + 2U));
            const auto space_pos = lhs.find(' ');
            if (space_pos != std::string::npos) {
                statement.secondary_expression = trim_copy(lhs.substr(0U, space_pos));
                const std::string fn_part = trim_copy(lhs.substr(space_pos + 1U));
                const auto paren_pos = fn_part.find('(');
                if (paren_pos != std::string::npos) {
                    statement.identifier = trim_copy(fn_part.substr(0U, paren_pos));
                    const auto close_paren = fn_part.rfind(')');
                    if (close_paren != std::string::npos && close_paren > paren_pos) {
                        statement.tertiary_expression = fn_part.substr(paren_pos + 1U, close_paren - paren_pos - 1U);
                    }
                } else {
                    statement.identifier = fn_part;
                }
            } else {
                statement.identifier = lhs;
            }
            auto [library_expression, declaration_tail] = split_declare_library_expression(rhs);
            statement.expression = std::move(library_expression);
            if (declaration_tail.size() > 2U &&
                starts_with_insensitive(declaration_tail, "AS") &&
                std::isspace(static_cast<unsigned char>(declaration_tail[2U])) != 0) {
                const std::string alias_and_tail = trim_copy(declaration_tail.substr(2U));
                std::size_t alias_end = 0U;
                while (alias_end < alias_and_tail.size() &&
                       std::isspace(static_cast<unsigned char>(alias_and_tail[alias_end])) == 0) {
                    ++alias_end;
                }
                statement.quaternary_expression = alias_and_tail.substr(0U, alias_end);
                declaration_tail = trim_copy(alias_and_tail.substr(alias_end));
            }
            if (statement.tertiary_expression.empty()) {
                statement.tertiary_expression = trim_copy(declaration_tail);
            }
        } else {
            statement.kind = StatementKind::no_op;
            statement.expression = body;
        }
        return;
    }
    if (starts_with_insensitive(line, "GATHER FROM ") || starts_with_insensitive(line, "GATHER MEMVAR") ||
        starts_with_insensitive(line, "GATHER NAME ")) {
        statement.kind = StatementKind::gather_command;
        const std::string body = trim_copy(line.substr(7U));
        statement.expression = extract_command_clause(body, "FROM", {"FIELDS", "MEMVAR", "NAME", "FOR"});
        statement.secondary_expression = extract_fields_command_clause(body, {"FROM", "MEMVAR", "NAME", "FOR"});
        const std::string name_source = extract_command_clause(body, "NAME", {"FROM", "FIELDS", "MEMVAR", "FOR"});
        if (!name_source.empty()) {
            statement.identifier = "name";
            statement.expression = name_source;
        } else {
            statement.identifier = has_keyword(body, "MEMVAR") ? "memvar" : std::string{};
        }
        statement.quaternary_expression = extract_command_clause(body, "FOR", {"FROM", "FIELDS", "MEMVAR", "NAME"});
        return;
    }

    const auto equals = line.find('=');
    if (!line.empty() && line[0] == '=') {
        statement.kind = StatementKind::expression;
        statement.expression = trim_copy(line.substr(1U));
    } else if (equals != std::string::npos) {
        statement.kind = StatementKind::assignment;
        statement.identifier = trim_copy(line.substr(0U, equals));
        statement.expression = trim_copy(line.substr(equals + 1U));
    } else {
        statement.kind = StatementKind::expression;
        statement.expression = line;
    }
}

}  // namespace

Program parse_program_impl(
    const std::string& path,
    const std::string* source_override,
    const std::map<std::string, std::string>* source_text_overrides = nullptr,
    const bool require_source_text_overrides = false) {
    Program program;
    program.path = normalize_path(path);
    program.source_lines = source_override == nullptr
        ? load_source_lines(path)
        : load_source_lines_from_text(*source_override);
    program.main.name = "main";

    Routine* current = &program.main;
    PrgClassDefinition* current_class = nullptr;
    NativeChildObjectDeclaration* current_child_object = nullptr;
    const auto finalize_open_routine = [&](const std::size_t end_line_exclusive)
    {
        if (current != &program.main && current != nullptr && current->body_end_line_exclusive == 0) {
            current->body_end_line_exclusive = end_line_exclusive;
        }
    };
    const std::vector<LogicalLine> logical_lines = source_override == nullptr
        ? load_preprocessed_logical_lines(path)
        : load_preprocessed_logical_lines_from_text(
              path,
              *source_override,
              source_text_overrides == nullptr
                  ? std::map<std::string, std::string>{}
                  : *source_text_overrides,
              require_source_text_overrides);
    for (const auto& logical_line : logical_lines) {
        const std::size_t line_number = logical_line.line_number;
        const std::string line = trim_copy(logical_line.text);
        if (line.empty()) {
            continue;
        }
        if (line[0] == '*' || starts_with_insensitive(line, "* ") || starts_with_insensitive(line, "#")) {
            continue;
        }

        const std::string trimmed_line = trim_copy(line);
        const std::string upper = uppercase_copy(line);
        const std::string trimmed_upper = uppercase_copy(trimmed_line);
        if (current_class == nullptr && current == &program.main && starts_with_insensitive(trimmed_line, "DEFINE CLASS ")) {
            const std::string body = trim_copy(trimmed_line.substr(13U));
            const std::size_t as_position = find_keyword_top_level(body, "AS");
            const std::size_t of_position =
                as_position == std::string::npos
                    ? std::string::npos
                    : find_keyword_top_level_from(body, "OF", as_position + 2U);

            PrgClassDefinition class_definition;
            class_definition.name = trim_copy(
                as_position == std::string::npos
                    ? body
                    : body.substr(0U, as_position));
            class_definition.base_class_name = trim_copy(
                as_position == std::string::npos
                    ? std::string{}
                    : of_position == std::string::npos
                        ? body.substr(as_position + 2U)
                        : body.substr(as_position + 2U, of_position - as_position - 2U));
            class_definition.base_class_source_path = trim_copy(
                of_position == std::string::npos
                    ? std::string{}
                    : body.substr(of_position + 2U));
            class_definition.declaration_location = {.file_path = normalize_path(path), .line = line_number};
            current_class = &program.classes[normalize_identifier(class_definition.name)];
            *current_class = std::move(class_definition);
            current = &program.main;
            continue;
        }
        if (current_child_object != nullptr && trimmed_upper == "ENDOBJECT") {
            current_child_object = nullptr;
            continue;
        }
        if (current_class != nullptr && trimmed_upper == "ENDDEFINE") {
            finalize_open_routine(line_number);
            current_child_object = nullptr;
            current_class = nullptr;
            current = &program.main;
            continue;
        }
        NativeMemberVisibility routine_visibility = NativeMemberVisibility::public_member;
        std::string routine_line = line;
        if (current_class != nullptr && current == &program.main) {
            const bool protected_declaration = starts_with_insensitive(trimmed_line, "PROTECTED ");
            const bool hidden_declaration = starts_with_insensitive(trimmed_line, "HIDDEN ");
            if (protected_declaration || hidden_declaration) {
                const std::size_t keyword_length = protected_declaration ? 9U : 7U;
                const std::string remainder = trim_copy(trimmed_line.substr(keyword_length));
                routine_visibility = protected_declaration
                    ? NativeMemberVisibility::protected_member
                    : NativeMemberVisibility::hidden_member;
                if (starts_with_insensitive(remainder, "PROCEDURE ") ||
                    starts_with_insensitive(remainder, "PROC ") ||
                    starts_with_insensitive(remainder, "FUNCTION ")) {
                    routine_line = remainder;
                } else if (!remainder.empty()) {
                    for (const std::string &raw_name : split_csv_like(remainder)) {
                        const std::string member_name = normalize_identifier(trim_copy(raw_name));
                        if (!member_name.empty()) {
                            current_class->member_visibility[member_name] = routine_visibility;
                        }
                    }
                    continue;
                }
            }
        }
        if (starts_with_insensitive(routine_line, "PROCEDURE ") ||
            starts_with_insensitive(routine_line, "PROC ") ||
            starts_with_insensitive(routine_line, "FUNCTION ")) {
            finalize_open_routine(line_number);
            const auto separator = routine_line.find(' ');
            std::string routine_signature = trim_copy(routine_line.substr(separator + 1U));
            std::string inline_parameter_clause;
            if (const std::size_t open_paren = routine_signature.find('(');
                open_paren != std::string::npos)
            {
                const std::size_t close_paren = routine_signature.rfind(')');
                if (close_paren != std::string::npos && close_paren > open_paren)
                {
                    inline_parameter_clause = trim_copy(
                        routine_signature.substr(open_paren + 1U, close_paren - open_paren - 1U));
                    routine_signature = trim_copy(routine_signature.substr(0U, open_paren));
                }
            }

            Routine routine;
            routine.name = std::move(routine_signature);
            routine.kind = starts_with_insensitive(routine_line, "FUNCTION ")
                ? RoutineKind::function
                : RoutineKind::procedure;
            routine.visibility = routine_visibility;
            routine.declaration_location = {.file_path = normalize_path(path), .line = line_number};
            if (current_class != nullptr) {
                current = &current_class->methods[normalize_identifier(routine.name)];
                current_class->member_visibility[normalize_identifier(routine.name)] = routine_visibility;
            } else {
                current = &program.routines[normalize_identifier(routine.name)];
            }
            *current = std::move(routine);
            if (!inline_parameter_clause.empty())
            {
                Statement parameters = make_statement(
                    StatementKind::lparameters_declaration,
                    path,
                    line_number,
                    "LPARAMETERS " + inline_parameter_clause);
                parameters.names = split_csv_like(inline_parameter_clause);
                current->statements.push_back(std::move(parameters));
            }
            continue;
        }
        if (starts_with_insensitive(line, "ENDPROC") || starts_with_insensitive(line, "ENDFUNC") || starts_with_insensitive(line, "END FUNC")) {
            finalize_open_routine(line_number);
            current = &program.main;
            continue;
        }
        if (current_child_object != nullptr) {
            Statement statement = make_statement(StatementKind::no_op, path, line_number, line);
            parse_default_statement(line, statement);
            if (statement.kind == StatementKind::assignment &&
                !trim_copy(statement.identifier).empty() &&
                !trim_copy(statement.expression).empty()) {
                current_child_object->property_statements.push_back(std::move(statement));
            }
            continue;
        }
        if (current_class != nullptr && current == &program.main && starts_with_insensitive(trimmed_line, "ADD OBJECT ")) {
            const std::string body = trim_copy(trimmed_line.substr(11U));
            const std::size_t as_position = find_keyword_top_level(body, "AS");
            if (as_position != std::string::npos) {
                const std::size_t of_position = find_keyword_top_level_from(body, "OF", as_position + 2U);
                const std::size_t with_position = find_keyword_top_level_from(body, "WITH", as_position + 2U);
                NativeChildObjectDeclaration declaration;
                declaration.name = trim_copy(body.substr(0U, as_position));
                const std::size_t class_end =
                    of_position == std::string::npos
                        ? with_position
                        : with_position == std::string::npos
                              ? of_position
                              : std::min(of_position, with_position);
                declaration.class_name = trim_copy(
                    class_end == std::string::npos
                        ? body.substr(as_position + 2U)
                        : body.substr(as_position + 2U, class_end - as_position - 2U));
                if (of_position != std::string::npos) {
                    declaration.source_path = trim_copy(
                        with_position == std::string::npos
                            ? body.substr(of_position + 2U)
                            : body.substr(of_position + 2U, with_position - of_position - 2U));
                }
                declaration.declaration_location = {.file_path = normalize_path(path), .line = line_number};
                declaration.text = trimmed_line;
                if (with_position != std::string::npos) {
                    declaration.property_statements = parse_child_object_property_clauses(
                        path,
                        line_number,
                        trim_copy(body.substr(with_position + 4U)));
                }
                if (!declaration.name.empty() && !declaration.class_name.empty()) {
                    current_class->child_object_declarations.push_back(std::move(declaration));
                    continue;
                }
            }
        }
        if (current_class != nullptr && current == &program.main && starts_with_insensitive(trimmed_line, "OBJECT ")) {
            const std::string body = trim_copy(trimmed_line.substr(7U));
            const std::size_t as_position = find_keyword_top_level(body, "AS");
            if (as_position != std::string::npos) {
                const std::size_t of_position = find_keyword_top_level_from(body, "OF", as_position + 2U);
                NativeChildObjectDeclaration declaration;
                declaration.name = trim_copy(body.substr(0U, as_position));
                declaration.class_name = trim_copy(
                    of_position == std::string::npos
                        ? body.substr(as_position + 2U)
                        : body.substr(as_position + 2U, of_position - as_position - 2U));
                if (of_position != std::string::npos) {
                    declaration.source_path = trim_copy(body.substr(of_position + 2U));
                }
                declaration.declaration_location = {.file_path = normalize_path(path), .line = line_number};
                declaration.text = trimmed_line;
                if (!declaration.name.empty() && !declaration.class_name.empty()) {
                    current_class->child_object_declarations.push_back(std::move(declaration));
                    current_child_object = &current_class->child_object_declarations.back();
                    continue;
                }
            }
        }

        Statement statement = make_statement(StatementKind::no_op, path, line_number, line);
        if (parse_control_flow_statement(line, upper, statement)) {
        } else if (upper == "THROW" || starts_with_insensitive(line, "THROW ")) {
            statement.kind = StatementKind::throw_statement;
            statement.expression = upper == "THROW" ? std::string{} : trim_copy(line.substr(6U));
        } else if (starts_with_insensitive(line, "DO FORM ")) {
            statement.kind = StatementKind::do_form;
            statement.identifier = trim_copy(line.substr(8U));
        } else if (starts_with_insensitive(line, "REPORT FORM ")) {
            statement.kind = StatementKind::report_form;
            const std::string body = trim_copy(line.substr(12U));
            statement.identifier = take_first_token(body);
            statement.secondary_expression = has_keyword(body, "PREVIEW") ? "preview" : std::string{};
            statement.tertiary_expression =
                extract_command_clause(body, "TO", {"PREVIEW", "NOCONSOLE", "PLAIN", "NOWAIT", "FOR", "WHILE"});
            statement.quaternary_expression =
                extract_command_clause(body, "FOR", {"WHILE", "TO", "PREVIEW", "NOCONSOLE", "PLAIN", "NOWAIT"});
            const std::string while_clause =
                extract_command_clause(body, "WHILE", {"FOR", "TO", "PREVIEW", "NOCONSOLE", "PLAIN", "NOWAIT"});
            if (!while_clause.empty()) {
                statement.names.push_back(while_clause);
            }
        } else if (starts_with_insensitive(line, "LABEL FORM ")) {
            statement.kind = StatementKind::label_form;
            const std::string body = trim_copy(line.substr(11U));
            statement.identifier = take_first_token(body);
            statement.secondary_expression = has_keyword(body, "PREVIEW") ? "preview" : std::string{};
            statement.tertiary_expression =
                extract_command_clause(body, "TO", {"PREVIEW", "NOCONSOLE", "PLAIN", "NOWAIT", "FOR", "WHILE"});
            statement.quaternary_expression =
                extract_command_clause(body, "FOR", {"WHILE", "TO", "PREVIEW", "NOCONSOLE", "PLAIN", "NOWAIT"});
            const std::string while_clause =
                extract_command_clause(body, "WHILE", {"FOR", "TO", "PREVIEW", "NOCONSOLE", "PLAIN", "NOWAIT"});
            if (!while_clause.empty()) {
                statement.names.push_back(while_clause);
            }
        } else if (starts_with_insensitive(line, "ACTIVATE POPUP ")) {
            statement.kind = StatementKind::activate_surface;
            statement.identifier = "popup";
            statement.expression = trim_copy(line.substr(15U));
        } else if (starts_with_insensitive(line, "ACTIVATE MENU ")) {
            statement.kind = StatementKind::activate_surface;
            statement.identifier = "menu";
            statement.expression = trim_copy(line.substr(14U));
        } else if (starts_with_insensitive(line, "DEACTIVATE POPUP ")) {
            statement.kind = StatementKind::deactivate_surface;
            statement.identifier = "popup";
            statement.expression = trim_copy(line.substr(17U));
        } else if (starts_with_insensitive(line, "DEACTIVATE MENU ")) {
            statement.kind = StatementKind::deactivate_surface;
            statement.identifier = "menu";
            statement.expression = trim_copy(line.substr(16U));
        } else if (starts_with_insensitive(line, "RELEASE POPUP ")) {
            statement.kind = StatementKind::release_surface;
            statement.identifier = "popup";
            statement.expression = trim_copy(line.substr(14U));
        } else if (starts_with_insensitive(line, "RELEASE MENU ")) {
            statement.kind = StatementKind::release_surface;
            statement.identifier = "menu";
            statement.expression = trim_copy(line.substr(13U));
        } else if (upper == "PUSH KEY" || starts_with_insensitive(line, "PUSH KEY ")) {
            statement.kind = StatementKind::push_key_command;
            const std::string body = upper == "PUSH KEY" ? std::string{} : trim_copy(line.substr(8U));
            statement.expression = body;
            statement.identifier = take_first_token(body);
        } else if (upper == "POP KEY" || starts_with_insensitive(line, "POP KEY ")) {
            statement.kind = StatementKind::pop_key_command;
            const std::string body = upper == "POP KEY" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = body;
            statement.identifier = take_first_token(body);
        } else if (upper == "PUSH MENU" || starts_with_insensitive(line, "PUSH MENU ")) {
            statement.kind = StatementKind::push_menu_command;
            const std::string body = upper == "PUSH MENU" ? std::string{} : trim_copy(line.substr(9U));
            statement.expression = body;
            statement.identifier = take_first_token(body);
        } else if (upper == "POP MENU" || starts_with_insensitive(line, "POP MENU ")) {
            statement.kind = StatementKind::pop_menu_command;
            const std::string body = upper == "POP MENU" ? std::string{} : trim_copy(line.substr(8U));
            statement.expression = body;
            statement.identifier = take_first_token(body);
        } else if (upper == "PUSH POPUP" || starts_with_insensitive(line, "PUSH POPUP ")) {
            statement.kind = StatementKind::push_popup_command;
            const std::string body = upper == "PUSH POPUP" ? std::string{} : trim_copy(line.substr(10U));
            statement.expression = body;
            statement.identifier = take_first_token(body);
        } else if (upper == "POP POPUP" || starts_with_insensitive(line, "POP POPUP ")) {
            statement.kind = StatementKind::pop_popup_command;
            const std::string body = upper == "POP POPUP" ? std::string{} : trim_copy(line.substr(9U));
            statement.expression = body;
            statement.identifier = take_first_token(body);
        } else if (upper == "RELEASE ALL" || starts_with_insensitive(line, "RELEASE ALL ")) {
            // RELEASE ALL [LIKE <pattern> | EXCEPT <pattern>]
            statement.kind = StatementKind::release_command;
            statement.identifier = "all";
            const std::string rest = trim_copy(line.substr(11U));
            const std::string rest_upper = uppercase_copy(rest);
            if (starts_with_insensitive(rest, "LIKE ")) {
                statement.expression = "like";
                statement.secondary_expression = trim_copy(rest.substr(5U));
            } else if (starts_with_insensitive(rest, "EXCEPT ")) {
                statement.expression = "except";
                statement.secondary_expression = trim_copy(rest.substr(7U));
            }
        } else if (starts_with_insensitive(line, "RELEASE ")) {
            // RELEASE <varlist>
            statement.kind = StatementKind::release_command;
            statement.identifier = "vars";
            statement.names = split_csv_like(trim_copy(line.substr(8U)));
        } else if (upper == "CLEAR MEMORY" || upper == "CLEAR ALL") {
            statement.kind = StatementKind::clear_memory_command;
            statement.identifier = upper == "CLEAR ALL" ? "all" : "memory";
        } else if (upper == "CANCEL") {
            statement.kind = StatementKind::cancel_statement;
        } else if (upper == "QUIT") {
            statement.kind = StatementKind::quit_statement;
        } else if (upper == "YIELD") {
            statement.kind = StatementKind::yield_statement;
        } else if (starts_with_insensitive(line, "YIELD ")) {
            statement.kind = StatementKind::yield_statement;
            statement.expression = trim_copy(line.substr(5U));
        } else if (starts_with_insensitive(line, "ENTER CRITICAL") || upper == "ENTER CRITICAL") {
            statement.kind = StatementKind::enter_critical_command;
            statement.identifier = upper == "ENTER CRITICAL" ? std::string{} : trim_copy(line.substr(14U));
        } else if (starts_with_insensitive(line, "EXIT CRITICAL") || upper == "EXIT CRITICAL") {
            statement.kind = StatementKind::exit_critical_command;
            statement.identifier = upper == "EXIT CRITICAL" ? std::string{} : trim_copy(line.substr(13U));
        } else if (starts_with_insensitive(line, "SPAWN ") || upper == "SPAWN" || starts_with_insensitive(line, "ASYNC ") || upper == "ASYNC") {
            statement.kind = StatementKind::spawn_command;
            const std::string body = trim_copy(line.substr(upper == "SPAWN" ? 5U : upper == "ASYNC" ? 5U : 6U));
            const std::size_t to_position = find_keyword_top_level(body, "TO");
            const std::string launch_part = to_position == std::string::npos ? body : trim_copy(body.substr(0U, to_position));
            const std::string target_part = to_position == std::string::npos ? std::string{} : trim_copy(body.substr(to_position + 2U));
            const std::size_t with_position = find_keyword_top_level(launch_part, "WITH");
            if (with_position == std::string::npos) {
                statement.identifier = launch_part;
            } else {
                statement.identifier = trim_copy(launch_part.substr(0U, with_position));
                statement.expression = trim_copy(launch_part.substr(with_position + 4U));
            }
            if (!target_part.empty()) {
                statement.names.push_back(target_part);
            }
        } else if (starts_with_insensitive(line, "AWAIT ") || upper == "AWAIT") {
            statement.kind = StatementKind::await_command;
            const std::string body = trim_copy(line.substr(upper == "AWAIT" ? 5U : 6U));
            const std::size_t to_position = find_keyword_top_level(body, "TO");
            statement.expression = to_position == std::string::npos ? body : trim_copy(body.substr(0U, to_position));
            if (to_position != std::string::npos) {
                const std::string target_part = trim_copy(body.substr(to_position + 2U));
                if (!target_part.empty()) {
                    statement.names.push_back(target_part);
                }
            }
        } else if (starts_with_insensitive(line, "DO ")) {
            statement.kind = StatementKind::do_command;
            const std::string body = trim_copy(line.substr(3U));
            const std::size_t with_position = find_keyword_top_level(body, "WITH");
            if (with_position == std::string::npos) {
                statement.identifier = body;
            } else {
                statement.identifier = trim_copy(body.substr(0U, with_position));
                statement.expression = trim_copy(body.substr(with_position + 4U));
            }
            } else if (starts_with_insensitive(line, "CALL ")) {
                statement.kind = StatementKind::call_command;
                const std::string body = trim_copy(line.substr(5U));
                const std::size_t with_position = find_keyword_top_level(body, "WITH");
                if (with_position == std::string::npos) {
                    statement.identifier = body;
                } else {
                    statement.identifier = trim_copy(body.substr(0U, with_position));
                    statement.expression = trim_copy(body.substr(with_position + 4U));
                }
        } else if (upper == "READ EVENTS") {
            statement.kind = StatementKind::read_events;
        } else if (upper == "CLEAR EVENTS") {
            statement.kind = StatementKind::clear_events;
        } else if (upper == "BEGIN TRANSACTION") {
            statement.kind = StatementKind::begin_transaction;
        } else if (upper == "END TRANSACTION") {
            statement.kind = StatementKind::end_transaction;
        } else if (upper == "ROLLBACK" || upper == "ROLLBACK TRANSACTION") {
            statement.kind = StatementKind::rollback_transaction;
        } else if (upper == "UNDO" || upper == "UNDO ALL") {
            statement.kind = StatementKind::undo_command;
            if (upper == "UNDO ALL") {
                statement.secondary_expression = "all";
            }
        } else if (upper == "DOEVENTS") {
            statement.kind = StatementKind::doevents_command;
        } else if (starts_with_insensitive(line, "LPARAMETERS ")) {
            statement.kind = StatementKind::lparameters_declaration;
            statement.names = split_csv_like(line.substr(12U));
        } else if (starts_with_insensitive(line, "LPARAMETER ")) {
            // VFP accepts the singular command as an abbreviation; legacy
            // sources such as ReportBuilder use it in object methods.
            statement.kind = StatementKind::lparameters_declaration;
            statement.names = split_csv_like(line.substr(11U));
        } else if (starts_with_insensitive(line, "PARAMETERS ")) {
            statement.kind = StatementKind::parameters_declaration;
            statement.names = split_csv_like(line.substr(11U));
        } else if (starts_with_insensitive(line, "SEEK ")) {
            statement.kind = StatementKind::seek_command;
            const std::string body = trim_copy(line.substr(5U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"ORDER", "TAG", "IN", "ASCENDING", "DESCENDING"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "IN", {"ORDER", "TAG", "ASCENDING", "DESCENDING"});
            statement.tertiary_expression = extract_command_clause(body, "ORDER", {"TAG", "IN", "ASCENDING", "DESCENDING"});
            if (statement.tertiary_expression.empty()) {
                const std::string tag_name = extract_command_clause(body, "TAG", {"ORDER", "IN", "ASCENDING", "DESCENDING"});
                if (!tag_name.empty()) {
                    statement.tertiary_expression = "TAG " + tag_name;
                }
            }
            if (find_keyword_top_level(body, "DESCENDING") != std::string::npos) {
                statement.quaternary_expression = "DESCENDING";
            } else if (find_keyword_top_level(body, "ASCENDING") != std::string::npos) {
                statement.quaternary_expression = "ASCENDING";
            }
        } else if (starts_with_insensitive(line, "CALCULATE ")) {
            statement.kind = StatementKind::calculate_command;
            const std::string body = trim_copy(line.substr(10U));
            const std::size_t for_position = find_keyword_top_level(body, "FOR");
            const std::size_t while_position = find_keyword_top_level(body, "WHILE");
            const std::size_t in_position = find_keyword_top_level(body, "IN");
            std::size_t tail_start = std::string::npos;
            if (for_position != std::string::npos) {
                tail_start = for_position;
            }
            if (while_position != std::string::npos) {
                tail_start = tail_start == std::string::npos
                    ? while_position
                    : std::min(tail_start, while_position);
            }
            if (in_position != std::string::npos) {
                tail_start = tail_start == std::string::npos
                    ? in_position
                    : std::min(tail_start, in_position);
            }
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "IN");
            statement.quaternary_expression = extract_command_clause(body, "WHILE", {"IN"});
        } else if (upper == "COUNT" || starts_with_insensitive(line, "COUNT ")) {
            statement.kind = StatementKind::count_command;
            const std::string body = upper == "COUNT" ? std::string{} : trim_copy(line.substr(6U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"FOR", "TO", "INTO", "WHILE", "IN", "NOOPTIMIZE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "FOR", {"WHILE", "TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.quaternary_expression = extract_command_clause(body, "IN", {"NOOPTIMIZE"});
            statement.identifier = extract_command_clause(body, "INTO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            if (statement.identifier.empty()) {
                statement.identifier = extract_command_clause(body, "TO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            }
        } else if (upper == "SUM" || starts_with_insensitive(line, "SUM ")) {
            statement.kind = StatementKind::sum_command;
            const std::string body = upper == "SUM" ? std::string{} : trim_copy(line.substr(4U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"FOR", "TO", "INTO", "WHILE", "IN", "NOOPTIMIZE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "FOR", {"WHILE", "TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.quaternary_expression = extract_command_clause(body, "IN", {"NOOPTIMIZE"});
            statement.identifier = extract_command_clause(body, "INTO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            if (statement.identifier.empty()) {
                statement.identifier = extract_command_clause(body, "TO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            }
        } else if (upper == "AVERAGE" || starts_with_insensitive(line, "AVERAGE ")) {
            statement.kind = StatementKind::average_command;
            const std::string body = upper == "AVERAGE" ? std::string{} : trim_copy(line.substr(8U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"FOR", "TO", "INTO", "WHILE", "IN", "NOOPTIMIZE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "FOR", {"WHILE", "TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.quaternary_expression = extract_command_clause(body, "IN", {"NOOPTIMIZE"});
            statement.identifier = extract_command_clause(body, "INTO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            if (statement.identifier.empty()) {
                statement.identifier = extract_command_clause(body, "TO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            }
        } else if (upper == "TOTAL" || starts_with_insensitive(line, "TOTAL ")) {
            statement.kind = StatementKind::total_command;
            statement.expression = upper == "TOTAL" ? std::string{} : trim_copy(line.substr(6U));
        } else if (logical_line.is_text_block && (upper == "TEXT" || starts_with_insensitive(line, "TEXT "))) {
            statement.kind = StatementKind::text_command;
            const std::string body = upper == "TEXT" ? std::string{} : trim_copy(line.substr(4U));
            statement.identifier = extract_command_clause(body, "TO", {"ADDITIVE", "NOSHOW", "TEXTMERGE", "PRETEXT"});
            statement.expression = logical_line.block_text;
            statement.secondary_expression = has_keyword(body, "ADDITIVE") ? "additive" : std::string{};
            statement.tertiary_expression = has_keyword(body, "TEXTMERGE") ? "textmerge" : std::string{};
            statement.quaternary_expression = has_keyword(body, "NOSHOW") ? "noshow" : std::string{};
        } else if (upper == "LOCATE" || starts_with_insensitive(line, "LOCATE ")) {
            statement.kind = StatementKind::locate_command;
            const std::string body = upper == "LOCATE" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (upper == "SCAN" || starts_with_insensitive(line, "SCAN ")) {
            statement.kind = StatementKind::scan_statement;
            const std::string body = upper == "SCAN" ? std::string{} : trim_copy(line.substr(5U));
            const std::size_t scope_end = find_first_keyword_top_level(body, {"FOR", "WHILE", "IN"});
            const std::string scope_text = scope_end == std::string::npos
                                               ? body
                                               : trim_copy(body.substr(0U, scope_end));
            std::string remaining_scope_text;
            const AggregateScopeClause scope = parse_aggregate_scope_clause(scope_text, remaining_scope_text);
            if (remaining_scope_text != trim_copy(scope_text)) {
                if (scope.kind == AggregateScopeKind::rest_records) {
                    statement.identifier = "rest";
                }
            }
            statement.expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (upper == "ENDSCAN") {
            statement.kind = StatementKind::endscan_statement;
        } else if (upper == "APPEND BLANK" || starts_with_insensitive(line, "APPEND BLANK ")) {
            statement.kind = StatementKind::append_blank_command;
            const std::string body = upper == "APPEND BLANK" ? std::string{} : trim_copy(line.substr(12U));
            statement.secondary_expression = trim_command_keyword(body, "IN");
        } else if (starts_with_insensitive(line, "REPLACE ")) {
            statement.kind = StatementKind::replace_command;
            const std::string body = trim_copy(line.substr(8U));
            const std::size_t tail_start =
                find_first_keyword_top_level(body, {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            const std::string assignments_and_scope =
                tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            const AggregateScopeClause scope =
                parse_aggregate_scope_clause(assignments_and_scope, statement.expression);
            if (statement.expression != trim_copy(assignments_and_scope)) {
                switch (scope.kind) {
                case AggregateScopeKind::all_records:
                    statement.identifier = "all";
                    break;
                case AggregateScopeKind::rest_records:
                    statement.identifier = "rest";
                    break;
                case AggregateScopeKind::next_records:
                    statement.identifier = "next";
                    break;
                case AggregateScopeKind::record:
                    statement.identifier = "record";
                    break;
                }
                if (!scope.raw_value.empty()) {
                    statement.names.push_back(scope.raw_value);
                }
            }
            statement.tertiary_expression =
                extract_command_clause(body, "FOR", {"WHILE", "IN", "NOOPTIMIZE"});
            statement.quaternary_expression =
                extract_command_clause(body, "WHILE", {"IN", "NOOPTIMIZE"});
            statement.secondary_expression = extract_command_clause(body, "IN", {"NOOPTIMIZE"});
        } else if (starts_with_insensitive(line, "UPDATE ")) {
            statement.kind = StatementKind::update_command;
            const std::string body = trim_copy(line.substr(7U));
            const std::size_t set_position = find_keyword_top_level(body, "SET");
            if (set_position == std::string::npos) {
                statement.identifier = body;
            } else {
                std::string target = trim_copy(body.substr(0U, set_position));
                if (starts_with_insensitive(target, "IN ")) {
                    statement.secondary_expression = trim_copy(target.substr(3U));
                    target.clear();
                }
                statement.identifier = target;
                const std::string update_tail = trim_copy(body.substr(set_position + 3U));
                const std::size_t tail_start = find_first_keyword_top_level(update_tail, {"WHERE", "FOR", "WHILE"});
                statement.expression = tail_start == std::string::npos ? update_tail : trim_copy(update_tail.substr(0U, tail_start));
                statement.tertiary_expression = extract_command_clause(update_tail, "WHERE", {"FOR", "WHILE"});
                if (statement.tertiary_expression.empty()) {
                    statement.tertiary_expression = extract_command_clause(update_tail, "FOR", {"WHERE", "WHILE"});
                }
                statement.quaternary_expression = extract_command_clause(update_tail, "WHILE", {"WHERE", "FOR"});
            }
        } else if (starts_with_insensitive(line, "DELETE FROM ")) {
            statement.kind = StatementKind::delete_from_command;
            const std::string body = trim_copy(line.substr(12U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"WHERE", "FOR", "WHILE"});
            statement.identifier = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.expression = extract_command_clause(body, "WHERE", {"FOR", "WHILE"});
            if (statement.expression.empty()) {
                statement.expression = extract_command_clause(body, "FOR", {"WHERE", "WHILE"});
            }
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"WHERE", "FOR"});
        } else if (starts_with_insensitive(line, "INSERT INTO ")) {
            statement.kind = StatementKind::insert_into_command;
            const std::string body = trim_copy(line.substr(12U));
            const std::size_t values_position = find_keyword_top_level(body, "VALUES");
            const std::size_t select_position = find_keyword_top_level(body, "SELECT");
            const bool inserts_query_rows =
                select_position != std::string::npos &&
                (values_position == std::string::npos || select_position < values_position);
            const std::size_t source_position = inserts_query_rows
                                                    ? select_position
                                                    : values_position;
            if (source_position == std::string::npos) {
                statement.identifier = body;
            } else {
                std::string target = trim_copy(body.substr(0U, source_position));
                const std::size_t open_paren = target.find('(');
                const std::size_t dynamic_target_close =
                    open_paren == 0U ? find_matching_parenthesis_in_text(target, open_paren) : std::string::npos;
                if (dynamic_target_close != std::string::npos) {
                    const std::size_t field_open = target.find('(', dynamic_target_close + 1U);
                    if (field_open != std::string::npos &&
                        find_matching_parenthesis_in_text(target, field_open) == target.size() - 1U) {
                        statement.expression = trim_copy(
                            target.substr(field_open + 1U, target.size() - field_open - 2U));
                    }
                    statement.identifier = trim_copy(target.substr(0U, dynamic_target_close + 1U));
                    target.clear();
                } else if (open_paren != std::string::npos) {
                    const std::size_t close_paren = target.rfind(')');
                    if (close_paren != std::string::npos && close_paren > open_paren) {
                        statement.expression = trim_copy(target.substr(open_paren + 1U, close_paren - open_paren - 1U));
                        target = trim_copy(target.substr(0U, open_paren));
                    }
                }
                if (statement.identifier.empty()) {
                    statement.identifier = target;
                }
                if (!inserts_query_rows) {
                    std::string values = trim_copy(body.substr(values_position + 6U));
                    if (values.size() >= 2U && values.front() == '(' && values.back() == ')') {
                        values = trim_copy(values.substr(1U, values.size() - 2U));
                    }
                    statement.secondary_expression = values;
                } else {
                    statement.secondary_expression = trim_copy(body.substr(select_position));
                    statement.tertiary_expression = "select";
                }
            }
        } else if (upper == "DELETE" || starts_with_insensitive(line, "DELETE ")) {
            statement.kind = StatementKind::delete_command;
            const std::string body = upper == "DELETE" ? std::string{} : trim_copy(line.substr(7U));
            const std::size_t clause_position = find_first_keyword_top_level(body, {"FOR", "WHILE", "IN"});
            const std::string scope_text = clause_position == std::string::npos
                                               ? body
                                               : trim_copy(body.substr(0U, clause_position));
            std::string remaining_scope_text;
            const AggregateScopeClause scope = parse_aggregate_scope_clause(scope_text, remaining_scope_text);
            if (remaining_scope_text != trim_copy(scope_text)) {
                switch (scope.kind) {
                case AggregateScopeKind::all_records:
                    statement.identifier = "all";
                    break;
                case AggregateScopeKind::rest_records:
                    statement.identifier = "rest";
                    break;
                case AggregateScopeKind::next_records:
                    statement.identifier = "next";
                    break;
                case AggregateScopeKind::record:
                    statement.identifier = "record";
                    break;
                }
                if (!scope.raw_value.empty()) {
                    statement.names.push_back(scope.raw_value);
                }
            }
            statement.expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (upper == "RECALL" || starts_with_insensitive(line, "RECALL ")) {
            statement.kind = StatementKind::recall_command;
            const std::string body = upper == "RECALL" ? std::string{} : trim_copy(line.substr(7U));
            const std::size_t clause_position = find_first_keyword_top_level(body, {"FOR", "WHILE", "IN"});
            const std::string scope_text = clause_position == std::string::npos
                                               ? body
                                               : trim_copy(body.substr(0U, clause_position));
            std::string remaining_scope_text;
            const AggregateScopeClause scope = parse_aggregate_scope_clause(scope_text, remaining_scope_text);
            if (remaining_scope_text != trim_copy(scope_text)) {
                switch (scope.kind) {
                case AggregateScopeKind::all_records:
                    statement.identifier = "all";
                    break;
                case AggregateScopeKind::rest_records:
                    statement.identifier = "rest";
                    break;
                case AggregateScopeKind::next_records:
                    statement.identifier = "next";
                    break;
                case AggregateScopeKind::record:
                    statement.identifier = "record";
                    break;
                }
                if (!scope.raw_value.empty()) {
                    statement.names.push_back(scope.raw_value);
                }
            }
            statement.expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (upper == "PACK" || starts_with_insensitive(line, "PACK ")) {
            statement.kind = StatementKind::pack_command;
            const std::string body = upper == "PACK" ? std::string{} : trim_copy(line.substr(5U));
            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "IN", {"MEMO", "DBF"});
            if (statement.secondary_expression.empty() && starts_with_insensitive(body, "IN ")) {
                statement.secondary_expression = trim_copy(body.substr(3U));
            }
        } else if (upper == "ZAP" || starts_with_insensitive(line, "ZAP ")) {
            statement.kind = StatementKind::zap_command;
            const std::string body = upper == "ZAP" ? std::string{} : trim_copy(line.substr(4U));
            statement.secondary_expression = extract_command_clause(body, "IN");
            if (statement.secondary_expression.empty() && starts_with_insensitive(body, "IN ")) {
                statement.secondary_expression = trim_copy(body.substr(3U));
            }
        } else if (upper == "UNLOCK" || starts_with_insensitive(line, "UNLOCK ")) {
            statement.kind = StatementKind::unlock_command;
            const std::string body = upper == "UNLOCK" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = body;
            statement.identifier = extract_command_clause(body, "RECORD", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
            if (statement.secondary_expression.empty() && starts_with_insensitive(body, "IN ")) {
                statement.secondary_expression = trim_copy(body.substr(3U));
            }
        } else if (starts_with_insensitive(line, "GOTO ") || starts_with_insensitive(line, "GO ")) {
            statement.kind = StatementKind::go_command;
            const std::string body = starts_with_insensitive(line, "GOTO ")
                ? trim_copy(line.substr(5U))
                : trim_copy(line.substr(3U));
            statement.expression = take_first_token(body);
            statement.secondary_expression = take_keyword_value(body, "IN");
        } else if (upper == "SKIP" || starts_with_insensitive(line, "SKIP ")) {
            statement.kind = StatementKind::skip_command;
            const std::string body = upper == "SKIP" ? std::string{} : trim_copy(line.substr(5U));
            if (starts_with_insensitive(body, "IN ")) {
                statement.expression = "1";
                statement.secondary_expression = trim_copy(body.substr(3U));
            } else {
                statement.expression = body.empty() ? "1" : take_first_token(body);
                statement.secondary_expression = take_keyword_value(body, "IN");
            }
        } else if (upper == "BROWSE" || starts_with_insensitive(line, "BROWSE ")) {
            statement.kind = StatementKind::browse_command;
            const std::string body = upper == "BROWSE" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "IN", {"FIELDS", "FOR", "WHILE", "NOWAIT"});
            statement.tertiary_expression = extract_fields_command_clause(body, {"FOR", "WHILE", "IN", "NOWAIT"});
            statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE", "IN", "FIELDS", "NOWAIT"});
        } else if (upper == "EDIT" || starts_with_insensitive(line, "EDIT ")) {
            statement.kind = StatementKind::edit_command;
            const std::string body = upper == "EDIT" ? std::string{} : trim_copy(line.substr(5U));
            statement.expression = extract_command_clause(body, "MEMO", {});
        } else if (upper == "CHANGE" || starts_with_insensitive(line, "CHANGE ")) {
            statement.kind = StatementKind::change_command;
            const std::string body = upper == "CHANGE" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = extract_command_clause(body, "FIELD", {});
        } else if (starts_with_insensitive(line, "INPUT ") || upper == "INPUT") {
            statement.kind = StatementKind::input_command;
            const std::string body = upper == "INPUT" ? std::string{} : trim_copy(line.substr(6U));
            // Body: ["<prompt>"] TO <var>
            const std::size_t to_pos = find_keyword_top_level(body, "TO");
            if (to_pos != std::string::npos) {
                const std::string prompt_part = trim_copy(body.substr(0U, to_pos));
                const std::string var_part = trim_copy(body.substr(to_pos + 2U));
                statement.expression = prompt_part;
                statement.identifier = var_part;
            } else {
                statement.identifier = trim_copy(body);
            }
        } else if (starts_with_insensitive(line, "ACCEPT ") || upper == "ACCEPT") {
            statement.kind = StatementKind::accept_command;
            const std::string body = upper == "ACCEPT" ? std::string{} : trim_copy(line.substr(7U));
            // Body: ["<prompt>"] TO <var>
            const std::size_t to_pos = find_keyword_top_level(body, "TO");
            if (to_pos != std::string::npos) {
                const std::string prompt_part = trim_copy(body.substr(0U, to_pos));
                const std::string var_part = trim_copy(body.substr(to_pos + 2U));
                statement.expression = prompt_part;
                statement.identifier = var_part;
            } else {
                statement.identifier = trim_copy(body);
            }
        } else if (upper == "GETFILE" || starts_with_insensitive(line, "GETFILE ") || starts_with_insensitive(line, "GETFILE(")) {
            statement.kind = StatementKind::getfile_command;
            std::string body = parse_dialog_command_body(line, "GETFILE");
            const std::string target_part = extract_dialog_target_clause(body);
            body = strip_dialog_argument_wrapping(std::move(body));

            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "PROMPT", {"TITLE", "DEFAULT", "FILTER"});
            statement.tertiary_expression = extract_command_clause(body, "TITLE", {"PROMPT", "DEFAULT", "FILTER"});
            statement.quaternary_expression = extract_command_clause(body, "DEFAULT", {"PROMPT", "TITLE", "FILTER"});
            statement.identifier = extract_command_clause(body, "FILTER", {"PROMPT", "TITLE", "DEFAULT"});

            const bool has_named_clause = find_first_keyword_top_level(body, {"PROMPT", "TITLE", "DEFAULT", "FILTER"}) != std::string::npos;
            if (!has_named_clause) {
                const std::vector<std::string> arguments = split_csv_like(body);
                assign_dialog_positional_if_empty(statement.identifier, arguments, 0U);
                assign_dialog_positional_if_empty(statement.secondary_expression, arguments, 1U);
                assign_dialog_positional_if_empty(statement.tertiary_expression, arguments, 2U);
                assign_dialog_positional_if_empty(statement.quaternary_expression, arguments, 3U);
            }

            if (!target_part.empty()) {
                statement.names.push_back(target_part);
            }
        } else if (upper == "PUTFILE" || starts_with_insensitive(line, "PUTFILE ") || starts_with_insensitive(line, "PUTFILE(")) {
            statement.kind = StatementKind::putfile_command;
            std::string body = parse_dialog_command_body(line, "PUTFILE");
            const std::string target_part = extract_dialog_target_clause(body);
            body = strip_dialog_argument_wrapping(std::move(body));

            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "PROMPT", {"TITLE", "DEFAULT", "FILTER"});
            statement.tertiary_expression = extract_command_clause(body, "TITLE", {"PROMPT", "DEFAULT", "FILTER"});
            statement.quaternary_expression = extract_command_clause(body, "DEFAULT", {"PROMPT", "TITLE", "FILTER"});
            statement.identifier = extract_command_clause(body, "FILTER", {"PROMPT", "TITLE", "DEFAULT"});

            const bool has_named_clause = find_first_keyword_top_level(body, {"PROMPT", "TITLE", "DEFAULT", "FILTER"}) != std::string::npos;
            if (!has_named_clause) {
                const std::vector<std::string> arguments = split_csv_like(body);
                assign_dialog_positional_if_empty(statement.identifier, arguments, 0U);
                assign_dialog_positional_if_empty(statement.secondary_expression, arguments, 1U);
                assign_dialog_positional_if_empty(statement.tertiary_expression, arguments, 2U);
                assign_dialog_positional_if_empty(statement.quaternary_expression, arguments, 3U);
            }

            if (!target_part.empty()) {
                statement.names.push_back(target_part);
            }
        } else if (upper == "GETDIR" || starts_with_insensitive(line, "GETDIR ") || starts_with_insensitive(line, "GETDIR(")) {
            statement.kind = StatementKind::getdir_command;
            std::string body = parse_dialog_command_body(line, "GETDIR");
            const std::string target_part = extract_dialog_target_clause(body);
            body = strip_dialog_argument_wrapping(std::move(body));

            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "PROMPT", {"TITLE", "DEFAULT"});
            statement.tertiary_expression = extract_command_clause(body, "TITLE", {"PROMPT", "DEFAULT"});
            statement.quaternary_expression = extract_command_clause(body, "DEFAULT", {"PROMPT", "TITLE"});

            const bool has_named_clause = find_first_keyword_top_level(body, {"PROMPT", "TITLE", "DEFAULT"}) != std::string::npos;
            if (!has_named_clause) {
                const std::vector<std::string> arguments = split_csv_like(body);
                assign_dialog_positional_if_empty(statement.quaternary_expression, arguments, 0U);
                assign_dialog_positional_if_empty(statement.secondary_expression, arguments, 1U);
                assign_dialog_positional_if_empty(statement.tertiary_expression, arguments, 2U);
            }

            if (!target_part.empty()) {
                statement.names.push_back(target_part);
            }
        } else if (upper == "INPUTBOX" || starts_with_insensitive(line, "INPUTBOX ") || starts_with_insensitive(line, "INPUTBOX(")) {
            statement.kind = StatementKind::inputbox_command;
            std::string body = parse_dialog_command_body(line, "INPUTBOX");
            const std::string target_part = extract_dialog_target_clause(body);
            body = strip_dialog_argument_wrapping(std::move(body));

            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "PROMPT", {"TITLE", "DEFAULT"});
            statement.tertiary_expression = extract_command_clause(body, "TITLE", {"PROMPT", "DEFAULT"});
            statement.quaternary_expression = extract_command_clause(body, "DEFAULT", {"PROMPT", "TITLE"});

            const bool has_named_clause = find_first_keyword_top_level(body, {"PROMPT", "TITLE", "DEFAULT"}) != std::string::npos;
            if (!has_named_clause) {
                const std::vector<std::string> arguments = split_csv_like(body);
                assign_dialog_positional_if_empty(statement.secondary_expression, arguments, 0U);
                assign_dialog_positional_if_empty(statement.tertiary_expression, arguments, 1U);
                assign_dialog_positional_if_empty(statement.quaternary_expression, arguments, 2U);
            }

            if (!target_part.empty()) {
                statement.names.push_back(target_part);
            }
        } else if (upper == "WAIT" || starts_with_insensitive(line, "WAIT ")) {
            statement.kind = StatementKind::wait_command;
            const std::string body = upper == "WAIT" ? std::string{} : trim_copy(line.substr(5U));
            const std::string body_upper = uppercase_copy(body);
            if (body_upper == "CLEAR" || starts_with_insensitive(body, "CLEAR")) {
                statement.identifier = "CLEAR";
            } else {
                std::string wait_body = body;
                if (starts_with_insensitive(body, "WINDOW ") || body_upper == "WINDOW") {
                    statement.identifier = "WINDOW";
                    wait_body = body_upper == "WINDOW" ? std::string{} : trim_copy(body.substr(7U));
                }

                const std::size_t clause_pos = find_first_keyword_top_level(wait_body, {"TO", "TIMEOUT", "NOWAIT", "NOCLEAR"});
                if (clause_pos != std::string::npos) {
                    statement.expression = trim_copy(wait_body.substr(0U, clause_pos));
                } else {
                    statement.expression = trim_copy(wait_body);
                }

                statement.secondary_expression = extract_command_clause(wait_body, "TIMEOUT", {"TO", "NOWAIT", "NOCLEAR"});
                statement.tertiary_expression = find_first_keyword_top_level(wait_body, {"NOWAIT"}) != std::string::npos
                    ? "NOWAIT"
                    : std::string{};
                statement.quaternary_expression = find_first_keyword_top_level(wait_body, {"NOCLEAR"}) != std::string::npos
                    ? "NOCLEAR"
                    : std::string{};

                const std::string target = extract_command_clause(wait_body, "TO", {"TIMEOUT", "NOWAIT", "NOCLEAR"});
                if (!target.empty()) {
                    statement.names.push_back(target);
                }
            }
        } else if (starts_with_insensitive(line, "SLEEP") || upper == "SLEEP") {
            statement.kind = StatementKind::sleep_command;
            std::string body = upper == "SLEEP" ? std::string{} : trim_copy(line.substr(5U));
            if (!body.empty() && body.front() == '(' && body.back() == ')' && body.size() >= 2U) {
                body = trim_copy(body.substr(1U, body.size() - 2U));
            }
            statement.expression = body;
        } else if (starts_with_insensitive(line, "KEYBOARD ") || upper == "KEYBOARD") {
            statement.kind = StatementKind::keyboard_command;
            const std::string body = upper == "KEYBOARD" ? std::string{} : trim_copy(line.substr(9U));
            const std::size_t clause_pos = find_first_keyword_top_level(body, {"PLAIN", "CLEAR"});
            const std::string keys_part = clause_pos == std::string::npos ? body : trim_copy(body.substr(0U, clause_pos));
            statement.expression = keys_part;
            statement.secondary_expression = find_first_keyword_top_level(body, {"PLAIN"}) != std::string::npos
                ? "PLAIN"
                : std::string{};
            statement.tertiary_expression = find_first_keyword_top_level(body, {"CLEAR"}) != std::string::npos
                ? "CLEAR"
                : std::string{};
        } else if (starts_with_insensitive(line, "DISPLAY ") || upper == "DISPLAY") {
            statement.kind = StatementKind::display_command;
            const std::string body = upper == "DISPLAY" ? std::string{} : trim_copy(line.substr(8U));
            if (starts_with_insensitive(body, "STRUCTURE")) {
                statement.identifier = "STRUCTURE";
                statement.expression = body.size() > 9U ? trim_copy(body.substr(9U)) : std::string{};
                statement.secondary_expression = extract_command_clause(statement.expression, "IN");
            } else if (starts_with_insensitive(body, "STATUS")) {
                statement.identifier = "STATUS";
                statement.expression = body.size() > 6U ? trim_copy(body.substr(6U)) : std::string{};
            } else if (starts_with_insensitive(body, "MEMORY")) {
                statement.identifier = "MEMORY";
                statement.expression = body.size() > 6U ? trim_copy(body.substr(6U)) : std::string{};
            } else {
                statement.identifier = "RECORDS";
                statement.expression = body;
                statement.secondary_expression = extract_command_clause(body, "IN", {"FIELDS", "FOR", "WHILE"});
                statement.tertiary_expression = extract_fields_command_clause(body, {"FOR", "WHILE", "IN"});
                statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE", "IN", "FIELDS"});
                const std::string while_clause = extract_command_clause(body, "WHILE", {"FOR", "IN", "FIELDS"});
                if (!while_clause.empty()) {
                    statement.names.push_back(while_clause);
                }
            }
        } else if (upper == "LIST" || starts_with_insensitive(line, "LIST ")) {
            statement.kind = StatementKind::list_command;
            const std::string body = upper == "LIST" ? std::string{} : trim_copy(line.substr(5U));
            if (starts_with_insensitive(body, "STRUCTURE")) {
                statement.identifier = "STRUCTURE";
                statement.expression = body.size() > 9U ? trim_copy(body.substr(9U)) : std::string{};
                statement.secondary_expression = extract_command_clause(statement.expression, "IN");
            } else if (starts_with_insensitive(body, "STATUS")) {
                statement.identifier = "STATUS";
                statement.expression = body.size() > 6U ? trim_copy(body.substr(6U)) : std::string{};
            } else if (starts_with_insensitive(body, "MEMORY")) {
                statement.identifier = "MEMORY";
                statement.expression = body.size() > 6U ? trim_copy(body.substr(6U)) : std::string{};
            } else {
                statement.identifier = "RECORDS";
                statement.expression = body;
                statement.secondary_expression = extract_command_clause(body, "IN", {"FIELDS", "FOR", "WHILE"});
                statement.tertiary_expression = extract_fields_command_clause(body, {"FOR", "WHILE", "IN"});
                statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE", "IN", "FIELDS"});
                const std::string while_clause = extract_command_clause(body, "WHILE", {"FOR", "IN", "FIELDS"});
                if (!while_clause.empty()) {
                    statement.names.push_back(while_clause);
                }
            }
        } else if (starts_with_insensitive(line, "SELECT ")) {
            statement.kind = StatementKind::select_command;
            statement.expression = trim_copy(line.substr(7U));
        } else if (upper == "USE" || starts_with_insensitive(line, "USE ")) {
            statement.kind = StatementKind::use_command;
            const std::string body = upper == "USE" ? std::string{} : trim_copy(line.substr(4U));
            if (starts_with_insensitive(body, "IN ")) {
                statement.secondary_expression = trim_copy(body.substr(3U));
            } else if (!body.empty()) {
                statement.expression = take_first_token(body);
                statement.identifier = take_keyword_value(body, "ALIAS");
                statement.secondary_expression = take_keyword_value(body, "IN");
                statement.tertiary_expression = has_keyword(body, "AGAIN") ? "again" : std::string{};
                if (has_keyword(body, "EXCLUSIVE")) {
                    statement.quaternary_expression = "exclusive";
                } else if (has_keyword(body, "SHARED")) {
                    statement.quaternary_expression = "shared";
                }
            }
        } else if (starts_with_insensitive(line, "OPEN DATABASE")) {
            statement.kind = StatementKind::open_database;
            const std::string body = trim_copy(line.substr(13U));
            const std::size_t tail_start = find_first_keyword_top_level(
                body,
                {"EXCLUSIVE", "SHARED", "NOUPDATE", "VALIDATE"});
            statement.expression =
                tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            if (has_keyword(body, "EXCLUSIVE")) {
                statement.secondary_expression = "exclusive";
            } else if (has_keyword(body, "SHARED")) {
                statement.secondary_expression = "shared";
            }
            statement.tertiary_expression =
                has_keyword(body, "NOUPDATE") ? "noupdate" : std::string{};
            statement.quaternary_expression =
                has_keyword(body, "VALIDATE") ? "validate" : std::string{};
        } else if (starts_with_insensitive(line, "SET DATASESSION TO ")) {
            statement.kind = StatementKind::set_datasession;
            statement.expression = trim_copy(line.substr(19U));
        } else if (upper == "SET PROCEDURE TO" || starts_with_insensitive(line, "SET PROCEDURE TO ")) {
            statement.kind = StatementKind::set_procedure;
            const std::string body = upper == "SET PROCEDURE TO" ? std::string{} : trim_copy(line.substr(16U));
            const std::size_t additive_position = find_keyword_top_level(body, "ADDITIVE");
            statement.expression = additive_position == std::string::npos ? body : trim_copy(body.substr(0U, additive_position));
            statement.secondary_expression = has_keyword(body, "ADDITIVE") ? "additive" : std::string{};
        } else if (starts_with_insensitive(line, "SET LIBRARY TO ")) {
            statement.kind = StatementKind::set_library;
            statement.expression = trim_copy(line.substr(15U));
        } else if (starts_with_insensitive(line, "SET ORDER TO ")) {
            statement.kind = StatementKind::set_order;
            const std::string body = trim_copy(line.substr(13U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"IN", "ASCENDING", "DESCENDING"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "IN", {"ASCENDING", "DESCENDING"});
            if (find_keyword_top_level(body, "DESCENDING") != std::string::npos) {
                statement.tertiary_expression = "DESCENDING";
            } else if (find_keyword_top_level(body, "ASCENDING") != std::string::npos) {
                statement.tertiary_expression = "ASCENDING";
            }
        } else if (starts_with_insensitive(line, "SET DEFAULT TO ")) {
            statement.kind = StatementKind::set_default;
            statement.expression = trim_copy(line.substr(15U));
        } else if (starts_with_insensitive(line, "SET MEMOWIDTH TO ")) {
            statement.kind = StatementKind::set_memowidth;
            statement.expression = trim_copy(line.substr(16U));
        } else if (starts_with_insensitive(line, "SET ")) {
            statement.kind = StatementKind::set_command;
            statement.expression = trim_copy(line.substr(4U));
        } else if (upper == "ON KEY" || starts_with_insensitive(line, "ON KEY ")) {
            statement.kind = StatementKind::on_key_command;
            std::string body = upper == "ON KEY" ? std::string{} : trim_copy(line.substr(6U));
            if (starts_with_insensitive(body, "LABEL ")) {
                body = trim_copy(body.substr(6U));
            }
            if (!body.empty()) {
                statement.identifier = uppercase_copy(take_first_token(body));
                statement.expression = trim_copy(body.substr(statement.identifier.size()));
            }
        } else if (upper == "ON ESCAPE" || starts_with_insensitive(line, "ON ESCAPE ")) {
            statement.kind = StatementKind::on_escape;
            statement.expression = upper == "ON ESCAPE" ? std::string{} : trim_copy(line.substr(10U));
        } else if (upper == "ON PAGE" || starts_with_insensitive(line, "ON PAGE ")) {
            statement.kind = StatementKind::on_page;
            const std::string body = upper == "ON PAGE" ? std::string{} : trim_copy(line.substr(7U));
            if (starts_with_insensitive(body, "AT LINE ")) {
                const std::string assignment = trim_copy(body.substr(8U));
                if (!assignment.empty() && assignment.front() == '(') {
                    const std::size_t close = find_matching_parenthesis_in_text(assignment, 0U);
                    if (close != std::string::npos) {
                        statement.secondary_expression = trim_copy(assignment.substr(0U, close + 1U));
                    }
                }
                if (statement.secondary_expression.empty()) {
                    statement.secondary_expression = take_first_token(assignment);
                }
                statement.expression = trim_copy(assignment.substr(statement.secondary_expression.size()));
            }
        } else if (upper == "ON ERROR" || starts_with_insensitive(line, "ON ERROR ")) {
            statement.kind = StatementKind::on_error;
            statement.expression = upper == "ON ERROR" ? std::string{} : trim_copy(line.substr(9U));
        } else if (starts_with_insensitive(line, "ON SHUTDOWN ")) {
            statement.kind = StatementKind::on_shutdown;
            statement.expression = trim_copy(line.substr(12U));
        } else if (starts_with_insensitive(line, "PUBLIC ")) {
            statement.kind = StatementKind::public_declaration;
            const std::string body = trim_copy(line.substr(7U));
            if (starts_with_insensitive(body, "ARRAY ") &&
                looks_like_array_declaration_body(body.substr(6U))) {
                statement.identifier = "array";
                statement.names = split_csv_like(body.substr(6U));
            } else {
                statement.names = split_csv_like(body);
            }
        } else if (starts_with_insensitive(line, "LOCAL ")) {
            statement.kind = StatementKind::local_declaration;
            const std::string body = trim_copy(line.substr(6U));
            if (starts_with_insensitive(body, "ARRAY ") &&
                looks_like_array_declaration_body(body.substr(6U))) {
                statement.identifier = "array";
                statement.names = split_csv_like(body.substr(6U));
            } else {
                statement.names = split_csv_like(body);
                for (std::string &name : statement.names) {
                    name = local_declaration_name(name);
                }
            }
        } else if (upper == "PRIVATE ALL" || starts_with_insensitive(line, "PRIVATE ALL ")) {
            // PRIVATE ALL [LIKE <pattern> | EXCEPT <pattern>]
            statement.kind = StatementKind::private_declaration;
            statement.identifier = "all";
            const std::string rest = trim_copy(line.substr(11U));
            if (starts_with_insensitive(rest, "LIKE ")) {
                statement.expression = "like";
                statement.secondary_expression = trim_copy(rest.substr(5U));
            } else if (starts_with_insensitive(rest, "EXCEPT ")) {
                statement.expression = "except";
                statement.secondary_expression = trim_copy(rest.substr(7U));
            }
        } else if (starts_with_insensitive(line, "PRIVATE ")) {
            statement.kind = StatementKind::private_declaration;
            const std::string body = trim_copy(line.substr(8U));
            if (starts_with_insensitive(body, "ARRAY ") &&
                looks_like_array_declaration_body(body.substr(6U))) {
                statement.identifier = "array";
                statement.names = split_csv_like(body.substr(6U));
            } else {
                statement.names = split_csv_like(body);
            }
        } else if (starts_with_insensitive(line, "DIMENSION ")) {
            statement.kind = StatementKind::dimension_command;
            statement.names = split_csv_like(line.substr(10U));
        } else if (starts_with_insensitive(line, "DECLARE ") &&
                   looks_like_array_declaration_body(line.substr(8U))) {
            statement.kind = StatementKind::dimension_command;
            statement.names = split_csv_like(line.substr(8U));
        } else if (starts_with_insensitive(line, "STORE ")) {
            statement.kind = StatementKind::store_command;
            const std::string body = trim_copy(line.substr(6U));
            const std::size_t to_position = find_keyword_top_level(body, "TO");
            if (to_position != std::string::npos) {
                statement.expression = trim_copy(body.substr(0U, to_position));
                statement.names = split_csv_like(trim_copy(body.substr(to_position + 2U)));
            }
        } else if (upper == "RETURN" || starts_with_insensitive(line, "RETURN ")) {
            statement.kind = StatementKind::return_statement;
            if (starts_with_insensitive(line, "RETURN ")) {
                statement.expression = trim_copy(line.substr(6U));
            }
        } else if (upper == "NODEFAULT") {
            statement.kind = StatementKind::nodefault_statement;
        } else if (upper == "CLOSE ALL" || upper == "CLOSE TABLES"
            || upper == "CLOSE DATABASES" || upper == "CLOSE DATABASE"
            || starts_with_insensitive(line, "CLOSE ALL")
            || starts_with_insensitive(line, "CLOSE TABLES")
            || starts_with_insensitive(line, "CLOSE DATABASES")) {
            statement.kind = StatementKind::close_command;
            // Store just the scope keyword (ALL, TABLES, DATABASES) as detail
            const std::size_t space_pos = upper.find(' ');
            statement.expression = space_pos != std::string::npos ? trim_copy(upper.substr(space_pos + 1U)) : "ALL";
        } else if (parse_storage_statement(line, statement)) {
        } else if (upper == "RETRY") {
            statement.kind = StatementKind::retry_statement;
        } else if (upper == "RESUME" || starts_with_insensitive(line, "RESUME ")) {
            statement.kind = StatementKind::resume_statement;
            if (starts_with_insensitive(line, "RESUME ")) {
                statement.expression = trim_copy(line.substr(7U));
            }
        }

        if (statement.kind == StatementKind::no_op) {
            parse_default_statement(line, statement);
        }

        if (current_class != nullptr && current == &program.main) {
            current_class->property_statements.push_back(std::move(statement));
        } else {
            current->statements.push_back(std::move(statement));
        }
    }

    finalize_open_routine(program.source_lines.size() + 1U);

    return program;
}

Program parse_program(const std::string& path) {
    return parse_program_impl(path, nullptr, nullptr, false);
}

Program parse_program_source(
    const std::string& logical_path,
    const std::string& source_text,
    const std::map<std::string, std::string>& source_text_overrides,
    const bool require_source_text_overrides) {
    return parse_program_impl(
        logical_path,
        &source_text,
        &source_text_overrides,
        require_source_text_overrides);
}

}  // namespace copperfin::runtime
