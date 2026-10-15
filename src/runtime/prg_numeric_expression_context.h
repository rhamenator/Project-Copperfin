// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once
#include <cmath>

namespace copperfin::runtime {

// RQ-CF-PRG-NUMERIC-UNARY-ZERO-CONTEXT-001: transient parse aggregation,
// NOT persistent value provenance. Nested/exceptional scopes merge observed
// runtime primaries into their enclosing expression without counters/wrapping.
class ScopedNumericExpressionContext {
public:
    explicit ScopedNumericExpressionContext(bool& runtime_primary) noexcept
        : runtime_primary_(runtime_primary), previous_(runtime_primary) {
        runtime_primary_ = false;
    }
    ~ScopedNumericExpressionContext() {
        runtime_primary_ = runtime_primary_ || previous_;
    }
    ScopedNumericExpressionContext(const ScopedNumericExpressionContext&) = delete;
    ScopedNumericExpressionContext& operator=(const ScopedNumericExpressionContext&) = delete;
private:
    bool& runtime_primary_;
    bool previous_;
};

// Bounded native contract and remaining producers:
// tests/fixtures/vfp9-numeric-unary-expression-context/zero-contract.md.
// Nonzero/unknown-constant behavior stays the old negation. Runtime zero
// preserves sign; known Numeric constant zero becomes negative zero. This
// neither rounds a value nor infers native Integer identity from its double.
inline double negated_numeric_expression_payload(
    double value, bool runtime_primary, bool known_numeric_attributes) noexcept {
    if (value == 0.0) {
        if (runtime_primary) return value;
        if (known_numeric_attributes) return std::copysign(0.0, -1.0);
    }
    return -value;
}

} // namespace copperfin::runtime
