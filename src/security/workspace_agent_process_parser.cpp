// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_process_parser.h"

#include "copperfin/security/sha256.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <system_error>
#include <utility>

namespace copperfin::security {

namespace {

bool path_has_embedded_nul(const std::filesystem::path& path) {
    const auto& native = path.native();
    return native.find(typename std::filesystem::path::value_type{}) !=
        std::filesystem::path::string_type::npos;
}

bool supported_contract(
    WorkspaceAgentProcessArgumentParserContract contract) noexcept {
    return contract ==
        WorkspaceAgentProcessArgumentParserContract::windows_c_runtime_argv_v1;
}

bool supported_dependency_contract(
    const WorkspaceAgentProcessParserDependencyContract contract) noexcept {
    return contract == WorkspaceAgentProcessParserDependencyContract::
        self_contained_launch_image_v1;
}

bool valid_sha256(const std::string_view digest) noexcept {
    return digest.size() == 64U &&
        std::all_of(digest.begin(), digest.end(), [](const char character) {
            return (character >= '0' && character <= '9') ||
                (character >= 'a' && character <= 'f');
        });
}

bool constant_time_equal(
    const std::string_view left,
    const std::string_view right) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
    std::uint8_t difference = 0U;
    for (std::size_t index = 0U; index < left.size(); ++index) {
        difference |= static_cast<std::uint8_t>(left[index]) ^
            static_cast<std::uint8_t>(right[index]);
    }
    return difference == 0U;
}

struct CapturedExecutable {
    PhysicalPathContainmentResult containment;
    std::string sha256;
};

std::optional<CapturedExecutable> capture_binding(
    const WorkspaceAgentWindowsProcessParserBinding& binding) {
    const auto& path = binding.trusted_absolute_executable;
    if (!supported_contract(binding.contract) ||
        !supported_dependency_contract(binding.dependency_contract) ||
        !valid_sha256(binding.expected_sha256) || path.empty() ||
        path_has_embedded_nul(path) || !path.is_absolute() ||
        path.parent_path().empty()) {
        return std::nullopt;
    }
    auto captured = inspect_physical_path_containment(path, path.parent_path());
    std::error_code filesystem_error;
    if (!captured.allowed || captured.identity.link_count != 1U ||
        captured.identity != binding.expected_identity ||
        !std::filesystem::is_regular_file(
            captured.canonical_path, filesystem_error) || filesystem_error) {
        return std::nullopt;
    }
    const auto snapshot = read_physically_contained_file_snapshot(
        captured,
        path.parent_path(),
        workspace_agent_maximum_windows_process_parser_image_bytes);
    if (!snapshot.ok) {
        return std::nullopt;
    }
    const auto digest = sha256_hex_for_text(snapshot.bytes);
    if (!digest.ok ||
        !constant_time_equal(digest.hex_digest, binding.expected_sha256)) {
        return std::nullopt;
    }
    return CapturedExecutable{
        .containment = snapshot.containment,
        .sha256 = digest.hex_digest};
}

WorkspaceAgentProcessParserAuthorization denied(std::string diagnostic_code) {
    WorkspaceAgentProcessParserAuthorization result;
    result.diagnostic_code = std::move(diagnostic_code);
    return result;
}

}  // namespace

WorkspaceAgentProcessParserBoundary::WorkspaceAgentProcessParserBoundary(
    std::vector<CapturedBinding> bindings)
    : bindings_(std::move(bindings)) {}

std::optional<WorkspaceAgentProcessParserBoundary>
WorkspaceAgentProcessParserBoundary::create(
    const WorkspaceAgentProcessParserConfiguration& configuration) {
    if (configuration.schema_version != 1U ||
        configuration.windows_bindings.empty() ||
        configuration.windows_bindings.size() >
            workspace_agent_maximum_windows_process_parser_bindings) {
        return std::nullopt;
    }

    std::vector<CapturedBinding> captured_bindings;
    captured_bindings.reserve(configuration.windows_bindings.size());
    for (const auto& binding : configuration.windows_bindings) {
        const auto captured = capture_binding(binding);
        if (!captured.has_value()) {
            return std::nullopt;
        }
        const bool duplicate = std::any_of(
            captured_bindings.begin(),
            captured_bindings.end(),
            [&](const CapturedBinding& prior) {
                return prior.canonical_executable ==
                        captured->containment.canonical_path ||
                    (prior.identity.storage_id ==
                         captured->containment.identity.storage_id &&
                     prior.identity.file_id ==
                         captured->containment.identity.file_id);
            });
        if (duplicate) {
            return std::nullopt;
        }
        captured_bindings.push_back({
            .canonical_executable = captured->containment.canonical_path,
            .identity = captured->containment.identity,
            .sha256 = captured->sha256,
            .contract = binding.contract});
    }
    return WorkspaceAgentProcessParserBoundary(std::move(captured_bindings));
}

WorkspaceAgentProcessParserAuthorization
WorkspaceAgentProcessParserBoundary::authorize_windows(
    const std::filesystem::path& canonical_executable,
    const PhysicalPathIdentity& executable_identity) const {
    const auto binding = std::find_if(
        bindings_.begin(), bindings_.end(), [&](const CapturedBinding& candidate) {
            return candidate.canonical_executable == canonical_executable;
        });
    if (binding == bindings_.end() ||
        binding->identity != executable_identity) {
        return denied("workspace_agent.process_argument_parser_not_trusted");
    }

    const auto current = inspect_physical_path_containment(
        binding->canonical_executable,
        binding->canonical_executable.parent_path());
    if (!current.allowed || current.canonical_path != binding->canonical_executable ||
        current.identity != binding->identity) {
        return denied("workspace_agent.process_argument_parser_identity_changed");
    }

    const auto snapshot = read_physically_contained_file_snapshot(
        current,
        binding->canonical_executable.parent_path(),
        workspace_agent_maximum_windows_process_parser_image_bytes);
    if (!snapshot.ok) {
        return denied("workspace_agent.process_argument_parser_identity_changed");
    }
    const auto digest = sha256_hex_for_text(snapshot.bytes);
    if (!digest.ok || !constant_time_equal(digest.hex_digest, binding->sha256)) {
        return denied("workspace_agent.process_argument_parser_contents_changed");
    }

    return {
        .allowed = true,
        .contract = binding->contract,
        .diagnostic_code = "workspace_agent.process_argument_parser_allowed"};
}

}  // namespace copperfin::security
