
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
    private static void SmokeAssetEditorLabelSectionPropertyGridHostUpdate()
    {
        var snapshot = BuildAssetEditorLabelSectionUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildLabelSectionUpdateHostResponseJson());
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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 42,
                "A label section host-update smoke should start from a section-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                throw new InvalidOperationException("Could not read the selected label section from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(sectionSelection)["TOP"]?.SetValue(sectionSelection, 3200);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "TOP", 2000);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a label section through the shared asset editor should invoke the Studio host exactly once");

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
                "Editing a label section through the shared asset editor should send one invariant VPOS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 42 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["TOP"]?.GetValue(refreshedSelection)?.ToString(), "3200", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a label section through the shared asset editor should preserve label identity and section-rooted continuity after the host-backed refresh");
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

    private static void SmokeAssetEditorLabelObjectPropertyGridHostUpdate()
    {
        var snapshot = BuildAssetEditorLabelObjectUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildLabelObjectUpdateHostResponseJson());
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
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A label object host-update smoke should start from an object-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected label object from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1500);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 1200);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1500"),
                "Editing a label object through the shared asset editor should send one invariant HPOS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1500", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a label object through the shared asset editor should preserve label identity, object-rooted selection, and containing section continuity after the host-backed refresh");
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

    private static void SmokeAssetEditorUnplacedLabelObjectPropertyGridHostUpdate()
    {
        var snapshot = BuildAssetEditorUnplacedLabelObjectUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildUnplacedLabelObjectUpdateHostResponseJson());
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
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9,
                "An unplaced label object host-update smoke should start from an object-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected unplaced label object from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1100);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 800);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing an unplaced label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("9") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1100"),
                "Editing an unplaced label object through the shared asset editor should send one invariant HPOS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1100", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing an unplaced label object through the shared asset editor should preserve label identity, unplaced object selection, and unplaced-scope continuity after the host-backed refresh");
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

    private static void SmokeAssetEditorLabelObjectPlacementIntoUnplacedRefreshesContinuity()
    {
        var snapshot = BuildAssetEditorLabelObjectUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildLabelObjectPlacementIntoUnplacedHostResponseJson());
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
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A label placement-transition smoke should start from a section-contained object-rooted property-grid selection");
            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label placement-transition smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected label object for the placement-transition smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["VPOS"]?.SetValue(objectSelection, 9000);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "VPOS", 2600);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Moving a label object into the unplaced tray through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("9000"),
                "Moving a label object into the unplaced tray through the shared asset editor should send one invariant VPOS update through the host property contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1200 T 9000 R 5200 B 9500   Size: 4000 x 500") &&
                   HasLabelTextContaining(control, "Unplaced objects: 1") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["VPOS"]?.GetValue(refreshedSelection)?.ToString(), "9000", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Moving a label object into the unplaced tray through the shared asset editor should refresh the shell summary and preserve record-rooted continuity in the new unplaced scope");
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

    private static void SmokeAssetEditorUnplacedLabelObjectHostUpdateRefreshesShellSummary()
    {
        var snapshot = BuildAssetEditorUnplacedLabelObjectUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildUnplacedLabelObjectPreviewRefreshHostResponseJson());
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
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9,
                "An unplaced label object summary-refresh smoke should start from an object-rooted property-grid selection");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "An unplaced label object summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected unplaced label object for the summary-refresh smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1100);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 800);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing an unplaced label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("9") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1100"),
                "Editing an unplaced label object through the shared asset editor should send one invariant HPOS update through the host property contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1100 T 700 R 5200 B 3100   Size: 4100 x 2400") &&
                   HasLabelTextContaining(control, "Unplaced objects: 1") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1100", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing an unplaced label object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving label identity, unplaced-object selection, and unplaced-scope continuity");
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

    private static void SmokeAssetEditorDeletedLabelObjectHostUpdateRefreshesShellSummary()
    {
        var snapshot = BuildAssetEditorDeletedLabelObjectUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildDeletedLabelObjectPreviewRefreshHostResponseJson());
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
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A deleted label object summary-refresh smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted label object for the summary-refresh smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1600);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 1400);
            Application.DoEvents();

            var deletedSection = snapshot.ReportLayout?.DeletedSections[0]
                ?? throw new InvalidOperationException("Could not read the deleted label section from the shared smoke snapshot.");
            var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1400 T 9400 R 5000 B 10000   Size: 3600 x 600") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1600", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a deleted label object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving label identity, deleted-object selection, and deleted-section continuity");
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

    private static void SmokeAssetEditorUndoRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorLabelUndoPreviewRefreshSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildLabelUndoPreviewRefreshHostResponseJson());
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
            SetCurrentSnapshot(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9 &&
                   control.CanHandleUndoCommand() &&
                   string.Equals(control.GetUndoCommandText(), "Undo Move orphan.note", StringComparison.Ordinal),
                "A label undo summary-refresh smoke should start from an undo-capable unplaced label object selection");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label undo summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            Expect(control.TryHandleUndoCommand(),
                "The shared asset editor should accept an undo command for an unplaced label selection when the snapshot exposes a host-backed undo label");
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Undoing through the shared label asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--undo-mode") &&
                   invocationArguments.Contains("command") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Undoing through the shared label asset editor should send one invariant undo command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1100 T 700 R 5200 B 3100   Size: 4100 x 2400") &&
                   HasLabelTextContaining(control, "Unplaced objects: 1") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1100", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Undoing through the shared label asset editor should refresh the shell summary from the returned snapshot while preserving label identity, unplaced-object selection, and unplaced-scope continuity");
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
