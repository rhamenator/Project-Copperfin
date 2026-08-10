// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/path.h"
#include "copperfin/platform/polyglot_artifact_admission.h"
#include "copperfin/security/sha256.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

static_assert(
    !std::is_default_constructible_v<
        copperfin::platform::PolyglotArtifactAdmissionResult>);
static_assert(
    !std::is_aggregate_v<
        copperfin::platform::PolyglotArtifactAdmissionResult>);

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::filesystem::path unique_temp_root() {
    const auto nonce = std::chrono::steady_clock::now()
                           .time_since_epoch()
                           .count();
    return std::filesystem::temp_directory_path() /
        ("copperfin_polyglot_artifact_admission_" +
         std::to_string(nonce));
}

std::string utf8_path(const std::filesystem::path& path) {
    return copperfin::platform::path_to_utf8_string(path);
}

copperfin::platform::PolyglotArtifactAdmissionRequest admission_request(
    const std::filesystem::path& artifact,
    const std::filesystem::path& allowed_root,
    std::string expected_sha256) {
    return {
        .capability_id = "interop.sample-v1",
        .process_policy = {
            .executable_name = utf8_path(artifact),
            .allowed_path_roots = {utf8_path(allowed_root)},
            .allowed_publishers = {},
            .require_trusted_signature = false},
        .expected_sha256 = std::move(expected_sha256)};
}

bool copy_executable(
    const std::filesystem::path& source,
    const std::filesystem::path& destination) {
    std::error_code error;
    std::filesystem::copy_file(
        source,
        destination,
        std::filesystem::copy_options::overwrite_existing,
        error);
    if (error) {
        return false;
    }
#ifndef _WIN32
    std::filesystem::permissions(
        destination,
        std::filesystem::perms::owner_read |
            std::filesystem::perms::owner_write |
            std::filesystem::perms::owner_exec,
        std::filesystem::perm_options::add,
        error);
#endif
    return !error;
}

void test_streaming_sha256(const std::filesystem::path& temp_root) {
    const auto empty = copperfin::security::sha256_hex_for_text("");
    expect(
        empty.ok &&
            empty.hex_digest ==
                "e3b0c44298fc1c149afbf4c8996fb924"
                "27ae41e4649b934ca495991b7852b855",
        "empty SHA-256 should match the standard vector; observed=" +
            empty.hex_digest);
    const auto abc = copperfin::security::sha256_hex_for_text("abc");
    expect(
        abc.ok &&
            abc.hex_digest ==
                "ba7816bf8f01cfea414140de5dae2223"
                "b00361a396177a9cb410ff61f20015ad",
        "short SHA-256 should match the standard vector; observed=" +
            abc.hex_digest);

    const auto million_a_path = temp_root / "million-a.bin";
    std::ofstream output(million_a_path, std::ios::binary | std::ios::trunc);
    const std::string chunk(1000U, 'a');
    for (std::size_t index = 0U; index < 1000U && output; ++index) {
        output.write(chunk.data(), static_cast<std::streamsize>(chunk.size()));
    }
    output.close();
    expect(static_cast<bool>(output), "large SHA-256 fixture should be written");
    const auto million_a =
        copperfin::security::sha256_hex_for_file(utf8_path(million_a_path));
    expect(
        million_a.ok &&
            million_a.hex_digest ==
                "cdc76e5c9914fb9281a1c7e284d73e67"
                "f1809a48a497200e046d39ccc7112cd0",
        "streaming file SHA-256 should match the million-a vector; observed=" +
            million_a.hex_digest);
}

void test_artifact_admission(const std::filesystem::path& running_executable) {
    namespace fs = std::filesystem;
    const fs::path temp_root = unique_temp_root();
    const fs::path artifact =
        temp_root /
        (std::string("candidate") + running_executable.extension().string());
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root, ignored);
    expect(!ignored, "artifact admission root should be created");
    if (ignored || !copy_executable(running_executable, artifact)) {
        expect(false, "artifact admission executable fixture should be copied");
        fs::remove_all(temp_root, ignored);
        return;
    }

    test_streaming_sha256(temp_root);
    const auto artifact_digest =
        copperfin::security::sha256_hex_for_file(utf8_path(artifact));
    expect(artifact_digest.ok, "artifact fixture should have a SHA-256 digest");
    if (!artifact_digest.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto request = admission_request(
        artifact,
        temp_root,
        artifact_digest.hex_digest);
    auto admitted = copperfin::platform::admit_polyglot_artifact(request);
    expect(
        admitted.ok() &&
            admitted.error_code() == "polyglot.artifact.admitted" &&
            admitted.capability_id() == request.capability_id &&
            admitted.artifact_sha256() == artifact_digest.hex_digest &&
            admitted.authorization().allowed,
        "matching rooted artifact identity and digest should be admitted");
    expect(
        copperfin::platform::revalidate_polyglot_artifact_admission(admitted),
        "unchanged artifact admission should revalidate");

    auto invalid_capability = request;
    invalid_capability.capability_id = "Interop.Sample";
    const auto invalid_capability_result =
        copperfin::platform::admit_polyglot_artifact(invalid_capability);
    expect(
        !invalid_capability_result.ok() &&
            invalid_capability_result.error_code() ==
                "polyglot.artifact.invalid_capability_id",
        "noncanonical capability id should fail before authorization");

    auto invalid_digest = request;
    invalid_digest.expected_sha256[0] = 'A';
    const auto invalid_digest_result =
        copperfin::platform::admit_polyglot_artifact(invalid_digest);
    expect(
        !invalid_digest_result.ok() &&
            invalid_digest_result.error_code() ==
                "polyglot.artifact.invalid_expected_sha256",
        "uppercase digest should fail canonical admission");

    auto missing_root = request;
    missing_root.process_policy.allowed_path_roots.clear();
    const auto missing_root_result =
        copperfin::platform::admit_polyglot_artifact(missing_root);
    expect(
        !missing_root_result.ok() &&
            missing_root_result.error_code() ==
                "polyglot.artifact.allowed_root_required",
        "artifact admission should require an explicit allowed root on every platform");

    auto wrong_root = request;
    wrong_root.process_policy.allowed_path_roots = {
        utf8_path(temp_root / "sibling")};
    const auto wrong_root_result =
        copperfin::platform::admit_polyglot_artifact(wrong_root);
    expect(
        !wrong_root_result.ok() &&
            wrong_root_result.error_code() ==
                "polyglot.artifact.authorization_denied" &&
            !wrong_root_result.authorization().allowed,
        "artifact outside the configured root should fail closed");

    auto wrong_digest = request;
    wrong_digest.expected_sha256[0] =
        wrong_digest.expected_sha256[0] == '0' ? '1' : '0';
    const auto wrong_digest_result =
        copperfin::platform::admit_polyglot_artifact(wrong_digest);
    expect(
        !wrong_digest_result.ok() &&
            wrong_digest_result.error_code() ==
                "polyglot.artifact.sha256_mismatch" &&
            wrong_digest_result.artifact_sha256().empty() &&
            !wrong_digest_result.authorization().allowed,
        "digest mismatch should revoke authorization metadata");

    auto content_admission =
        copperfin::platform::admit_polyglot_artifact(request);
    std::ofstream append(artifact, std::ios::binary | std::ios::app);
    append.write("changed", 7);
    append.close();
    expect(
        static_cast<bool>(append),
        "artifact content-change fixture should be written");
    expect(
        !copperfin::platform::revalidate_polyglot_artifact_admission(
            content_admission) &&
            !content_admission.ok() &&
            content_admission.error_code() ==
                "polyglot.artifact.contents_changed_before_execution" &&
            content_admission.artifact_sha256().empty() &&
            !content_admission.authorization().allowed,
        "in-place artifact content change should revoke admission");
    const std::string content_change_error = content_admission.error_code();
    expect(
        !copperfin::platform::revalidate_polyglot_artifact_admission(
            content_admission) &&
            content_admission.error_code() == content_change_error,
        "revoked admission should remain revoked without losing its reason");

    expect(
        copy_executable(running_executable, artifact),
        "artifact fixture should be restored after content-change case");
    const auto restored_digest =
        copperfin::security::sha256_hex_for_file(utf8_path(artifact));
    auto identity_admission = copperfin::platform::admit_polyglot_artifact(
        admission_request(artifact, temp_root, restored_digest.hex_digest));
    fs::remove(artifact, ignored);
    expect(!ignored, "artifact replacement fixture should remove admitted identity");
    expect(
        copy_executable(running_executable, artifact),
        "artifact replacement fixture should create a new physical identity");
    expect(
        !copperfin::platform::revalidate_polyglot_artifact_admission(
            identity_admission) &&
            identity_admission.error_code() ==
                "polyglot.artifact.changed_before_execution",
        "physical artifact replacement should revoke admission even for identical bytes");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main(int argc, char** argv) {
    const std::filesystem::path running_executable =
        copperfin::platform::resolve_running_executable_path(
            argc > 0 && argv[0] != nullptr
                ? std::filesystem::path(argv[0])
                : std::filesystem::path{});
    expect(
        !running_executable.empty(),
        "running test executable path should resolve");
    if (!running_executable.empty()) {
        test_artifact_admission(running_executable);
    }
    if (failures != 0) {
        std::cerr << failures
                  << " polyglot artifact admission test(s) failed\n";
        return 1;
    }
    std::cout << "All polyglot artifact admission tests passed\n";
    return 0;
}
