// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_tool_registry.h"

#include <array>
#include <cstddef>

namespace copperfin::security {

namespace {

constexpr std::array<WorkspaceAgentToolDefinition, 7U> product_tools{{
    {
        workspace_agent_tool_workspace_inspect,
        {.read_workspace_files = true},
        WorkspaceAgentToolTargetKind::workspace_file
    },
    {
        workspace_agent_tool_workspace_apply_edit,
        {.read_workspace_files = true, .write_workspace_files = true},
        WorkspaceAgentToolTargetKind::workspace_file
    },
    {
        workspace_agent_tool_workspace_run_process,
        {
            .read_workspace_files = true,
            .write_workspace_files = true,
            .run_local_processes = true
        },
        WorkspaceAgentToolTargetKind::workspace_process
    },
    {
        workspace_agent_tool_local_inspect,
        {.read_workspace_files = true, .access_outside_workspace = true},
        WorkspaceAgentToolTargetKind::local_file
    },
    {
        workspace_agent_tool_local_apply_edit,
        {
            .read_workspace_files = true,
            .write_workspace_files = true,
            .access_outside_workspace = true
        },
        WorkspaceAgentToolTargetKind::local_file
    },
    {
        workspace_agent_tool_local_run_process,
        {
            .read_workspace_files = true,
            .write_workspace_files = true,
            .run_local_processes = true,
            .access_outside_workspace = true,
            .use_network = true
        },
        WorkspaceAgentToolTargetKind::local_process
    },
    {
        workspace_agent_tool_network_request,
        {.use_network = true},
        WorkspaceAgentToolTargetKind::network_endpoint
    }
}};

constexpr bool is_lowercase_ascii_letter(char value) noexcept {
    return value >= 'a' && value <= 'z';
}

constexpr bool is_ascii_digit(char value) noexcept {
    return value >= '0' && value <= '9';
}

constexpr bool is_valid_tool_id(std::string_view id) noexcept {
    if (id.empty() || !is_lowercase_ascii_letter(id.front()) || id.back() == '.') {
        return false;
    }
    bool segment_start = false;
    for (const char character : id) {
        if (character == '.') {
            if (segment_start) {
                return false;
            }
            segment_start = true;
            continue;
        }
        if (segment_start && !is_lowercase_ascii_letter(character)) {
            return false;
        }
        if (!is_lowercase_ascii_letter(character) && !is_ascii_digit(character) &&
            character != '_') {
            return false;
        }
        segment_start = false;
    }
    return true;
}

constexpr bool has_requirement(const WorkspaceAgentToolRequirements& requirements) noexcept {
    return requirements.read_workspace_files ||
        requirements.write_workspace_files ||
        requirements.run_local_processes ||
        requirements.access_outside_workspace ||
        requirements.use_network ||
        requirements.elevate_privileges;
}

constexpr bool target_kind_matches_requirements(
    const WorkspaceAgentToolDefinition& definition) noexcept {
    const auto& requirements = definition.requirements;
    switch (definition.target_kind) {
        case WorkspaceAgentToolTargetKind::workspace_file:
            return requirements.read_workspace_files &&
                !requirements.run_local_processes &&
                !requirements.access_outside_workspace &&
                !requirements.use_network;
        case WorkspaceAgentToolTargetKind::workspace_process:
            return requirements.read_workspace_files &&
                requirements.write_workspace_files &&
                requirements.run_local_processes &&
                !requirements.access_outside_workspace &&
                !requirements.use_network;
        case WorkspaceAgentToolTargetKind::local_file:
            return requirements.read_workspace_files &&
                !requirements.run_local_processes &&
                requirements.access_outside_workspace &&
                !requirements.use_network;
        case WorkspaceAgentToolTargetKind::local_process:
            return requirements.read_workspace_files &&
                requirements.write_workspace_files &&
                requirements.run_local_processes &&
                requirements.access_outside_workspace &&
                requirements.use_network;
        case WorkspaceAgentToolTargetKind::network_endpoint:
            return requirements.use_network &&
                !requirements.read_workspace_files &&
                !requirements.write_workspace_files &&
                !requirements.run_local_processes &&
                !requirements.access_outside_workspace;
    }
    return false;
}

constexpr bool registry_is_valid() noexcept {
    for (std::size_t index = 0U; index < product_tools.size(); ++index) {
        const auto& definition = product_tools[index];
        if (!is_valid_tool_id(definition.id) ||
            !has_requirement(definition.requirements) ||
            !target_kind_matches_requirements(definition) ||
            definition.requirements.elevate_privileges) {
            return false;
        }
        for (std::size_t prior = 0U; prior < index; ++prior) {
            if (product_tools[prior].id == definition.id) {
                return false;
            }
        }
    }
    return true;
}

static_assert(
    registry_is_valid(),
    "The product-owned workspace-agent tool registry must have unique canonical "
    "identifiers, coherent nonempty requirements and target kinds, and no "
    "elevation capability.");

}  // namespace

std::span<const WorkspaceAgentToolDefinition>
workspace_agent_product_tool_definitions() noexcept {
    return product_tools;
}

const WorkspaceAgentToolDefinition* find_workspace_agent_product_tool(
    std::string_view id) noexcept {
    for (const auto& definition : product_tools) {
        if (definition.id == id) {
            return &definition;
        }
    }
    return nullptr;
}

}  // namespace copperfin::security
