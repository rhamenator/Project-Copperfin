// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class StudioWorkspaceAgentPolicyDialog : Form
{
    private sealed class ModeItem
    {
        internal ModeItem(CopperfinWorkspaceAgentModeDescriptor descriptor, string displayName)
        {
            Descriptor = descriptor;
            DisplayName = displayName;
        }

        internal CopperfinWorkspaceAgentModeDescriptor Descriptor { get; }
        internal string DisplayName { get; }
        public override string ToString() => DisplayName;
    }

    private readonly CopperfinWorkspaceAgentPolicyDescriptor descriptor;
    private readonly CopperfinLocalization localization;
    private readonly Label activationStatusLabel;
    private readonly ComboBox modeSelector;
    private readonly TextBox detailsTextBox;

    internal StudioWorkspaceAgentPolicyDialog(
        CopperfinWorkspaceAgentPolicyDescriptor descriptor,
        CopperfinLocalization localization)
    {
        this.descriptor = descriptor ?? throw new ArgumentNullException(nameof(descriptor));
        this.localization = localization ?? throw new ArgumentNullException(nameof(localization));

        Text = localization.Text("Studio.WorkspaceAgent.Title");
        Width = 720;
        Height = 610;
        MinimumSize = new Size(620, 500);
        StartPosition = FormStartPosition.CenterParent;
        ShowInTaskbar = false;

        activationStatusLabel = new Label
        {
            AccessibleName = localization.Text("Studio.WorkspaceAgent.ActivationUnavailable"),
            AutoSize = true,
            Dock = DockStyle.Top,
            Font = new Font(Font, FontStyle.Bold),
            Padding = new Padding(0, 0, 0, 8),
            Text = localization.Text("Studio.WorkspaceAgent.ActivationUnavailable")
        };
        var modeLabel = new Label
        {
            AutoSize = true,
            Dock = DockStyle.Top,
            Padding = new Padding(0, 8, 0, 4),
            Text = localization.Text("Studio.WorkspaceAgent.ModeLabel")
        };
        modeSelector = new ComboBox
        {
            AccessibleName = localization.Text("Studio.WorkspaceAgent.ModeLabel"),
            Dock = DockStyle.Top,
            DropDownStyle = ComboBoxStyle.DropDownList
        };
        foreach (var mode in descriptor.Modes)
        {
            modeSelector.Items.Add(new ModeItem(mode, ModeDisplayName(mode.Name)));
        }
        modeSelector.SelectedIndexChanged += (_, _) => RefreshDetails();

        detailsTextBox = new TextBox
        {
            AccessibleName = localization.Text("Studio.WorkspaceAgent.Capabilities"),
            Dock = DockStyle.Fill,
            Multiline = true,
            ReadOnly = true,
            ScrollBars = ScrollBars.Vertical,
            BackColor = SystemColors.Window,
            Font = new Font(FontFamily.GenericMonospace, Font.Size)
        };

        var closeButton = new Button
        {
            AccessibleName = localization.Text("Studio.WorkspaceAgent.Close"),
            AutoSize = true,
            DialogResult = DialogResult.OK,
            Text = localization.Text("Studio.WorkspaceAgent.Close")
        };
        var buttonPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Bottom,
            FlowDirection = FlowDirection.RightToLeft,
            Padding = new Padding(0, 10, 0, 0)
        };
        buttonPanel.Controls.Add(closeButton);

        var content = new Panel
        {
            Dock = DockStyle.Fill,
            Padding = new Padding(16)
        };
        content.Controls.Add(detailsTextBox);
        content.Controls.Add(modeSelector);
        content.Controls.Add(modeLabel);
        content.Controls.Add(activationStatusLabel);
        content.Controls.Add(buttonPanel);
        Controls.Add(content);
        AcceptButton = closeButton;
        CancelButton = closeButton;

        var defaultIndex = descriptor.Modes.FindIndex(mode =>
            string.Equals(mode.Name, descriptor.DefaultMode, StringComparison.Ordinal));
        modeSelector.SelectedIndex = defaultIndex >= 0 ? defaultIndex : 0;
    }

    internal int ModeCount => modeSelector.Items.Count;
    internal string SelectedModeName =>
        (modeSelector.SelectedItem as ModeItem)?.Descriptor.Name ?? string.Empty;
    internal string DetailsText => detailsTextBox.Text;
    internal string ActivationStatusText => activationStatusLabel.Text;
    internal string ModeSelectorAccessibleName => modeSelector.AccessibleName;
    internal string DetailsAccessibleName => detailsTextBox.AccessibleName;

    internal void SelectModeForTest(string name)
    {
        for (var index = 0; index < modeSelector.Items.Count; ++index)
        {
            if (modeSelector.Items[index] is ModeItem item &&
                string.Equals(item.Descriptor.Name, name, StringComparison.Ordinal))
            {
                modeSelector.SelectedIndex = index;
                return;
            }
        }
    }

    private string ModeDisplayName(string name)
    {
        return name switch
        {
            "advisory" => localization.Text("Studio.WorkspaceAgent.Mode.Advisory"),
            "workspace_sandbox" => localization.Text("Studio.WorkspaceAgent.Mode.WorkspaceSandbox"),
            "unrestricted_local" => localization.Text("Studio.WorkspaceAgent.Mode.UnrestrictedLocal"),
            _ => name
        };
    }

    private void RefreshDetails()
    {
        if (modeSelector.SelectedItem is not ModeItem selected || selected.Descriptor.Capabilities is null)
        {
            detailsTextBox.Clear();
            return;
        }

        var capabilities = selected.Descriptor.Capabilities;
        var lines = new List<string>
        {
            localization.Text("Studio.WorkspaceAgent.Capabilities"),
            CapabilityLine("ReadWorkspaceFiles", capabilities.ReadWorkspaceFiles),
            CapabilityLine("WriteWorkspaceFiles", capabilities.WriteWorkspaceFiles),
            CapabilityLine("RunLocalProcesses", capabilities.RunLocalProcesses),
            CapabilityLine("AccessOutsideWorkspace", capabilities.AccessOutsideWorkspace),
            CapabilityLine("UseNetwork", capabilities.UseNetwork),
            CapabilityLine("ElevatePrivileges", capabilities.ElevatePrivileges)
        };
        if (string.Equals(selected.Descriptor.Name, "unrestricted_local", StringComparison.Ordinal))
        {
            var warning = descriptor.UnrestrictedWarning!;
            lines.Add(string.Empty);
            lines.Add(localization.Text("Studio.WorkspaceAgent.Warning"));
            lines.Add(warning.Title);
            lines.Add(warning.Body);
            lines.Add(warning.Acknowledgement);
        }
        detailsTextBox.Text = string.Join(Environment.NewLine, lines);
    }

    private string CapabilityLine(string key, bool enabled)
    {
        var value = localization.Text(enabled
            ? "AssetEditor.Summary.Boolean.True"
            : "AssetEditor.Summary.Boolean.False");
        return localization.Format(
            "Studio.WorkspaceAgent.CapabilityLine",
            localization.Text("Studio.WorkspaceAgent.Capability." + key),
            value);
    }
}
