// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_table_structure_helpers.h"

#include "prg_engine_command_helpers.h"
#include "prg_engine_helpers.h"

#include <charconv>
#include <cstdint>
#include <limits>
#include <sstream>
#include <system_error>

namespace copperfin::runtime {

namespace {

std::string take_type_token(std::istringstream& stream) {
    std::string type_text;
    stream >> type_text;
    if (type_text.empty()) {
        return {};
    }

    if (type_text.find('(') != std::string::npos && type_text.find(')') == std::string::npos) {
        std::string next;
        while (stream >> next) {
            type_text += next;
            if (next.find(')') != std::string::npos) {
                break;
            }
        }
    }
    return type_text;
}

std::optional<std::uint8_t> parse_field_dimension(std::string text) {
    text = trim_copy(std::move(text));
    if (text.empty()) {
        return std::nullopt;
    }

    unsigned int value = 0U;
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const auto result = std::from_chars(begin, end, value, 10);
    if (result.ec != std::errc{} || result.ptr != end ||
        value > std::numeric_limits<std::uint8_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(value);
}

}  // namespace

std::optional<TableFieldDeclaration> parse_table_field_declaration(std::string text) {
    text = trim_copy(std::move(text));
    if (text.empty()) {
        return std::nullopt;
    }

    std::istringstream stream(text);
    std::string name;
    stream >> name;
    std::string type_text = take_type_token(stream);
    if (name.empty() || type_text.empty()) {
        return std::nullopt;
    }

    std::string tail;
    std::getline(stream, tail);
    tail = trim_copy(std::move(tail));

    name = unquote_identifier(name);
    type_text = uppercase_copy(trim_copy(type_text));
    std::uint8_t length = 0U;
    std::uint8_t decimals = 0U;

    const std::size_t open_paren = type_text.find('(');
    std::string type_name = open_paren == std::string::npos ? type_text : type_text.substr(0U, open_paren);
    if (open_paren != std::string::npos) {
        if (type_text.back() != ')') {
            return std::nullopt;
        }
        const std::string inside = type_text.substr(open_paren + 1U, type_text.size() - open_paren - 2U);
        const std::string trimmed_inside = trim_copy(inside);
        const std::vector<std::string> parts = split_csv_like(inside);
        if (trimmed_inside.empty() || parts.empty() || parts.size() > 2U || trimmed_inside.back() == ',') {
            return std::nullopt;
        }

        const auto parsed_length = parse_field_dimension(parts[0]);
        if (!parsed_length.has_value()) {
            return std::nullopt;
        }
        length = *parsed_length;

        if (parts.size() == 2U) {
            const auto parsed_decimals = parse_field_dimension(parts[1]);
            if (!parsed_decimals.has_value()) {
                return std::nullopt;
            }
            decimals = *parsed_decimals;
        }
    }

    char type = '\0';
    if (type_name == "C" || type_name == "CHAR" || type_name == "CHARACTER") {
        type = 'C';
        if (length == 0U) {
            length = 10U;
        }
    } else if (type_name == "V" || type_name == "VARCHAR") {
        type = 'V';
        if (length == 0U) {
            length = 10U;
        }
    } else if (type_name == "N" || type_name == "NUMERIC") {
        type = 'N';
        if (length == 0U) {
            length = 10U;
        }
    } else if (type_name == "F" || type_name == "FLOAT") {
        type = 'F';
        if (length == 0U) {
            length = 10U;
        }
    } else if (type_name == "B" || type_name == "DOUBLE") {
        type = 'B';
        length = 8U;
    } else if (type_name == "Q" || type_name == "VARBINARY") {
        type = 'Q';
        if (length == 0U) {
            length = 10U;
        }
    } else if (type_name == "L" || type_name == "LOGICAL") {
        type = 'L';
        length = 1U;
    } else if (type_name == "D" || type_name == "DATE") {
        type = 'D';
        length = 8U;
    } else if (type_name == "I" || type_name == "INTEGER") {
        type = 'I';
        length = 4U;
    } else if (type_name == "Y" || type_name == "CURRENCY") {
        type = 'Y';
        length = 8U;
        decimals = 4U;
    } else if (type_name == "T" || type_name == "DATETIME") {
        type = 'T';
        length = 8U;
    } else if (type_name == "M" || type_name == "MEMO") {
        type = 'M';
        length = 4U;
    }

    if (type == '\0' || name.empty()) {
        return std::nullopt;
    }

    TableFieldDeclaration declaration{
        .descriptor = vfp::DbfFieldDescriptor{
            .name = name,
            .type = type,
            .offset = 0U,
            .length = length,
            .decimal_count = decimals
        }
    };

    if (!tail.empty()) {
        const std::size_t default_pos = find_keyword_top_level(tail, "DEFAULT");
        if (default_pos != std::string::npos) {
            std::string default_tail = trim_copy(tail.substr(default_pos + 7U));
            const std::size_t stop_pos = find_first_keyword_top_level(default_tail, {"NULL", "NOT"});
            declaration.default_expression = stop_pos == std::string::npos
                ? default_tail
                : trim_copy(default_tail.substr(0U, stop_pos));
            declaration.has_default = !declaration.default_expression.empty();
        }
        if (find_keyword_top_level(tail, "NOT NULL") != std::string::npos) {
            declaration.nullable = false;
        } else if (find_keyword_top_level(tail, "NULL") != std::string::npos) {
            declaration.nullable = true;
        }
    }

    return declaration;
}

std::vector<TableFieldDeclaration> parse_table_field_declarations(const std::string& field_list) {
    std::vector<TableFieldDeclaration> declarations;
    for (const std::string& part : split_csv_like(field_list)) {
        auto declaration = parse_table_field_declaration(part);
        if (!declaration.has_value()) {
            return {};
        }
        declarations.push_back(std::move(*declaration));
    }
    return declarations;
}

std::vector<vfp::DbfFieldDescriptor> table_field_descriptors(const std::vector<TableFieldDeclaration>& declarations) {
    std::vector<vfp::DbfFieldDescriptor> fields;
    fields.reserve(declarations.size());
    for (const auto& declaration : declarations) {
        fields.push_back(declaration.descriptor);
    }
    return fields;
}

}  // namespace copperfin::runtime
