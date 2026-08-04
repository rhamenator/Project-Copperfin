
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
    private static void SmokeAssetEditorDeletedReportSettingsMissingColorHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingColorSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingColorHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "COLOR") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing COLOR host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing COLOR.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["COLOR"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing COLOR host-update smoke should surface the blank editable COLOR property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 1);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "COLOR", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing COLOR");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("COLOR") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted COLOR update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "COLOR"), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing COLOR");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingColSpacingHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingColSpacingSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingColSpacingHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "COLSPACING") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing COLSPACING host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing COLSPACING.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["COLSPACING"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing COLSPACING host-update smoke should surface the blank editable COLSPACING property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 240);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "COLSPACING", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing COLSPACING");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("COLSPACING") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("240"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted COLSPACING update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "COLSPACING"), "240", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing COLSPACING");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingColWidthHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingColWidthSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingColWidthHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "COLWIDTH") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing COLWIDTH host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing COLWIDTH.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["COLWIDTH"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing COLWIDTH host-update smoke should surface the blank editable COLWIDTH property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 3000);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "COLWIDTH", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing COLWIDTH");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("COLWIDTH") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("3000"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted COLWIDTH update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "COLWIDTH"), "3000", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing COLWIDTH");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingColsHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingColsSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingColsHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "COLS") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing COLS host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing COLS.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["COLS"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing COLS host-update smoke should surface the blank editable COLS property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 4);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "COLS", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing COLS");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("COLS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("4"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted COLS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "COLS"), "4", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing COLS");
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
