#pragma once

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"

#include <optional>
#include <string>
#include <utility>

namespace copperfin::runtime {

std::string trim_copy(std::string value);
std::string lowercase_copy(std::string value);
bool starts_with_insensitive(const std::string& value, const std::string& prefix);
std::string normalize_identifier(std::string value);
std::string normalize_memory_variable_identifier(std::string value);
std::string normalize_path(const std::string& value);
bool is_index_file_path(const std::string& value);
std::string unquote_string(std::string value);
std::string take_first_token(std::string value);
std::pair<std::string, std::string> split_first_word(std::string value);
std::string take_keyword_value(const std::string& text, const std::string& keyword);
std::string uppercase_copy(std::string value);
bool is_bare_identifier_text(const std::string& value);
std::string collapse_identifier(const std::string& value);
std::string unquote_identifier(std::string value);
std::string normalize_index_value(std::string value);
std::optional<double> try_parse_numeric_index_value(const std::string& value);
int compare_index_keys(
    const std::string& left,
    const std::string& right,
    const std::string& key_domain_hint);
std::optional<std::string> record_field_value(const vfp::DbfRecord& record, const std::string& field_name);
std::string evaluate_index_expression(const std::string& expression, const vfp::DbfRecord& record);
bool has_keyword(const std::string& text, const std::string& keyword);
bool parse_object_handle_reference(const PrgValue& value, int& handle, std::string& prog_id);
PrgValue make_empty_value();
PrgValue make_boolean_value(bool value);
PrgValue make_number_value(double value);
PrgValue make_string_value(std::string value);
PrgValue make_int64_value(std::int64_t value);
PrgValue make_uint64_value(std::uint64_t value);
bool value_as_bool(const PrgValue& value);
double value_as_number(const PrgValue& value);
std::string value_as_string(const PrgValue& value);

}  // namespace copperfin::runtime
