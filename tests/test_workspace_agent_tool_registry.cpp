// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_tool_registry.h"
#include "copperfin/security/workspace_agent_session.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace {

using copperfin::security::WorkspaceAgentToolDefinition;
using copperfin::security::WorkspaceAgentToolRequirements;

int failures = 0;

template <typename T>
concept HasCallerSelectedRequirements = requires(T value) {
    value.requirements;
};

static_assert(
    !HasCallerSelectedRequirements<
        copperfin::security::WorkspaceAgentToolPreflightRequest>,
    "RQ-CF-AGENT-008: the public preflight request must not expose caller-selected requirements");

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

bool equal_requirements(
    const WorkspaceAgentToolRequirements& left,
    const WorkspaceAgentToolRequirements& right) {
    return left.read_workspace_files == right.read_workspace_files &&
        left.write_workspace_files == right.write_workspace_files &&
        left.run_local_processes == right.run_local_processes &&
        left.access_outside_workspace == right.access_outside_workspace &&
        left.use_network == right.use_network &&
        left.elevate_privileges == right.elevate_privileges;
}

void test_exact_product_inventory_and_requirements() {
    struct ExpectedDefinition {
        std::string_view id;
        WorkspaceAgentToolRequirements requirements;
    };
    const std::array<ExpectedDefinition, 7U> expected{{
        {
            copperfin::security::workspace_agent_tool_workspace_inspect,
            {.read_workspace_files = true}
        },
        {
            copperfin::security::workspace_agent_tool_workspace_apply_edit,
            {.read_workspace_files = true, .write_workspace_files = true}
        },
        {
            copperfin::security::workspace_agent_tool_workspace_run_process,
            {
                .read_workspace_files = true,
                .write_workspace_files = true,
                .run_local_processes = true
            }
        },
        {
            copperfin::security::workspace_agent_tool_local_inspect,
            {.read_workspace_files = true, .access_outside_workspace = true}
        },
        {
            copperfin::security::workspace_agent_tool_local_apply_edit,
            {
                .read_workspace_files = true,
                .write_workspace_files = true,
                .access_outside_workspace = true
            }
        },
        {
            copperfin::security::workspace_agent_tool_local_run_process,
            {
                .read_workspace_files = true,
                .write_workspace_files = true,
                .run_local_processes = true,
                .access_outside_workspace = true,
                .use_network = true
            }
        },
        {
            copperfin::security::workspace_agent_tool_network_request,
            {.use_network = true}
        }
    }};

    const auto definitions =
        copperfin::security::workspace_agent_product_tool_definitions();
    expect(definitions.size() == expected.size(),
           "RQ-CF-AGENT-008: the product registry must expose only its exact inventory");
    if (definitions.size() != expected.size()) {
        return;
    }
    for (std::size_t index = 0U; index < expected.size(); ++index) {
        expect(definitions[index].id == expected[index].id &&
                   equal_requirements(
                       definitions[index].requirements,
                       expected[index].requirements),
               "RQ-CF-AGENT-008: every stable tool id must retain its complete product-owned requirements");
        expect(!definitions[index].requirements.elevate_privileges,
               "RQ-CF-AGENT-008: no registered tool may request privilege elevation");
    }
}

void test_lookup_is_exact_and_fail_closed() {
    for (const WorkspaceAgentToolDefinition& definition :
         copperfin::security::workspace_agent_product_tool_definitions()) {
        expect(
            copperfin::security::find_workspace_agent_product_tool(definition.id) ==
                &definition,
            "RQ-CF-AGENT-008: exact stable ids must resolve to immutable product definitions");
    }

    for (const std::string_view invalid : {
             "",
             "workspace.inspect",
             "Workspace.inspect.v1",
             "workspace.inspect.v1 ",
             " workspace.inspect.v1",
             "workspace.inspect.v2",
             "workspace/inspect/v1",
             "provider.custom.v1"}) {
        expect(copperfin::security::find_workspace_agent_product_tool(invalid) == nullptr,
               "RQ-CF-AGENT-008: aliases, unknown versions, and provider-defined ids must fail closed");
    }

    const std::string embedded_nul{"workspace.inspect.v1\0suffix", 27U};
    expect(copperfin::security::find_workspace_agent_product_tool(embedded_nul) == nullptr,
           "RQ-CF-AGENT-008: an embedded-NUL id must not match its visible prefix");
}

}  // namespace

int main() {
    test_exact_product_inventory_and_requirements();
    test_lookup_is_exact_and_fail_closed();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
