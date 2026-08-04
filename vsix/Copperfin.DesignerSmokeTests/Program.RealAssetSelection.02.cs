
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
    private static void SmokeRealAssetSettingsAuxiliaryPrintSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        IReadOnlyDictionary<string, CopperfinStudioNamedValue>? expectedSettings = null;

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if (string.IsNullOrWhiteSpace(candidatePath) || !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            var candidateLayout = candidateSnapshot.Document?.ReportLayout;
            if (!candidateSnapshot.Success || candidateLayout is null)
            {
                continue;
            }

            var ascii = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "ASCII", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var collate = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "COLLATE", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var copies = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "COPIES", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            if (ascii is null || collate is null || copies is null)
            {
                continue;
            }

            selectedPath = candidatePath;
            expectedSettings = new Dictionary<string, CopperfinStudioNamedValue>(StringComparer.OrdinalIgnoreCase)
            {
                [ascii.Name] = ascii,
                [collate.Name] = collate,
                [copies.Name] = copies
            };
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || expectedSettings is null)
        {
            Console.WriteLine("SKIP: real auxiliary-print settings candidate not found.");
            return;
        }

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
        control.LoadDocument(selectedPath!);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);
        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, "Settings", StringComparison.Ordinal)));
        Expect(loaded, $"real asset auxiliary-print smoke should surface the settings scope for {selectedPath}");
        if (!loaded)
        {
            TearDownForm(hostForm);
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = string.Equals(item.Text, "Settings", StringComparison.Ordinal);
        }

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        Application.DoEvents();

        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection settingsSelection &&
               settingsSelection.RecordIndex == expectedSettings["ASCII"].RecordIndex &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "ASCII"), expectedSettings["ASCII"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "COLLATE"), expectedSettings["COLLATE"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "COPIES"), expectedSettings["COPIES"].Value, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared auxiliary-print continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetLabelSettingsSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        IReadOnlyDictionary<string, CopperfinStudioNamedValue>? expectedSettings = null;

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if (string.IsNullOrWhiteSpace(candidatePath) || !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            var candidateLayout = candidateSnapshot.Document?.ReportLayout;
            if (!candidateSnapshot.Success || candidateLayout is null || !candidateLayout.IsLabel)
            {
                continue;
            }

            var orientation = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "ORIENTATION", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var paperSize = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "PAPERSIZE", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var color = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "COLOR", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var verticalGrid = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "GRIDV", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var horizontalGrid = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "GRIDH", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));

            if (orientation is null || paperSize is null || color is null || verticalGrid is null || horizontalGrid is null)
            {
                continue;
            }

            selectedPath = candidatePath;
            expectedSettings = new Dictionary<string, CopperfinStudioNamedValue>(StringComparer.OrdinalIgnoreCase)
            {
                [orientation.Name] = orientation,
                [paperSize.Name] = paperSize,
                [color.Name] = color,
                [verticalGrid.Name] = verticalGrid,
                [horizontalGrid.Name] = horizontalGrid
            };
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || expectedSettings is null)
        {
            Console.WriteLine("SKIP: real label settings candidate not found.");
            return;
        }

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
        control.LoadDocument(selectedPath!);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);
        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, "Settings", StringComparison.Ordinal)));
        Expect(loaded, $"real label settings smoke should surface the settings scope for {selectedPath}");
        if (!loaded)
        {
            TearDownForm(hostForm);
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = string.Equals(item.Text, "Settings", StringComparison.Ordinal);
        }

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        Application.DoEvents();

        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection settingsSelection &&
               settingsSelection.RecordIndex == expectedSettings["ORIENTATION"].RecordIndex &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "ORIENTATION"), expectedSettings["ORIENTATION"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "PAPERSIZE"), expectedSettings["PAPERSIZE"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "COLOR"), expectedSettings["COLOR"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "GRIDV"), expectedSettings["GRIDV"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "GRIDH"), expectedSettings["GRIDH"].Value, StringComparison.Ordinal),
            $"real label settings smoke should expose shared settings continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetLabelSettingsDocumentTitleSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        string? expectedDocumentTitle = null;
        var localization = new CopperfinLocalization("en-US");
        var settingsScopeTitle = localization.Text("AssetEditor.ReportSection.Settings");

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if (string.IsNullOrWhiteSpace(candidatePath) || !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            var candidateLayout = candidateSnapshot.Document?.ReportLayout;
            var candidateDocumentTitle = candidateLayout?.DocumentTitle;
            if (!candidateSnapshot.Success ||
                candidateLayout is null ||
                !candidateLayout.IsLabel ||
                string.IsNullOrWhiteSpace(candidateDocumentTitle))
            {
                continue;
            }

            selectedPath = candidatePath;
            expectedDocumentTitle = candidateDocumentTitle;
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || string.IsNullOrWhiteSpace(expectedDocumentTitle))
        {
            Console.WriteLine("SKIP: real label document-title settings candidate not found.");
            return;
        }

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
        control.LoadDocument(selectedPath!);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);
        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal)));
        Expect(loaded, $"real label document-title smoke should surface the settings scope for {selectedPath}");
        if (!loaded)
        {
            TearDownForm(hostForm);
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal);
        }

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        Application.DoEvents();

        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection settingsSelection &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "DOCUMENTTITLE"), expectedDocumentTitle, StringComparison.Ordinal),
            $"real label document-title smoke should expose shared document-title continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetLabelSettingsPreviewBoundsSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        CopperfinStudioReportLayout? selectedLayout = null;
        var localization = new CopperfinLocalization("en-US");
        var settingsScopeTitle = localization.Text("AssetEditor.ReportSection.Settings");

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if (string.IsNullOrWhiteSpace(candidatePath) || !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            var candidateLayout = candidateSnapshot.Document?.ReportLayout;
            if (!candidateSnapshot.Success ||
                candidateLayout is null ||
                !candidateLayout.IsLabel ||
                !candidateLayout.PreviewBoundsAvailable ||
                candidateLayout.Settings.Count == 0)
            {
                continue;
            }

            selectedPath = candidatePath;
            selectedLayout = candidateLayout;
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || selectedLayout is null)
        {
            Console.WriteLine("SKIP: real label preview-bounds settings candidate not found.");
            return;
        }

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
        control.LoadDocument(selectedPath!);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);
        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal)));
        Expect(loaded, $"real label preview-bounds smoke should surface the settings scope for {selectedPath}");
        if (!loaded)
        {
            TearDownForm(hostForm);
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal);
        }

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        Application.DoEvents();

        var expectedPreviewBounds = localization.Format(
            "AssetEditor.Property.BoundsValue",
            selectedLayout.PreviewBoundsLeft,
            selectedLayout.PreviewBoundsTop,
            selectedLayout.PreviewBoundsRight,
            selectedLayout.PreviewBoundsBottom,
            selectedLayout.PreviewBoundsWidth,
            selectedLayout.PreviewBoundsHeight);

        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection settingsSelection &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "PREVIEWBOUNDS"), expectedPreviewBounds, StringComparison.Ordinal),
            $"real label preview-bounds smoke should expose shared settings continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsPreviewBoundsSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        CopperfinStudioReportLayout? selectedLayout = null;
        var localization = new CopperfinLocalization("en-US");
        var settingsScopeTitle = localization.Text("AssetEditor.ReportSection.Settings");

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if (string.IsNullOrWhiteSpace(candidatePath) || !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            var candidateLayout = candidateSnapshot.Document?.ReportLayout;
            if (!candidateSnapshot.Success ||
                candidateLayout is null ||
                !candidateLayout.PreviewBoundsAvailable ||
                candidateLayout.Settings.Count == 0)
            {
                continue;
            }

            selectedPath = candidatePath;
            selectedLayout = candidateLayout;
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || selectedLayout is null)
        {
            Console.WriteLine("SKIP: real settings preview-bounds candidate with root settings metadata not found.");
            return;
        }

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
        control.LoadDocument(selectedPath!);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);
        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal)));
        Expect(loaded, $"real asset settings smoke should surface the settings scope for {selectedPath}");
        if (!loaded)
        {
            TearDownForm(hostForm);
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal);
        }

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        Application.DoEvents();

        var expectedPreviewBounds = localization.Format(
            "AssetEditor.Property.BoundsValue",
            selectedLayout.PreviewBoundsLeft,
            selectedLayout.PreviewBoundsTop,
            selectedLayout.PreviewBoundsRight,
            selectedLayout.PreviewBoundsBottom,
            selectedLayout.PreviewBoundsWidth,
            selectedLayout.PreviewBoundsHeight);

        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection settingsSelection &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "PREVIEWBOUNDS"), expectedPreviewBounds, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared preview-bounds continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsDeletedPreviewBoundsSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        CopperfinStudioReportLayout? selectedLayout = null;
        string? expectedScopeTitle = null;
        var localization = new CopperfinLocalization("en-US");
        var settingsScopeTitle = localization.Text("AssetEditor.ReportSection.Settings");
        var deletedSettingsScopeTitle = localization.Format(
            "AssetEditor.ReportSection.Deleted",
            settingsScopeTitle);

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if (string.IsNullOrWhiteSpace(candidatePath) || !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            var candidateLayout = candidateSnapshot.Document?.ReportLayout;
            if (!candidateSnapshot.Success ||
                candidateLayout is null ||
                !candidateLayout.DeletedPreviewBoundsAvailable)
            {
                continue;
            }

            if (candidateLayout.Settings.Count > 0)
            {
                selectedPath = candidatePath;
                selectedLayout = candidateLayout;
                expectedScopeTitle = settingsScopeTitle;
                break;
            }

            if (candidateLayout.DeletedSettings.Count > 0)
            {
                selectedPath = candidatePath;
                selectedLayout = candidateLayout;
                expectedScopeTitle = deletedSettingsScopeTitle;
                break;
            }
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || selectedLayout is null || string.IsNullOrWhiteSpace(expectedScopeTitle))
        {
            Console.WriteLine("SKIP: real deleted-preview-bounds settings candidate not found.");
            return;
        }

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
        control.LoadDocument(selectedPath!);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);
        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, expectedScopeTitle, StringComparison.Ordinal)));
        Expect(loaded, $"real asset settings smoke should surface the {expectedScopeTitle} scope for {selectedPath}");
        if (!loaded)
        {
            TearDownForm(hostForm);
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = string.Equals(item.Text, expectedScopeTitle, StringComparison.Ordinal);
        }

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        Application.DoEvents();

        var expectedDeletedPreviewBounds = localization.Format(
            "AssetEditor.Property.BoundsValue",
            selectedLayout.DeletedPreviewBoundsLeft,
            selectedLayout.DeletedPreviewBoundsTop,
            selectedLayout.DeletedPreviewBoundsRight,
            selectedLayout.DeletedPreviewBoundsBottom,
            selectedLayout.DeletedPreviewBoundsWidth,
            selectedLayout.DeletedPreviewBoundsHeight);

        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection settingsSelection &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "DELETEDPREVIEWBOUNDS"), expectedDeletedPreviewBounds, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared deleted-preview-bounds continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetDeletedPreviewBoundsSelectionAfterDelete(
        string? sourcePath,
        int recordIndex,
        string expectedUniqueId,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted-preview candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealDeletedPreviewWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var localization = new CopperfinLocalization("en-US");
        var settingsScopeTitle = localization.Text("AssetEditor.ReportSection.Settings");
        var deletedSettingsScopeTitle = localization.Format(
            "AssetEditor.ReportSection.Deleted",
            settingsScopeTitle);

        try
        {
            var deleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(deleteResult.Success && deleteResult.Document?.ReportLayout is not null,
                $"real deleted-preview smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document?.ReportLayout is null)
            {
                return;
            }

            var reloadedAfterDelete = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDelete.Success && reloadedAfterDelete.Document?.ReportLayout is not null,
                $"real deleted-preview smoke should reload deleted snapshot data for {sourcePath}");
            if (!reloadedAfterDelete.Success || reloadedAfterDelete.Document?.ReportLayout is null)
            {
                return;
            }

            var reportLayout = reloadedAfterDelete.Document.ReportLayout;
            Expect(reportLayout.IsLabel == expectLabel,
                $"real deleted-preview smoke should preserve report/label identity for {sourcePath}");
            Expect(reportLayout.DeletedPreviewBoundsAvailable,
                $"real deleted-preview smoke should expose deleted preview bounds after deletion for {sourcePath}");

            var expectedScopeTitle = reportLayout.Settings.Count > 0
                ? settingsScopeTitle
                : reportLayout.DeletedSettings.Count > 0
                    ? deletedSettingsScopeTitle
                    : null;
            Expect(!string.IsNullOrWhiteSpace(expectedScopeTitle),
                $"real deleted-preview smoke should expose a settings scope after deletion for {sourcePath}");
            if (string.IsNullOrWhiteSpace(expectedScopeTitle))
            {
                return;
            }

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
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, expectedScopeTitle, StringComparison.Ordinal)));
            Expect(loaded, $"real deleted-preview smoke should surface the {expectedScopeTitle} scope for {sourcePath}");
            if (!loaded)
            {
                TearDownForm(hostForm);
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = string.Equals(item.Text, expectedScopeTitle, StringComparison.Ordinal);
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var expectedDeletedPreviewBounds = localization.Format(
                "AssetEditor.Property.BoundsValue",
                reportLayout.DeletedPreviewBoundsLeft,
                reportLayout.DeletedPreviewBoundsTop,
                reportLayout.DeletedPreviewBoundsRight,
                reportLayout.DeletedPreviewBoundsBottom,
                reportLayout.DeletedPreviewBoundsWidth,
                reportLayout.DeletedPreviewBoundsHeight);

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection settingsSelection &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(ReadSelectionPropertyValue(settingsSelection, "DELETEDPREVIEWBOUNDS"), expectedDeletedPreviewBounds, StringComparison.Ordinal),
                $"real deleted-preview smoke should expose shared deleted-preview-bounds continuity for {sourcePath}");

            TearDownForm(hostForm);
        }
        finally
        {
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
