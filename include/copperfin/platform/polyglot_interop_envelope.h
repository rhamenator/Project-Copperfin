// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace copperfin::platform {

enum class PolyglotInteropEnvelopeKind : std::uint8_t {
    success,
    error
};

enum class PolyglotInteropEnvelopeError : std::uint8_t {
    none,
    document_required,
    document_too_large,
    invalid_limits,
    invalid_json,
    invalid_document,
    invalid_envelope_version,
    invalid_kind,
    invalid_capability_id,
    capability_id_mismatch,
    correlation_id_required,
    correlation_id_mismatch,
    invalid_protocol_version,
    protocol_version_mismatch,
    payload_required,
    error_required,
    invalid_error_code
};

struct PolyglotInteropEnvelopeExpectation {
    std::string capability_id;
    std::string correlation_id;
    std::string protocol_version;
    std::size_t max_document_bytes = std::size_t{1024U} * 1024U;
    std::uint32_t max_nesting_depth = 32U;
};

struct PolyglotInteropEnvelope {
    PolyglotInteropEnvelopeKind kind = PolyglotInteropEnvelopeKind::error;
    std::string capability_id;
    std::string correlation_id;
    std::string protocol_version;
    std::string payload_json;
    std::string candidate_error_code;
    std::string candidate_error_message;
    bool candidate_error_retryable = false;
};

struct PolyglotInteropEnvelopeResult {
    PolyglotInteropEnvelope envelope;
    PolyglotInteropEnvelopeError error = PolyglotInteropEnvelopeError::none;
    std::string error_code;

    [[nodiscard]] bool ok() const noexcept {
        return error == PolyglotInteropEnvelopeError::none;
    }
};

[[nodiscard]] PolyglotInteropEnvelopeResult parse_polyglot_interop_envelope(
    std::string_view document,
    const PolyglotInteropEnvelopeExpectation& expectation);

}  // namespace copperfin::platform
