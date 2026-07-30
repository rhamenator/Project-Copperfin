// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/package/launcher_inventory_trust.h"

#include "base64.h"
#include "ed25519_verify.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace copperfin::package_trust {
namespace {

constexpr std::string_view kVersion = "launcher_inventory_version=1";
constexpr std::string_view kHashAlgorithm = "hash_algorithm=sha256";
constexpr std::string_view kSignatureAlgorithm = "signature_algorithm=ed25519";
constexpr std::string_view kSidecarVersion = "launcher_signature_version=1";

bool safe_token(std::string_view value) {
    if (value.empty()) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](const char ch) {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_' || ch == '.';
    });
}

bool safe_package_file_name(std::string_view value) {
    if (value.empty() || value == "." || value == ".." ||
        value.find_first_of("/\\|\r\n") != std::string_view::npos) {
        return false;
    }
    return value.find("..") == std::string_view::npos &&
        std::none_of(value.begin(), value.end(), [](const char ch) {
            return std::iscntrl(static_cast<unsigned char>(ch)) != 0;
        });
}

bool valid_digest(std::string_view value) {
    if (value.size() != 64U) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](const char ch) {
        return (ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'f');
    });
}

bool contains_whitespace(std::string_view value) {
    return std::any_of(value.begin(), value.end(), [](const char ch) {
        return std::isspace(static_cast<unsigned char>(ch)) != 0;
    });
}

bool canonical_signature_base64(std::string_view value) {
    if (value.size() != 88U || value.substr(86U) != "==") {
        return false;
    }
    return std::all_of(value.begin(), value.begin() + 86U, [](const char ch) {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') || ch == '+' || ch == '/';
    });
}

int role_order(std::string_view role) {
    if (role == "public_apphost") {
        return 0;
    }
    if (role == "runtime_required") {
        return 1;
    }
    if (role == "debug_optional") {
        return 2;
    }
    return -1;
}

bool valid_artifact(const LauncherInventoryArtifact& artifact) {
    return role_order(artifact.role) >= 0 &&
        safe_package_file_name(artifact.package_relative_path) &&
        valid_digest(artifact.sha256);
}

bool artifact_less(
    const LauncherInventoryArtifact& left,
    const LauncherInventoryArtifact& right) {
    const int left_role = role_order(left.role);
    const int right_role = role_order(right.role);
    if (left_role != right_role) {
        return left_role < right_role;
    }
    return left.package_relative_path < right.package_relative_path;
}

bool duplicate_artifact(
    const LauncherInventoryArtifact& left,
    const LauncherInventoryArtifact& right) {
    return left.role == right.role &&
        left.package_relative_path == right.package_relative_path;
}

std::vector<std::string_view> split_lines(std::string_view envelope) {
    std::vector<std::string_view> lines;
    std::size_t start = 0U;
    while (start < envelope.size()) {
        const std::size_t end = envelope.find('\n', start);
        const std::size_t line_end = end == std::string_view::npos ? envelope.size() : end;
        if (line_end == start || envelope[line_end - 1U] == '\r') {
            return {};
        }
        lines.push_back(envelope.substr(start, line_end - start));
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1U;
    }
    if (envelope.empty() || envelope.back() != '\n' || start != envelope.size()) {
        return {};
    }
    return lines;
}

std::optional<LauncherInventorySignatureSidecar> parse_signature_sidecar(
    std::string_view sidecar) {
    const auto lines = split_lines(sidecar);
    if (lines.size() != 4U || lines[0] != kSidecarVersion ||
        lines[1] != kSignatureAlgorithm ||
        !lines[2].starts_with("signer_key_id=") ||
        !lines[3].starts_with("signature_base64=")) {
        return std::nullopt;
    }

    const std::string_view signer_key_id = lines[2].substr(
        std::string_view("signer_key_id=").size());
    const std::string_view signature_base64 = lines[3].substr(
        std::string_view("signature_base64=").size());
    if (!safe_token(signer_key_id) || !canonical_signature_base64(signature_base64) ||
        contains_whitespace(signature_base64)) {
        return std::nullopt;
    }

    const auto decoded = licensing::base64_decode(std::string(signature_base64));
    if (!decoded.has_value() || decoded->size() != 64U) {
        return std::nullopt;
    }

    LauncherInventorySignatureSidecar result;
    result.signer_key_id = std::string(signer_key_id);
    std::copy(decoded->begin(), decoded->end(), result.detached_signature.begin());
    return result;
}

bool parse_artifact(
    std::string_view value,
    LauncherInventoryArtifact& artifact) {
    const std::size_t first = value.find('|');
    const std::size_t second = first == std::string_view::npos
        ? std::string_view::npos
        : value.find('|', first + 1U);
    if (first == std::string_view::npos || second == std::string_view::npos ||
        value.find('|', second + 1U) != std::string_view::npos) {
        return false;
    }
    artifact = {
        std::string(value.substr(0U, first)),
        std::string(value.substr(first + 1U, second - first - 1U)),
        std::string(value.substr(second + 1U))
    };
    return valid_artifact(artifact);
}

bool parse_envelope(
    std::string_view envelope,
    std::string& signer_key_id,
    std::vector<LauncherInventoryArtifact>& artifacts) {
    const auto lines = split_lines(envelope);
    if (lines.size() < 4U || lines[0] != kVersion ||
        lines[1] != kHashAlgorithm || lines[2] != kSignatureAlgorithm ||
        !lines[3].starts_with("signer_key_id=")) {
        return false;
    }
    signer_key_id = std::string(lines[3].substr(std::string_view("signer_key_id=").size()));
    if (!safe_token(signer_key_id)) {
        return false;
    }
    artifacts.clear();
    for (std::size_t index = 4U; index < lines.size(); ++index) {
        if (!lines[index].starts_with("artifact=")) {
            return false;
        }
        LauncherInventoryArtifact artifact;
        if (!parse_artifact(
                lines[index].substr(std::string_view("artifact=").size()),
                artifact)) {
            return false;
        }
        artifacts.push_back(std::move(artifact));
    }
    if (artifacts.empty()) {
        return false;
    }
    std::sort(artifacts.begin(), artifacts.end(), artifact_less);
    return std::adjacent_find(artifacts.begin(), artifacts.end(), duplicate_artifact) == artifacts.end();
}

}  // namespace

std::string canonical_launcher_inventory_envelope(
    const std::string_view signer_key_id,
    const std::span<const LauncherInventoryArtifact> artifacts) {
    if (!safe_token(signer_key_id) || artifacts.empty() ||
        std::any_of(artifacts.begin(), artifacts.end(), [](const auto& artifact) {
            return !valid_artifact(artifact);
        })) {
        return {};
    }
    std::vector<LauncherInventoryArtifact> sorted(artifacts.begin(), artifacts.end());
    std::sort(sorted.begin(), sorted.end(), artifact_less);
    if (std::adjacent_find(sorted.begin(), sorted.end(), duplicate_artifact) != sorted.end()) {
        return {};
    }

    std::ostringstream output;
    output << kVersion << '\n'
           << kHashAlgorithm << '\n'
           << kSignatureAlgorithm << '\n'
           << "signer_key_id=" << signer_key_id << '\n';
    for (const auto& artifact : sorted) {
        output << "artifact=" << artifact.role << '|'
               << artifact.package_relative_path << '|'
               << artifact.sha256 << '\n';
    }
    return output.str();
}

bool launcher_inventory_envelope_matches_artifacts(
    const std::string_view envelope,
    const std::string_view signer_key_id,
    const std::span<const LauncherInventoryArtifact> artifacts) {
    const std::string canonical = canonical_launcher_inventory_envelope(signer_key_id, artifacts);
    return !canonical.empty() && canonical == envelope;
}

std::optional<LauncherInventorySignatureSidecar>
parse_launcher_inventory_signature_sidecar(const std::string_view sidecar) {
    return parse_signature_sidecar(sidecar);
}

LauncherInventoryVerificationResult verify_signed_launcher_inventory(
    const std::string_view envelope,
    const std::array<std::uint8_t, 64>& detached_signature,
    const std::span<const LauncherInventoryTrustedKey> trusted_keys) {
    LauncherInventoryVerificationResult result;
    std::string signer_key_id;
    std::vector<LauncherInventoryArtifact> artifacts;
    if (!parse_envelope(envelope, signer_key_id, artifacts)) {
        return result;
    }
    const std::string canonical = canonical_launcher_inventory_envelope(signer_key_id, artifacts);
    if (canonical.empty() || canonical != envelope) {
        return result;
    }
    result.signer_key_id = signer_key_id;
    const auto key_matches = std::count_if(
        trusted_keys.begin(),
        trusted_keys.end(),
        [&](const auto& candidate) {
            return candidate.key_id == signer_key_id;
        });
    if (key_matches == 0U) {
        result.status = LauncherInventoryVerificationStatus::unknown_signer;
        return result;
    }
    if (key_matches != 1U) {
        result.status = LauncherInventoryVerificationStatus::ambiguous_signer;
        return result;
    }
    const auto key = std::find_if(trusted_keys.begin(), trusted_keys.end(), [&](const auto& candidate) {
        return candidate.key_id == signer_key_id;
    });
    if (!licensing::ed25519_verify_detached(envelope, detached_signature, key->public_key)) {
        result.status = LauncherInventoryVerificationStatus::invalid_signature;
        return result;
    }
    result.status = LauncherInventoryVerificationStatus::valid;
    return result;
}

LauncherInventoryVerificationResult verify_signed_launcher_inventory(
    const std::string_view envelope,
    const std::string_view signature_sidecar,
    const std::span<const LauncherInventoryTrustedKey> trusted_keys) {
    const auto parsed_sidecar = parse_signature_sidecar(signature_sidecar);
    if (!parsed_sidecar.has_value()) {
        return {};
    }

    auto result = verify_signed_launcher_inventory(
        envelope,
        parsed_sidecar->detached_signature,
        trusted_keys);
    if (result.status == LauncherInventoryVerificationStatus::malformed_envelope ||
        result.signer_key_id != parsed_sidecar->signer_key_id) {
        result.status = LauncherInventoryVerificationStatus::malformed_envelope;
    }
    return result;
}

}  // namespace copperfin::package_trust
