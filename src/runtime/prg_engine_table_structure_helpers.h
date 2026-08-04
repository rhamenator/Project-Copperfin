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
    bool nullable = true;
    bool has_default = false;
    std::string default_expression{};
};

std::optional<TableFieldDeclaration> parse_table_field_declaration(std::string text);
std::vector<TableFieldDeclaration> parse_table_field_declarations(const std::string& field_list);
std::vector<vfp::DbfFieldDescriptor> table_field_descriptors(const std::vector<TableFieldDeclaration>& declarations);

}  // namespace copperfin::runtime
