
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
    private static void SmokeRealAssetColumnSetupSelectionAfterExprUpdate(
        string? sourcePath,
        string exprPayload,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real column-setup candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealColumnSetupWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var localization = new CopperfinLocalization("en-US");
        var settingsScopeTitle = localization.Text("AssetEditor.ReportSection.Settings");

        try
        {
            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                0,
                "EXPR",
                exprPayload);
            Expect(updateResult.Success && updateResult.Document?.ReportLayout is not null,
                $"real column-setup smoke should update EXPR for {sourcePath}");
            if (!updateResult.Success || updateResult.Document?.ReportLayout is null)
            {
                return;
            }

            var expectedSettings = updateResult.Document.ReportLayout.Settings
                .Where(setting =>
                    string.Equals(setting.Name, "COLS", StringComparison.OrdinalIgnoreCase) ||
                    string.Equals(setting.Name, "COLWIDTH", StringComparison.OrdinalIgnoreCase) ||
                    string.Equals(setting.Name, "COLSPACING", StringComparison.OrdinalIgnoreCase))
                .ToDictionary(setting => setting.Name, StringComparer.OrdinalIgnoreCase);
            Expect(expectedSettings.Count == 3,
                $"real column-setup smoke should expose column-setup settings after EXPR update for {sourcePath}");
            if (expectedSettings.Count != 3)
            {
                return;
            }

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document?.ReportLayout is not null,
                $"real column-setup smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document?.ReportLayout is null)
            {
                return;
            }

            Expect(reloadedAfterUpdate.Document.ReportLayout.IsLabel == expectLabel,
                $"real column-setup smoke should preserve report/label identity for {sourcePath}");

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
                () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal)));
            Expect(loaded, $"real column-setup smoke should surface the settings scope for {sourcePath}");
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
                   settingsSelection.RecordIndex == expectedSettings["COLS"].RecordIndex &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(ReadSelectionPropertyValue(settingsSelection, "COLS"), expectedSettings["COLS"].Value, StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(settingsSelection, "COLWIDTH"), expectedSettings["COLWIDTH"].Value, StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(settingsSelection, "COLSPACING"), expectedSettings["COLSPACING"].Value, StringComparison.Ordinal),
                $"real column-setup smoke should expose shared column-setup continuity for {sourcePath}");

            TearDownForm(hostForm);

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real column-setup smoke should undo the EXPR update for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document?.ReportLayout is not null,
                $"real column-setup smoke should reload undone snapshot data for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document?.ReportLayout is not null)
            {
                var restoredSettings = reloadedAfterUndo.Document.ReportLayout.Settings
                    .Where(setting =>
                        string.Equals(setting.Name, "COLS", StringComparison.OrdinalIgnoreCase) ||
                        string.Equals(setting.Name, "COLWIDTH", StringComparison.OrdinalIgnoreCase) ||
                        string.Equals(setting.Name, "COLSPACING", StringComparison.OrdinalIgnoreCase))
                    .ToList();
                Expect(restoredSettings.Count == 0,
                    $"real column-setup smoke should restore the original no-column-setup state for {sourcePath}");
            }
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
