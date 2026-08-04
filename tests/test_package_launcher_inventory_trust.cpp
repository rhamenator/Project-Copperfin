// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/package/launcher_inventory_trust.h"
#include "ed25519_verify.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {

using copperfin::package_trust::LauncherInventoryArtifact;
using copperfin::package_trust::LauncherInventoryTrustedKey;
using copperfin::package_trust::LauncherInventoryVerificationStatus;

constexpr std::array<std::uint8_t, 32> kFixturePublicKey{
    0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7,
    0xd5, 0x4b, 0xfe, 0xd3, 0xc9, 0x64, 0x07, 0x3a,
    0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6, 0x23, 0x25,
    0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a
};

constexpr std::array<std::uint8_t, 64> kFixtureEmptyMessageSignature{
    0xe5, 0x56, 0x43, 0x00, 0xc3, 0x60, 0xac, 0x72,
    0x90, 0x86, 0xe2, 0xcc, 0x80, 0x6e, 0x82, 0x8a,
    0x84, 0x87, 0x7f, 0x1e, 0xb8, 0xe5, 0xd9, 0x74,
    0xd8, 0x73, 0xe0, 0x65, 0x22, 0x49, 0x01, 0x55,
    0x5f, 0xb8, 0x82, 0x15, 0x90, 0xa3, 0x3b, 0xac,
    0xc6, 0x1e, 0x39, 0x70, 0x1c, 0xf9, 0xb4, 0x6b,
    0xd2, 0x5b, 0xf5, 0xf0, 0x59, 0x5b, 0xbe, 0x24,
    0x65, 0x51, 0x41, 0x43, 0x8e, 0x7a, 0x10, 0x0b
};

constexpr std::array<std::uint8_t, 64> kFixtureEnvelopeSignature{
    0xf8, 0xca, 0x7d, 0x9b, 0xd0, 0x26, 0x5a, 0x97,
    0xa1, 0x1e, 0x6f, 0x1c, 0xb7, 0x3d, 0x99, 0x8f,
    0x8c, 0x34, 0x50, 0x7a, 0xc9, 0x03, 0x5e, 0xd7,
    0xe6, 0x88, 0xf1, 0xa9, 0x98, 0x01, 0x99, 0x0e,
    0x9b, 0x9c, 0xe6, 0xe1, 0x70, 0x0e, 0xc2, 0x21,
    0xdc, 0xb5, 0x7b, 0x91, 0xc7, 0x13, 0x87, 0xcc,
    0x24, 0x33, 0xc0, 0xb7, 0x81, 0x94, 0xdc, 0x27,
    0xc9, 0xfc, 0xac, 0xdf, 0x83, 0x63, 0x2b, 0x0f
};

constexpr std::string_view kFixtureSignatureSidecar =
    "launcher_signature_version=1\n"
    "signature_algorithm=ed25519\n"
    "signer_key_id=rfc8032\n"
    "signature_base64=+Mp9m9AmWpehHm8ctz2Zj4w0UHrJA17X5ojxqZgBmQ6bnObhcA7CIdy1e5HHE4fMJDPAt4GU3CfJ/Kzfg2MrDw==\n";

void expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

std::vector<LauncherInventoryArtifact> fixture_artifacts() {
    return {
        {"runtime_required", "Copperfin.GeneratedLauncher.deps.json",
         "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
        {"public_apphost", "CopperfinApp.exe",
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"},
        {"debug_optional", "Copperfin.GeneratedLauncher.pdb",
         "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"}
    };
}

}  // namespace

int main() {
    const auto artifacts = fixture_artifacts();
    const std::string envelope = copperfin::package_trust::canonical_launcher_inventory_envelope(
        "rfc8032",
        artifacts);
    expect(!envelope.empty(), "valid fixture inventory should canonicalize");
    expect(
        envelope.find("artifact=public_apphost|CopperfinApp.exe|") != std::string::npos,
        "canonical inventory should order public apphost first");
    expect(
        envelope.find("artifact=runtime_required|Copperfin.GeneratedLauncher.deps.json|") !=
            std::string::npos,
        "canonical inventory should retain runtime sidecars");

    const std::array<LauncherInventoryTrustedKey, 1> trusted_keys{{
        {"rfc8032", kFixturePublicKey}
    }};
    const auto valid = copperfin::package_trust::verify_signed_launcher_inventory(
        envelope,
        kFixtureEnvelopeSignature,
        trusted_keys);
    expect(
        valid.status == LauncherInventoryVerificationStatus::valid,
        "canonical inventory signed by the public RFC fixture must verify");
    const auto parsed_sidecar =
        copperfin::package_trust::parse_launcher_inventory_signature_sidecar(
            kFixtureSignatureSidecar);
    expect(parsed_sidecar.has_value(), "valid signature sidecar should parse");
    expect(
        parsed_sidecar->signer_key_id == "rfc8032" &&
            parsed_sidecar->detached_signature == kFixtureEnvelopeSignature,
        "signature sidecar should preserve signer ID and detached bytes");
    const auto valid_sidecar = copperfin::package_trust::verify_signed_launcher_inventory(
        envelope,
        kFixtureSignatureSidecar,
        trusted_keys);
    expect(
        valid_sidecar.status == LauncherInventoryVerificationStatus::valid,
        "valid textual signature sidecar must verify");
    expect(
        copperfin::package_trust::launcher_inventory_envelope_matches_artifacts(
            envelope,
            "rfc8032",
            artifacts),
        "signed inventory must match the selected manifest artifact records");
    auto replaced_manifest_artifacts = artifacts;
    replaced_manifest_artifacts.front().sha256[0] =
        replaced_manifest_artifacts.front().sha256[0] == 'a' ? 'e' : 'a';
    expect(
        !copperfin::package_trust::launcher_inventory_envelope_matches_artifacts(
            envelope,
            "rfc8032",
            replaced_manifest_artifacts),
        "a signed inventory must not authenticate replaced manifest artifact records");
    const auto unknown = copperfin::package_trust::verify_signed_launcher_inventory(
        envelope,
        kFixtureEmptyMessageSignature,
        trusted_keys);
    expect(
        unknown.status == LauncherInventoryVerificationStatus::invalid_signature,
        "a valid envelope with an unrelated signature must fail closed");

    auto modified = envelope;
    modified.replace(modified.find("aaaa"), 4U, "eeee");
    const auto modified_result = copperfin::package_trust::verify_signed_launcher_inventory(
        modified,
        kFixtureEmptyMessageSignature,
        trusted_keys);
    expect(
        modified_result.status == LauncherInventoryVerificationStatus::invalid_signature,
        "modified inventory must not become trusted through recomputed unsigned hashes");

    const std::array<LauncherInventoryTrustedKey, 1> wrong_keys{{
        {"other-key", kFixturePublicKey}
    }};
    const auto unknown_signer = copperfin::package_trust::verify_signed_launcher_inventory(
        envelope,
        kFixtureEmptyMessageSignature,
        wrong_keys);
    expect(
        unknown_signer.status == LauncherInventoryVerificationStatus::unknown_signer,
        "unknown signer IDs must fail closed before signature acceptance");

    const std::array<LauncherInventoryTrustedKey, 2> duplicate_keys{{
        {"rfc8032", kFixturePublicKey},
        {"rfc8032", kFixturePublicKey}
    }};
    const auto ambiguous_signer =
        copperfin::package_trust::verify_signed_launcher_inventory(
            envelope,
            kFixtureEnvelopeSignature,
            duplicate_keys);
    expect(
        ambiguous_signer.status == LauncherInventoryVerificationStatus::ambiguous_signer,
        "duplicate trusted signer IDs must fail closed before signature acceptance");

    const auto malformed = copperfin::package_trust::verify_signed_launcher_inventory(
        "launcher_inventory_version=1\n"
        "hash_algorithm=sha256\n"
        "signature_algorithm=ed25519\n"
        "signer_key_id=rfc8032\n"
        "artifact=runtime_required|../escape|bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n",
        kFixtureEmptyMessageSignature,
        trusted_keys);
    expect(
        malformed.status == LauncherInventoryVerificationStatus::malformed_envelope,
        "traversal inventory paths must be rejected as malformed");

    const auto malformed_sidecar =
        copperfin::package_trust::verify_signed_launcher_inventory(
            envelope,
            "launcher_signature_version=1\n"
            "signature_algorithm=ed25519\n"
            "signer_key_id=rfc8032\n"
            "signature_base64=not-a-signature\n",
            trusted_keys);
    expect(
        malformed_sidecar.status == LauncherInventoryVerificationStatus::malformed_envelope,
        "malformed signature sidecars must fail closed");

    const auto noncanonical_sidecar =
        copperfin::package_trust::verify_signed_launcher_inventory(
            envelope,
            "launcher_signature_version=1\n"
            "signature_algorithm=ed25519\n"
            "signer_key_id=rfc8032\n"
            "signature_base64=+Mp9m9AmWpehHm8ctz2Zj4w0UHrJA17X5ojxqZgBmQ6bnObhcA7CIdy1e5HHE4fMJDPAt4GU3CfJ/Kzfg=0=\n",
            trusted_keys);
    expect(
        noncanonical_sidecar.status == LauncherInventoryVerificationStatus::malformed_envelope,
        "noncanonical signature base64 padding must fail closed");

    const auto mismatched_sidecar =
        copperfin::package_trust::verify_signed_launcher_inventory(
            envelope,
            "launcher_signature_version=1\n"
            "signature_algorithm=ed25519\n"
            "signer_key_id=other-key\n"
            "signature_base64=+Mp9m9AmWpehHm8ctz2Zj4w0UHrJA17X5ojxqZgBmQ6bnObhcA7CIdy1e5HHE4fMJDPAt4GU3CfJ/Kzfg2MrDw==\n",
            trusted_keys);
    expect(
        mismatched_sidecar.status == LauncherInventoryVerificationStatus::malformed_envelope,
        "sidecar signer mismatch must fail as malformed");

    auto duplicates = artifacts;
    duplicates.push_back(artifacts.front());
    expect(
        copperfin::package_trust::canonical_launcher_inventory_envelope(
            "rfc8032",
            duplicates).empty(),
        "duplicate inventory records must not canonicalize");

    // The RFC 8032 empty-message vector proves the vendored verify-only
    // primitive without embedding a private or machine-specific key.
    const auto primitive = copperfin::licensing::ed25519_verify_detached(
        {},
        kFixtureEmptyMessageSignature,
        kFixturePublicKey);
    expect(primitive, "RFC 8032 Ed25519 fixture should verify");
    return 0;
}
