// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <optional>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_date_time_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::function<std::string(const std::string&)>& set_callback);

std::string format_runtime_date_for_set(
    int year,
    int month,
    int day,
    const std::function<std::string(const std::string&)>& set_callback);

std::string format_runtime_datetime_for_set(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second,
    const std::function<std::string(const std::string&)>& set_callback);

// A braced literal as installed VFP9 reads it (probe retained at
// ~/temp/vfp9-probes/datetime-types-c24/result-literals.txt, #5920). The text includes the braces.
//   strict   {^yyyy-mm-dd[,][h:m[:s[.f]] [a|p[m]]]} (separators - / .): a Date, or a DateTime with a time part
//   empty    {}  {  /  /  }  {//} (empty Date)   { : }  {/ /: } (empty DateTime)
//   not_strict  any other braced text: left to the caller (it stays a Character value, as before)
//   ambiguous   a ^ literal that does not match the grammar (VFP error 2032)
//   invalid     a ^ literal whose month, day or time is out of range (VFP error 2034)
enum class BracedLiteralKind { not_strict, date, datetime, empty_date, empty_datetime, ambiguous, invalid };

struct BracedLiteral {
    BracedLiteralKind kind = BracedLiteralKind::not_strict;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
};

BracedLiteral parse_braced_date_time_literal(const std::string& literal);

std::optional<PrgValue> evaluate_date_time_additive(
    const PrgValue& left,
    const PrgValue& right,
    bool subtract,
    const std::function<std::string(const std::string&)>& set_callback);

std::optional<int> compare_date_time_values(
    const PrgValue& left,
    const PrgValue& right,
    const std::function<std::string(const std::string&)>& set_callback);

}  // namespace copperfin::runtime
