// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>

namespace copperfin::test_dbf_table {

extern int failures;

void expect(bool condition, const std::string& message);

void test_memo_field_create_replace_and_append_round_trip();
void test_general_and_picture_memo_fields_round_trip();

}  // namespace copperfin::test_dbf_table
