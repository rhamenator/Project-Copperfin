
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
    private static void SmokeReportSurfaceObjectDragging()
    {
        using var surface = new CopperfinDesignSurfaceControl
        {
            Size = new Size(1400, 1000)
        };

        var objects = new List<CopperfinStudioSnapshotObject>
        {
            new()
            {
                RecordIndex = 6,
                Title = "customer.company",
                Subtitle = "field",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "1200" },
                    new() { Name = "VPOS", Value = "2600" },
                    new() { Name = "WIDTH", Value = "4000" },
                    new() { Name = "HEIGHT", Value = "500" },
                    new() { Name = "EXPR", Value = "customer.company" }
                }
            },
            new()
            {
                RecordIndex = 13,
                Deleted = true,
                Title = "deleted.footer.total",
                Subtitle = "field",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "1400" },
                    new() { Name = "VPOS", Value = "9400" },
                    new() { Name = "WIDTH", Value = "3600" },
                    new() { Name = "HEIGHT", Value = "600" },
                    new() { Name = "EXPR", Value = "deleted.footer.total" }
                }
            },
            new()
            {
                RecordIndex = 9,
                Title = "orphan.note",
                Subtitle = "field",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "800" },
                    new() { Name = "VPOS", Value = "700" },
                    new() { Name = "WIDTH", Value = "2400" },
                    new() { Name = "HEIGHT", Value = "450" },
                    new() { Name = "EXPR", Value = "orphan.note" }
                }
            }
        };

        var layout = new CopperfinStudioReportLayout
        {
            Sections = new List<CopperfinStudioReportSection>
            {
                new()
                {
                    Id = "detail_1",
                    Title = "Detail",
                    BandKind = "detail",
                    RecordIndex = 42,
                    Top = 2000,
                    Height = 5000,
                    Objects = new List<CopperfinStudioReportLayoutObject>
                    {
                        new()
                        {
                            RecordIndex = 6,
                            ObjectKind = "field",
                            Title = "customer.company",
                            Expression = "customer.company",
                            Left = 1200,
                            Top = 2600,
                            Width = 4000,
                            Height = 500
                        }
                    }
                }
            },
            DeletedSections = new List<CopperfinStudioReportSection>
            {
                new()
                {
                    Id = "deleted_footer",
                    Title = "Summary",
                    BandKind = "summary",
                    RecordIndex = 51,
                    Deleted = true,
                    Top = 9000,
                    Height = 1400,
                    Objects = new List<CopperfinStudioReportLayoutObject>
                    {
                        new()
                        {
                            RecordIndex = 13,
                            ObjectKind = "field",
                            Title = "deleted.footer.total",
                            Expression = "deleted.footer.total",
                            Left = 1400,
                            Top = 9400,
                            Width = 3600,
                            Height = 600
                        }
                    }
                }
            },
            UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
            {
                new()
                {
                    RecordIndex = 9,
                    ObjectKind = "field",
                    Title = "orphan.note",
                    Expression = "orphan.note",
                    Left = 800,
                    Top = 700,
                    Width = 2400,
                    Height = 450
                }
            }
        };

        surface.LoadReportLayout(layout, objects);
        RenderDesignSurface(surface);

        var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
        var moves = new List<(int RecordIndex, int Left, int Top)>();
        surface.ObjectMoved += (recordIndex, left, top) => moves.Add((recordIndex, left, top));

        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 0)), 18, 12);
        var expectedLiveLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
        var expectedLiveTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 6 &&
               moves[0].Left == expectedLiveLeft &&
               moves[0].Top == expectedLiveTop,
            "Dragging a live report object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging a live report object on the shared surface should keep the containing live section highlighted");

        moves.Clear();
        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 1)), 14, 10);
        var expectedDeletedLeft = (int)Math.Round(1400 + (14 / Math.Max(0.2F, scale)));
        var expectedDeletedTop = (int)Math.Round(9400 + (10 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 13 &&
               moves[0].Left == expectedDeletedLeft &&
               moves[0].Top == expectedDeletedTop,
            "Dragging a deleted report object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging a deleted report object on the shared surface should keep the containing deleted section highlighted");

        moves.Clear();
        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 2)), 16, 9);
        var expectedUnplacedLeft = (int)Math.Round(800 + (16 / Math.Max(0.2F, scale)));
        var expectedUnplacedTop = (int)Math.Round(700 + (9 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 9 &&
               moves[0].Left == expectedUnplacedLeft &&
               moves[0].Top == expectedUnplacedTop,
            "Dragging an unplaced report object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging an unplaced report object on the shared surface should keep the unplaced-object tray highlighted");
    }

    private static void SmokeLabelSurfaceObjectDragging()
    {
        using var surface = new CopperfinDesignSurfaceControl
        {
            Size = new Size(1400, 1000)
        };

        var snapshot = BuildLabelSurfaceInteractionSmokeSnapshot();
        var reportLayout = snapshot.ReportLayout ?? throw new InvalidOperationException("Could not build shared label surface layout snapshot.");
        surface.LoadReportLayout(reportLayout, snapshot.Objects);
        RenderDesignSurface(surface);

        var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
        var moves = new List<(int RecordIndex, int Left, int Top)>();
        surface.ObjectMoved += (recordIndex, left, top) => moves.Add((recordIndex, left, top));

        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 0)), 18, 12);
        var expectedLiveLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
        var expectedLiveTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 6 &&
               moves[0].Left == expectedLiveLeft &&
               moves[0].Top == expectedLiveTop,
            "Dragging a live label object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging a live label object on the shared surface should keep the containing live section highlighted");

        moves.Clear();
        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 1)), 14, 10);
        var expectedDeletedLeft = (int)Math.Round(1400 + (14 / Math.Max(0.2F, scale)));
        var expectedDeletedTop = (int)Math.Round(9400 + (10 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 13 &&
               moves[0].Left == expectedDeletedLeft &&
               moves[0].Top == expectedDeletedTop,
            "Dragging a deleted label object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging a deleted label object on the shared surface should keep the containing deleted section highlighted");

        moves.Clear();
        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 2)), 16, 9);
        var expectedUnplacedLeft = (int)Math.Round(800 + (16 / Math.Max(0.2F, scale)));
        var expectedUnplacedTop = (int)Math.Round(700 + (9 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 9 &&
               moves[0].Left == expectedUnplacedLeft &&
               moves[0].Top == expectedUnplacedTop,
            "Dragging an unplaced label object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging an unplaced label object on the shared surface should keep the unplaced-object tray highlighted");
    }

    private static void SmokeAssetEditorReportDragUsesBatchStudioHostUpdate()
    {
        var snapshot = BuildAssetEditorBatchUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildBatchUpdateHostResponseJson());
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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");
            RenderDesignSurface(surface);

            var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
            var start = GetCenter(ReadSurfaceObjectRectangle(surface, 0));
            ClickDesignSurface(surface, start);
            Application.DoEvents();

            DragDesignSurface(surface, start, 18, 12);
            Application.DoEvents();

            var expectedLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
            var expectedTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Dragging a report object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--visual-object-update-batch") &&
                   invocationArguments.Contains("--selected-record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains(expectedLeft.ToString()) &&
                   invocationArguments.Contains(expectedTop.ToString()) &&
                   !invocationArguments.Contains("--set-property"),
                "Dragging a report object through the shared asset editor should send one batch update with invariant HPOS/VPOS changes");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Dragging a report object through the shared asset editor should preserve section and object selection continuity after the batch refresh");
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

    private static void SmokeAssetEditorReportDragRefreshesShellSummary()
    {
        var snapshot = BuildAssetEditorBatchUpdateSmokeSnapshot();
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
            CreateFakeStudioHostScript(scriptPath, BuildBatchUpdatePreviewRefreshHostResponseJson());
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
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");
            RenderDesignSurface(surface);

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A drag summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
            var start = GetCenter(ReadSurfaceObjectRectangle(surface, 0));
            ClickDesignSurface(surface, start);
            Application.DoEvents();

            DragDesignSurface(surface, start, 18, 12);
            Application.DoEvents();

            var expectedLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
            var expectedTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Dragging a report object through the shared asset editor for summary refresh should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--visual-object-update-batch") &&
                   invocationArguments.Contains("--selected-record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains(expectedLeft.ToString()) &&
                   invocationArguments.Contains(expectedTop.ToString()),
                "Dragging a report object through the shared asset editor for summary refresh should send one invariant batch update through the host contract");

            var refreshed = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                    HasLabelTextContaining(control, "Preview bounds: L 1500 T 2800 R 6100 B 6300   Size: 4600 x 3500") &&
                    string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                    string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                    propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                    refreshedSelection.RecordIndex == 6 &&
                    string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1500", StringComparison.Ordinal) &&
                    string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                    ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6);
            Expect(refreshed,
                "Dragging a report object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving report identity, object selection, and containing-section continuity");
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

    private static void SmokeAssetEditorLabelDragRefreshesShellSummary()
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
            CreateFakeStudioHostScript(scriptPath, BuildBatchLabelUpdatePreviewRefreshHostResponseJson());
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
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");
            RenderDesignSurface(surface);

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label drag summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
            var start = GetCenter(ReadSurfaceObjectRectangle(surface, 0));
            ClickDesignSurface(surface, start);
            Application.DoEvents();

            DragDesignSurface(surface, start, 18, 12);
            Application.DoEvents();

            var expectedLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
            var expectedTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Dragging a label object through the shared asset editor for summary refresh should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--visual-object-update-batch") &&
                   invocationArguments.Contains("--selected-record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains(expectedLeft.ToString()) &&
                   invocationArguments.Contains(expectedTop.ToString()),
                "Dragging a label object through the shared asset editor for summary refresh should send one invariant batch update through the host contract");

            var refreshed = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                    HasLabelTextContaining(control, "Preview bounds: L 1500 T 2800 R 6100 B 6300   Size: 4600 x 3500") &&
                    string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                    string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                    propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                    refreshedSelection.RecordIndex == 6 &&
                    string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1500", StringComparison.Ordinal) &&
                    string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                    ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6);
            Expect(refreshed,
                "Dragging a label object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving label identity, object selection, and containing-section continuity");
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
