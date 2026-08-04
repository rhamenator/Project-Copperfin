
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
    private static void SmokeAssetEditorDeletedReportSettingsMissingPrintQualityHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingPrintQualitySmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingPrintQualityHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "PRINTQUALITY") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing PRINTQUALITY host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing PRINTQUALITY.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["PRINTQUALITY"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing PRINTQUALITY host-update smoke should surface the blank editable PRINTQUALITY property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 600);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "PRINTQUALITY", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing PRINTQUALITY");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("PRINTQUALITY") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("600"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted PRINTQUALITY update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "PRINTQUALITY"), "600", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing PRINTQUALITY");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingYResolutionHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingYResolutionSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingYResolutionHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "YRESOLUTION") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing YRESOLUTION host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing YRESOLUTION.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["YRESOLUTION"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing YRESOLUTION host-update smoke should surface the blank editable YRESOLUTION property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 600);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "YRESOLUTION", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing YRESOLUTION");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("YRESOLUTION") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("600"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted YRESOLUTION update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "YRESOLUTION"), "600", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing YRESOLUTION");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingTTOptionHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingTTOptionSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingTTOptionHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "TTOPTION") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing TTOPTION host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing TTOPTION.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["TTOPTION"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing TTOPTION host-update smoke should surface the blank editable TTOPTION property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 1);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "TTOPTION", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing TTOPTION");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("TTOPTION") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted TTOPTION update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "TTOPTION"), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing TTOPTION");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingAsciiHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingAsciiSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingAsciiHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "ASCII") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing ASCII host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing ASCII.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["ASCII"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing ASCII host-update smoke should surface the blank editable ASCII property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 8);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "ASCII", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing ASCII");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("ASCII") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("8"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted ASCII update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "ASCII"), "8", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing ASCII");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingCollateHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingCollateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingCollateHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "COLLATE") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing COLLATE host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing COLLATE.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["COLLATE"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing COLLATE host-update smoke should surface the blank editable COLLATE property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 1);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "COLLATE", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing COLLATE");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("COLLATE") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted COLLATE update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "COLLATE"), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing COLLATE");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingCopiesHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingCopiesSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingCopiesHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "COPIES") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing COPIES host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing COPIES.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["COPIES"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing COPIES host-update smoke should surface the blank editable COPIES property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 3);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "COPIES", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing COPIES");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("COPIES") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("3"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted COPIES update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "COPIES"), "3", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing COPIES");
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
