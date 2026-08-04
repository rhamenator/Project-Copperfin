// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinDesignSurfaceTheme
{
    private CopperfinDesignSurfaceTheme(
        Color background,
        Color foreground,
        Color grid,
        Color pageFill,
        Color pageBorder,
        Color sectionFill,
        Color sectionBorder,
        Color sectionHeaderFill,
        Color sectionHeaderText,
        Color deletedFill,
        Color deletedBorder,
        Color deletedHeaderFill,
        Color deletedHeaderText,
        Color selectedBorder,
        Color selectedDeletedBorder,
        Color selectedHeaderFill,
        Color selectedDeletedHeaderFill,
        Color reportObjectFill,
        Color reportObjectSelectedFill,
        Color reportObjectBorder,
        Color reportObjectSelectedBorder,
        Color labelObjectFill,
        Color labelObjectSelectedFill,
        Color labelObjectBorder,
        Color labelObjectSelectedBorder,
        Color genericObjectFill,
        Color genericObjectSelectedFill,
        Color genericObjectBorder,
        Color genericObjectSelectedBorder,
        Color deletedObjectFill,
        Color deletedObjectSelectedFill,
        Color deletedObjectBorder,
        Color deletedObjectSelectedBorder)
    {
        Background = background;
        Foreground = foreground;
        Grid = grid;
        PageFill = pageFill;
        PageBorder = pageBorder;
        SectionFill = sectionFill;
        SectionBorder = sectionBorder;
        SectionHeaderFill = sectionHeaderFill;
        SectionHeaderText = sectionHeaderText;
        DeletedFill = deletedFill;
        DeletedBorder = deletedBorder;
        DeletedHeaderFill = deletedHeaderFill;
        DeletedHeaderText = deletedHeaderText;
        SelectedBorder = selectedBorder;
        SelectedDeletedBorder = selectedDeletedBorder;
        SelectedHeaderFill = selectedHeaderFill;
        SelectedDeletedHeaderFill = selectedDeletedHeaderFill;
        ReportObjectFill = reportObjectFill;
        ReportObjectSelectedFill = reportObjectSelectedFill;
        ReportObjectBorder = reportObjectBorder;
        ReportObjectSelectedBorder = reportObjectSelectedBorder;
        LabelObjectFill = labelObjectFill;
        LabelObjectSelectedFill = labelObjectSelectedFill;
        LabelObjectBorder = labelObjectBorder;
        LabelObjectSelectedBorder = labelObjectSelectedBorder;
        GenericObjectFill = genericObjectFill;
        GenericObjectSelectedFill = genericObjectSelectedFill;
        GenericObjectBorder = genericObjectBorder;
        GenericObjectSelectedBorder = genericObjectSelectedBorder;
        DeletedObjectFill = deletedObjectFill;
        DeletedObjectSelectedFill = deletedObjectSelectedFill;
        DeletedObjectBorder = deletedObjectBorder;
        DeletedObjectSelectedBorder = deletedObjectSelectedBorder;
    }

    public Color Background { get; }
    public Color Foreground { get; }
    public Color Grid { get; }
    public Color PageFill { get; }
    public Color PageBorder { get; }
    public Color SectionFill { get; }
    public Color SectionBorder { get; }
    public Color SectionHeaderFill { get; }
    public Color SectionHeaderText { get; }
    public Color DeletedFill { get; }
    public Color DeletedBorder { get; }
    public Color DeletedHeaderFill { get; }
    public Color DeletedHeaderText { get; }
    public Color SelectedBorder { get; }
    public Color SelectedDeletedBorder { get; }
    public Color SelectedHeaderFill { get; }
    public Color SelectedDeletedHeaderFill { get; }
    public Color ReportObjectFill { get; }
    public Color ReportObjectSelectedFill { get; }
    public Color ReportObjectBorder { get; }
    public Color ReportObjectSelectedBorder { get; }
    public Color LabelObjectFill { get; }
    public Color LabelObjectSelectedFill { get; }
    public Color LabelObjectBorder { get; }
    public Color LabelObjectSelectedBorder { get; }
    public Color GenericObjectFill { get; }
    public Color GenericObjectSelectedFill { get; }
    public Color GenericObjectBorder { get; }
    public Color GenericObjectSelectedBorder { get; }
    public Color DeletedObjectFill { get; }
    public Color DeletedObjectSelectedFill { get; }
    public Color DeletedObjectBorder { get; }
    public Color DeletedObjectSelectedBorder { get; }

    public static CopperfinDesignSurfaceTheme Default { get; } = new(
        Color.White,
        Color.FromArgb(28, 32, 39),
        Color.FromArgb(236, 239, 244),
        Color.FromArgb(248, 249, 252),
        Color.FromArgb(210, 214, 222),
        Color.White,
        Color.FromArgb(212, 218, 228),
        Color.FromArgb(233, 238, 247),
        Color.FromArgb(44, 52, 64),
        Color.FromArgb(255, 244, 244),
        Color.FromArgb(218, 176, 176),
        Color.FromArgb(252, 224, 224),
        Color.FromArgb(130, 41, 41),
        Color.FromArgb(174, 86, 24),
        Color.FromArgb(140, 52, 52),
        Color.FromArgb(255, 239, 220),
        Color.FromArgb(249, 212, 212),
        Color.FromArgb(214, 230, 250),
        Color.FromArgb(254, 220, 188),
        Color.FromArgb(52, 97, 164),
        Color.FromArgb(174, 86, 24),
        Color.FromArgb(224, 239, 214),
        Color.FromArgb(255, 230, 192),
        Color.FromArgb(64, 122, 70),
        Color.FromArgb(152, 86, 12),
        Color.FromArgb(205, 223, 247),
        Color.FromArgb(255, 211, 171),
        Color.FromArgb(68, 114, 196),
        Color.FromArgb(201, 96, 36),
        Color.FromArgb(246, 228, 225),
        Color.FromArgb(252, 220, 216),
        Color.FromArgb(166, 91, 84),
        Color.FromArgb(163, 63, 54));

    public static CopperfinDesignSurfaceTheme FromHostColors(
        Color background,
        Color foreground,
        bool highContrast = false)
    {
        if (highContrast)
        {
            return new CopperfinDesignSurfaceTheme(
                background,
                foreground,
                foreground,
                background,
                foreground,
                background,
                foreground,
                background,
                foreground,
                background,
                foreground,
                background,
                foreground,
                SystemColors.Highlight,
                SystemColors.Highlight,
                background,
                background,
                background,
                SystemColors.Highlight,
                foreground,
                SystemColors.Highlight,
                background,
                SystemColors.Highlight,
                foreground,
                SystemColors.Highlight,
                background,
                SystemColors.Highlight,
                foreground,
                SystemColors.Highlight,
                background,
                SystemColors.Highlight,
                foreground,
                SystemColors.Highlight);
        }

        var dark = background.GetBrightness() < 0.5F;
        var light = dark ? Color.White : Color.Black;
        var accent = Color.FromArgb(86, 156, 214);
        var selection = Color.FromArgb(224, 139, 54);
        var deleted = Color.FromArgb(205, 80, 80);
        var green = Color.FromArgb(105, 180, 115);

        Color Blend(Color color, float amount) => BlendColors(background, color, amount);

        return new CopperfinDesignSurfaceTheme(
            background,
            foreground,
            Blend(foreground, dark ? 0.22F : 0.16F),
            Blend(light, dark ? 0.08F : 0.035F),
            Blend(foreground, dark ? 0.34F : 0.22F),
            Blend(light, dark ? 0.06F : 0.02F),
            Blend(foreground, dark ? 0.30F : 0.20F),
            Blend(accent, dark ? 0.32F : 0.16F),
            foreground,
            Blend(deleted, dark ? 0.24F : 0.12F),
            Blend(deleted, dark ? 0.60F : 0.48F),
            Blend(deleted, dark ? 0.34F : 0.18F),
            Blend(deleted, dark ? 0.86F : 0.68F),
            Blend(selection, dark ? 0.78F : 0.58F),
            Blend(deleted, dark ? 0.78F : 0.62F),
            Blend(selection, dark ? 0.32F : 0.18F),
            Blend(deleted, dark ? 0.32F : 0.18F),
            Blend(accent, dark ? 0.42F : 0.22F),
            Blend(selection, dark ? 0.68F : 0.42F),
            Blend(accent, dark ? 0.80F : 0.58F),
            Blend(selection, dark ? 0.86F : 0.72F),
            Blend(green, dark ? 0.42F : 0.20F),
            Blend(selection, dark ? 0.76F : 0.50F),
            Blend(green, dark ? 0.78F : 0.56F),
            Blend(selection, dark ? 0.82F : 0.58F),
            Blend(accent, dark ? 0.35F : 0.18F),
            Blend(selection, dark ? 0.76F : 0.48F),
            Blend(accent, dark ? 0.74F : 0.52F),
            Blend(selection, dark ? 0.88F : 0.70F),
            Blend(deleted, dark ? 0.32F : 0.16F),
            Blend(deleted, dark ? 0.58F : 0.38F),
            Blend(deleted, dark ? 0.72F : 0.52F),
            Blend(deleted, dark ? 0.90F : 0.76F));
    }

    private static Color BlendColors(Color background, Color foreground, float amount)
    {
        amount = Math.Max(0.0F, Math.Min(1.0F, amount));
        return Color.FromArgb(
            255,
            (int)Math.Round(background.R + ((foreground.R - background.R) * amount)),
            (int)Math.Round(background.G + ((foreground.G - background.G) * amount)),
            (int)Math.Round(background.B + ((foreground.B - background.B) * amount)));
    }
}

internal sealed class CopperfinDesignSurfaceControl : Control
{
    private readonly CopperfinLocalization localization;

    private sealed class SurfaceObject
    {
        public CopperfinStudioSnapshotObject Source { get; set; } = null!;
        public int? ContainingReportSectionRecordIndex { get; set; }
        public bool InUnplacedReportObjects { get; set; }
        public RectangleF Bounds { get; set; }
        public Rectangle PixelBounds { get; set; }
        public string Caption { get; set; } = string.Empty;
    }

    private sealed class ReportSectionVisual
    {
        public bool Deleted { get; set; }
        public int RecordIndex { get; set; }
        public string Title { get; set; } = string.Empty;
        public string HeaderTitle { get; set; } = string.Empty;
        public string BandKind { get; set; } = string.Empty;
        public int Top { get; set; }
        public int Height { get; set; }
        public int DeletedObjectCount { get; set; }
        public Rectangle PixelBounds { get; set; }
        public Rectangle HeaderBounds { get; set; }
        public List<SurfaceObject> Objects { get; } = new();
    }

    private readonly List<SurfaceObject> objects = new();
    private readonly List<ReportSectionVisual> reportSections = new();
    private readonly List<SurfaceObject> unplacedReportObjects = new();
    private string assetFamily = string.Empty;
    private int? selectedRecordIndex;
    private int? selectedReportSectionRecordIndex;
    private bool unplacedReportObjectsSelected;
    private int? dragRecordIndex;
    private Point lastMousePoint;
    private Rectangle unplacedTrayBounds;
    private Rectangle unplacedTrayHeaderBounds;
    private CopperfinStudioReportLayout? reportLayout;
    private Color surfaceTextColor = CopperfinDesignSurfaceTheme.Default.Foreground;
    private Color surfaceGridColor = CopperfinDesignSurfaceTheme.Default.Grid;
    private CopperfinDesignSurfaceTheme theme = CopperfinDesignSurfaceTheme.Default;

    public event Action<int>? SelectedRecordChanged;
    public event Action<int>? SelectedReportSectionChanged;
    public event Action? SelectedUnplacedObjectsChanged;
    public event Action<int, int, int>? ObjectMoved;

    public CopperfinDesignSurfaceControl(CopperfinLocalization? localization = null)
    {
        this.localization = localization ?? CopperfinLocalization.FromEnvironment();
        DoubleBuffered = true;
        BackColor = theme.Background;
        MinimumSize = new Size(400, 260);
    }

    internal void ApplyVisualStudioHostTheme(
        Color background,
        Color foreground,
        bool highContrast = false)
    {
        theme = CopperfinDesignSurfaceTheme.FromHostColors(background, foreground, highContrast);
        BackColor = background;
        surfaceTextColor = foreground;
        surfaceGridColor = theme.Grid;
        Invalidate();
    }

    internal void ResetVisualStudioHostTheme()
    {
        theme = CopperfinDesignSurfaceTheme.Default;
        BackColor = theme.Background;
        surfaceTextColor = theme.Foreground;
        surfaceGridColor = theme.Grid;
        Invalidate();
    }

    public void LoadObjects(string assetFamily, IReadOnlyList<CopperfinStudioSnapshotObject> snapshotObjects)
    {
        this.assetFamily = assetFamily ?? string.Empty;
        reportLayout = null;
        reportSections.Clear();
        unplacedReportObjects.Clear();
        objects.Clear();
        selectedRecordIndex = null;
        selectedReportSectionRecordIndex = null;
        unplacedReportObjectsSelected = false;
        unplacedTrayBounds = Rectangle.Empty;
        unplacedTrayHeaderBounds = Rectangle.Empty;
        foreach (var snapshotObject in snapshotObjects)
        {
            if (!TryBuildBounds(this.assetFamily, snapshotObject, out var bounds))
            {
                continue;
            }

            objects.Add(new SurfaceObject
            {
                Source = snapshotObject,
                Bounds = bounds,
                PixelBounds = Rectangle.Empty,
                Caption = BuildObjectCaption(this.assetFamily, snapshotObject)
            });
        }

        Invalidate();
    }

    public void LoadReportLayout(CopperfinStudioReportLayout layout, IReadOnlyList<CopperfinStudioSnapshotObject> snapshotObjects)
    {
        assetFamily = layout.IsLabel ? "label" : "report";
        reportLayout = layout;
        reportSections.Clear();
        unplacedReportObjects.Clear();
        objects.Clear();
        selectedRecordIndex = null;
        selectedReportSectionRecordIndex = null;
        unplacedReportObjectsSelected = false;
        unplacedTrayBounds = Rectangle.Empty;
        unplacedTrayHeaderBounds = Rectangle.Empty;

        var lookup = snapshotObjects.ToDictionary(item => item.RecordIndex);
        foreach (var section in layout.Sections)
        {
            reportSections.Add(BuildReportSectionVisual(section, deleted: false, lookup, layout.DeletedObjects));
        }

        foreach (var section in layout.DeletedSections)
        {
            reportSections.Add(BuildReportSectionVisual(section, deleted: true, lookup, layout.DeletedObjects));
        }

        foreach (var layoutObject in layout.UnplacedObjects)
        {
            if (!lookup.TryGetValue(layoutObject.RecordIndex, out var snapshotObject))
            {
                continue;
            }

            var bounds = new RectangleF(
                layoutObject.Left,
                layoutObject.Top,
                Math.Max(120, layoutObject.Width),
                Math.Max(120, layoutObject.Height));

            var surfaceObject = new SurfaceObject
            {
                Source = snapshotObject,
                InUnplacedReportObjects = true,
                Bounds = bounds,
                PixelBounds = Rectangle.Empty,
                Caption = string.IsNullOrWhiteSpace(layoutObject.Title)
                    ? BuildObjectCaption(assetFamily, snapshotObject)
                    : layoutObject.Title
            };

            unplacedReportObjects.Add(surfaceObject);
            objects.Add(surfaceObject);
        }

        Invalidate();
    }

    public void SelectRecord(int? recordIndex)
    {
        selectedRecordIndex = recordIndex;
        if (!recordIndex.HasValue)
        {
            selectedReportSectionRecordIndex = null;
            unplacedReportObjectsSelected = false;
            Invalidate();
            return;
        }

        var selectedObject = objects.FirstOrDefault(item => item.Source.RecordIndex == recordIndex.Value);
        selectedReportSectionRecordIndex = selectedObject?.ContainingReportSectionRecordIndex;
        unplacedReportObjectsSelected = selectedObject?.InUnplacedReportObjects == true;
        Invalidate();
    }

    public void SelectReportSection(int? recordIndex)
    {
        selectedReportSectionRecordIndex = recordIndex;
        selectedRecordIndex = null;
        unplacedReportObjectsSelected = false;
        Invalidate();
    }

    public void SelectUnplacedObjects()
    {
        unplacedReportObjectsSelected = true;
        selectedRecordIndex = null;
        selectedReportSectionRecordIndex = null;
        Invalidate();
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);

        e.Graphics.Clear(BackColor);
        e.Graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

        if (reportLayout is not null && (reportSections.Count > 0 || unplacedReportObjects.Count > 0))
        {
            PaintReportLayout(e);
            return;
        }

        PaintGenericLayout(e);
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        base.OnMouseDown(e);

        var hit = objects.LastOrDefault(item => item.PixelBounds.Contains(e.Location));
        if (hit is null)
        {
            if (TrySelectReportScope(e.Location))
            {
                return;
            }

            return;
        }

        SelectRecord(hit.Source.RecordIndex);
        dragRecordIndex = hit.Source.RecordIndex;
        lastMousePoint = e.Location;
        SelectedRecordChanged?.Invoke(hit.Source.RecordIndex);
    }

    protected override void OnMouseMove(MouseEventArgs e)
    {
        base.OnMouseMove(e);

        if (dragRecordIndex is null || e.Button != MouseButtons.Left)
        {
            return;
        }

        Cursor = Cursors.SizeAll;
    }

    protected override void OnMouseUp(MouseEventArgs e)
    {
        base.OnMouseUp(e);

        if (dragRecordIndex is null)
        {
            return;
        }

        Cursor = Cursors.Default;

        var moved = objects.FirstOrDefault(item => item.Source.RecordIndex == dragRecordIndex.Value);
        if (moved is null)
        {
            dragRecordIndex = null;
            return;
        }

        var scale = reportLayout is not null
            ? CalculateReportScale()
            : CalculateGenericScale();

        var deltaX = e.Location.X - lastMousePoint.X;
        var deltaY = e.Location.Y - lastMousePoint.Y;
        if (Math.Abs(deltaX) > 0 || Math.Abs(deltaY) > 0)
        {
            var (horizontalName, verticalName) = GetCoordinatePropertyNames(assetFamily);
            var left = ExtractNumericProperty(moved.Source, horizontalName);
            var top = ExtractNumericProperty(moved.Source, verticalName);
            if (left.HasValue && top.HasValue)
            {
                var newLeft = Math.Max(0, (int)Math.Round(left.Value + (deltaX / Math.Max(0.2F, scale))));
                var newTop = Math.Max(0, (int)Math.Round(top.Value + (deltaY / Math.Max(0.2F, scale))));
                ObjectMoved?.Invoke(moved.Source.RecordIndex, newLeft, newTop);
            }
        }

        dragRecordIndex = null;
    }

    private void PaintGenericLayout(PaintEventArgs e)
    {
        if (objects.Count == 0)
        {
            using var brush = new SolidBrush(surfaceTextColor);
            e.Graphics.DrawString(
                this.localization.Text("AssetEditor.DesignSurface.NoObjectsWithLayout"),
                Font,
                brush,
                new RectangleF(24, 24, Width - 48, Height - 48));
            return;
        }

        var logicalBounds = CalculateLogicalBounds();
        const float padding = 24.0F;
        var availableWidth = Math.Max(80.0F, Width - (padding * 2.0F));
        var availableHeight = Math.Max(80.0F, Height - (padding * 2.0F));
        var scaleX = availableWidth / Math.Max(1.0F, logicalBounds.Width);
        var scaleY = availableHeight / Math.Max(1.0F, logicalBounds.Height);
        var scale = Math.Max(0.2F, Math.Min(scaleX, scaleY));

        using var gridPen = new Pen(surfaceGridColor);
        for (var x = padding; x < Width - padding; x += 24)
        {
            e.Graphics.DrawLine(gridPen, x, padding, x, Height - padding);
        }
        for (var y = padding; y < Height - padding; y += 24)
        {
            e.Graphics.DrawLine(gridPen, padding, y, Width - padding, y);
        }

        for (var index = 0; index < objects.Count; ++index)
        {
            var item = objects[index];
            var pixelBounds = new Rectangle(
                (int)Math.Round(padding + ((item.Bounds.Left - logicalBounds.Left) * scale)),
                (int)Math.Round(padding + ((item.Bounds.Top - logicalBounds.Top) * scale)),
                Math.Max(24, (int)Math.Round(item.Bounds.Width * scale)),
                Math.Max(18, (int)Math.Round(item.Bounds.Height * scale)));

            item.PixelBounds = pixelBounds;
            objects[index] = item;
            DrawSurfaceObject(e.Graphics, item, selectedRecordIndex == item.Source.RecordIndex, assetFamily);
        }
    }

    private void PaintReportLayout(PaintEventArgs e)
    {
        using var pageFill = new SolidBrush(theme.PageFill);
        using var pageBorder = new Pen(theme.PageBorder);
        using var textBrush = new SolidBrush(surfaceTextColor);
        using var sectionFill = new SolidBrush(theme.SectionFill);
        using var sectionBorder = new Pen(theme.SectionBorder);
        using var sectionHeaderFill = new SolidBrush(theme.SectionHeaderFill);
        using var sectionHeaderText = new SolidBrush(theme.SectionHeaderText);
        using var deletedSectionFill = new SolidBrush(theme.DeletedFill);
        using var deletedSectionBorder = new Pen(theme.DeletedBorder);
        using var deletedSectionHeaderFill = new SolidBrush(theme.DeletedHeaderFill);
        using var deletedSectionHeaderText = new SolidBrush(theme.DeletedHeaderText);
        using var selectedSectionBorder = new Pen(theme.SelectedBorder, 2.0F);
        using var selectedDeletedSectionBorder = new Pen(theme.SelectedDeletedBorder, 2.0F);
        using var selectedSectionHeaderFill = new SolidBrush(theme.SelectedHeaderFill);
        using var selectedDeletedSectionHeaderFill = new SolidBrush(theme.SelectedDeletedHeaderFill);

        const int outerPadding = 24;
        const int headerHeight = 28;
        const int sectionSpacing = 12;
        const int sectionInnerPadding = 12;
        const int leftGutter = 32;
        const int unplacedMinimumHeight = 64;

        var maxRight = reportSections
            .SelectMany(section => section.Objects)
            .Select(item => item.Bounds.Right)
            .Concat(unplacedReportObjects.Select(item => item.Bounds.Right))
            .DefaultIfEmpty(40000.0F)
            .Max();
        var unplacedLogicalTop = unplacedReportObjects.Count == 0
            ? 0
            : unplacedReportObjects.Min(item => item.Bounds.Top);
        var unplacedLogicalHeight = unplacedReportObjects.Count == 0
            ? 0
            : Math.Max(
                unplacedMinimumHeight,
                (int)Math.Round(unplacedReportObjects.Max(item => item.Bounds.Bottom) - unplacedLogicalTop));
        var totalSectionHeight = reportSections.Sum(section => Math.Max(400, section.Height)) +
                                 (reportSections.Count * (headerHeight + sectionSpacing + sectionInnerPadding * 2)) +
                                 (unplacedReportObjects.Count == 0 ? 0 : headerHeight + sectionSpacing + sectionInnerPadding * 2 + unplacedLogicalHeight);

        var availableWidth = Math.Max(400, Width - (outerPadding * 2) - leftGutter);
        var availableHeight = Math.Max(200, Height - (outerPadding * 2));
        var scaleX = availableWidth / Math.Max(1.0F, maxRight);
        var scaleY = availableHeight / Math.Max(1.0F, totalSectionHeight);
        var scale = Math.Max(0.12F, Math.Min(scaleX, scaleY));

        var pageBounds = new Rectangle(
            outerPadding,
            outerPadding,
            Width - (outerPadding * 2),
            Height - (outerPadding * 2));
        e.Graphics.FillRectangle(pageFill, pageBounds);
        e.Graphics.DrawRectangle(pageBorder, pageBounds);

        var currentY = outerPadding + 12;
        for (var sectionIndex = 0; sectionIndex < reportSections.Count; ++sectionIndex)
        {
            var section = reportSections[sectionIndex];
            var scaledHeight = Math.Max(72, (int)Math.Round(Math.Max(400, section.Height) * scale));
            var sectionBounds = new Rectangle(
                outerPadding + leftGutter,
                currentY,
                Math.Max(180, (int)Math.Round(maxRight * scale)),
                headerHeight + (sectionInnerPadding * 2) + scaledHeight);
            var headerBounds = new Rectangle(sectionBounds.X, sectionBounds.Y, sectionBounds.Width, headerHeight);

            section.PixelBounds = sectionBounds;
            section.HeaderBounds = headerBounds;

            var sectionSelected = selectedReportSectionRecordIndex == section.RecordIndex;
            var currentSectionFill = section.Deleted ? deletedSectionFill : sectionFill;
            var currentSectionBorder = sectionSelected
                ? (section.Deleted ? selectedDeletedSectionBorder : selectedSectionBorder)
                : (section.Deleted ? deletedSectionBorder : sectionBorder);
            var currentSectionHeaderFill = sectionSelected
                ? (section.Deleted ? selectedDeletedSectionHeaderFill : selectedSectionHeaderFill)
                : (section.Deleted ? deletedSectionHeaderFill : sectionHeaderFill);
            var currentSectionHeaderText = section.Deleted ? deletedSectionHeaderText : sectionHeaderText;
            e.Graphics.FillRectangle(currentSectionFill, sectionBounds);
            e.Graphics.DrawRectangle(currentSectionBorder, sectionBounds);
            e.Graphics.FillRectangle(currentSectionHeaderFill, headerBounds);
            e.Graphics.DrawRectangle(currentSectionBorder, headerBounds);

            e.Graphics.DrawString(section.HeaderTitle, Font, currentSectionHeaderText, headerBounds.X + 10, headerBounds.Y + 6);
            using (var smallFont = new Font(Font.FontFamily, Math.Max(8.0F, Font.Size - 1.0F), FontStyle.Regular))
            {
                e.Graphics.DrawString(
                    BuildReportBandKindDisplayText(section.BandKind),
                    smallFont,
                    section.Deleted ? deletedSectionHeaderText : textBrush,
                    headerBounds.Right - 140,
                    headerBounds.Y + 7);
            }

            var bodyTop = headerBounds.Bottom + sectionInnerPadding;
            foreach (var item in section.Objects)
            {
                var relativeTop = Math.Max(0, item.Bounds.Top - section.Top);
                item.PixelBounds = new Rectangle(
                    sectionBounds.X + 12 + (int)Math.Round(item.Bounds.Left * scale),
                    bodyTop + (int)Math.Round(relativeTop * scale),
                    Math.Max(30, (int)Math.Round(item.Bounds.Width * scale)),
                    Math.Max(18, (int)Math.Round(item.Bounds.Height * scale)));
                DrawSurfaceObject(e.Graphics, item, selectedRecordIndex == item.Source.RecordIndex, assetFamily);
            }

            using var gutterBrush = new SolidBrush(surfaceTextColor);
            e.Graphics.DrawString($"{sectionIndex + 1}", Font, gutterBrush, outerPadding + 6, currentY + 6);
            currentY += sectionBounds.Height + sectionSpacing;
        }

        if (unplacedReportObjects.Count == 0)
        {
            return;
        }

        var unplacedScaledHeight = Math.Max(unplacedMinimumHeight, (int)Math.Round(unplacedLogicalHeight * scale));
        var unplacedBounds = new Rectangle(
            outerPadding + leftGutter,
            currentY,
            Math.Max(180, (int)Math.Round(maxRight * scale)),
            headerHeight + (sectionInnerPadding * 2) + unplacedScaledHeight);
        var unplacedHeaderBounds = new Rectangle(unplacedBounds.X, unplacedBounds.Y, unplacedBounds.Width, headerHeight);
        unplacedTrayBounds = unplacedBounds;
        unplacedTrayHeaderBounds = unplacedHeaderBounds;

        var currentUnplacedBorder = unplacedReportObjectsSelected ? selectedSectionBorder : sectionBorder;
        var currentUnplacedHeaderFill = unplacedReportObjectsSelected ? selectedSectionHeaderFill : sectionHeaderFill;
        e.Graphics.FillRectangle(sectionFill, unplacedBounds);
        e.Graphics.DrawRectangle(currentUnplacedBorder, unplacedBounds);
        e.Graphics.FillRectangle(currentUnplacedHeaderFill, unplacedHeaderBounds);
        e.Graphics.DrawRectangle(currentUnplacedBorder, unplacedHeaderBounds);
        e.Graphics.DrawString(
            BuildUnplacedTrayTitle(unplacedReportObjects.Count),
            Font,
            sectionHeaderText,
            unplacedHeaderBounds.X + 10,
            unplacedHeaderBounds.Y + 6);

        var unplacedBodyTop = unplacedHeaderBounds.Bottom + sectionInnerPadding;
        foreach (var item in unplacedReportObjects)
        {
            var relativeTop = Math.Max(0, item.Bounds.Top - unplacedLogicalTop);
            item.PixelBounds = new Rectangle(
                unplacedBounds.X + 12 + (int)Math.Round(item.Bounds.Left * scale),
                unplacedBodyTop + (int)Math.Round(relativeTop * scale),
                Math.Max(30, (int)Math.Round(item.Bounds.Width * scale)),
                Math.Max(18, (int)Math.Round(item.Bounds.Height * scale)));
            DrawSurfaceObject(e.Graphics, item, selectedRecordIndex == item.Source.RecordIndex, assetFamily);
        }
    }

    private void DrawSurfaceObject(Graphics graphics, SurfaceObject item, bool selected, string assetFamily)
    {
        var deleted = item.Source.Deleted;
        var fillColor = deleted
            ? (selected ? theme.DeletedObjectSelectedFill : theme.DeletedObjectFill)
            : assetFamily switch
            {
            "report" => selected ? theme.ReportObjectSelectedFill : theme.ReportObjectFill,
            "label" => selected ? theme.LabelObjectSelectedFill : theme.LabelObjectFill,
            _ => selected ? theme.GenericObjectSelectedFill : theme.GenericObjectFill
            };
        var borderColor = deleted
            ? (selected ? theme.DeletedObjectSelectedBorder : theme.DeletedObjectBorder)
            : assetFamily switch
            {
            "report" => selected ? theme.ReportObjectSelectedBorder : theme.ReportObjectBorder,
            "label" => selected ? theme.LabelObjectSelectedBorder : theme.LabelObjectBorder,
            _ => selected ? theme.GenericObjectSelectedBorder : theme.GenericObjectBorder
            };

        using var fill = new SolidBrush(fillColor);
        using var border = new Pen(borderColor, selected ? 2.2F : 1.4F);
        using var textBrush = new SolidBrush(surfaceTextColor);
        graphics.FillRectangle(fill, item.PixelBounds);
        graphics.DrawRectangle(border, item.PixelBounds);

        var captionBounds = new RectangleF(
            item.PixelBounds.X + 4,
            item.PixelBounds.Y + 3,
            item.PixelBounds.Width - 8,
            item.PixelBounds.Height - 6);
        graphics.DrawString(item.Caption, SystemFonts.MessageBoxFont, textBrush, captionBounds);
    }

    private float CalculateGenericScale()
    {
        if (objects.Count == 0)
        {
            return 1.0F;
        }

        var logicalBounds = CalculateLogicalBounds();
        const float padding = 24.0F;
        var availableWidth = Math.Max(80.0F, Width - (padding * 2.0F));
        var availableHeight = Math.Max(80.0F, Height - (padding * 2.0F));
        var scaleX = availableWidth / Math.Max(1.0F, logicalBounds.Width);
        var scaleY = availableHeight / Math.Max(1.0F, logicalBounds.Height);
        return Math.Max(0.2F, Math.Min(scaleX, scaleY));
    }

    private float CalculateReportScale()
    {
        if (reportSections.Count == 0 && unplacedReportObjects.Count == 0)
        {
            return 1.0F;
        }

        const int outerPadding = 24;
        const int leftGutter = 32;
        const int headerHeight = 28;
        const int sectionSpacing = 12;
        const int sectionInnerPadding = 12;
        const int unplacedMinimumHeight = 64;

        var maxRight = reportSections
            .SelectMany(section => section.Objects)
            .Select(item => item.Bounds.Right)
            .Concat(unplacedReportObjects.Select(item => item.Bounds.Right))
            .DefaultIfEmpty(40000.0F)
            .Max();
        var unplacedLogicalTop = unplacedReportObjects.Count == 0
            ? 0
            : unplacedReportObjects.Min(item => item.Bounds.Top);
        var unplacedLogicalHeight = unplacedReportObjects.Count == 0
            ? 0
            : Math.Max(
                unplacedMinimumHeight,
                (int)Math.Round(unplacedReportObjects.Max(item => item.Bounds.Bottom) - unplacedLogicalTop));
        var totalSectionHeight = reportSections.Sum(section => Math.Max(400, section.Height)) +
                                 (reportSections.Count * (headerHeight + sectionSpacing + sectionInnerPadding * 2)) +
                                 (unplacedReportObjects.Count == 0 ? 0 : headerHeight + sectionSpacing + sectionInnerPadding * 2 + unplacedLogicalHeight);
        var availableWidth = Math.Max(400, Width - (outerPadding * 2) - leftGutter);
        var availableHeight = Math.Max(200, Height - (outerPadding * 2));
        var scaleX = availableWidth / Math.Max(1.0F, maxRight);
        var scaleY = availableHeight / Math.Max(1.0F, totalSectionHeight);
        return Math.Max(0.12F, Math.Min(scaleX, scaleY));
    }

    private bool TrySelectReportScope(Point location)
    {
        if (reportLayout is null)
        {
            return false;
        }

        var section = reportSections.LastOrDefault(candidate => candidate.PixelBounds.Contains(location));
        if (section is not null)
        {
            selectedRecordIndex = null;
            selectedReportSectionRecordIndex = section.RecordIndex;
            unplacedReportObjectsSelected = false;
            dragRecordIndex = null;
            SelectedReportSectionChanged?.Invoke(section.RecordIndex);
            Invalidate();
            return true;
        }

        if (unplacedReportObjects.Count > 0 && unplacedTrayBounds.Contains(location))
        {
            selectedRecordIndex = null;
            selectedReportSectionRecordIndex = null;
            unplacedReportObjectsSelected = true;
            dragRecordIndex = null;
            SelectedUnplacedObjectsChanged?.Invoke();
            Invalidate();
            return true;
        }

        return false;
    }

    private ReportSectionVisual BuildReportSectionVisual(
        CopperfinStudioReportSection section,
        bool deleted,
        IReadOnlyDictionary<int, CopperfinStudioSnapshotObject> lookup,
        IReadOnlyList<CopperfinStudioReportLayoutObject> deletedObjects)
    {
        var visual = new ReportSectionVisual
        {
            Deleted = deleted,
            RecordIndex = section.RecordIndex,
            Title = section.Title,
            HeaderTitle = deleted
                ? BuildDeletedReportSectionHeaderTitle(BuildReportSectionHeaderTitle(section.Title, section.DeletedObjectCount))
                : BuildReportSectionHeaderTitle(section.Title, section.DeletedObjectCount),
            BandKind = section.BandKind,
            Top = section.Top,
            Height = Math.Max(400, section.Height),
            DeletedObjectCount = section.DeletedObjectCount
        };

        var visibleObjects = section.Objects
            .Concat(deletedObjects.Where(item => item.ContainingSectionRecordIndex == section.RecordIndex))
            .OrderBy(item => item.Top)
            .ThenBy(item => item.Left)
            .ThenBy(item => item.RecordIndex);

        foreach (var layoutObject in visibleObjects)
        {
            if (!lookup.TryGetValue(layoutObject.RecordIndex, out var snapshotObject))
            {
                continue;
            }

            var bounds = new RectangleF(
                layoutObject.Left,
                layoutObject.Top,
                Math.Max(120, layoutObject.Width),
                Math.Max(120, layoutObject.Height));

            var surfaceObject = new SurfaceObject
            {
                Source = snapshotObject,
                ContainingReportSectionRecordIndex = section.RecordIndex,
                Bounds = bounds,
                PixelBounds = Rectangle.Empty,
                Caption = string.IsNullOrWhiteSpace(layoutObject.Title)
                    ? BuildObjectCaption(assetFamily, snapshotObject)
                    : layoutObject.Title
            };

            visual.Objects.Add(surfaceObject);
            objects.Add(surfaceObject);
        }

        return visual;
    }

    private string BuildReportSectionHeaderTitle(string title, int deletedObjectCount)
    {
        if (deletedObjectCount <= 0)
        {
            return title;
        }

        return this.localization.Format(
            "AssetEditor.DesignSurface.ReportSectionDeletedObjects",
            title,
            deletedObjectCount);
    }

    private string BuildUnplacedTrayTitle(int count)
    {
        return this.localization.Format("AssetEditor.DesignSurface.UnplacedObjects", count);
    }

    private string BuildDeletedReportSectionHeaderTitle(string title)
    {
        return this.localization.Format("AssetEditor.ReportSection.Deleted", title);
    }

    private string BuildReportBandKindDisplayText(string bandKind)
    {
        var key = bandKind switch
        {
            "title" => "AssetEditor.ReportBandKind.Title",
            "page_header" => "AssetEditor.ReportBandKind.PageHeader",
            "column_header" => "AssetEditor.ReportBandKind.ColumnHeader",
            "group_header" => "AssetEditor.ReportBandKind.GroupHeader",
            "detail" => "AssetEditor.ReportBandKind.Detail",
            "detail_header" => "AssetEditor.ReportBandKind.DetailHeader",
            "detail_footer" => "AssetEditor.ReportBandKind.DetailFooter",
            "group_footer" => "AssetEditor.ReportBandKind.GroupFooter",
            "column_footer" => "AssetEditor.ReportBandKind.ColumnFooter",
            "page_footer" => "AssetEditor.ReportBandKind.PageFooter",
            "summary" => "AssetEditor.ReportBandKind.Summary",
            "other" => "AssetEditor.ReportBandKind.Other",
            _ => string.Empty
        };

        if (!string.IsNullOrWhiteSpace(key))
        {
            return this.localization.Text(key);
        }

        return bandKind.Replace('_', ' ');
    }

    private RectangleF CalculateLogicalBounds()
    {
        var minLeft = objects.Min(item => item.Bounds.Left);
        var minTop = objects.Min(item => item.Bounds.Top);
        var maxRight = objects.Max(item => item.Bounds.Right);
        var maxBottom = objects.Max(item => item.Bounds.Bottom);

        return RectangleF.FromLTRB(minLeft, minTop, maxRight, maxBottom);
    }

    private static bool TryBuildBounds(string assetFamily, CopperfinStudioSnapshotObject snapshotObject, out RectangleF bounds)
    {
        bounds = RectangleF.Empty;
        var (horizontalName, verticalName) = GetCoordinatePropertyNames(assetFamily);
        var (widthName, heightName) = GetSizePropertyNames(assetFamily);
        var left = ExtractNumericProperty(snapshotObject, horizontalName);
        var top = ExtractNumericProperty(snapshotObject, verticalName);
        var width = ExtractNumericProperty(snapshotObject, widthName);
        var height = ExtractNumericProperty(snapshotObject, heightName);
        if (!left.HasValue || !top.HasValue || !width.HasValue || !height.HasValue)
        {
            return false;
        }

        bounds = new RectangleF(left.Value, top.Value, Math.Max(8, width.Value), Math.Max(8, height.Value));
        return true;
    }

    private static (string Horizontal, string Vertical) GetCoordinatePropertyNames(string assetFamily)
    {
        return assetFamily is "report" or "label"
            ? ("HPOS", "VPOS")
            : ("Left", "Top");
    }

    private static (string Width, string Height) GetSizePropertyNames(string assetFamily)
    {
        return assetFamily is "report" or "label"
            ? ("WIDTH", "HEIGHT")
            : ("Width", "Height");
    }

    private static int? ExtractNumericProperty(CopperfinStudioSnapshotObject snapshotObject, string propertyName)
    {
        var property = snapshotObject.Properties.FirstOrDefault(item => item.Name == propertyName);
        if (property is null)
        {
            return null;
        }

        if (double.TryParse(
                property.Value,
                NumberStyles.Float,
                CultureInfo.InvariantCulture,
                out var floating))
        {
            return (int)Math.Round(floating);
        }
        return null;
    }

    private string BuildObjectCaption(string assetFamily, CopperfinStudioSnapshotObject snapshotObject)
    {
        string[] candidateNames = assetFamily switch
        {
            "report" or "label" => new[] { "EXPR", "NAME" },
            "menu" => new[] { "PROMPT", "NAME" },
            _ => new[] { "Caption", "OBJNAME", "NAME" }
        };

        foreach (var candidate in candidateNames)
        {
            var caption = snapshotObject.Properties.FirstOrDefault(item => item.Name == candidate)?.Value ?? string.Empty;
            if (!string.IsNullOrWhiteSpace(caption) && caption != "<memo block 0>")
            {
                return caption.Trim('"');
            }
        }

        return string.IsNullOrWhiteSpace(snapshotObject.Title)
            ? BuildFallbackObjectTitle(snapshotObject.RecordIndex)
            : snapshotObject.Title;
    }

    private string BuildFallbackObjectTitle(int recordIndex)
    {
        return this.localization.Format("AssetEditor.ObjectFallbackTitle", recordIndex);
    }
}
