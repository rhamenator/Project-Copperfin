// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace copperfin::platform {

std::string json_escape_string(std::string_view value);

enum class JsonValueKind {
    invalid,
    null_value,
    boolean,
    number,
    string,
    array,
    object
};

enum class JsonSelectionError {
    none,
    document_required,
    document_too_large,
    invalid_limits,
    invalid_utf8,
    invalid_json,
    value_count_exceeded,
    invalid_pointer,
    value_not_found
};

struct JsonDocumentLimits final {
    std::size_t max_document_bytes = 1024U * 1024U;
    std::uint32_t max_nesting_depth = 64U;
    std::size_t max_value_count = 65536U;
};

struct JsonSelectionResult final {
    JsonSelectionError error = JsonSelectionError::none;
    JsonValueKind kind = JsonValueKind::invalid;
    std::string raw_json;
    std::string decoded_string;

    [[nodiscard]] bool ok() const noexcept {
        return error == JsonSelectionError::none;
    }
};

[[nodiscard]] JsonSelectionResult select_json_value(
    std::string_view document,
    std::string_view json_pointer = {},
    const JsonDocumentLimits& limits = {});

[[nodiscard]] std::string_view json_value_kind_name(JsonValueKind kind) noexcept;

}  // namespace copperfin::platform
