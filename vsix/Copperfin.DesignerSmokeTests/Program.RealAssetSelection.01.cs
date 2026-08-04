
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
    private static void SmokeAssetEditorWithRealAsset(string? path, string expectSection)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real asset candidate" : path)} not found.");
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
        control.LoadDocument(path!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => FindListViews(control).Any(list => list.Items.Count > 0));
        Expect(loaded, $"editor should load snapshot data for {path}");

        var sectionFound = FindListViews(control)
            .SelectMany(list => list.Items.Cast<ListViewItem>())
            .Any(item => string.Equals(item.Text, expectSection, StringComparison.OrdinalIgnoreCase) ||
                         item.Text.IndexOf(expectSection, StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(sectionFound, $"editor should surface section '{expectSection}' for {path}");
        Expect(HasLabelTextContaining(control, "Sections:") &&
               HasLabelTextContaining(control, "Settings:") &&
               HasLabelTextContaining(control, "Unplaced objects:"),
            $"editor should surface a report layout summary for {path}");

        var designSurface = FindDesignSurface(control);
        Expect(designSurface is not null, $"design surface should exist for {path}");
        if (designSurface is not null)
        {
            using var bitmap = new Bitmap(Math.Max(1, designSurface.Width), Math.Max(1, designSurface.Height));
            designSurface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
            Expect(CountNonWhitePixels(bitmap) > 5000, $"design surface should render visible content for {path}");
        }

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetGroupingExplorerSelection(string? sourcePath)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real grouped asset candidate" : sourcePath)} not found.");
            return;
        }

        var loadedSnapshot = CopperfinStudioSnapshotClient.TryLoad(sourcePath!);
        Expect(loadedSnapshot.Success && loadedSnapshot.Document?.ReportLayout?.Groupings.Count > 0,
            $"real asset grouping smoke should load grouping metadata for {sourcePath}");
        var loadedReportLayout = loadedSnapshot.Document?.ReportLayout;
        if (!loadedSnapshot.Success || loadedReportLayout?.Groupings.Count <= 0)
        {
            return;
        }

        var reportLayout = loadedReportLayout;
        var grouping = reportLayout!.Groupings[0];
        var expectedExplorerTitle = string.IsNullOrWhiteSpace(grouping.Expression)
            ? $"Grouping {grouping.GroupingIndex}"
            : $"Grouping {grouping.GroupingIndex} - {grouping.Expression}";
        var expectedGrouping = new ExpectedReportGroupingMetadata
        {
            GroupingIndex = grouping.GroupingIndex,
            GroupingNestingDepth = grouping.NestingDepth,
            GroupingExpression = grouping.Expression,
            GroupingExpressionFieldIndex = grouping.ExpressionFieldIndex,
            GroupingExpressionMemoBlockNumber = grouping.ExpressionMemoBlockNumber,
            HeaderSectionId = grouping.HeaderSectionId,
            HeaderRecordIndex = grouping.HeaderRecordIndex,
            HeaderDeleted = grouping.HeaderDeleted,
            FooterSectionId = grouping.FooterSectionId,
            FooterRecordIndex = grouping.FooterRecordIndex,
            FooterDeleted = grouping.FooterDeleted,
            HeaderStateDisplay = grouping.HeaderDeleted ? "Deleted" : "Live",
            FooterStateDisplay = grouping.FooterDeleted ? "Deleted" : "Live"
        };

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
        control.LoadDocument(sourcePath!);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);
        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, expectedExplorerTitle, StringComparison.Ordinal)));
        Expect(loaded, $"real asset grouping smoke should surface explorer row '{expectedExplorerTitle}' for {sourcePath}");
        if (!loaded)
        {
            TearDownForm(hostForm);
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = string.Equals(item.Text, expectedExplorerTitle, StringComparison.Ordinal);
        }

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        Application.DoEvents();

        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection groupingSelection &&
               SelectionMatchesExpectedReportGrouping(groupingSelection, expectedGrouping) &&
               objectListView.Items.Count == 0,
            $"real asset grouping smoke should expose shared grouping metadata continuity for {sourcePath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsSortMetadataSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        CopperfinStudioNamedValue? sortSetting = null;

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if (string.IsNullOrWhiteSpace(candidatePath) || !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            if (!candidateSnapshot.Success)
            {
                continue;
            }

            var candidateSortSetting = candidateSnapshot.Document?.ReportLayout?.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "TAG", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            if (candidateSortSetting is null)
            {
                continue;
            }

            selectedPath = candidatePath;
            sortSetting = candidateSortSetting;
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || sortSetting is null)
        {
            Console.WriteLine("SKIP: real settings sort candidate with root TAG metadata not found.");
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
        Expect(loaded, $"real asset settings smoke should surface the settings scope for {selectedPath}");
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
               settingsSelection.RecordIndex == (sortSetting.RecordIndex) &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "SORTEXPRESSION"), sortSetting.Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "SORTEXPRESSIONFIELD"), sortSetting.FieldIndex?.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "SORTEXPRESSIONMEMO"), sortSetting.MemoBlockNumber.ToString(), StringComparison.Ordinal),
            $"real asset settings smoke should expose shared sort metadata continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsDocumentTitleSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        string? documentTitle = null;

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if (string.IsNullOrWhiteSpace(candidatePath) || !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            var candidateTitle = candidateSnapshot.Document?.ReportLayout?.DocumentTitle;
            if (!candidateSnapshot.Success || string.IsNullOrWhiteSpace(candidateTitle))
            {
                continue;
            }

            selectedPath = candidatePath;
            documentTitle = candidateTitle;
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || string.IsNullOrWhiteSpace(documentTitle))
        {
            Console.WriteLine("SKIP: real settings document-title candidate not found.");
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
        Expect(loaded, $"real asset settings smoke should surface the settings scope for {selectedPath}");
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
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "DOCUMENTTITLE"), documentTitle, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared document-title continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsPageSetupSelection(params string?[] sourcePaths)
    {
        var reportSettings = new Dictionary<string, CopperfinStudioNamedValue>(StringComparer.OrdinalIgnoreCase);
        var labelSettings = new Dictionary<string, CopperfinStudioNamedValue>(StringComparer.OrdinalIgnoreCase);
        string? reportPath = null;
        string? labelPath = null;
        const string reportAssetFamily = "report";
        const string labelAssetFamily = "label";
        var requiredSettingNames = new[]
        {
            "ORIENTATION",
            "PAPERSIZE",
            "GRIDV",
            "GRIDH"
        };

        foreach (var candidatePath in EnumerateResolvedRealReportAssetPaths(sourcePaths))
        {
            if ((reportPath is not null && labelPath is not null) ||
                string.IsNullOrWhiteSpace(candidatePath) ||
                !File.Exists(candidatePath))
            {
                continue;
            }

            var candidateSnapshot = CopperfinStudioSnapshotClient.TryLoad(candidatePath!);
            var candidateDocument = candidateSnapshot.Document;
            var candidateLayout = candidateDocument?.ReportLayout;
            if (!candidateSnapshot.Success || candidateDocument is null || candidateLayout is null)
            {
                continue;
            }

            var candidateSettings = requiredSettingNames
                .Select(settingName => candidateLayout.Settings.FirstOrDefault(setting =>
                    string.Equals(setting.Name, settingName, StringComparison.OrdinalIgnoreCase) &&
                    !string.IsNullOrWhiteSpace(setting.Value)))
                .ToList();
            if (candidateSettings.Any(setting => setting is null))
            {
                continue;
            }

            var destinationPath = candidateDocument.AssetFamily switch
            {
                reportAssetFamily when reportPath is null => candidatePath,
                labelAssetFamily when labelPath is null => candidatePath,
                _ => null
            };
            if (destinationPath is null)
            {
                continue;
            }

            var destinationSettings = candidateDocument.AssetFamily switch
            {
                reportAssetFamily => reportSettings,
                labelAssetFamily => labelSettings,
                _ => throw new InvalidOperationException("Unexpected real asset family for page-setup settings continuity smoke.")
            };

            foreach (var setting in candidateSettings)
            {
                destinationSettings[setting!.Name] = setting;
            }

            if (string.Equals(candidateDocument.AssetFamily, reportAssetFamily, StringComparison.OrdinalIgnoreCase))
            {
                reportPath = destinationPath;
            }
            else if (string.Equals(candidateDocument.AssetFamily, labelAssetFamily, StringComparison.OrdinalIgnoreCase))
            {
                labelPath = destinationPath;
            }
        }

        if (string.IsNullOrWhiteSpace(reportPath) || string.IsNullOrWhiteSpace(labelPath))
        {
            Console.WriteLine("SKIP: real page-setup settings candidates for both report and label assets not found.");
            return;
        }

        VerifyRealAssetSettingsPageSetupSelection(reportPath!, reportSettings);
        VerifyRealAssetSettingsPageSetupSelection(labelPath!, labelSettings);
    }

    private static void VerifyRealAssetSettingsPageSetupSelection(
        string selectedPath,
        IReadOnlyDictionary<string, CopperfinStudioNamedValue> expectedSettings)
    {
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
        Expect(loaded, $"real asset settings smoke should surface the settings scope for {selectedPath}");
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
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "GRIDV"), expectedSettings["GRIDV"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "GRIDH"), expectedSettings["GRIDH"].Value, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared page-setup/grid continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsPaperDimensionsSelection(params string?[] sourcePaths)
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

            var paperLength = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "PAPERLENGTH", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var paperWidth = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "PAPERWIDTH", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));

            if (paperLength is null || paperWidth is null)
            {
                continue;
            }

            selectedPath = candidatePath;
            expectedSettings = new Dictionary<string, CopperfinStudioNamedValue>(StringComparer.OrdinalIgnoreCase)
            {
                [paperLength.Name] = paperLength,
                [paperWidth.Name] = paperWidth
            };
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || expectedSettings is null)
        {
            Console.WriteLine("SKIP: real paper-dimension settings candidate not found.");
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
        Expect(loaded, $"real asset paper-dimension smoke should surface the settings scope for {selectedPath}");
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
               settingsSelection.RecordIndex == expectedSettings["PAPERLENGTH"].RecordIndex &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "PAPERLENGTH"), expectedSettings["PAPERLENGTH"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "PAPERWIDTH"), expectedSettings["PAPERWIDTH"].Value, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared paper-dimension continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsPrinterIdentitySelection(params string?[] sourcePaths)
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

            var printerDriver = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "DRIVER", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var printerDevice = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "DEVICE", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var printerOutput = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "OUTPUT", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));

            if (printerDriver is null || printerDevice is null || printerOutput is null)
            {
                continue;
            }

            selectedPath = candidatePath;
            expectedSettings = new Dictionary<string, CopperfinStudioNamedValue>(StringComparer.OrdinalIgnoreCase)
            {
                [printerDriver.Name] = printerDriver,
                [printerDevice.Name] = printerDevice,
                [printerOutput.Name] = printerOutput
            };
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || expectedSettings is null)
        {
            Console.WriteLine("SKIP: real printer-identity settings candidate not found.");
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
        Expect(loaded, $"real asset printer-identity smoke should surface the settings scope for {selectedPath}");
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
               settingsSelection.RecordIndex == expectedSettings["DRIVER"].RecordIndex &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "DRIVER"), expectedSettings["DRIVER"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "DEVICE"), expectedSettings["DEVICE"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "OUTPUT"), expectedSettings["OUTPUT"].Value, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared printer-identity continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsPrintProfileSelection(params string?[] sourcePaths)
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

            var defaultSource = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "DEFAULTSOURCE", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var printQuality = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "PRINTQUALITY", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var yResolution = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "YRESOLUTION", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            var ttOption = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "TTOPTION", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));

            if (defaultSource is null || printQuality is null || yResolution is null || ttOption is null)
            {
                continue;
            }

            selectedPath = candidatePath;
            expectedSettings = new Dictionary<string, CopperfinStudioNamedValue>(StringComparer.OrdinalIgnoreCase)
            {
                [defaultSource.Name] = defaultSource,
                [printQuality.Name] = printQuality,
                [yResolution.Name] = yResolution,
                [ttOption.Name] = ttOption
            };
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || expectedSettings is null)
        {
            Console.WriteLine("SKIP: real print-profile settings candidate not found.");
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
        Expect(loaded, $"real asset print-profile smoke should surface the settings scope for {selectedPath}");
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
               settingsSelection.RecordIndex == expectedSettings["DEFAULTSOURCE"].RecordIndex &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "DEFAULTSOURCE"), expectedSettings["DEFAULTSOURCE"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "PRINTQUALITY"), expectedSettings["PRINTQUALITY"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "YRESOLUTION"), expectedSettings["YRESOLUTION"].Value, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "TTOPTION"), expectedSettings["TTOPTION"].Value, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared print-profile continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetSettingsColorSelection(params string?[] sourcePaths)
    {
        string? selectedPath = null;
        CopperfinStudioNamedValue? expectedColor = null;

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

            var color = candidateLayout.Settings.FirstOrDefault(setting =>
                string.Equals(setting.Name, "COLOR", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(setting.Value));
            if (color is null)
            {
                continue;
            }

            selectedPath = candidatePath;
            expectedColor = color;
            break;
        }

        if (string.IsNullOrWhiteSpace(selectedPath) || expectedColor is null)
        {
            Console.WriteLine("SKIP: real color settings candidate not found.");
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
        Expect(loaded, $"real asset color smoke should surface the settings scope for {selectedPath}");
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
               settingsSelection.RecordIndex == expectedColor.RecordIndex &&
               objectListView.Items.Count == 0 &&
               string.Equals(ReadSelectionPropertyValue(settingsSelection, "COLOR"), expectedColor.Value, StringComparison.Ordinal),
            $"real asset settings smoke should expose shared color continuity for {selectedPath}");

        TearDownForm(hostForm);
    }

}
