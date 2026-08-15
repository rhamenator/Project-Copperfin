// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_process_parser.h"

#include <algorithm>
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

std::optional<PhysicalPathContainmentResult> capture_binding(
    const WorkspaceAgentWindowsProcessParserBinding& binding) {
    const auto& path = binding.trusted_absolute_executable;
    if (!supported_contract(binding.contract) || path.empty() ||
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
    return captured;
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
                return prior.canonical_executable == captured->canonical_path ||
                    (prior.identity.storage_id == captured->identity.storage_id &&
                     prior.identity.file_id == captured->identity.file_id);
            });
        if (duplicate) {
            return std::nullopt;
        }
        captured_bindings.push_back({
            .canonical_executable = captured->canonical_path,
            .identity = captured->identity,
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

    return {
        .allowed = true,
        .contract = binding->contract,
        .diagnostic_code = "workspace_agent.process_argument_parser_allowed"};
}

}  // namespace copperfin::security
