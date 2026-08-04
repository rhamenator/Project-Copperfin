
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
    private static void SmokeAssetEditorDistributeHorizontallyObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorDistributeLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDistributeHorizontalLabelObjectHostResponseJson());
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
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[1].Selected = true;
            objectListView.Items[1].Focused = true;
            objectListView.Items[2].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value", "last.value" }) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   distributeHorizontalButton.Visible &&
                   distributeHorizontalButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), "1900", StringComparison.Ordinal),
                "A label distribute-horizontal smoke should start from a three-object live selection with the focused row selected in the shared property grid");

            distributeHorizontalButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Distributing label objects horizontally through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--distribute-object") &&
                   invocationArguments.Contains("--distribution-mode") &&
                   invocationArguments.Contains("horizontal") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Count(argument => string.Equals(argument, "--distribute-target-unique-id", StringComparison.Ordinal)) == 3 &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("last-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Distributing label objects horizontally through the shared asset editor should send one invariant distribute-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var distributedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Distributed objects horizontally. Snapshot loaded: 3 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.SelectedItems.Count == 3 &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.SubItems[2].Text, "7", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), "2500", StringComparison.Ordinal) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   distributeHorizontalButton.Visible &&
                   distributeHorizontalButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   distributedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(distributedObject, "HPOS"), "2500", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 7 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Distributing label objects horizontally through the shared asset editor should preserve focused multi-selection continuity and refresh the distributed geometry");
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

    private static void SmokeAssetEditorDistributeVerticallyObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorDistributeVerticalLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDistributeVerticalLabelObjectHostResponseJson());
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
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var distributeVerticalButton = GetPrivateButton(control, "distributeVerticalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[1].Selected = true;
            objectListView.Items[1].Focused = true;
            objectListView.Items[2].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value", "last.value" }) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   distributeHorizontalButton.Visible &&
                   distributeHorizontalButton.Enabled &&
                   distributeVerticalButton.Visible &&
                   distributeVerticalButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), "2600", StringComparison.Ordinal),
                "A label distribute-vertical smoke should start from a three-object live selection with the focused row selected in the shared property grid");

            distributeVerticalButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Distributing label objects vertically through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--distribute-object") &&
                   invocationArguments.Contains("--distribution-mode") &&
                   invocationArguments.Contains("vertical") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Count(argument => string.Equals(argument, "--distribute-target-unique-id", StringComparison.Ordinal)) == 3 &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("last-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Distributing label objects vertically through the shared asset editor should send one invariant distribute-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var distributedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Distributed objects vertically. Snapshot loaded: 3 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.SelectedItems.Count == 3 &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.SubItems[2].Text, "7", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), "3200", StringComparison.Ordinal) &&
                   alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   distributeHorizontalButton.Visible &&
                   distributeHorizontalButton.Enabled &&
                   distributeVerticalButton.Visible &&
                   distributeVerticalButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   distributedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(distributedObject, "UNIQUEID"), "middle-field-guid", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(distributedObject, "VPOS"), "3200", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(distributedObject, "HPOS"), "1900", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 7 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Distributing label objects vertically through the shared asset editor should preserve focused multi-selection continuity and refresh the distributed geometry");
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

    private static void SmokeAssetEditorSnapVerticallyObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorSnapLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildSnapVerticalLabelObjectHostResponseJson());
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
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var snapHorizontalButton = GetPrivateButton(control, "snapHorizontalObjectButton");
            var snapVerticalButton = GetPrivateButton(control, "snapVerticalObjectButton");
            var snapToGridButton = GetPrivateButton(control, "snapToGridObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[0].Focused = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "snap.value" }) &&
                   !alignLeftButton.Visible &&
                   !distributeHorizontalButton.Visible &&
                   snapHorizontalButton.Visible &&
                   snapHorizontalButton.Enabled &&
                   snapVerticalButton.Visible &&
                   snapVerticalButton.Enabled &&
                   snapToGridButton.Visible &&
                   snapToGridButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), "1901", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), "2605", StringComparison.Ordinal),
                "A label snap-vertical smoke should start from a single live selection with grid-backed geometry exposed in the shared property grid");

            snapVerticalButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Snapping a label object vertically through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--snap-object") &&
                   invocationArguments.Contains("--snap-mode") &&
                   invocationArguments.Contains("vertical") &&
                   invocationArguments.Contains("--grid-width") &&
                   invocationArguments.Contains("12") &&
                   invocationArguments.Contains("--grid-height") &&
                   invocationArguments.Contains("12") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Count(argument => string.Equals(argument, "--snap-target-unique-id", StringComparison.Ordinal)) == 1 &&
                   invocationArguments.Contains("snap-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Snapping a label object vertically through the shared asset editor should send one invariant snap-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var snappedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Snapped objects to the vertical report grid. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.SelectedItems.Count == 1 &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.SubItems[2].Text, "7", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), "1901", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), "2604", StringComparison.Ordinal) &&
                   snapHorizontalButton.Visible &&
                   snapHorizontalButton.Enabled &&
                   snapVerticalButton.Visible &&
                   snapVerticalButton.Enabled &&
                   snapToGridButton.Visible &&
                   snapToGridButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   snappedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(snappedObject, "UNIQUEID"), "snap-field-guid", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(snappedObject, "HPOS"), "1901", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(snappedObject, "VPOS"), "2604", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 7 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Snapping a label object vertically through the shared asset editor should preserve live section/object continuity and refresh vertical geometry only");
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

    private static void SmokeAssetEditorSnapHorizontallyObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorSnapLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildSnapHorizontalLabelObjectHostResponseJson());
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
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var snapHorizontalButton = GetPrivateButton(control, "snapHorizontalObjectButton");
            var snapToGridButton = GetPrivateButton(control, "snapToGridObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[0].Focused = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "snap.value" }) &&
                   !alignLeftButton.Visible &&
                   !distributeHorizontalButton.Visible &&
                   snapHorizontalButton.Visible &&
                   snapHorizontalButton.Enabled &&
                   snapToGridButton.Visible &&
                   snapToGridButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), "1901", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), "2605", StringComparison.Ordinal),
                "A label snap-horizontal smoke should start from a single live selection with grid-backed geometry exposed in the shared property grid");

            snapHorizontalButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Snapping a label object horizontally through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--snap-object") &&
                   invocationArguments.Contains("--snap-mode") &&
                   invocationArguments.Contains("horizontal") &&
                   invocationArguments.Contains("--grid-width") &&
                   invocationArguments.Contains("12") &&
                   invocationArguments.Contains("--grid-height") &&
                   invocationArguments.Contains("12") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Count(argument => string.Equals(argument, "--snap-target-unique-id", StringComparison.Ordinal)) == 1 &&
                   invocationArguments.Contains("snap-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Snapping a label object horizontally through the shared asset editor should send one invariant snap-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var snappedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Snapped objects to the horizontal report grid. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.SelectedItems.Count == 1 &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.SubItems[2].Text, "7", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), "1908", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), "2605", StringComparison.Ordinal) &&
                   snapHorizontalButton.Visible &&
                   snapHorizontalButton.Enabled &&
                   snapToGridButton.Visible &&
                   snapToGridButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   snappedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(snappedObject, "UNIQUEID"), "snap-field-guid", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(snappedObject, "HPOS"), "1908", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(snappedObject, "VPOS"), "2605", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 7 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Snapping a label object horizontally through the shared asset editor should preserve live section/object continuity and refresh horizontal geometry only");
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

    private static void SmokeAssetEditorSnapToGridObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorSnapLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildSnapToGridLabelObjectHostResponseJson());
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
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var snapToGridButton = GetPrivateButton(control, "snapToGridObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            objectListView.Items[0].Focused = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "snap.value" }) &&
                   !alignLeftButton.Visible &&
                   !distributeHorizontalButton.Visible &&
                   snapToGridButton.Visible &&
                   snapToGridButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), "1901", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), "2605", StringComparison.Ordinal),
                "A label snap-to-grid smoke should start from a single live selection with grid-backed geometry exposed in the shared property grid");

            snapToGridButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Snapping a label object to grid through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--snap-object") &&
                   invocationArguments.Contains("--snap-mode") &&
                   invocationArguments.Contains("both") &&
                   invocationArguments.Contains("--grid-width") &&
                   invocationArguments.Contains("12") &&
                   invocationArguments.Contains("--grid-height") &&
                   invocationArguments.Contains("12") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Count(argument => string.Equals(argument, "--snap-target-unique-id", StringComparison.Ordinal)) == 1 &&
                   invocationArguments.Contains("snap-field-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Snapping a label object to grid through the shared asset editor should send one invariant snap-object command through the host contract");

            var refreshedSnapshot = GetCurrentSnapshot(control);
            var snappedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == 7);
            Expect(HasLabelTextContaining(control, "Snapped objects to the report grid. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.SelectedItems.Count == 1 &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.SubItems[2].Text, "7", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 7 &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), "1908", StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), "2604", StringComparison.Ordinal) &&
                   snapToGridButton.Visible &&
                   snapToGridButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   snappedObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(snappedObject, "HPOS"), "1908", StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(snappedObject, "VPOS"), "2604", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 7 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Snapping a label object to grid through the shared asset editor should preserve live selection continuity and refresh snapped geometry");
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
