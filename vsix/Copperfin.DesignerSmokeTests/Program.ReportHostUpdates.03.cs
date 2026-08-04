
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
    private static void SmokeAssetEditorDeletedReportSettingsMissingPropertyGridHostUpdate(
        string propertyName,
        object updatedPropertyValue,
        string expectedUpdatedSelectionValue,
        Func<string> buildUpdatedHostResponseJson)
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingMarginsSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, buildUpdatedHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, propertyName) ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                $"A deleted report settings missing {propertyName} host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException($"Could not read the selected deleted report settings from the shared asset editor for missing {propertyName}.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)[propertyName];
            Expect(propertyDescriptor is not null,
                $"A deleted report settings missing {propertyName} host-update smoke should surface the blank editable property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, updatedPropertyValue);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                $"Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing {propertyName}");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains(propertyName) &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains(expectedUpdatedSelectionValue),
                $"Editing deleted report settings through the shared asset editor should send one invariant deleted {propertyName} update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, propertyName), expectedUpdatedSelectionValue, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                $"Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing {propertyName}");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingTagHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingTagSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingTagHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "TAG") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"] is null &&
                   TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"] is null &&
                   TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"] is null &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing TAG host-update smoke should start from a deleted settings-rooted property-grid selection without derived sort metadata");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing TAG.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["TAG"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing TAG host-update smoke should surface the blank editable TAG property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, "deleted.customer.region");
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "TAG", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing TAG");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("TAG") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("deleted.customer.region"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted TAG update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "TAG"), "deleted.customer.region", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.region", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "31", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity and surface derived sort metadata after adding missing TAG");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingDefaultSourceHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingDefaultSourceSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingDefaultSourceHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "DEFAULTSOURCE") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing DEFAULTSOURCE host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing DEFAULTSOURCE.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["DEFAULTSOURCE"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing DEFAULTSOURCE host-update smoke should surface the blank editable DEFAULTSOURCE property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 17);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "DEFAULTSOURCE", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing DEFAULTSOURCE");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("DEFAULTSOURCE") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("17"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted DEFAULTSOURCE update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "DEFAULTSOURCE"), "17", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing DEFAULTSOURCE");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingDriverHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingDriverSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingDriverHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "DRIVER") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing DRIVER host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing DRIVER.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["DRIVER"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing DRIVER host-update smoke should surface the blank editable DRIVER property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, "deleted.cups");
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "DRIVER", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing DRIVER");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("DRIVER") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("deleted.cups"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted DRIVER update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "DRIVER"), "deleted.cups", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing DRIVER");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingDeviceHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingDeviceSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingDeviceHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "DEVICE") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing DEVICE host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing DEVICE.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["DEVICE"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing DEVICE host-update smoke should surface the blank editable DEVICE property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, "Deleted Queue B");
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "DEVICE", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing DEVICE");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("DEVICE") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("Deleted Queue B"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted DEVICE update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "DEVICE"), "Deleted Queue B", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing DEVICE");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingOutputHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingOutputSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingOutputHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "OUTPUT") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing OUTPUT host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing OUTPUT.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["OUTPUT"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing OUTPUT host-update smoke should surface the blank editable OUTPUT property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, "DeletedOutput.prn");
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "OUTPUT", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing OUTPUT");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("OUTPUT") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("DeletedOutput.prn"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted OUTPUT update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OUTPUT"), "DeletedOutput.prn", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing OUTPUT");
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
