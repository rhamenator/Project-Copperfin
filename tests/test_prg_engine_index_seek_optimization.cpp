#include "copperfin/runtime/index_seek_optimizer.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <iostream>

namespace {

using namespace copperfin::test_support;

void test_index_operator_kind_values() {
    expect(static_cast<int>(copperfin::runtime::IndexOperatorKind::equal) == 0,
        "IndexOperatorKind::equal should remain stable for serialization and diagnostics");
    expect(static_cast<int>(copperfin::runtime::IndexOperatorKind::unsupported) == 12,
        "IndexOperatorKind::unsupported should remain stable for fallback planning");
}

void test_optimization_confidence_values() {
    expect(static_cast<int>(copperfin::runtime::OptimizationConfidence::not_applicable) == 0,
        "OptimizationConfidence::not_applicable should map to the lowest confidence");
    expect(static_cast<int>(copperfin::runtime::OptimizationConfidence::high) == 3,
        "OptimizationConfidence::high should map to the highest confidence");
}

void test_expression_pattern_struct_defaults() {
    copperfin::runtime::IndexExpressionPattern pattern{};
    expect(pattern.operator_kind == copperfin::runtime::IndexOperatorKind::unsupported,
        "IndexExpressionPattern should default to unsupported operator");
    expect(pattern.confidence == copperfin::runtime::OptimizationConfidence::not_applicable,
        "IndexExpressionPattern should default to not_applicable confidence");
    expect(!pattern.field.has_value(),
        "IndexExpressionPattern should default to no selected field");
    expect(pattern.operands.empty(),
        "IndexExpressionPattern should default to no operands");
}

void test_index_seek_plan_struct_defaults() {
    copperfin::runtime::IndexSeekPlan plan{};
    expect(!plan.can_optimize,
        "IndexSeekPlan should default to non-optimized execution");
    expect(plan.strategy == copperfin::runtime::IndexSeekPlan::ExecutionStrategy::linear_scan,
        "IndexSeekPlan should default to linear scan strategy");
    expect(!plan.selected_order.has_value(),
        "IndexSeekPlan should default to no selected order");
}

void test_index_seek_plan_can_hold_selected_order() {
    copperfin::runtime::IndexOrderCandidate candidate{};
    candidate.order_name = "NAME";
    candidate.order_expression = "UPPER(name)";
    candidate.match_reason = "expression contains normalized field";

    copperfin::runtime::IndexSeekPlan plan{};
    plan.can_optimize = true;
    plan.strategy = copperfin::runtime::IndexSeekPlan::ExecutionStrategy::index_seek;
    plan.selected_order = candidate;

    expect(plan.can_optimize,
        "IndexSeekPlan should preserve explicit optimization enablement");
    expect(plan.selected_order.has_value(),
        "IndexSeekPlan should preserve selected order candidate");
    if (plan.selected_order.has_value()) {
        expect(plan.selected_order->order_name == "NAME",
            "Selected order candidate name should round-trip through IndexSeekPlan");
        expect(plan.selected_order->match_reason == "expression contains normalized field",
            "Selected order candidate match reason should round-trip through IndexSeekPlan");
    }
}

}  // namespace

int main() {
    test_index_operator_kind_values();
    test_optimization_confidence_values();
    test_expression_pattern_struct_defaults();
    test_index_seek_plan_struct_defaults();
    test_index_seek_plan_can_hold_selected_order();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
