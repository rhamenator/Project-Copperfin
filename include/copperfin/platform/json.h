// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

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

struct JsonObjectMembersResult final {
    JsonSelectionError error = JsonSelectionError::none;
    std::vector<std::string> names;

    [[nodiscard]] bool ok() const noexcept {
        return error == JsonSelectionError::none;
    }
};

struct JsonDocumentState;
struct JsonDocumentParseResult;

// An immutable, bounded JSON document parsed once for repeated selections.
// It owns its source bytes and exposes copies of selected values only.
class JsonDocument final {
public:
    JsonDocument() = default;

    [[nodiscard]] JsonSelectionResult select(
        std::string_view json_pointer = {}) const;
    [[nodiscard]] JsonObjectMembersResult object_member_names(
        std::string_view json_pointer = {}) const;

private:
    friend JsonDocumentParseResult parse_json_document(
        std::string_view document,
        const JsonDocumentLimits& limits);
    explicit JsonDocument(std::shared_ptr<const JsonDocumentState> state)
        : state_(std::move(state)) {}

    std::shared_ptr<const JsonDocumentState> state_;
};

struct JsonDocumentParseResult final {
    JsonSelectionError error = JsonSelectionError::none;
    JsonDocument document;

    [[nodiscard]] bool ok() const noexcept {
        return error == JsonSelectionError::none;
    }
};

// Parses and validates a bounded UTF-8 JSON document once. Callers that need
// several selections should retain the resulting JsonDocument instead of
// repeatedly reparsing the same untrusted input.
[[nodiscard]] JsonDocumentParseResult parse_json_document(
    std::string_view document,
    const JsonDocumentLimits& limits = {});

[[nodiscard]] JsonSelectionResult select_json_value(
    std::string_view document,
    std::string_view json_pointer = {},
    const JsonDocumentLimits& limits = {});

[[nodiscard]] JsonObjectMembersResult select_json_object_member_names(
    std::string_view document,
    std::string_view json_pointer = {},
    const JsonDocumentLimits& limits = {});

[[nodiscard]] std::string_view json_value_kind_name(JsonValueKind kind) noexcept;

}  // namespace copperfin::platform
