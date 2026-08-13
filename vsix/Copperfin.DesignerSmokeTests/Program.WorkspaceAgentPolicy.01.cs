// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static readonly string WorkspaceAgentPolicyJson = "{" +
        "\"schemaVersion\":1,\"status\":\"ok\",\"descriptorOnly\":true," +
        "\"activationAvailable\":false,\"defaultMode\":\"advisory\"," +
        "\"featureEnabledByDefault\":false,\"providerAuthenticationGrantsLocalAuthority\":false," +
        "\"activation\":{\"permissionId\":\"ai.workspace_agent\",\"trustedProductUiRequired\":true," +
        "\"auditSinkRequired\":true,\"unrestrictedWarningRequired\":true,\"privilegeElevationAllowed\":false}," +
        "\"unrestrictedWarning\":{\"id\":\"workspace-agent.unrestricted-local.v1\"," +
        "\"title\":\"[!! localized title !!]\",\"body\":\"[!! localized body !!]\"," +
        "\"acknowledgement\":\"[!! localized acknowledgement !!]\"}," +
        "\"modes\":[" +
        "{\"name\":\"advisory\",\"capabilities\":{" + CapabilityJson(false, false, false, false, false) + "}}," +
        "{\"name\":\"workspace_sandbox\",\"capabilities\":{" + CapabilityJson(true, true, true, false, false) + "}}," +
        "{\"name\":\"unrestricted_local\",\"capabilities\":{" + CapabilityJson(true, true, true, true, true) + "}}]}";

    private static string CapabilityJson(bool read, bool write, bool run, bool outside, bool network)
    {
        return $"\"readWorkspaceFiles\":{JsonBool(read)},\"writeWorkspaceFiles\":{JsonBool(write)}," +
               $"\"runLocalProcesses\":{JsonBool(run)},\"accessOutsideWorkspace\":{JsonBool(outside)}," +
               $"\"useNetwork\":{JsonBool(network)},\"elevatePrivileges\":false";
    }

    private static string JsonBool(bool value) => value ? "true" : "false";

    private static void SmokeManagedWorkspaceAgentPolicyContract()
    {
        // Verification of RQ-CF-AGENT-003.
        Expect(string.Equals(
                CopperfinStudioHostBridge.BuildWorkspaceAgentPolicyArguments(),
                "--workspace-agent-policy --json",
                StringComparison.Ordinal),
            "managed workspace-agent client should use the descriptor-only host grammar");

        var valid = CopperfinWorkspaceAgentPolicyClient.TryParse(WorkspaceAgentPolicyJson);
        Expect(valid.Success && valid.Descriptor is not null &&
               valid.Descriptor.Modes.Count == 3 && !valid.Descriptor.ActivationAvailable,
            "managed workspace-agent client should accept the exact descriptor-only policy contract with localized prose");

        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"schemaVersion\":1", "\"schemaVersion\":2"),
            "unsupported schema versions");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"featureEnabledByDefault\":false,", string.Empty),
            "omitted false-valued security fields");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"descriptorOnly\":true", "\"descriptorOnly\":\"true\""),
            "incorrectly typed security fields");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"schemaVersion\":1", "\"schemaVersion\":1,\"unexpected\":true"),
            "unknown envelope fields");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"descriptorOnly\":true", "\"descriptorOnly\":false,\"descriptorOnly\":true"),
            "duplicate envelope members");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"permissionId\":\"ai.workspace_agent\"", "\"permissionId\":\"developer\",\"permissionId\":\"ai.workspace_agent\""),
            "duplicate activation members");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"id\":\"workspace-agent.unrestricted-local.v1\"", "\"id\":\"stale\",\"id\":\"workspace-agent.unrestricted-local.v1\""),
            "duplicate warning members");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"name\":\"advisory\"", "\"name\":\"other\",\"name\":\"advisory\""),
            "duplicate mode members");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"readWorkspaceFiles\":false", "\"readWorkspaceFiles\":true,\"readWorkspaceFiles\":false"),
            "duplicate capability members");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"descriptorOnly\":true", "\"descriptorOnly\":false,\"descriptor\\u004fnly\":true"),
            "escaped duplicate member names");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"descriptorOnly\":true", "\"descriptorOnly\":false"),
            "non-descriptor endpoints");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"activationAvailable\":false", "\"activationAvailable\":true"),
            "host-side activation claims");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"providerAuthenticationGrantsLocalAuthority\":false", "\"providerAuthenticationGrantsLocalAuthority\":true"),
            "provider authentication as local authorization");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"permissionId\":\"ai.workspace_agent\"", "\"permissionId\":\"developer\""),
            "substituted permission identifiers");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"id\":\"workspace-agent.unrestricted-local.v1\"", "\"id\":\"workspace-agent.unrestricted-local.stale\""),
            "stale unrestricted warning identifiers");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"name\":\"workspace_sandbox\"", "\"name\":\"unrestricted_local\""),
            "duplicate access modes");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"name\":\"workspace_sandbox\"", "\"name\":\"sandbox\""),
            "unknown access-mode aliases");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"accessOutsideWorkspace\":false", "\"accessOutsideWorkspace\":true"),
            "capability escalation");
        ExpectPolicyRejected(WorkspaceAgentPolicyJson.Replace("\"elevatePrivileges\":false", "\"elevatePrivileges\":true"),
            "privilege elevation");
        ExpectPolicyRejected("not-json", "malformed JSON");
    }

    private static void ExpectPolicyRejected(string json, string description)
    {
        var result = CopperfinWorkspaceAgentPolicyClient.TryParse(json);
        Expect(!result.Success &&
               string.Equals(result.DiagnosticCode, "workspace-agent-policy.invalid-contract", StringComparison.Ordinal),
            "managed workspace-agent client should fail closed for " + description);
    }
}
