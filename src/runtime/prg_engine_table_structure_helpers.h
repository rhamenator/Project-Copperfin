// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/vfp/dbf_table.h"

#include <optional>
#include <string>
#include <vector>

namespace copperfin::runtime {

struct TableFieldDeclaration {
    vfp::DbfFieldDescriptor descriptor;
    // `nullable` defaults true and is consulted only for NOT-NULL-constraint
    // enforcement (field_rules_from_declarations()): a field with no
    // explicit clause has no NOT NULL rule, same as one with an explicit
    // NULL clause. #6047's on-disk _NullFlags bit is a materially bigger
    // claim than "not enforced NOT NULL", so it must key off
    // `null_clause_specified` (true only when the declaration text actually
    // contained NULL or NOT NULL) as well, not `nullable` alone -- otherwise
    // every ordinary field with no clause at all (the common case) would
    // silently start consuming a bitmap bit. See table_field_descriptors().
    bool nullable = true;
    bool null_clause_specified = false;
    bool has_default = false;
    std::string default_expression{};
};

std::optional<TableFieldDeclaration> parse_table_field_declaration(std::string text);
std::vector<TableFieldDeclaration> parse_table_field_declarations(const std::string& field_list);
std::vector<vfp::DbfFieldDescriptor> table_field_descriptors(const std::vector<TableFieldDeclaration>& declarations);

}  // namespace copperfin::runtime
