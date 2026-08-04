
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
    private static void SmokeAssetEditorAlignLeftObjectCommandRefreshesReportShellSummary()
    {
        var snapshot = BuildAssetEditorReorderFrontReportObjectSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildAlignLeftReportObjectHostResponseJson());
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
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var alignLeftButton = GetPrivateButton(control, "alignLeftObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[0].Focused = true;
            objectListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value", "last.value" }) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), "1000", StringComparison.Ordinal),
                "A report align-left smoke should start from a multi-selection with the anchor row selected in the shared property grid");

            alignLeftButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Aligning report objects left through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--align-object") &&
                   invocationArguments.Contains("--alignment-mode") &&
                   invocationArguments.Contains("left") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--anchor-unique-id") &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("--align-target-unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Aligning report objects left through the shared asset editor should send one invariant align-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var alignedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Aligned objects left. Snapshot loaded: 3 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.SubItems[2].Text, "6", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), "1000", StringComparison.Ordinal) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   alignedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(alignedObject, "HPOS"), "1000", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Aligning report objects left through the shared asset editor should preserve anchor selection continuity and refresh the aligned target geometry");
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

    private static void SmokeAssetEditorAlignTopObjectCommandRefreshesReportShellSummary()
    {
        var snapshot = BuildAssetEditorAlignTopReportObjectSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildAlignTopReportObjectHostResponseJson());
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
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var alignLeftButton = GetPrivateButton(control, "alignLeftObjectButton");
            var alignTopButton = GetPrivateButton(control, "alignTopObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[0].Focused = true;
            objectListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value", "last.value" }) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   alignTopButton.Visible &&
                   alignTopButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), "2400", StringComparison.Ordinal),
                "A report align-top smoke should start from a multi-selection with the anchor row selected in the shared property grid");

            alignTopButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Aligning report objects top through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--align-object") &&
                   invocationArguments.Contains("--alignment-mode") &&
                   invocationArguments.Contains("top") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--anchor-unique-id") &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("--align-target-unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Aligning report objects top through the shared asset editor should send one invariant align-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var alignedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Aligned objects top. Snapshot loaded: 3 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.SubItems[2].Text, "6", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), "2400", StringComparison.Ordinal) &&
                   alignTopButton.Visible &&
                   alignTopButton.Enabled &&
                   alignedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(alignedObject, "VPOS"), "2400", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(alignedObject, "HPOS"), "2100", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Aligning report objects top through the shared asset editor should preserve anchor selection continuity and refresh the aligned target geometry");
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

    private static void SmokeAssetEditorMatchSizeObjectCommandRefreshesReportShellSummary()
    {
        var snapshot = BuildAssetEditorResizeReportObjectSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildResizeToAnchorReportObjectHostResponseJson());
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
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var alignLeftButton = GetPrivateButton(control, "alignLeftObjectButton");
            var matchSizeButton = GetPrivateButton(control, "matchSizeObjectButton");
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[0].Focused = true;
            objectListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Take(2).Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value" }) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   matchSizeButton.Visible &&
                   matchSizeButton.Enabled &&
                   !distributeHorizontalButton.Visible &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "WIDTH"), "3200", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HEIGHT"), "700", StringComparison.Ordinal),
                "A report match-size smoke should start from a multi-selection with the anchor row selected in the shared property grid");

            matchSizeButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Matching report object sizes through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--resize-object") &&
                   invocationArguments.Contains("--resize-mode") &&
                   invocationArguments.Contains("size") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--anchor-unique-id") &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("--resize-target-unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Matching report object sizes through the shared asset editor should send one invariant resize-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var resizedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Matched objects to the anchor size. Snapshot loaded: 2 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.SelectedItems.Count == 2 &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.SubItems[2].Text, "6", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "WIDTH"), "3200", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HEIGHT"), "700", StringComparison.Ordinal) &&
                   matchSizeButton.Visible &&
                   matchSizeButton.Enabled &&
                   resizedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(resizedObject, "WIDTH"), "3200", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(resizedObject, "HEIGHT"), "700", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Matching report object sizes through the shared asset editor should preserve anchor selection continuity and refresh the resized target geometry");
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

    private static void SmokeAssetEditorMatchWidthObjectCommandRefreshesReportShellSummary()
    {
        var snapshot = BuildAssetEditorResizeReportObjectSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildResizeToAnchorWidthReportObjectHostResponseJson());
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
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var alignLeftButton = GetPrivateButton(control, "alignLeftObjectButton");
            var matchWidthButton = GetPrivateButton(control, "matchWidthObjectButton");
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[0].Focused = true;
            objectListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Take(2).Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value" }) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   matchWidthButton.Visible &&
                   matchWidthButton.Enabled &&
                   !distributeHorizontalButton.Visible &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "WIDTH"), "3200", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HEIGHT"), "700", StringComparison.Ordinal),
                "A report match-width smoke should start from a multi-selection with the anchor row selected in the shared property grid");

            matchWidthButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Matching report object widths through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--resize-object") &&
                   invocationArguments.Contains("--resize-mode") &&
                   invocationArguments.Contains("width") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--anchor-unique-id") &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("--resize-target-unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Matching report object widths through the shared asset editor should send one invariant resize-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var resizedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Matched objects to the anchor width. Snapshot loaded: 2 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.SelectedItems.Count == 2 &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.SubItems[2].Text, "6", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "WIDTH"), "3200", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HEIGHT"), "700", StringComparison.Ordinal) &&
                   matchWidthButton.Visible &&
                   matchWidthButton.Enabled &&
                   resizedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(resizedObject, "WIDTH"), "3200", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(resizedObject, "HEIGHT"), "900", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Matching report object widths through the shared asset editor should preserve anchor selection continuity and refresh the resized target geometry");
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

    private static void SmokeAssetEditorMatchHeightObjectCommandRefreshesReportShellSummary()
    {
        var snapshot = BuildAssetEditorResizeReportObjectSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildResizeToAnchorHeightReportObjectHostResponseJson());
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
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var alignLeftButton = GetPrivateButton(control, "alignLeftObjectButton");
            var matchHeightButton = GetPrivateButton(control, "matchHeightObjectButton");
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[0].Focused = true;
            objectListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Take(2).Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value" }) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   matchHeightButton.Visible &&
                   matchHeightButton.Enabled &&
                   !distributeHorizontalButton.Visible &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "WIDTH"), "3200", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HEIGHT"), "700", StringComparison.Ordinal),
                "A report match-height smoke should start from a multi-selection with the anchor row selected in the shared property grid");

            matchHeightButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Matching report object heights through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--resize-object") &&
                   invocationArguments.Contains("--resize-mode") &&
                   invocationArguments.Contains("height") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--anchor-unique-id") &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("--resize-target-unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Matching report object heights through the shared asset editor should send one invariant resize-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var resizedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Matched objects to the anchor height. Snapshot loaded: 2 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.SelectedItems.Count == 2 &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.SubItems[2].Text, "6", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "WIDTH"), "3200", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HEIGHT"), "700", StringComparison.Ordinal) &&
                   matchHeightButton.Visible &&
                   matchHeightButton.Enabled &&
                   resizedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(resizedObject, "WIDTH"), "4200", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(resizedObject, "HEIGHT"), "700", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Matching report object heights through the shared asset editor should preserve anchor selection continuity and refresh the resized target geometry");
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
