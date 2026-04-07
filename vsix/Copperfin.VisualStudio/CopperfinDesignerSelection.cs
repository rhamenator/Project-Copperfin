using System.ComponentModel;
using System.Linq;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinDesignerSelection
{
    public int RecordIndex { get; set; }

    [ReadOnly(true)]
    [DisplayName("Name")]
    public string ObjectName { get; set; } = string.Empty;

    [ReadOnly(true)]
    public string BaseClass { get; set; } = string.Empty;

    [ReadOnly(true)]
    public string Parent { get; set; } = string.Empty;

    public int Left { get; set; }

    public int Top { get; set; }

    public int Width { get; set; }

    public int Height { get; set; }

    public string Caption { get; set; } = string.Empty;

    public static CopperfinDesignerSelection? FromSnapshot(CopperfinStudioSnapshotObject snapshotObject)
    {
        int TryRead(string name) {
            var property = snapshotObject.Properties.FirstOrDefault(item => item.Name == name);
            return property is not null && int.TryParse(property.Value, out var value) ? value : 0;
        }

        string Read(string name) {
            return snapshotObject.Properties.FirstOrDefault(item => item.Name == name)?.Value?.Trim('"') ?? string.Empty;
        }

        return new CopperfinDesignerSelection
        {
            RecordIndex = snapshotObject.RecordIndex,
            ObjectName = snapshotObject.Properties.FirstOrDefault(item => item.Name == "OBJNAME")?.Value ?? snapshotObject.Title,
            BaseClass = snapshotObject.Subtitle,
            Parent = Read("PARENT"),
            Left = TryRead("Left"),
            Top = TryRead("Top"),
            Width = TryRead("Width"),
            Height = TryRead("Height"),
            Caption = Read("Caption")
        };
    }
}
