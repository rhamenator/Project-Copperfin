
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
    private static void SmokeAssetEditorReportSectionPropertyGridHostUpdate()
    {
        var snapshot = BuildAssetEditorSectionUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildSectionUpdateHostResponseJson());
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
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 42,
                "A report section host-update smoke should start from a section-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                throw new InvalidOperationException("Could not read the selected report section from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(sectionSelection)["TOP"]?.SetValue(sectionSelection, 3200);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "TOP", 2000);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a report section through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("42") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("3200"),
                "Editing a report section through the shared asset editor should send one invariant VPOS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 42 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["TOP"]?.GetValue(refreshedSelection)?.ToString(), "3200", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a report section through the shared asset editor should preserve section-rooted selection continuity after the host-backed refresh");

            Expect(string.Equals(sectionListView.SelectedItems[0].SubItems[2].Text, "3200", StringComparison.Ordinal),
                "Editing a report section through the shared asset editor should refresh the visible section geometry from the returned snapshot");
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

    private static void SmokeAssetEditorReportGroupingPropertyGridHostUpdate()
    {
        var snapshot = BuildAssetEditorGroupingUpdateSmokeSnapshot();
        var expectedGrouping = new ExpectedReportGroupingMetadata
        {
            GroupingIndex = 1,
            GroupingNestingDepth = 2,
            GroupingExpression = "customer.region",
            GroupingExpressionFieldIndex = 2,
            GroupingExpressionMemoBlockNumber = 7,
            HeaderSectionId = "group_header_7",
            HeaderRecordIndex = 41,
            HeaderDeleted = false,
            FooterSectionId = "group_footer_7",
            FooterRecordIndex = 47,
            FooterDeleted = true,
            HeaderStateDisplay = "Live",
            FooterStateDisplay = "Deleted"
        };
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
            CreateFakeStudioHostScript(scriptPath, BuildGroupingUpdateHostResponseJson());
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
            sectionListView.Items.Cast<ListViewItem>()
                .First(item => string.Equals(item.Text, "Grouping 1 - customer.country", StringComparison.Ordinal))
                .Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 41 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "GROUPINGEXPRESSION"), "customer.country", StringComparison.Ordinal),
                "A report grouping host-update smoke should start from a grouping-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection groupingSelection)
            {
                throw new InvalidOperationException("Could not read the selected report grouping from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(groupingSelection)["GROUPINGEXPRESSION"]?.SetValue(groupingSelection, "customer.region");
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "GROUPINGEXPRESSION", "customer.country");
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a report grouping through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("41") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("EXPR") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("customer.region"),
                "Editing a report grouping through the shared asset editor should send one invariant EXPR update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Grouping 1 - customer.region", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 41 &&
                   SelectionMatchesExpectedReportGrouping(refreshedSelection, expectedGrouping) &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a report grouping through the shared asset editor should preserve grouping-rooted selection continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                Directory.Delete(tempRoot, recursive: true);
            }
            catch
            {
            }
        }
    }

    private static void SmokeAssetEditorReportSettingsPropertyGridHostUpdate()
    {
        var snapshot = BuildAssetEditorSettingsUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildSettingsUpdateHostResponseJson());
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
            var settingsItem = sectionListView.Items.Cast<ListViewItem>()
                .First(item => string.Equals(item.Text, "Settings", StringComparison.Ordinal));
            settingsItem.Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 0 &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLS"]?.GetValue(initialSelection)?.ToString(), "2", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLWIDTH"]?.GetValue(initialSelection)?.ToString(), "3600", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLSPACING"]?.GetValue(initialSelection)?.ToString(), "120", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["PAPERLENGTH"]?.GetValue(initialSelection)?.ToString(), "2794", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["PAPERWIDTH"]?.GetValue(initialSelection)?.ToString(), "2159", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DRIVER"]?.GetValue(initialSelection)?.ToString(), "winspool", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DEVICE"]?.GetValue(initialSelection)?.ToString(), "FinePrint 2000", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["OUTPUT"]?.GetValue(initialSelection)?.ToString(), "FPR4:", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DEFAULTSOURCE"]?.GetValue(initialSelection)?.ToString(), "15", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["PRINTQUALITY"]?.GetValue(initialSelection)?.ToString(), "600", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["YRESOLUTION"]?.GetValue(initialSelection)?.ToString(), "600", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["TTOPTION"]?.GetValue(initialSelection)?.ToString(), "3", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLOR"]?.GetValue(initialSelection)?.ToString(), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["ASCII"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLLATE"]?.GetValue(initialSelection)?.ToString(), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COPIES"]?.GetValue(initialSelection)?.ToString(), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["LEFTMARGIN"]?.GetValue(initialSelection)?.ToString(), "15", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["RIGHTMARGIN"]?.GetValue(initialSelection)?.ToString(), "25", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "11", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["PREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 0 T 2000 R 5200 B 8100   Size: 5200 x 6100", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A report settings host-update smoke should start from a settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected report settings from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(settingsSelection)["COLWIDTH"]?.SetValue(settingsSelection, 4200);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "COLWIDTH", 3600);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing report settings through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("COLWIDTH") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("4200"),
                "Editing report settings through the shared asset editor should send one invariant column-width update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLS"]?.GetValue(refreshedSelection)?.ToString(), "2", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLWIDTH"]?.GetValue(refreshedSelection)?.ToString(), "4200", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLSPACING"]?.GetValue(refreshedSelection)?.ToString(), "120", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["PAPERLENGTH"]?.GetValue(refreshedSelection)?.ToString(), "2794", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["PAPERWIDTH"]?.GetValue(refreshedSelection)?.ToString(), "2159", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DRIVER"]?.GetValue(refreshedSelection)?.ToString(), "winspool", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DEVICE"]?.GetValue(refreshedSelection)?.ToString(), "FinePrint 2000", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OUTPUT"]?.GetValue(refreshedSelection)?.ToString(), "FPR4:", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DEFAULTSOURCE"]?.GetValue(refreshedSelection)?.ToString(), "15", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["PRINTQUALITY"]?.GetValue(refreshedSelection)?.ToString(), "600", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["YRESOLUTION"]?.GetValue(refreshedSelection)?.ToString(), "600", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["TTOPTION"]?.GetValue(refreshedSelection)?.ToString(), "3", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLOR"]?.GetValue(refreshedSelection)?.ToString(), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["ASCII"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLLATE"]?.GetValue(refreshedSelection)?.ToString(), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COPIES"]?.GetValue(refreshedSelection)?.ToString(), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["LEFTMARGIN"]?.GetValue(refreshedSelection)?.ToString(), "15", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["RIGHTMARGIN"]?.GetValue(refreshedSelection)?.ToString(), "25", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "11", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["PREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 0 T 2000 R 5200 B 8100   Size: 5200 x 6100", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing report settings through the shared asset editor should preserve column-setup settings-rooted selection continuity after the host-backed refresh");
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

    private static void SmokeAssetEditorDeletedReportSettingsPropertyGridHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsUpdateHostResponseJson());
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
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLS"]?.GetValue(initialSelection)?.ToString(), "3", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLWIDTH"]?.GetValue(initialSelection)?.ToString(), "2400", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLSPACING"]?.GetValue(initialSelection)?.ToString(), "180", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["PAPERLENGTH"]?.GetValue(initialSelection)?.ToString(), "4318", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["PAPERWIDTH"]?.GetValue(initialSelection)?.ToString(), "2794", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DRIVER"]?.GetValue(initialSelection)?.ToString(), "deleted.winspool", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DEVICE"]?.GetValue(initialSelection)?.ToString(), "Deleted Printer", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["OUTPUT"]?.GetValue(initialSelection)?.ToString(), "DPRN:", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DEFAULTSOURCE"]?.GetValue(initialSelection)?.ToString(), "16", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["PRINTQUALITY"]?.GetValue(initialSelection)?.ToString(), "1200", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["YRESOLUTION"]?.GetValue(initialSelection)?.ToString(), "1200", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["TTOPTION"]?.GetValue(initialSelection)?.ToString(), "2", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLOR"]?.GetValue(initialSelection)?.ToString(), "2", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["ASCII"]?.GetValue(initialSelection)?.ToString(), "10", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COLLATE"]?.GetValue(initialSelection)?.ToString(), "0", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["COPIES"]?.GetValue(initialSelection)?.ToString(), "2", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["LEFTMARGIN"]?.GetValue(initialSelection)?.ToString(), "35", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["RIGHTMARGIN"]?.GetValue(initialSelection)?.ToString(), "45", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(settingsSelection)["COLSPACING"]?.SetValue(settingsSelection, 240);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "COLSPACING", 180);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once");

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
                "Editing deleted report settings through the shared asset editor should send one invariant deleted column-spacing update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLS"]?.GetValue(refreshedSelection)?.ToString(), "3", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLWIDTH"]?.GetValue(refreshedSelection)?.ToString(), "2400", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLSPACING"]?.GetValue(refreshedSelection)?.ToString(), "240", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["PAPERLENGTH"]?.GetValue(refreshedSelection)?.ToString(), "4318", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["PAPERWIDTH"]?.GetValue(refreshedSelection)?.ToString(), "2794", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DRIVER"]?.GetValue(refreshedSelection)?.ToString(), "deleted.winspool", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DEVICE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Printer", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OUTPUT"]?.GetValue(refreshedSelection)?.ToString(), "DPRN:", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DEFAULTSOURCE"]?.GetValue(refreshedSelection)?.ToString(), "16", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["PRINTQUALITY"]?.GetValue(refreshedSelection)?.ToString(), "1200", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["YRESOLUTION"]?.GetValue(refreshedSelection)?.ToString(), "1200", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["TTOPTION"]?.GetValue(refreshedSelection)?.ToString(), "2", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLOR"]?.GetValue(refreshedSelection)?.ToString(), "2", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["ASCII"]?.GetValue(refreshedSelection)?.ToString(), "10", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COLLATE"]?.GetValue(refreshedSelection)?.ToString(), "0", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["COPIES"]?.GetValue(refreshedSelection)?.ToString(), "2", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["LEFTMARGIN"]?.GetValue(refreshedSelection)?.ToString(), "35", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["RIGHTMARGIN"]?.GetValue(refreshedSelection)?.ToString(), "45", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted column-setup settings-rooted continuity after the host-backed refresh");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingTopBottomMarginHostUpdate()
    {
        SmokeAssetEditorDeletedReportSettingsMissingPropertyGridHostUpdate(
            propertyName: "TOPMARGIN",
            updatedPropertyValue: 40,
            expectedUpdatedSelectionValue: "40",
            buildUpdatedHostResponseJson: BuildDeletedSettingsMissingTopMarginHostResponseJson);
        SmokeAssetEditorDeletedReportSettingsMissingPropertyGridHostUpdate(
            propertyName: "BOTMARGIN",
            updatedPropertyValue: 55,
            expectedUpdatedSelectionValue: "55",
            buildUpdatedHostResponseJson: BuildDeletedSettingsMissingBottomMarginHostResponseJson);
    }

    private static void SmokeAssetEditorDeletedReportSettingsMissingOrientationHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingOrientationSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingOrientationHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "ORIENTATION") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing ORIENTATION host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing ORIENTATION.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["ORIENTATION"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing ORIENTATION host-update smoke should surface the blank editable ORIENTATION property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 0);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "ORIENTATION", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing ORIENTATION");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("ORIENTATION") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("0"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted ORIENTATION update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "ORIENTATION"), "0", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing ORIENTATION");
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

    private static void SmokeAssetEditorDeletedReportSettingsMissingPaperSizeHostUpdate()
    {
        var snapshot = BuildAssetEditorDeletedSettingsMissingPaperSizeSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedSettingsMissingPaperSizeHostResponseJson());
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "PAPERSIZE") ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DOCUMENTTITLE"]?.GetValue(initialSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSION"]?.GetValue(initialSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONFIELD"]?.GetValue(initialSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["SORTEXPRESSIONMEMO"]?.GetValue(initialSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(initialSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
                "A deleted report settings missing PAPERSIZE host-update smoke should start from a deleted settings-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report settings from the shared asset editor for missing PAPERSIZE.");
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)["PAPERSIZE"];
            Expect(propertyDescriptor is not null,
                "A deleted report settings missing PAPERSIZE host-update smoke should surface the blank editable PAPERSIZE property");
            if (propertyDescriptor is null)
            {
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, 9);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "PAPERSIZE", string.Empty);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing deleted report settings through the shared asset editor should invoke the Studio host exactly once when adding missing PAPERSIZE");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("0") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("PAPERSIZE") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("9"),
                "Editing deleted report settings through the shared asset editor should send one invariant deleted PAPERSIZE update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 0 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SETTINGSSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "PAPERSIZE"), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DOCUMENTTITLE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSION"]?.GetValue(refreshedSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONFIELD"]?.GetValue(refreshedSelection)?.ToString(), "9", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SORTEXPRESSIONMEMO"]?.GetValue(refreshedSelection)?.ToString(), "21", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["DELETEDPREVIEWBOUNDS"]?.GetValue(refreshedSelection)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal) &&
                   objectListView.Items.Count == 0 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing deleted report settings through the shared asset editor should preserve deleted settings-rooted continuity after adding missing PAPERSIZE");
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
