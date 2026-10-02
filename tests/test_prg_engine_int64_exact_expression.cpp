// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_compatibility_error.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

namespace {

using namespace copperfin::test_support;
using copperfin::runtime::make_int64_value;
using copperfin::runtime::make_number_value;
using copperfin::runtime::make_uint64_value;
using copperfin::runtime::PrgCompatibilityError;
using copperfin::runtime::PrgValue;
using copperfin::runtime::PrgValueKind;

// Governing requirement: RQ-CF-PRG-INT64-EXACT-EXPRESSION-001 (#6035).
//
// 64-bit integer values (the exact kinds a DECLAREd external function returns) are added, subtracted, multiplied,
// divided, negated and ordered without a round trip through double, which loses the low bits above 2^53 and makes the
// out-of-range casts undefined behavior. VFP9 has no 64-bit integer type, so these rows are not VFP9 rows: every
// expected value below is computed with Python big integers independently of Copperfin.

constexpr std::int64_t kInt64Max = std::numeric_limits<std::int64_t>::max();
constexpr std::int64_t kInt64Min = std::numeric_limits<std::int64_t>::min();
constexpr std::uint64_t kUint64Max = std::numeric_limits<std::uint64_t>::max();
constexpr std::int64_t kBeyondDouble = 9007199254740993LL;  // 2^53 + 1, not a double

std::string describe(const std::optional<PrgValue> &value) {
    if (!value.has_value()) {
        return "nullopt";
    }
    switch (value->kind) {
    case PrgValueKind::int64:
        return "int64:" + std::to_string(value->int64_value);
    case PrgValueKind::uint64:
        return "uint64:" + std::to_string(value->uint64_value);
    case PrgValueKind::number:
        return "number";
    default:
        return "other";
    }
}

std::string i64(const std::int64_t value) {
    return "int64:" + std::to_string(value);
}

std::string u64(const std::uint64_t value) {
    return "uint64:" + std::to_string(value);
}

// "ERR39" when the operation raises numeric overflow, "ERR-div0" for the integer-division-by-zero error.
std::string arithmetic(const char operation, const PrgValue &left, const PrgValue &right) {
    try {
        return describe(copperfin::runtime::try_exact_integer_arithmetic(operation, left, right));
    } catch (const PrgCompatibilityError &error) {
        return "ERR" + std::to_string(error.error_code());
    } catch (const std::runtime_error &) {
        return "ERR-div0";
    }
}

void check_arithmetic(
    const std::string &name, const char operation, const PrgValue &left, const PrgValue &right, const std::string &expected) {
    const std::string actual = arithmetic(operation, left, right);
    expect(actual == expected, "#6035 " + name + ": expected " + expected + ", got " + actual);
}

void check_compare(const std::string &name, const PrgValue &left, const PrgValue &right, const int expected) {
    const auto actual = copperfin::runtime::try_exact_integer_compare(left, right);
    expect(actual.has_value() && *actual == expected,
           "#6035 " + name + ": expected " + std::to_string(expected) + ", got " +
               (actual.has_value() ? std::to_string(*actual) : std::string("nullopt")));
}

std::string negate(const PrgValue &value) {
    try {
        return describe(copperfin::runtime::negate_exact_integer(value));
    } catch (const PrgCompatibilityError &error) {
        return "ERR" + std::to_string(error.error_code());
    }
}

void test_exact_arithmetic() {
    // The issue's own cases: the native fixture returns 9007199254740993.
    check_arithmetic("exact+1", '+', make_int64_value(kBeyondDouble), make_number_value(1.0), i64(9007199254740994LL));
    check_arithmetic("exact*2", '*', make_int64_value(kBeyondDouble), make_number_value(2.0), i64(18014398509481986LL));
    check_arithmetic("1+exact", '+', make_number_value(1.0), make_int64_value(kBeyondDouble), i64(9007199254740994LL));
    check_arithmetic("exact-1", '-', make_int64_value(kBeyondDouble), make_number_value(1.0), i64(9007199254740992LL));
    check_arithmetic("1-exact", '-', make_number_value(1.0), make_int64_value(kBeyondDouble), i64(-9007199254740992LL));
    // The managed fixture's unsigned value 18014398509481985 fits int64, so the result is int64.
    check_arithmetic("uint+1", '+', make_uint64_value(18014398509481985ULL), make_number_value(1.0), i64(18014398509481986LL));
    check_arithmetic("int+int", '+', make_int64_value(kBeyondDouble), make_int64_value(kBeyondDouble), i64(18014398509481986LL));
    check_arithmetic("int64max+uint64 1", '+', make_int64_value(kInt64Max), make_uint64_value(1U), u64(9223372036854775808ULL));
    check_arithmetic("min+max", '+', make_int64_value(kInt64Min), make_int64_value(kInt64Max), i64(-1));
    check_arithmetic("zero stays int64", '-', make_int64_value(kBeyondDouble), make_int64_value(kBeyondDouble), i64(0));
    check_arithmetic("uint32 product", '*', make_uint64_value(4294967296ULL), make_uint64_value(4294967295ULL),
                     u64(18446744069414584320ULL));
    check_arithmetic("mixed sign product", '*', make_int64_value(-3), make_uint64_value(5U), i64(-15));
    check_arithmetic("zero product", '*', make_int64_value(0), make_uint64_value(kUint64Max), i64(0));
    check_arithmetic("uint64max-uint64max", '-', make_uint64_value(kUint64Max), make_uint64_value(kUint64Max), i64(0));
    check_arithmetic("uint64max-1", '-', make_uint64_value(kUint64Max), make_int64_value(1), u64(kUint64Max - 1U));

    // Overflow is numeric overflow (error 39), never signed overflow or a wrapped value.
    check_arithmetic("int64max+1", '+', make_int64_value(kInt64Max), make_int64_value(1), "ERR39");
    check_arithmetic("int64min-1", '-', make_int64_value(kInt64Min), make_int64_value(1), "ERR39");
    check_arithmetic("0-int64min", '-', make_int64_value(0), make_int64_value(kInt64Min), "ERR39");
    check_arithmetic("int64max*2", '*', make_int64_value(kInt64Max), make_int64_value(2), "ERR39");
    check_arithmetic("int64min*-1", '*', make_int64_value(kInt64Min), make_int64_value(-1), "ERR39");
    check_arithmetic("uint64max+1", '+', make_uint64_value(kUint64Max), make_int64_value(1), "ERR39");
    check_arithmetic("2^63*2 (uint64)", '*', make_uint64_value(9223372036854775808ULL), make_int64_value(2), "ERR39");
    check_arithmetic("uint64 below int64min", '-', make_int64_value(kInt64Min), make_uint64_value(1U), "ERR39");
    // A result above INT64_MAX is uint64 only when an operand is uint64; a Numeric operand does not widen it.
    check_arithmetic("int64max+Numeric 2^63", '+', make_int64_value(kInt64Max), make_number_value(9223372036854775808.0),
                     "ERR39");
    check_arithmetic("uint64 0+Numeric 2^63", '+', make_uint64_value(0U), make_number_value(9223372036854775808.0),
                     u64(9223372036854775808ULL));
    check_arithmetic("int64max+Numeric 2^63+2^11", '+', make_int64_value(kInt64Max), make_number_value(9223372036854777856.0),
                     "ERR39");

    // Truncating integer division, only between two 64-bit kinds.
    check_arithmetic("exact/2", '/', make_int64_value(kBeyondDouble), make_int64_value(2), i64(4503599627370496LL));
    check_arithmetic("-exact/2", '/', make_int64_value(-kBeyondDouble), make_int64_value(2), i64(-4503599627370496LL));
    check_arithmetic("exact/-2", '/', make_int64_value(kBeyondDouble), make_int64_value(-2), i64(-4503599627370496LL));
    check_arithmetic("uint64max/3", '/', make_uint64_value(kUint64Max), make_int64_value(3), i64(6148914691236517205LL));
    check_arithmetic("uint64max/1", '/', make_uint64_value(kUint64Max), make_int64_value(1), u64(kUint64Max));
    check_arithmetic("1/uint64max", '/', make_int64_value(1), make_uint64_value(kUint64Max), i64(0));
    check_arithmetic("-1/uint64max", '/', make_int64_value(-1), make_uint64_value(kUint64Max), i64(0));
    check_arithmetic("int64min/-1", '/', make_int64_value(kInt64Min), make_int64_value(-1), "ERR39");
    check_arithmetic("int64min/1", '/', make_int64_value(kInt64Min), make_int64_value(1), i64(kInt64Min));
    check_arithmetic("x/0", '/', make_int64_value(kBeyondDouble), make_int64_value(0), "ERR-div0");
    check_arithmetic("exact/Numeric keeps Numeric", '/', make_int64_value(kBeyondDouble), make_number_value(2.0), "nullopt");

    // Not eligible: the caller keeps its existing double arithmetic.
    check_arithmetic("fraction", '+', make_int64_value(1), make_number_value(0.5), "nullopt");
    check_arithmetic("two Numerics", '+', make_number_value(1.0), make_number_value(2.0), "nullopt");
    check_arithmetic("Numeric beyond uint64", '+', make_int64_value(1), make_number_value(1e300), "nullopt");
    check_arithmetic("Numeric below int64", '+', make_int64_value(1), make_number_value(-1e19), "nullopt");
    check_arithmetic("NaN", '+', make_int64_value(1), make_number_value(std::numeric_limits<double>::quiet_NaN()), "nullopt");
    check_arithmetic("infinity", '+', make_int64_value(1), make_number_value(std::numeric_limits<double>::infinity()), "nullopt");
    check_arithmetic("negative zero", '+', make_int64_value(7), make_number_value(-0.0), i64(7));
}

void test_exact_ordering_and_equality() {
    // 2^53 + 1 is not a double: the neighbouring double 2^53 must still compare less.
    check_compare("exact vs 2^53", make_int64_value(kBeyondDouble), make_number_value(9007199254740992.0), 1);
    check_compare("2^53 vs exact", make_number_value(9007199254740992.0), make_int64_value(kBeyondDouble), -1);
    check_compare("adjacent int64", make_int64_value(kBeyondDouble), make_int64_value(kBeyondDouble + 1), -1);
    check_compare("adjacent int64 reversed", make_int64_value(kBeyondDouble + 1), make_int64_value(kBeyondDouble), 1);
    check_compare("equal", make_int64_value(kBeyondDouble), make_int64_value(kBeyondDouble), 0);
    check_compare("uint64max vs -1", make_uint64_value(kUint64Max), make_int64_value(-1), 1);
    check_compare("-1 vs uint64 0", make_int64_value(-1), make_uint64_value(0U), -1);
    check_compare("2^63 vs int64max", make_uint64_value(9223372036854775808ULL), make_int64_value(kInt64Max), 1);
    check_compare("int64max vs 2^63", make_int64_value(kInt64Max), make_uint64_value(9223372036854775808ULL), -1);
    check_compare("int64min vs -int64max", make_int64_value(kInt64Min), make_int64_value(-kInt64Max), -1);
    check_compare("negatives", make_int64_value(-5), make_int64_value(-3), -1);
    check_compare("negatives reversed", make_int64_value(-3), make_int64_value(-5), 1);
    check_compare("int vs equal Numeric", make_int64_value(5), make_number_value(5.0), 0);
    check_compare("negative zero", make_number_value(-0.0), make_int64_value(0), 0);
    check_compare("uint64 2^64-2048 vs Numeric", make_uint64_value(18446744073709549568ULL),
                  make_number_value(18446744073709549568.0), 0);
    expect(!copperfin::runtime::try_exact_integer_compare(make_int64_value(1), make_number_value(1.5)).has_value(),
           "#6035 a fractional Numeric is not an exact integer operand");
    expect(!copperfin::runtime::try_exact_integer_compare(make_number_value(1.0), make_number_value(2.0)).has_value(),
           "#6035 two Numerics are not handled by the exact path");

    expect(!copperfin::runtime::numeric_prg_values_equal(make_int64_value(kBeyondDouble), make_number_value(9007199254740992.0)),
           "#6035 2^53+1 must not equal 2^53 (the double tolerance used to merge them)");
    expect(copperfin::runtime::numeric_prg_values_equal(make_int64_value(5), make_number_value(5.0)),
           "#6035 an int64 equals the same-valued Numeric");
    expect(!copperfin::runtime::numeric_prg_values_equal(make_uint64_value(9223372036854775808ULL), make_int64_value(kInt64Min)),
           "#6035 2^63 as uint64 must not equal INT64_MIN");
}

void test_unary_minus() {
    expect(negate(make_int64_value(5)) == i64(-5), "#6035 -5");
    expect(negate(make_int64_value(-5)) == i64(5), "#6035 --5");
    expect(negate(make_int64_value(0)) == i64(0), "#6035 -0");
    expect(negate(make_int64_value(kBeyondDouble)) == i64(-kBeyondDouble), "#6035 negating 2^53+1 stays exact");
    expect(negate(make_int64_value(kInt64Max)) == i64(-kInt64Max), "#6035 -INT64_MAX");
    expect(negate(make_int64_value(kInt64Min)) == "ERR39", "#6035 -INT64_MIN is numeric overflow, not signed overflow");
    expect(negate(make_uint64_value(9223372036854775808ULL)) == i64(kInt64Min), "#6035 -2^63 (uint64) is INT64_MIN");
    expect(negate(make_uint64_value(9223372036854775809ULL)) == "ERR39", "#6035 -(2^63+1) (uint64) is numeric overflow");
    expect(negate(make_uint64_value(kUint64Max)) == "ERR39", "#6035 -UINT64_MAX is numeric overflow");
    expect(negate(make_uint64_value(0U)) == i64(0), "#6035 -0 (uint64)");
}

}  // namespace

int main() {
    test_exact_arithmetic();
    test_exact_ordering_and_equality();
    test_unary_minus();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
