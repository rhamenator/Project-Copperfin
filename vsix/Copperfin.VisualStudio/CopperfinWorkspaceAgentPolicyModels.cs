// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System.Collections.Generic;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinWorkspaceAgentCapabilities
{
    public bool ReadWorkspaceFiles { get; set; }
    public bool WriteWorkspaceFiles { get; set; }
    public bool RunLocalProcesses { get; set; }
    public bool AccessOutsideWorkspace { get; set; }
    public bool UseNetwork { get; set; }
    public bool ElevatePrivileges { get; set; }
}

internal sealed class CopperfinWorkspaceAgentModeDescriptor
{
    public string Name { get; set; } = string.Empty;
    public CopperfinWorkspaceAgentCapabilities? Capabilities { get; set; }
}

internal sealed class CopperfinWorkspaceAgentActivationDescriptor
{
    public string PermissionId { get; set; } = string.Empty;
    public bool TrustedProductUiRequired { get; set; }
    public bool AuditSinkRequired { get; set; }
    public bool UnrestrictedWarningRequired { get; set; }
    public bool PrivilegeElevationAllowed { get; set; }
}

internal sealed class CopperfinWorkspaceAgentWarningDescriptor
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Body { get; set; } = string.Empty;
    public string Acknowledgement { get; set; } = string.Empty;
}

internal sealed class CopperfinWorkspaceAgentPolicyDescriptor
{
    public int SchemaVersion { get; set; }
    public string Status { get; set; } = string.Empty;
    public bool DescriptorOnly { get; set; }
    public bool ActivationAvailable { get; set; }
    public string DefaultMode { get; set; } = string.Empty;
    public bool FeatureEnabledByDefault { get; set; }
    public bool ProviderAuthenticationGrantsLocalAuthority { get; set; }
    public CopperfinWorkspaceAgentActivationDescriptor? Activation { get; set; }
    public CopperfinWorkspaceAgentWarningDescriptor? UnrestrictedWarning { get; set; }
    public List<CopperfinWorkspaceAgentModeDescriptor> Modes { get; set; } = new();
}

internal sealed class CopperfinWorkspaceAgentPolicyResult
{
    public bool Success { get; set; }
    public string DiagnosticCode { get; set; } = string.Empty;
    public string Error { get; set; } = string.Empty;
    public CopperfinWorkspaceAgentPolicyDescriptor? Descriptor { get; set; }
}
