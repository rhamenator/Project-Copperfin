// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_rejects_numeric_selector_errors() {
    const auto invalid_record_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record", "-1"
    });
    expect(!invalid_record_result.ok,
        "#1714: launch contract should reject negative record selectors");
    expect(invalid_record_result.error == "The --record value must be an unsigned integer.",
        "#1714: launch contract should report invalid record selector values");

    const auto missing_record_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record"
    });
    expect(!missing_record_result.ok,
        "#1714: launch contract should reject missing record selector values");
    expect(missing_record_result.error == "Missing value after --record.",
        "#1714: launch contract should report missing record selector values");

    const auto invalid_line_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--line", "-1"
    });
    expect(!invalid_line_result.ok,
        "#1714: launch contract should reject negative line selectors");
    expect(invalid_line_result.error == "The --line value must be an unsigned integer.",
        "#1714: launch contract should report invalid line selector values");

    const auto missing_line_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--line"
    });
    expect(!missing_line_result.ok,
        "#1714: launch contract should reject missing line selector values");
    expect(missing_line_result.error == "Missing value after --line.",
        "#1714: launch contract should report missing line selector values");

    const auto invalid_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column", "-1"
    });
    expect(!invalid_column_result.ok,
        "#1714: launch contract should reject negative column selectors");
    expect(invalid_column_result.error == "The --column value must be an unsigned integer.",
        "#1714: launch contract should report invalid column selector values");

    const auto missing_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column"
    });
    expect(!missing_column_result.ok,
        "#1714: launch contract should reject missing column selector values");
    expect(missing_column_result.error == "Missing value after --column.",
        "#1714: launch contract should report missing column selector values");
}

void test_parse_launch_arguments_rejects_unknown_selection_context() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selection-context", "mystery"
    });
    expect(!result.ok, "#962: launch contract should reject unknown selection-context tokens");
}

void test_parse_launch_arguments_rejects_missing_selection_context() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selection-context"
    });
    expect(!result.ok, "#962: launch contract should reject missing selection-context values");
}

}  // namespace cf_test_studio_host
