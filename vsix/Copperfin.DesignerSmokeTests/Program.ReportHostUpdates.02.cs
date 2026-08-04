
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
    private static void SmokeAssetEditorDeletedReportSettingsMissingGridVHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingGridVSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingGridVHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            var sectionListView = GetPrivateListView(control, "sectionListView");
            var deletedSettingsItem = sectionListView.Items.Cast<ListViewItem>()
                .First(item => string.Equals(item.Text, "Settings (deleted)", StringComparison.Ordinal));
            deletedSettingsItem.Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 0 &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SETTINGSSTATE"]?.GetValue(initialSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "GRIDV") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing GRIDV host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing GRIDV.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["GRIDV"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing GRIDV host-update smoke should surface the blank editable GRIDV property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 6);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "GRIDV", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing GRIDV");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("GRIDV") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("6"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted GRIDV update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "GRIDV"), "6", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing GRIDV");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedReportSettingsMissingGridHHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingGridHSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingGridHHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            var sectionListView = GetPrivateListView(control, "sectionListView");
            var deletedSettingsItem = sectionListView.Items.Cast<ListViewItem>()
                .First(item => string.Equals(item.Text, "Settings (deleted)", StringComparison.Ordinal));
            deletedSettingsItem.Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 0 &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SETTINGSSTATE"]?.GetValue(initialSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "GRIDH") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing GRIDH host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing GRIDH.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["GRIDH"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing GRIDH host-update smoke should surface the blank editable GRIDH property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 10);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "GRIDH", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing GRIDH");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("GRIDH") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("10"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted GRIDH update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "GRIDH"), "10", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing GRIDH");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedReportSettingsMissingPaperLengthHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingPaperLengthSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingPaperLengthHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            var sectionListView = GetPrivateListView(control, "sectionListView");
            var deletedSettingsItem = sectionListView.Items.Cast<ListViewItem>()
                .First(item => string.Equals(item.Text, "Settings (deleted)", StringComparison.Ordinal));
            deletedSettingsItem.Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 0 &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SETTINGSSTATE"]?.GetValue(initialSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "PAPERLENGTH") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing PAPERLENGTH host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing PAPERLENGTH.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["PAPERLENGTH"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing PAPERLENGTH host-update smoke should surface the blank editable PAPERLENGTH property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 5588);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "PAPERLENGTH", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing PAPERLENGTH");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("PAPERLENGTH") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("5588"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted PAPERLENGTH update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "PAPERLENGTH"), "5588", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing PAPERLENGTH");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedReportSettingsMissingPaperWidthHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingPaperWidthSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingPaperWidthHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            var sectionListView = GetPrivateListView(control, "sectionListView");
            var deletedSettingsItem = sectionListView.Items.Cast<ListViewItem>()
                .First(item => string.Equals(item.Text, "Settings (deleted)", StringComparison.Ordinal));
            deletedSettingsItem.Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 0 &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SETTINGSSTATE"]?.GetValue(initialSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "PAPERWIDTH") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing PAPERWIDTH host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing PAPERWIDTH.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["PAPERWIDTH"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing PAPERWIDTH host-update smoke should surface the blank editable PAPERWIDTH property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 4318);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "PAPERWIDTH", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing PAPERWIDTH");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("PAPERWIDTH") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("4318"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted PAPERWIDTH update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "PAPERWIDTH"), "4318", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing PAPERWIDTH");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedReportSettingsMissingLeftMarginHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingLeftMarginSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingLeftMarginHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            var sectionListView = GetPrivateListView(control, "sectionListView");
            var deletedSettingsItem = sectionListView.Items.Cast<ListViewItem>()
                .First(item => string.Equals(item.Text, "Settings (deleted)", StringComparison.Ordinal));
            deletedSettingsItem.Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 0 &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SETTINGSSTATE"]?.GetValue(initialSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "LEFTMARGIN") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing LEFTMARGIN host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing LEFTMARGIN.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["LEFTMARGIN"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing LEFTMARGIN host-update smoke should surface the blank editable LEFTMARGIN property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 50);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "LEFTMARGIN", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing LEFTMARGIN");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("LEFTMARGIN") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("50"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted LEFTMARGIN update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "LEFTMARGIN"), "50", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing LEFTMARGIN");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedReportSettingsMissingRightMarginHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingRightMarginSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingRightMarginHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            var sectionListView = GetPrivateListView(control, "sectionListView");
            var deletedSettingsItem = sectionListView.Items.Cast<ListViewItem>()
                .First(item => string.Equals(item.Text, "Settings (deleted)", StringComparison.Ordinal));
            deletedSettingsItem.Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 0 &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SETTINGSSTATE"]?.GetValue(initialSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "RIGHTMARGIN") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing RIGHTMARGIN host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing RIGHTMARGIN.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["RIGHTMARGIN"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing RIGHTMARGIN host-update smoke should surface the blank editable RIGHTMARGIN property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 60);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "RIGHTMARGIN", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing RIGHTMARGIN");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("RIGHTMARGIN") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("60"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted RIGHTMARGIN update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "RIGHTMARGIN"), "60", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing RIGHTMARGIN");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

}
