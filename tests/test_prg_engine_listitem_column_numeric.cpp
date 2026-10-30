// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <string>

namespace {
using namespace copperfin::runtime;
using copperfin::test_support::expect;
using Result = std::optional<std::int32_t>;

PrgValue attributed(double value, std::uint32_t decimals) {
    auto result = make_number_value(value);
    result.numeric_metadata = PrgNumericMetadata{13U, decimals, PrgNumericStorage::real};
    return result;
}

void check(const PrgValue& value, Result modern, Result legacy, const char* label) {
    expect(checked_listitem_column_numeric_argument(value, NumericBehavior::copperfin) == modern,
        std::string("ListItem column COPPERFIN ") + label);
    expect(checked_listitem_column_numeric_argument(value, NumericBehavior::vfp9) == legacy,
        std::string("ListItem column VFP9 ") + label);
}
}

int main() {
    // Native-backed exact/integral foundation. Missing cells are handled by
    // the future caller; conversion itself may retain a positive INT32 column.
    check(make_number_value(1.0), 1, 1, "one");
    check(attributed(2.0, 1U), 2, 2, "attributed integral");
    check(make_number_value(2147483647.0), INT32_MAX, INT32_MAX, "INT32_MAX");
    check(make_number_value(2147483648.0), {}, {}, "above positive ceiling");
    check(make_number_value(-4294967295.0), {}, 1, "native negative alias one");
    check(make_number_value(-8589934591.0), {}, 1, "native second-cycle alias one");
    check(make_int64_value(22), 22, 22, "exact signed");
    check(make_int64_value(-4294967274LL), {}, 22, "exact signed alias");
    check(make_uint64_value(33), 33, 33, "exact unsigned");
    check(make_uint64_value(4294967329ULL), {}, 33, "exact unsigned alias");

    // Deliberate fail-closed frontier: these are known native-valid values,
    // but no fractional result is admitted before the complete metadata rule.
    check(attributed(2.5, 1U), {}, {}, "ordinary fractional deferred");
    const double near_one = 1.0 - 90.0 * std::ldexp(1.0, -53);
    check(attributed(near_one, 2U), {}, {}, "near one decimal2 deferred");
    check(attributed(near_one, 14U), {}, {}, "same bits decimal14 deferred");
    check(attributed(0.9, 0U), {}, {}, "sub-one invalid");

    auto typed_null = attributed(2.0, 0U);
    typed_null.is_null = true;
    check(typed_null, {}, {}, "typed NULL");
    check(make_number_value(std::numeric_limits<double>::quiet_NaN()), {}, {}, "NaN");
    check(make_number_value(std::numeric_limits<double>::infinity()), {}, {}, "infinity");
    check(make_string_value("2"), {}, {}, "Character");
    check(make_currency_value(20000), {}, {}, "Currency");

    std::cout << "ListItem supplied-column exact foundation passed\n";
    return 0;
}
