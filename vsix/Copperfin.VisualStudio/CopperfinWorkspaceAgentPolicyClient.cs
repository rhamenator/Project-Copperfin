// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Web.Script.Serialization;

namespace Copperfin.VisualStudio;

internal static class CopperfinWorkspaceAgentPolicyClient
{
    // Implements RQ-CF-AGENT-003: strict, read-only managed consumption.
    private const string InvalidContract = "workspace-agent-policy.invalid-contract";
    private const string HostMissing = "workspace-agent-policy.host-missing";
    private const string HostFailed = "workspace-agent-policy.host-failed";
    private const string HostTimedOut = "workspace-agent-policy.host-timed-out";

    public static CopperfinWorkspaceAgentPolicyResult TryLoad(
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return Failure(HostMissing, localization.Text("AssetEditor.Dialog.StudioHostMissing"));
        }

        var startInfo = CopperfinStudioHostBridge.CreateProcessStartInfo(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildWorkspaceAgentPolicyArguments(),
            localization: localization,
            redirectOutput: true,
            createNoWindow: true);
        var processResult = CopperfinProcessRunner.Run(startInfo, timeoutMilliseconds: 15000);
        if (!processResult.Started)
        {
            return Failure(HostFailed, localization.Text("AssetEditor.Dialog.StudioHostCouldNotStart"));
        }
        if (processResult.TimedOut)
        {
            return Failure(HostTimedOut, localization.Text("AssetEditor.Dialog.StudioHostTimedOut"));
        }
        if (processResult.ExitCode != 0)
        {
            var error = string.IsNullOrWhiteSpace(processResult.StandardError)
                ? processResult.StandardOutput.Trim()
                : processResult.StandardError.Trim();
            return Failure(HostFailed, error);
        }

        return TryParse(processResult.StandardOutput);
    }

    internal static CopperfinWorkspaceAgentPolicyResult TryParse(string json)
    {
        try
        {
            if (!CopperfinStrictJsonMemberValidator.HasValidUniqueMembers(json))
            {
                return Failure(InvalidContract,
                    "The workspace-agent policy JSON is malformed or contains duplicate members.");
            }
            var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 };
            var shapeError = ValidateShape(serializer.DeserializeObject(json));
            if (!string.IsNullOrEmpty(shapeError))
            {
                return Failure(InvalidContract, shapeError);
            }
            var descriptor = serializer.Deserialize<CopperfinWorkspaceAgentPolicyDescriptor>(json);
            var validationError = Validate(descriptor);
            if (!string.IsNullOrEmpty(validationError))
            {
                return Failure(InvalidContract, validationError);
            }

            return new CopperfinWorkspaceAgentPolicyResult
            {
                Success = true,
                Descriptor = descriptor
            };
        }
        catch (InvalidOperationException ex)
        {
            return Failure(InvalidContract, ex.Message);
        }
        catch (ArgumentException ex)
        {
            return Failure(InvalidContract, ex.Message);
        }
    }

    private static string ValidateShape(object? value)
    {
        if (value is not IDictionary<string, object> root ||
            !HasExactKeys(root,
                "schemaVersion", "status", "descriptorOnly", "activationAvailable", "defaultMode",
                "featureEnabledByDefault", "providerAuthenticationGrantsLocalAuthority", "activation",
                "unrestrictedWarning", "modes") ||
            !HasType<int>(root, "schemaVersion") || !HasType<string>(root, "status") ||
            !HasType<bool>(root, "descriptorOnly") || !HasType<bool>(root, "activationAvailable") ||
            !HasType<string>(root, "defaultMode") || !HasType<bool>(root, "featureEnabledByDefault") ||
            !HasType<bool>(root, "providerAuthenticationGrantsLocalAuthority"))
        {
            return "The workspace-agent policy envelope has missing, unknown, or incorrectly typed fields.";
        }

        if (root["activation"] is not IDictionary<string, object> activation ||
            !HasExactKeys(activation, "permissionId", "trustedProductUiRequired", "auditSinkRequired",
                "unrestrictedWarningRequired", "privilegeElevationAllowed") ||
            !HasType<string>(activation, "permissionId") ||
            !HasType<bool>(activation, "trustedProductUiRequired") ||
            !HasType<bool>(activation, "auditSinkRequired") ||
            !HasType<bool>(activation, "unrestrictedWarningRequired") ||
            !HasType<bool>(activation, "privilegeElevationAllowed"))
        {
            return "The workspace-agent activation descriptor has missing, unknown, or incorrectly typed fields.";
        }

        if (root["unrestrictedWarning"] is not IDictionary<string, object> warning ||
            !HasExactKeys(warning, "id", "title", "body", "acknowledgement") ||
            !HasType<string>(warning, "id") || !HasType<string>(warning, "title") ||
            !HasType<string>(warning, "body") || !HasType<string>(warning, "acknowledgement"))
        {
            return "The workspace-agent warning descriptor has missing, unknown, or incorrectly typed fields.";
        }

        if (root["modes"] is not object[] modes || modes.Length != 3)
        {
            return "The workspace-agent modes field is missing, incorrectly typed, or has the wrong cardinality.";
        }
        foreach (var valueMode in modes)
        {
            if (valueMode is not IDictionary<string, object> mode ||
                !HasExactKeys(mode, "name", "capabilities") || !HasType<string>(mode, "name") ||
                mode["capabilities"] is not IDictionary<string, object> capabilities ||
                !HasExactKeys(capabilities, "readWorkspaceFiles", "writeWorkspaceFiles", "runLocalProcesses",
                    "accessOutsideWorkspace", "useNetwork", "elevatePrivileges") ||
                !capabilities.Keys.All(key => capabilities[key] is bool))
            {
                return "A workspace-agent mode has missing, unknown, or incorrectly typed fields.";
            }
        }

        return string.Empty;
    }

    private static bool HasType<T>(IDictionary<string, object> values, string key)
    {
        return values.TryGetValue(key, out var value) && value is T;
    }

    private static bool HasExactKeys(IDictionary<string, object> values, params string[] expected)
    {
        return values.Count == expected.Length && expected.All(values.ContainsKey);
    }

    private static string Validate(CopperfinWorkspaceAgentPolicyDescriptor? descriptor)
    {
        if (descriptor is null || descriptor.SchemaVersion != 1 ||
            !string.Equals(descriptor.Status, "ok", StringComparison.Ordinal) ||
            !descriptor.DescriptorOnly || descriptor.ActivationAvailable ||
            !string.Equals(descriptor.DefaultMode, "advisory", StringComparison.Ordinal) ||
            descriptor.FeatureEnabledByDefault || descriptor.ProviderAuthenticationGrantsLocalAuthority)
        {
            return "The workspace-agent policy envelope is not the supported descriptor-only schema.";
        }

        var activation = descriptor.Activation;
        if (activation is null ||
            !string.Equals(activation.PermissionId, "ai.workspace_agent", StringComparison.Ordinal) ||
            !activation.TrustedProductUiRequired || !activation.AuditSinkRequired ||
            !activation.UnrestrictedWarningRequired || activation.PrivilegeElevationAllowed)
        {
            return "The workspace-agent activation gates do not match the required fail-closed contract.";
        }

        var warning = descriptor.UnrestrictedWarning;
        if (warning is null ||
            !string.Equals(warning.Id, "workspace-agent.unrestricted-local.v1", StringComparison.Ordinal) ||
            string.IsNullOrWhiteSpace(warning.Title) || string.IsNullOrWhiteSpace(warning.Body) ||
            string.IsNullOrWhiteSpace(warning.Acknowledgement))
        {
            return "The current unrestricted-local warning contract is missing or invalid.";
        }

        if (descriptor.Modes is null || descriptor.Modes.Count != 3)
        {
            return "The workspace-agent policy must contain exactly three access modes.";
        }

        var modes = new Dictionary<string, CopperfinWorkspaceAgentCapabilities>(StringComparer.Ordinal);
        foreach (var mode in descriptor.Modes)
        {
            if (mode is null || mode.Capabilities is null || string.IsNullOrEmpty(mode.Name) ||
                modes.ContainsKey(mode.Name))
            {
                return "The workspace-agent policy contains a missing or duplicate access mode.";
            }
            modes.Add(mode.Name, mode.Capabilities);
        }

        if (!HasCapabilities(modes, "advisory", false, false, false, false, false) ||
            !HasCapabilities(modes, "workspace_sandbox", true, true, true, false, false) ||
            !HasCapabilities(modes, "unrestricted_local", true, true, true, true, true) ||
            modes.Keys.Any(name => name != "advisory" && name != "workspace_sandbox" && name != "unrestricted_local"))
        {
            return "The workspace-agent access modes or capabilities are not the supported contract.";
        }

        return string.Empty;
    }

    private static bool HasCapabilities(
        IReadOnlyDictionary<string, CopperfinWorkspaceAgentCapabilities> modes,
        string name,
        bool readWorkspaceFiles,
        bool writeWorkspaceFiles,
        bool runLocalProcesses,
        bool accessOutsideWorkspace,
        bool useNetwork)
    {
        return modes.TryGetValue(name, out var capabilities) &&
               capabilities.ReadWorkspaceFiles == readWorkspaceFiles &&
               capabilities.WriteWorkspaceFiles == writeWorkspaceFiles &&
               capabilities.RunLocalProcesses == runLocalProcesses &&
               capabilities.AccessOutsideWorkspace == accessOutsideWorkspace &&
               capabilities.UseNetwork == useNetwork &&
               !capabilities.ElevatePrivileges;
    }

    private static CopperfinWorkspaceAgentPolicyResult Failure(string diagnosticCode, string error)
    {
        return new CopperfinWorkspaceAgentPolicyResult
        {
            Success = false,
            DiagnosticCode = diagnosticCode,
            Error = error
        };
    }
}
