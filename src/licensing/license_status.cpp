// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/licensing/license_status.h"
#include "copperfin/licensing/ed25519_public_key.h"

#include <array>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>

#include "base64.h"
#include "canonical_payload_serializer.h"
#include "ed25519_verify.h"
#include "license_classifier.h"
#include "license_payload_parser.h"

namespace copperfin::licensing {

namespace {

std::string current_date_iso8601() {
    const std::time_t now = std::time(nullptr);
    std::tm utc_time{};
#ifdef _WIN32
    gmtime_s(&utc_time, &now);
#else
    gmtime_r(&now, &utc_time);
#endif
    char buffer[16] = {};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &utc_time);
    return std::string(buffer);
}

const SignerPublicKey* find_signer_key(std::string_view key_id, std::span<const SignerPublicKey> signer_keys) {
    for (const auto& candidate : signer_keys) {
        if (candidate.key_id == key_id) {
            return &candidate;
        }
    }
    return nullptr;
}

}  // namespace

std::string_view license_state_name(LicenseState state) {
    switch (state) {
        case LicenseState::free:
            return "free";
        case LicenseState::subscription_active:
            return "subscription_active";
        case LicenseState::subscription_expired:
            return "subscription_expired";
        case LicenseState::perpetual_current:
            return "perpetual_current";
        case LicenseState::perpetual_out_of_version:
            return "perpetual_out_of_version";
        case LicenseState::invalid_signature:
            return "invalid_signature";
        case LicenseState::malformed:
            return "malformed";
        case LicenseState::file_unreadable:
            return "file_unreadable";
    }
    return "unknown";
}

LicenseStatus load_license_status(
    const std::filesystem::path& executable_path,
    const std::optional<std::filesystem::path>& explicit_override,
    std::span<const SignerPublicKey> signer_keys) {
    LicenseStatus status;

    std::filesystem::path resolved_path;
    bool path_was_explicit = false;

    std::string env_license_path;
    bool has_env_license_path = false;
#ifdef _WIN32
    char* raw_env_value = nullptr;
    std::size_t raw_env_length = 0;
    if (_dupenv_s(&raw_env_value, &raw_env_length, "COPPERFIN_LICENSE_PATH") == 0 && raw_env_value != nullptr) {
        env_license_path = raw_env_value;
        has_env_license_path = !env_license_path.empty();
        std::free(raw_env_value);
    }
#else
    if (const char* raw_env_value = std::getenv("COPPERFIN_LICENSE_PATH"); raw_env_value != nullptr && raw_env_value[0] != '\0') {
        env_license_path = raw_env_value;
        has_env_license_path = true;
    }
#endif

    if (explicit_override.has_value() && !explicit_override->empty()) {
        resolved_path = *explicit_override;
        path_was_explicit = true;
    } else if (has_env_license_path) {
        resolved_path = std::filesystem::path(env_license_path);
        path_was_explicit = true;
    } else {
        resolved_path = executable_path.parent_path() / "license.cflicense";
        path_was_explicit = false;
    }

    std::error_code existence_error;
    const bool file_exists = std::filesystem::is_regular_file(resolved_path, existence_error);
    if (!file_exists) {
        if (path_was_explicit) {
            status.state = LicenseState::file_unreadable;
            status.diagnostic = "license file not found at explicitly configured path";
            status.source_path = resolved_path.string();
        } else {
            status.state = LicenseState::free;
        }
        return status;
    }

    std::ifstream input(resolved_path, std::ios::binary);
    if (!input) {
        status.state = LicenseState::file_unreadable;
        status.diagnostic = "license file exists but could not be opened";
        status.source_path = resolved_path.string();
        return status;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string contents = buffer.str();
    status.source_path = resolved_path.string();

    const ParsedLicenseFile parsed = parse_license_file(contents);
    if (!parsed.ok) {
        status.state = LicenseState::malformed;
        status.diagnostic = parsed.error;
        return status;
    }

    if (parsed.signature_algorithm != "ed25519") {
        status.state = LicenseState::malformed;
        status.diagnostic = "unsupported signature_algorithm";
        return status;
    }

    const auto signer_key_it = parsed.payload_fields.find("signer_key_id");
    if (signer_key_it == parsed.payload_fields.end() ||
        signer_key_it->second.kind != PayloadValue::Kind::string_value) {
        status.state = LicenseState::malformed;
        status.diagnostic = "missing or invalid signer_key_id";
        return status;
    }

    const SignerPublicKey* matched_key = find_signer_key(signer_key_it->second.as_string, signer_keys);
    if (matched_key == nullptr) {
        status.state = LicenseState::invalid_signature;
        status.diagnostic = "unknown signer_key_id";
        return status;
    }

    const auto signature_bytes = base64_decode(parsed.signature_base64);
    if (!signature_bytes.has_value() || signature_bytes->size() != 64U) {
        status.state = LicenseState::invalid_signature;
        status.diagnostic = "signature is not valid base64 of 64 bytes";
        return status;
    }

    std::array<std::uint8_t, 64> signature_array{};
    std::copy(signature_bytes->begin(), signature_bytes->end(), signature_array.begin());

    const std::string canonical = canonicalize_payload(parsed.payload_fields);

    if (!ed25519_verify_detached(canonical, signature_array, matched_key->public_key)) {
        status.state = LicenseState::invalid_signature;
        status.diagnostic = "signature verification failed";
        return status;
    }

    LicenseStatus classified = classify_verified_payload(parsed.payload_fields, kCopperfinMajorVersion, current_date_iso8601());
    classified.source_path = status.source_path;
    return classified;
}

}  // namespace copperfin::licensing
