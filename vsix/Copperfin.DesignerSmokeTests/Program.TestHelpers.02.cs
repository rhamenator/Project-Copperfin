
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
    private static int ReadPrivateListCount(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(instance) is not System.Collections.ICollection collection)
        {
            throw new InvalidOperationException($"Could not read private list field {fieldName}.");
        }

        return collection.Count;
    }

    private static int? ReadPrivateNullableInt(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field is null)
        {
            throw new InvalidOperationException($"Could not read private nullable int field {fieldName}.");
        }

        return field.GetValue(instance) as int?;
    }

    private static bool ReadPrivateBoolField(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(instance) is not bool value)
        {
            throw new InvalidOperationException($"Could not read private bool field {fieldName}.");
        }

        return value;
    }

    private static string ReadPrivateStringField(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(instance) is not string value)
        {
            throw new InvalidOperationException($"Could not read private string field {fieldName}.");
        }

        return value;
    }

    private static int ReadReportSectionProperty(CopperfinDesignSurfaceControl surface, int index, string propertyName)
    {
        var section = ReadReportSectionVisual(surface, index);
        var property = section.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(section) is not int value)
        {
            throw new InvalidOperationException($"Could not read report-section property {propertyName}.");
        }

        return value;
    }

    private static string ReadReportSectionPropertyText(CopperfinDesignSurfaceControl surface, int index, string propertyName)
    {
        var section = ReadReportSectionVisual(surface, index);
        var property = section.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(section) is not string value)
        {
            throw new InvalidOperationException($"Could not read report-section text property {propertyName}.");
        }

        return value;
    }

    private static bool ReadReportSectionPropertyBool(CopperfinDesignSurfaceControl surface, int index, string propertyName)
    {
        var section = ReadReportSectionVisual(surface, index);
        var property = section.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(section) is not bool value)
        {
            throw new InvalidOperationException($"Could not read report-section boolean property {propertyName}.");
        }

        return value;
    }

    private static Rectangle ReadReportSectionRectangle(CopperfinDesignSurfaceControl surface, int index, string propertyName)
    {
        var section = ReadReportSectionVisual(surface, index);
        var property = section.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(section) is not Rectangle value)
        {
            throw new InvalidOperationException($"Could not read report-section rectangle property {propertyName}.");
        }

        return value;
    }

    private static object ReadReportSectionVisual(CopperfinDesignSurfaceControl surface, int index)
    {
        var field = typeof(CopperfinDesignSurfaceControl).GetField("reportSections", BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(surface) is not System.Collections.IList sections || sections.Count <= index)
        {
            throw new InvalidOperationException("Could not read shared report-section visuals.");
        }

        return sections[index]!;
    }

    private static Rectangle ReadPrivateRectangle(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(instance) is not Rectangle rectangle)
        {
            throw new InvalidOperationException($"Could not read private rectangle field {fieldName}.");
        }

        return rectangle;
    }

    private static Point GetCenter(Rectangle rectangle)
    {
        return new Point(rectangle.Left + (rectangle.Width / 2), rectangle.Top + (rectangle.Height / 2));
    }

    private static CopperfinDesignSurfaceControl? FindDesignSurface(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is CopperfinDesignSurfaceControl surface)
            {
                return surface;
            }

            var nested = FindDesignSurface(child);
            if (nested is not null)
            {
                return nested;
            }
        }

        return null;
    }

    private static ListView GetPrivateListView(CopperfinAssetEditorControl control, string fieldName)
    {
        return GetPrivateField<ListView>(control, fieldName)
            ?? throw new InvalidOperationException($"Could not read private list view {fieldName}.");
    }

    private static string CreateSmokeAssetFile(string tempRoot, string fileName)
    {
        var assetPath = Path.Combine(tempRoot, fileName);
        File.WriteAllText(assetPath, string.Empty);
        return assetPath;
    }

    private static Label GetPrivateLabel(CopperfinAssetEditorControl control, string fieldName)
    {
        return GetPrivateField<Label>(control, fieldName)
            ?? throw new InvalidOperationException($"Could not read private label {fieldName}.");
    }

    private static Button GetPrivateButton(CopperfinAssetEditorControl control, string fieldName)
    {
        return GetPrivateField<Button>(control, fieldName)
            ?? throw new InvalidOperationException($"Could not read private button {fieldName}.");
    }

    private static PropertyGrid GetPrivatePropertyGrid(CopperfinAssetEditorControl control)
    {
        return GetPrivateField<PropertyGrid>(control, "propertyGrid")
            ?? throw new InvalidOperationException("Could not read private property grid.");
    }

    private static T? GetPrivateField<T>(CopperfinAssetEditorControl control, string fieldName)
        where T : class
    {
        var field = typeof(CopperfinAssetEditorControl).GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        return field?.GetValue(control) as T;
    }

    private static void SetCurrentSnapshot(CopperfinAssetEditorControl control, CopperfinStudioSnapshotDocument snapshot)
    {
        var field = typeof(CopperfinAssetEditorControl).GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        if (field is null)
        {
            throw new InvalidOperationException("Could not set private currentSnapshot.");
        }

        field.SetValue(control, snapshot);
    }

    private static CopperfinStudioSnapshotDocument GetCurrentSnapshot(CopperfinAssetEditorControl control)
    {
        var field = typeof(CopperfinAssetEditorControl).GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        if (field is null)
        {
            throw new InvalidOperationException("Could not read private currentSnapshot.");
        }

        return (CopperfinStudioSnapshotDocument?)field.GetValue(control) ??
               throw new InvalidOperationException("Could not read current snapshot value.");
    }

    private static void SetPrivateField(object instance, string fieldName, object? value)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field is null)
        {
            throw new InvalidOperationException($"Could not set private field {fieldName}.");
        }

        field.SetValue(instance, value);
    }

    private static IEnumerable<Label> FindLabels(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is Label label)
            {
                yield return label;
            }

            foreach (var nested in FindLabels(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasLabelText(Control root, string text)
    {
        foreach (var label in FindLabels(root))
        {
            if (string.Equals(label.Text, text, StringComparison.Ordinal))
            {
                return true;
            }
        }

        return false;
    }

    private static bool HasLabelTextContaining(Control root, string text)
    {
        foreach (var label in FindLabels(root))
        {
            if (label.Text.IndexOf(text, StringComparison.Ordinal) >= 0)
            {
                return true;
            }
        }

        return false;
    }

    private static IEnumerable<RichTextBox> FindRichTextBoxes(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is RichTextBox textBox)
            {
                yield return textBox;
            }

            foreach (var nested in FindRichTextBoxes(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasRichTextBoxTextContaining(Control root, string text)
    {
        foreach (var textBox in FindRichTextBoxes(root))
        {
            if (textBox.Text.IndexOf(text, StringComparison.Ordinal) >= 0)
            {
                return true;
            }
        }

        return false;
    }

    private static IEnumerable<TabControl> FindTabControls(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is TabControl tabControl)
            {
                yield return tabControl;
            }

            foreach (var nested in FindTabControls(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasTabPageText(Control root, string text)
    {
        foreach (var tabControl in FindTabControls(root))
        {
            foreach (TabPage tabPage in tabControl.TabPages)
            {
                if (string.Equals(tabPage.Text, text, StringComparison.Ordinal))
                {
                    return true;
                }
            }
        }

        return false;
    }

    private static int CountNonWhitePixels(Bitmap bitmap)
    {
        var count = 0;
        for (var y = 0; y < bitmap.Height; y += 2)
        {
            for (var x = 0; x < bitmap.Width; x += 2)
            {
                if (bitmap.GetPixel(x, y).ToArgb() != Color.White.ToArgb())
                {
                    count++;
                }
            }
        }

        return count;
    }

    private static IEnumerable<CheckBox> FindCheckBoxes(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is CheckBox checkBox)
            {
                yield return checkBox;
            }

            foreach (var nested in FindCheckBoxes(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasCheckBoxText(Control root, string text)
    {
        foreach (var checkBox in FindCheckBoxes(root))
        {
            if (string.Equals(checkBox.Text, text, StringComparison.Ordinal))
            {
                return true;
            }
        }

        return false;
    }

    private static IEnumerable<Button> FindButtons(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is Button button)
            {
                yield return button;
            }

            foreach (var nested in FindButtons(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasButtonText(Control root, string text)
    {
        foreach (var button in FindButtons(root))
        {
            if (string.Equals(button.Text, text, StringComparison.Ordinal))
            {
                return true;
            }
        }

        return false;
    }

    private static void ExpectSelectionUpdate(CopperfinDesignerSelection selection, string propertyName, object value, string expectedSerializedValue, string message)
    {
        TypeDescriptor.GetProperties(selection)[propertyName]?.SetValue(selection, value);
        Expect(selection.TryGetUpdate(propertyName, out var targetName, out var serializedValue) &&
               string.Equals(targetName, propertyName, StringComparison.Ordinal) &&
               string.Equals(serializedValue, expectedSerializedValue, StringComparison.Ordinal),
            message);
    }

    private static void Expect(bool condition, string message)
    {
        if (condition)
        {
            Console.WriteLine("PASS: " + message);
            return;
        }

        Console.Error.WriteLine("FAIL: " + message);
        failures++;
    }
}
