using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinDesignSurfaceControl : Control
{
    private sealed class SurfaceObject
    {
        public CopperfinStudioSnapshotObject Source { get; set; } = null!;
        public RectangleF Bounds { get; set; }
        public Rectangle PixelBounds { get; set; }
        public string Caption { get; set; } = string.Empty;
    }

    private readonly List<SurfaceObject> objects = new();
    private string assetFamily = string.Empty;
    private int? selectedRecordIndex;
    private int? dragRecordIndex;
    private Point lastMousePoint;

    public event Action<int>? SelectedRecordChanged;
    public event Action<int, int, int>? ObjectMoved;

    public CopperfinDesignSurfaceControl()
    {
        DoubleBuffered = true;
        BackColor = Color.White;
        MinimumSize = new Size(400, 260);
    }

    public void LoadObjects(string assetFamily, IReadOnlyList<CopperfinStudioSnapshotObject> snapshotObjects)
    {
        this.assetFamily = assetFamily ?? string.Empty;
        objects.Clear();
        foreach (var snapshotObject in snapshotObjects) {
            if (!TryBuildBounds(this.assetFamily, snapshotObject, out var bounds)) {
                continue;
            }

            objects.Add(new SurfaceObject
            {
                Source = snapshotObject,
                Bounds = bounds,
                PixelBounds = Rectangle.Empty,
                Caption = ExtractCaption(this.assetFamily, snapshotObject)
            });
        }

        Invalidate();
    }

    public void SelectRecord(int? recordIndex)
    {
        selectedRecordIndex = recordIndex;
        Invalidate();
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);

        e.Graphics.Clear(BackColor);
        e.Graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

        if (objects.Count == 0) {
            using var brush = new SolidBrush(Color.FromArgb(96, 102, 118));
            e.Graphics.DrawString(
                "No visual objects with layout coordinates are available yet.",
                Font,
                brush,
                new RectangleF(24, 24, Width - 48, Height - 48));
            return;
        }

        var logicalBounds = CalculateLogicalBounds();
        const float padding = 24.0F;
        var availableWidth = Math.Max(80.0F, Width - (padding * 2.0F));
        var availableHeight = Math.Max(80.0F, Height - (padding * 2.0F));
        var scaleX = availableWidth / logicalBounds.Width;
        var scaleY = availableHeight / logicalBounds.Height;
        var scale = Math.Max(0.2F, Math.Min(scaleX, scaleY));

        using var gridPen = new Pen(Color.FromArgb(236, 239, 244));
        for (var x = padding; x < Width - padding; x += 24) {
            e.Graphics.DrawLine(gridPen, x, padding, x, Height - padding);
        }
        for (var y = padding; y < Height - padding; y += 24) {
            e.Graphics.DrawLine(gridPen, padding, y, Width - padding, y);
        }

        for (var index = 0; index < objects.Count; ++index) {
            var item = objects[index];
            var pixelBounds = new Rectangle(
                (int)Math.Round(padding + ((item.Bounds.Left - logicalBounds.Left) * scale)),
                (int)Math.Round(padding + ((item.Bounds.Top - logicalBounds.Top) * scale)),
                Math.Max(24, (int)Math.Round(item.Bounds.Width * scale)),
                Math.Max(18, (int)Math.Round(item.Bounds.Height * scale)));

            objects[index] = new SurfaceObject
            {
                Source = item.Source,
                Bounds = item.Bounds,
                PixelBounds = pixelBounds,
                Caption = item.Caption
            };

            var selected = selectedRecordIndex == item.Source.RecordIndex;
            using var fill = new SolidBrush(selected ? Color.FromArgb(255, 211, 171) : Color.FromArgb(205, 223, 247));
            using var border = new Pen(selected ? Color.FromArgb(201, 96, 36) : Color.FromArgb(68, 114, 196), selected ? 2.2F : 1.5F);
            using var textBrush = new SolidBrush(Color.FromArgb(28, 32, 39));

            e.Graphics.FillRectangle(fill, pixelBounds);
            e.Graphics.DrawRectangle(border, pixelBounds);
            e.Graphics.DrawString(item.Caption, Font, textBrush, pixelBounds.X + 4, pixelBounds.Y + 4);
        }
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        base.OnMouseDown(e);

        var hit = objects.LastOrDefault(item => item.PixelBounds.Contains(e.Location));
        if (hit is null) {
            return;
        }

        selectedRecordIndex = hit.Source.RecordIndex;
        dragRecordIndex = hit.Source.RecordIndex;
        lastMousePoint = e.Location;
        SelectedRecordChanged?.Invoke(hit.Source.RecordIndex);
        Invalidate();
    }

    protected override void OnMouseMove(MouseEventArgs e)
    {
        base.OnMouseMove(e);

        if (dragRecordIndex is null || e.Button != MouseButtons.Left) {
            return;
        }

        Cursor = Cursors.SizeAll;
    }

    protected override void OnMouseUp(MouseEventArgs e)
    {
        base.OnMouseUp(e);

        if (dragRecordIndex is null) {
            return;
        }

        Cursor = Cursors.Default;

        var moved = objects.FirstOrDefault(item => item.Source.RecordIndex == dragRecordIndex.Value);
        if (moved is null) {
            dragRecordIndex = null;
            return;
        }

        var logicalBounds = CalculateLogicalBounds();
        const float padding = 24.0F;
        var availableWidth = Math.Max(80.0F, Width - (padding * 2.0F));
        var availableHeight = Math.Max(80.0F, Height - (padding * 2.0F));
        var scaleX = availableWidth / logicalBounds.Width;
        var scaleY = availableHeight / logicalBounds.Height;
        var scale = Math.Max(0.2F, Math.Min(scaleX, scaleY));

        var deltaX = e.Location.X - lastMousePoint.X;
        var deltaY = e.Location.Y - lastMousePoint.Y;
        if (Math.Abs(deltaX) > 0 || Math.Abs(deltaY) > 0) {
            var (horizontalName, verticalName) = GetCoordinatePropertyNames(assetFamily);
            var left = ExtractNumericProperty(moved.Source, horizontalName);
            var top = ExtractNumericProperty(moved.Source, verticalName);
            if (left.HasValue && top.HasValue) {
                var newLeft = Math.Max(0, (int)Math.Round(left.Value + (deltaX / scale)));
                var newTop = Math.Max(0, (int)Math.Round(top.Value + (deltaY / scale)));
                ObjectMoved?.Invoke(moved.Source.RecordIndex, newLeft, newTop);
            }
        }

        dragRecordIndex = null;
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
        if (!left.HasValue || !top.HasValue || !width.HasValue || !height.HasValue) {
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
        if (property is null) {
            return null;
        }
        return int.TryParse(property.Value, out var value) ? value : null;
    }

    private static string ExtractCaption(string assetFamily, CopperfinStudioSnapshotObject snapshotObject)
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

        return string.IsNullOrWhiteSpace(snapshotObject.Title) ? $"Record {snapshotObject.RecordIndex}" : snapshotObject.Title;
    }
}
