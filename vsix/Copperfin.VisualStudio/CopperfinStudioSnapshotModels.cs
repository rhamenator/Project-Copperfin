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

internal sealed class CopperfinStudioSnapshotResult
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioSnapshotDocument? Document { get; set; }
}
