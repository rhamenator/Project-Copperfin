using System.Collections.Generic;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinStudioSnapshotEnvelope
{
    public string Status { get; set; } = string.Empty;
    public CopperfinStudioSnapshotDocument Document { get; set; } = new();
}

internal sealed class CopperfinStudioSnapshotDocument
{
    public string Path { get; set; } = string.Empty;
    public string DisplayName { get; set; } = string.Empty;
    public string Kind { get; set; } = string.Empty;
    public bool ReadOnly { get; set; }
    public bool LaunchedFromVisualStudio { get; set; }
    public bool HasSidecar { get; set; }
    public string SidecarPath { get; set; } = string.Empty;
    public string AssetFamily { get; set; } = string.Empty;
    public int IndexCount { get; set; }
    public string HeaderVersionDescription { get; set; } = string.Empty;
    public int FieldCount { get; set; }
    public int RecordCount { get; set; }
    public List<CopperfinStudioSnapshotField> Fields { get; set; } = new();
    public CopperfinStudioReportLayout? ReportLayout { get; set; }
    public List<CopperfinStudioSnapshotObject> Objects { get; set; } = new();
}

internal sealed class CopperfinStudioSnapshotField
{
    public string Name { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;
    public int Length { get; set; }
    public int DecimalCount { get; set; }
}

internal sealed class CopperfinStudioSnapshotObject
{
    public int RecordIndex { get; set; }
    public bool Deleted { get; set; }
    public string Title { get; set; } = string.Empty;
    public string Subtitle { get; set; } = string.Empty;
    public List<CopperfinStudioSnapshotProperty> Properties { get; set; } = new();
}

internal sealed class CopperfinStudioSnapshotProperty
{
    public string Name { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;
    public bool IsNull { get; set; }
    public string Value { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioReportLayout
{
    public bool IsLabel { get; set; }
    public string DocumentTitle { get; set; } = string.Empty;
    public List<CopperfinStudioNamedValue> Settings { get; set; } = new();
    public List<CopperfinStudioReportSection> Sections { get; set; } = new();
    public List<CopperfinStudioReportLayoutObject> UnplacedObjects { get; set; } = new();
}

internal sealed class CopperfinStudioNamedValue
{
    public string Name { get; set; } = string.Empty;
    public string Value { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioReportSection
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string BandKind { get; set; } = string.Empty;
    public int RecordIndex { get; set; }
    public int Top { get; set; }
    public int Height { get; set; }
    public List<CopperfinStudioReportLayoutObject> Objects { get; set; } = new();
}

internal sealed class CopperfinStudioReportLayoutObject
{
    public int RecordIndex { get; set; }
    public string ObjectKind { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Expression { get; set; } = string.Empty;
    public int Left { get; set; }
    public int Top { get; set; }
    public int Width { get; set; }
    public int Height { get; set; }
    public List<CopperfinStudioNamedValue> Highlights { get; set; } = new();
}

internal sealed class CopperfinStudioSnapshotResult
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioSnapshotDocument? Document { get; set; }
}
