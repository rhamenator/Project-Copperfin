using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.Linq;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinDesignerSelection : ICustomTypeDescriptor
{
    private sealed class SelectionField
    {
        public string Name { get; set; } = string.Empty;
        public string DisplayName { get; set; } = string.Empty;
        public Type ValueType { get; set; } = typeof(string);
        public bool IsReadOnly { get; set; }
        public string CurrentValue { get; set; } = string.Empty;
        public Func<string, object?> Deserialize { get; set; } = static value => value;
        public Func<object?, string> Serialize { get; set; } = static value => value?.ToString() ?? string.Empty;
        public Func<object?, string> Store { get; set; } = static value => value?.ToString() ?? string.Empty;
    }

    private sealed class SelectionPropertyDescriptor : PropertyDescriptor
    {
        private readonly SelectionField field;

        public SelectionPropertyDescriptor(SelectionField field)
            : base(field.Name, Array.Empty<Attribute>())
        {
            this.field = field;
        }

        public override Type ComponentType => typeof(CopperfinDesignerSelection);

        public override bool IsReadOnly => this.field.IsReadOnly;

        public override Type PropertyType => this.field.ValueType;

        public override bool CanResetValue(object component) => false;

        public override object? GetValue(object component) {
            return ((CopperfinDesignerSelection)component).GetValue(field.Name);
        }

        public override void ResetValue(object component) {
        }

        public override void SetValue(object component, object? value) {
            ((CopperfinDesignerSelection)component).SetValue(field.Name, value);
            OnValueChanged(component, EventArgs.Empty);
        }

        public override bool ShouldSerializeValue(object component) => false;

        public override string DisplayName => this.field.DisplayName;
    }

    private readonly List<SelectionField> fields = new();
    private readonly Dictionary<string, SelectionField> fieldMap = new(StringComparer.OrdinalIgnoreCase);

    public int RecordIndex { get; private set; }

    public static CopperfinDesignerSelection? FromSnapshot(string assetFamily, CopperfinStudioSnapshotObject snapshotObject, CopperfinLocalization? localization = null)
    {
        var selection = new CopperfinDesignerSelection
        {
            RecordIndex = snapshotObject.RecordIndex
        };

        string L(string key, string fallback) => localization?.Text(key) ?? fallback;

        switch (assetFamily)
        {
            case "form":
            case "class_library":
                selection.AddReadOnlyString("OBJNAME", L("AssetEditor.Property.ObjectName", "Object Name"), selection.Read(snapshotObject, "OBJNAME", fallback: snapshotObject.Title));
                selection.AddReadOnlyString("BASECLASS", L("AssetEditor.Property.BaseClass", "Base Class"), selection.Read(snapshotObject, "BASECLASS", "CLASS", fallback: snapshotObject.Subtitle));
                selection.AddReadOnlyString("PARENT", L("AssetEditor.Property.Parent", "Parent"), selection.Read(snapshotObject, "PARENT"));
                selection.AddEditableInt("Left", L("AssetEditor.Property.Left", "Left"), selection.Read(snapshotObject, "Left"));
                selection.AddEditableInt("Top", L("AssetEditor.Column.Top", "Top"), selection.Read(snapshotObject, "Top"));
                selection.AddEditableInt("Width", L("AssetEditor.Property.Width", "Width"), selection.Read(snapshotObject, "Width"));
                selection.AddEditableInt("Height", L("AssetEditor.Property.Height", "Height"), selection.Read(snapshotObject, "Height"));
                selection.AddEditableString("Caption", L("AssetEditor.Property.Caption", "Caption"), selection.Read(snapshotObject, "Caption"), requiresFoxStringLiteral: true);
                break;

            case "report":
            case "label":
                selection.AddReadOnlyInt("OBJTYPE", L("AssetEditor.Property.ObjectType", "Object Type"), selection.Read(snapshotObject, "OBJTYPE"));
                selection.AddReadOnlyInt("OBJCODE", L("AssetEditor.Property.ObjectCode", "Object Code"), selection.Read(snapshotObject, "OBJCODE"));
                selection.AddReadOnlyInt("RECORDINDEX", L("AssetEditor.Column.Record", "Record"), snapshotObject.RecordIndex.ToString(CultureInfo.InvariantCulture));
                selection.AddReadOnlyString("OBJECTSTATE", L("AssetEditor.Property.ObjectState", "Object State"), BuildStateText(localization, snapshotObject.Deleted));
                selection.AddEditableString("EXPR", L("AssetEditor.Property.Expression", "Expression"), selection.Read(snapshotObject, "EXPR"));
                selection.AddEditableInt("HPOS", L("AssetEditor.Property.Left", "Left"), selection.Read(snapshotObject, "HPOS"));
                selection.AddEditableInt("VPOS", L("AssetEditor.Column.Top", "Top"), selection.Read(snapshotObject, "VPOS"));
                selection.AddEditableInt("WIDTH", L("AssetEditor.Property.Width", "Width"), selection.Read(snapshotObject, "WIDTH"));
                selection.AddEditableInt("HEIGHT", L("AssetEditor.Property.Height", "Height"), selection.Read(snapshotObject, "HEIGHT"));
                selection.AddEditableString("FONTFACE", L("AssetEditor.Property.FontFace", "Font Face"), selection.Read(snapshotObject, "FONTFACE"));
                selection.AddEditableInt("FONTSTYLE", L("AssetEditor.Property.FontStyle", "Font Style"), selection.Read(snapshotObject, "FONTSTYLE"));
                selection.AddEditableInt("FONTSIZE", L("AssetEditor.Property.FontSize", "Font Size"), selection.Read(snapshotObject, "FONTSIZE"));
                break;

            case "menu":
                selection.AddReadOnlyInt("OBJTYPE", L("AssetEditor.Property.ObjectType", "Object Type"), selection.Read(snapshotObject, "OBJTYPE"));
                selection.AddReadOnlyInt("OBJCODE", L("AssetEditor.Property.ObjectCode", "Object Code"), selection.Read(snapshotObject, "OBJCODE"));
                selection.AddEditableString("NAME", L("AssetEditor.Property.Name", "Name"), selection.Read(snapshotObject, "NAME", fallback: snapshotObject.Title));
                selection.AddEditableString("PROMPT", L("AssetEditor.Property.Prompt", "Prompt"), selection.Read(snapshotObject, "PROMPT"));
                selection.AddEditableString("COMMAND", L("AssetEditor.Property.Command", "Command"), selection.Read(snapshotObject, "COMMAND"));
                selection.AddEditableString("PROCEDURE", L("AssetEditor.Property.Procedure", "Procedure"), selection.Read(snapshotObject, "PROCEDURE"));
                selection.AddEditableString("MESSAGE", L("AssetEditor.Property.Message", "Message"), selection.Read(snapshotObject, "MESSAGE"));
                selection.AddEditableString("KEYLABEL", L("AssetEditor.Property.KeyLabel", "Key Label"), selection.Read(snapshotObject, "KEYLABEL"));
                selection.AddReadOnlyString("LEVELNAME", L("AssetEditor.Property.Level", "Level"), selection.Read(snapshotObject, "LEVELNAME", fallback: snapshotObject.Subtitle));
                selection.AddReadOnlyString("ITEMNUM", L("AssetEditor.Property.ItemNumber", "Item Number"), selection.Read(snapshotObject, "ITEMNUM"));
                break;

            case "project":
                selection.AddReadOnlyString("NAME", L("AssetEditor.Property.ProjectItem", "Project Item"), selection.Read(snapshotObject, "NAME", fallback: snapshotObject.Title));
                selection.AddReadOnlyString("TYPE", L("AssetEditor.Property.Type", "Type"), selection.Read(snapshotObject, "TYPE", fallback: snapshotObject.Subtitle));
                selection.AddReadOnlyString("KEY", L("AssetEditor.Property.Key", "Key"), selection.Read(snapshotObject, "KEY"));
                selection.AddEditableString("COMMENTS", L("AssetEditor.Property.Comments", "Comments"), selection.Read(snapshotObject, "COMMENTS"));
                selection.AddEditableBool("EXCLUDE", L("AssetEditor.Property.Exclude", "Exclude"), selection.Read(snapshotObject, "EXCLUDE"));
                selection.AddEditableBool("MAINPROG", L("AssetEditor.Property.MainProgram", "Main Program"), selection.Read(snapshotObject, "MAINPROG"));
                selection.AddEditableBool("DEBUG", L("AssetEditor.Property.Debug", "Debug"), selection.Read(snapshotObject, "DEBUG"));
                break;

            default:
                selection.AddReadOnlyString("NAME", L("AssetEditor.Property.Name", "Name"), snapshotObject.Title);
                selection.AddReadOnlyString("TYPE", L("AssetEditor.Property.Type", "Type"), snapshotObject.Subtitle);
                break;
        }

        return selection;
    }

    public static CopperfinDesignerSelection FromReportSection(CopperfinStudioReportSection section, CopperfinLocalization localization)
    {
        var selection = new CopperfinDesignerSelection
        {
            RecordIndex = section.RecordIndex
        };

        selection.AddReadOnlyString("TITLE", localization.Text("AssetEditor.Property.SectionTitle"), section.Title);
        selection.AddReadOnlyString("ID", localization.Text("AssetEditor.Property.SectionId"), section.Id);
        selection.AddReadOnlyInt(
            "RECORDINDEX",
            localization.Text("AssetEditor.Column.Record"),
            section.RecordIndex.ToString(CultureInfo.InvariantCulture));
        selection.AddReadOnlyInt(
            "OBJECTCOUNT",
            localization.Text("AssetEditor.Column.Objects"),
            section.Objects.Count.ToString(CultureInfo.InvariantCulture));
        selection.AddReadOnlyString(
            "SECTIONSTATE",
            localization.Text("AssetEditor.Property.SectionState"),
            BuildStateText(localization, section.Deleted));
        selection.AddReadOnlyString(
            "BANDKIND",
            localization.Text("AssetEditor.Property.BandKind"),
            BuildReportBandKindDisplayText(localization, section.BandKind));
        selection.AddEditableInt("TOP", localization.Text("AssetEditor.Column.Top"), section.Top.ToString(CultureInfo.InvariantCulture));
        selection.AddEditableInt("HEIGHT", localization.Text("AssetEditor.Property.Height"), section.Height.ToString(CultureInfo.InvariantCulture));
        if (section.GroupingContextAvailable ||
            !string.IsNullOrWhiteSpace(section.Expression) ||
            section.ExpressionFieldIndex.HasValue ||
            section.ExpressionMemoBlockNumber > 0)
        {
            selection.AddEditableString(
                "EXPR",
                localization.Text("AssetEditor.Property.Expression"),
                section.Expression ?? string.Empty);

            if (section.ExpressionFieldIndex.HasValue)
            {
                selection.AddReadOnlyInt(
                    "EXPRESSIONFIELD",
                    localization.Text("AssetEditor.Property.ExpressionFieldIndex"),
                    section.ExpressionFieldIndex.Value.ToString(CultureInfo.InvariantCulture));
            }

            if (section.ExpressionMemoBlockNumber > 0)
            {
                selection.AddReadOnlyInt(
                    "EXPRESSIONMEMO",
                    localization.Text("AssetEditor.Property.ExpressionMemoBlock"),
                    section.ExpressionMemoBlockNumber.ToString(CultureInfo.InvariantCulture));
            }
        }

        selection.AddReadOnlyInt(
            "DELETEDOBJECTCOUNT",
            localization.Text("AssetEditor.Property.DeletedObjects"),
            section.DeletedObjectCount.ToString(CultureInfo.InvariantCulture));

        if (!string.IsNullOrWhiteSpace(section.GroupRole))
        {
            selection.AddReadOnlyString(
                "GROUPROLE",
                localization.Text("AssetEditor.Property.GroupRole"),
                BuildGroupingRoleDisplayText(localization, section.GroupRole ?? string.Empty));
        }

        if (section.GroupingIndex.HasValue)
        {
            selection.AddReadOnlyInt(
                "GROUPINGINDEX",
                localization.Text("AssetEditor.Property.GroupingIndex"),
                section.GroupingIndex.Value.ToString(CultureInfo.InvariantCulture));
        }

        if (section.GroupingNestingDepth.HasValue)
        {
            selection.AddReadOnlyInt(
                "GROUPINGNESTINGDEPTH",
                localization.Text("AssetEditor.Property.GroupingNestingDepth"),
                section.GroupingNestingDepth.Value.ToString(CultureInfo.InvariantCulture));
        }

        if (!string.IsNullOrWhiteSpace(section.GroupPartnerSectionId))
        {
            selection.AddReadOnlyString(
                "GROUPPARTNERSECTIONID",
                localization.Text("AssetEditor.Property.GroupPartnerSectionId"),
                section.GroupPartnerSectionId ?? string.Empty);
        }

        if (section.GroupPartnerRecordIndex.HasValue)
        {
            selection.AddReadOnlyInt(
                "GROUPPARTNERRECORD",
                localization.Text("AssetEditor.Property.GroupPartnerRecord"),
                section.GroupPartnerRecordIndex.Value.ToString(CultureInfo.InvariantCulture));
        }

        if (!string.IsNullOrWhiteSpace(section.GroupPartnerSectionId) || section.GroupPartnerRecordIndex.HasValue)
        {
            selection.AddReadOnlyString(
                "GROUPPARTNERSTATE",
                localization.Text("AssetEditor.Property.GroupPartnerState"),
                BuildStateText(localization, section.GroupPartnerDeleted));
        }

        if (section.GroupingContextAvailable ||
            !string.IsNullOrWhiteSpace(section.GroupingExpression) ||
            section.GroupingExpressionFieldIndex.HasValue ||
            section.GroupingExpressionMemoBlockNumber > 0)
        {
            selection.AddReadOnlyString(
                "GROUPINGEXPRESSION",
                localization.Text("AssetEditor.Property.GroupingExpression"),
                section.GroupingExpression ?? string.Empty);

            if (section.GroupingExpressionFieldIndex.HasValue)
            {
                selection.AddReadOnlyInt(
                    "GROUPINGEXPRESSIONFIELD",
                    localization.Text("AssetEditor.Property.GroupingExpressionFieldIndex"),
                    section.GroupingExpressionFieldIndex.Value.ToString(CultureInfo.InvariantCulture));
            }

            if (section.GroupingExpressionMemoBlockNumber > 0)
            {
                selection.AddReadOnlyInt(
                    "GROUPINGEXPRESSIONMEMO",
                    localization.Text("AssetEditor.Property.GroupingExpressionMemoBlock"),
                    section.GroupingExpressionMemoBlockNumber.ToString(CultureInfo.InvariantCulture));
            }
        }

        return selection;
    }

    public static CopperfinDesignerSelection FromReportSettings(
        IReadOnlyList<CopperfinStudioNamedValue> settings,
        CopperfinLocalization localization,
        bool deleted = false,
        CopperfinStudioReportLayout? reportLayout = null)
    {
        var selection = new CopperfinDesignerSelection
        {
            RecordIndex = settings.FirstOrDefault()?.RecordIndex ?? 0
        };

        selection.AddReadOnlyInt(
            "RECORDINDEX",
            localization.Text("AssetEditor.Column.Record"),
            selection.RecordIndex.ToString(CultureInfo.InvariantCulture));
        selection.AddReadOnlyInt(
            "SETTINGCOUNT",
            localization.Text("AssetEditor.Property.SettingsCount"),
            settings.Count.ToString(CultureInfo.InvariantCulture));
        selection.AddReadOnlyString(
            "SETTINGSSTATE",
            localization.Text("AssetEditor.Property.SettingsState"),
            BuildStateText(localization, deleted));

        if (!string.IsNullOrWhiteSpace(reportLayout?.DocumentTitle))
        {
            selection.AddReadOnlyString(
                "DOCUMENTTITLE",
                localization.Text("AssetEditor.Property.DocumentTitle"),
                reportLayout!.DocumentTitle);
        }

        var sortSetting = settings.FirstOrDefault(setting =>
            string.Equals(setting.Name, "TAG", StringComparison.OrdinalIgnoreCase));
        if (sortSetting is not null && !string.IsNullOrWhiteSpace(sortSetting.Value))
        {
            selection.AddReadOnlyString(
                "SORTEXPRESSION",
                localization.Text("AssetEditor.Property.ActiveSortExpression"),
                sortSetting.Value);

            if (sortSetting.FieldIndex.HasValue)
            {
                selection.AddReadOnlyInt(
                    "SORTEXPRESSIONFIELD",
                    localization.Text("AssetEditor.Property.SortExpressionFieldIndex"),
                    sortSetting.FieldIndex.Value.ToString(CultureInfo.InvariantCulture));
            }

            if (sortSetting.MemoBlockNumber > 0)
            {
                selection.AddReadOnlyInt(
                    "SORTEXPRESSIONMEMO",
                    localization.Text("AssetEditor.Property.SortExpressionMemoBlock"),
                    sortSetting.MemoBlockNumber.ToString(CultureInfo.InvariantCulture));
            }
        }

        if (reportLayout?.PreviewBoundsAvailable == true)
        {
            selection.AddReadOnlyString(
                "PREVIEWBOUNDS",
                localization.Text("AssetEditor.Property.PreviewBounds"),
                localization.Format(
                    "AssetEditor.Property.BoundsValue",
                    reportLayout.PreviewBoundsLeft,
                    reportLayout.PreviewBoundsTop,
                    reportLayout.PreviewBoundsRight,
                    reportLayout.PreviewBoundsBottom,
                    reportLayout.PreviewBoundsWidth,
                    reportLayout.PreviewBoundsHeight));
        }

        if (reportLayout?.DeletedPreviewBoundsAvailable == true)
        {
            selection.AddReadOnlyString(
                "DELETEDPREVIEWBOUNDS",
                localization.Text("AssetEditor.Property.DeletedPreviewBounds"),
                localization.Format(
                    "AssetEditor.Property.BoundsValue",
                    reportLayout.DeletedPreviewBoundsLeft,
                    reportLayout.DeletedPreviewBoundsTop,
                    reportLayout.DeletedPreviewBoundsRight,
                    reportLayout.DeletedPreviewBoundsBottom,
                    reportLayout.DeletedPreviewBoundsWidth,
                    reportLayout.DeletedPreviewBoundsHeight));
        }

        var seenNames = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (var setting in settings)
        {
            if (string.IsNullOrWhiteSpace(setting.Name) || !seenNames.Add(setting.Name))
            {
                continue;
            }

            var displayName = BuildReportSettingDisplayText(localization, setting.Name);
            if (IsNumericReportSetting(setting.Name) &&
                int.TryParse(setting.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out _))
            {
                selection.AddEditableInt(setting.Name, displayName, setting.Value);
                continue;
            }

            selection.AddEditableString(setting.Name, displayName, setting.Value);
        }

        return selection;
    }

    public static CopperfinDesignerSelection FromReportGrouping(
        CopperfinStudioReportGrouping grouping,
        CopperfinLocalization localization)
    {
        var selection = new CopperfinDesignerSelection
        {
            RecordIndex = grouping.HeaderRecordIndex ?? grouping.FooterRecordIndex ?? 0
        };

        selection.AddReadOnlyInt(
            "GROUPINGINDEX",
            localization.Text("AssetEditor.Property.GroupingIndex"),
            grouping.GroupingIndex.ToString(CultureInfo.InvariantCulture));
        selection.AddReadOnlyInt(
            "GROUPINGNESTINGDEPTH",
            localization.Text("AssetEditor.Property.GroupingNestingDepth"),
            grouping.NestingDepth.ToString(CultureInfo.InvariantCulture));
        selection.AddReadOnlyString(
            "GROUPINGEXPRESSION",
            localization.Text("AssetEditor.Property.GroupingExpression"),
            grouping.Expression);

        if (grouping.ExpressionFieldIndex.HasValue)
        {
            selection.AddReadOnlyInt(
                "GROUPINGEXPRESSIONFIELD",
                localization.Text("AssetEditor.Property.GroupingExpressionFieldIndex"),
                grouping.ExpressionFieldIndex.Value.ToString(CultureInfo.InvariantCulture));
        }

        if (grouping.ExpressionMemoBlockNumber > 0)
        {
            selection.AddReadOnlyInt(
                "GROUPINGEXPRESSIONMEMO",
                localization.Text("AssetEditor.Property.GroupingExpressionMemoBlock"),
                grouping.ExpressionMemoBlockNumber.ToString(CultureInfo.InvariantCulture));
        }

        if (!string.IsNullOrWhiteSpace(grouping.HeaderSectionId))
        {
            selection.AddReadOnlyString(
                "GROUPHEADERSECTIONID",
                localization.Text("AssetEditor.Property.GroupingHeaderSectionId"),
                grouping.HeaderSectionId);
        }

        if (grouping.HeaderRecordIndex.HasValue)
        {
            selection.AddReadOnlyInt(
                "GROUPHEADERRECORD",
                localization.Text("AssetEditor.Property.GroupingHeaderRecord"),
                grouping.HeaderRecordIndex.Value.ToString(CultureInfo.InvariantCulture));
        }

        if (!string.IsNullOrWhiteSpace(grouping.HeaderSectionId) || grouping.HeaderRecordIndex.HasValue)
        {
            selection.AddReadOnlyString(
                "GROUPHEADERSTATE",
                localization.Text("AssetEditor.Property.GroupingHeaderState"),
                BuildStateText(localization, grouping.HeaderDeleted));
        }

        if (!string.IsNullOrWhiteSpace(grouping.FooterSectionId))
        {
            selection.AddReadOnlyString(
                "GROUPFOOTERSECTIONID",
                localization.Text("AssetEditor.Property.GroupingFooterSectionId"),
                grouping.FooterSectionId);
        }

        if (grouping.FooterRecordIndex.HasValue)
        {
            selection.AddReadOnlyInt(
                "GROUPFOOTERRECORD",
                localization.Text("AssetEditor.Property.GroupingFooterRecord"),
                grouping.FooterRecordIndex.Value.ToString(CultureInfo.InvariantCulture));
        }

        if (!string.IsNullOrWhiteSpace(grouping.FooterSectionId) || grouping.FooterRecordIndex.HasValue)
        {
            selection.AddReadOnlyString(
                "GROUPFOOTERSTATE",
                localization.Text("AssetEditor.Property.GroupingFooterState"),
                BuildStateText(localization, grouping.FooterDeleted));
        }

        return selection;
    }

    private static string BuildReportBandKindDisplayText(CopperfinLocalization localization, string bandKind)
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
            return localization.Text(key);
        }

        return bandKind.Replace('_', ' ');
    }

    private static string BuildGroupingRoleDisplayText(CopperfinLocalization localization, string groupRole)
    {
        var key = groupRole switch
        {
            "header" => "AssetEditor.GroupRole.Header",
            "footer" => "AssetEditor.GroupRole.Footer",
            _ => string.Empty
        };

        if (!string.IsNullOrWhiteSpace(key))
        {
            return localization.Text(key);
        }

        return groupRole.Replace('_', ' ');
    }

    private static string BuildReportSettingDisplayText(CopperfinLocalization localization, string settingName)
    {
        var key = settingName.ToUpperInvariant() switch
        {
            "DRIVER" => "AssetEditor.Property.PrinterDriver",
            "DEVICE" => "AssetEditor.Property.PrinterDevice",
            "OUTPUT" => "AssetEditor.Property.PrinterOutput",
            "DEFAULTSOURCE" => "AssetEditor.Property.DefaultSource",
            "PRINTQUALITY" => "AssetEditor.Property.PrintQuality",
            "YRESOLUTION" => "AssetEditor.Property.YResolution",
            "TTOPTION" => "AssetEditor.Property.TrueTypeOption",
            "ORIENTATION" => "AssetEditor.Property.Orientation",
            "PAPERSIZE" => "AssetEditor.Property.PaperSize",
            "PAPERLENGTH" => "AssetEditor.Property.PaperLength",
            "PAPERWIDTH" => "AssetEditor.Property.PaperWidth",
            "TOPMARGIN" => "AssetEditor.Property.TopMargin",
            "BOTMARGIN" => "AssetEditor.Property.BottomMargin",
            "LEFTMARGIN" => "AssetEditor.Property.LeftMargin",
            "RIGHTMARGIN" => "AssetEditor.Property.RightMargin",
            "COLS" => "AssetEditor.Property.Columns",
            "COLWIDTH" => "AssetEditor.Property.ColumnWidth",
            "COLSPACING" => "AssetEditor.Property.ColumnSpacing",
            "GRIDV" => "AssetEditor.Property.VerticalGrid",
            "GRIDH" => "AssetEditor.Property.HorizontalGrid",
            "TAG" => "AssetEditor.Property.SortExpression",
            _ => string.Empty
        };

        return string.IsNullOrWhiteSpace(key)
            ? settingName
            : localization.Text(key);
    }

    private static bool IsNumericReportSetting(string settingName)
    {
        return settingName.ToUpperInvariant() is
            "DEFAULTSOURCE" or
            "PRINTQUALITY" or
            "YRESOLUTION" or
            "TTOPTION" or
            "ORIENTATION" or
            "PAPERSIZE" or
            "PAPERLENGTH" or
            "PAPERWIDTH" or
            "TOPMARGIN" or
            "BOTMARGIN" or
            "LEFTMARGIN" or
            "RIGHTMARGIN" or
            "COLS" or
            "COLWIDTH" or
            "COLSPACING" or
            "GRIDV" or
            "GRIDH";
    }

    private static string BuildStateText(CopperfinLocalization? localization, bool deleted)
    {
        if (localization is not null)
        {
            return localization.Text(deleted ? "AssetEditor.State.Deleted" : "AssetEditor.State.Live");
        }

        return deleted ? "Deleted" : "Live";
    }

    public bool TryGetUpdate(string propertyName, out string targetName, out string serializedValue)
    {
        targetName = string.Empty;
        serializedValue = string.Empty;

        if (!fieldMap.TryGetValue(propertyName, out var field) || field.IsReadOnly)
        {
            return false;
        }

        targetName = field.Name;
        serializedValue = field.Serialize(field.Deserialize(field.CurrentValue));
        return true;
    }

    public bool TryGetDisplayName(string propertyName, out string displayName)
    {
        displayName = string.Empty;
        if (!fieldMap.TryGetValue(propertyName, out var field))
        {
            return false;
        }

        displayName = field.DisplayName;
        return true;
    }

    private string Read(CopperfinStudioSnapshotObject snapshotObject, string propertyName, string? alternateName = null, string? fallback = null)
    {
        var value = snapshotObject.Properties.FirstOrDefault(item => item.Name == propertyName)?.Value;
        if (string.IsNullOrWhiteSpace(value) && !string.IsNullOrWhiteSpace(alternateName))
        {
            value = snapshotObject.Properties.FirstOrDefault(item => item.Name == alternateName)?.Value;
        }

        if (string.IsNullOrWhiteSpace(value) || value == "<memo block 0>")
        {
            return fallback ?? string.Empty;
        }

        return (value ?? string.Empty).Trim().Trim('"');
    }

    private void AddReadOnlyString(string name, string displayName, string value)
    {
        AddField(new SelectionField
        {
            Name = name,
            DisplayName = displayName,
            ValueType = typeof(string),
            IsReadOnly = true,
            CurrentValue = value
        });
    }

    private void AddEditableString(string name, string displayName, string value, bool requiresFoxStringLiteral = false)
    {
        AddField(new SelectionField
        {
            Name = name,
            DisplayName = displayName,
            ValueType = typeof(string),
            IsReadOnly = false,
            CurrentValue = value,
            Deserialize = static text => text,
            Serialize = requiresFoxStringLiteral
                ? static value => SerializeFoxString(value?.ToString() ?? string.Empty)
                : static value => value?.ToString() ?? string.Empty
        });
    }

    private void AddReadOnlyInt(string name, string displayName, string value)
    {
        AddField(new SelectionField
        {
            Name = name,
            DisplayName = displayName,
            ValueType = typeof(int),
            IsReadOnly = true,
            CurrentValue = NormalizeInt(value),
            Deserialize = static text => ParseInt(text)
        });
    }

    private void AddEditableInt(string name, string displayName, string value)
    {
        AddField(new SelectionField
        {
            Name = name,
            DisplayName = displayName,
            ValueType = typeof(int),
            IsReadOnly = false,
            CurrentValue = NormalizeInt(value),
            Deserialize = static text => ParseInt(text),
            Serialize = static input => Convert.ToInt32(input ?? 0, CultureInfo.InvariantCulture).ToString(CultureInfo.InvariantCulture),
            Store = static input => Convert.ToInt32(input ?? 0, CultureInfo.InvariantCulture).ToString(CultureInfo.InvariantCulture)
        });
    }

    private void AddEditableBool(string name, string displayName, string value)
    {
        AddField(new SelectionField
        {
            Name = name,
            DisplayName = displayName,
            ValueType = typeof(bool),
            IsReadOnly = false,
            CurrentValue = NormalizeBool(value),
            Deserialize = static text => ParseBool(text),
            Serialize = static input => Convert.ToBoolean(input ?? false, CultureInfo.InvariantCulture) ? "true" : "false",
            Store = static input => Convert.ToBoolean(input ?? false, CultureInfo.InvariantCulture) ? "true" : "false"
        });
    }

    private void AddField(SelectionField field)
    {
        fields.Add(field);
        fieldMap[field.Name] = field;
    }

    private object? GetValue(string propertyName)
    {
        return fieldMap.TryGetValue(propertyName, out var field)
            ? field.Deserialize(field.CurrentValue)
            : null;
    }

    private void SetValue(string propertyName, object? value)
    {
        if (!fieldMap.TryGetValue(propertyName, out var field) || field.IsReadOnly)
        {
            return;
        }

        field.CurrentValue = field.Store(value);
    }

    private static int ParseInt(string value)
    {
        return int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var parsed)
            ? parsed
            : 0;
    }

    private static bool ParseBool(string value)
    {
        return string.Equals(value, "true", StringComparison.OrdinalIgnoreCase) ||
               string.Equals(value, "t", StringComparison.OrdinalIgnoreCase) ||
               string.Equals(value, ".t.", StringComparison.OrdinalIgnoreCase) ||
               string.Equals(value, "y", StringComparison.OrdinalIgnoreCase) ||
               string.Equals(value, "yes", StringComparison.OrdinalIgnoreCase);
    }

    private static string NormalizeInt(string value)
    {
        return int.TryParse(value, NumberStyles.Any, CultureInfo.InvariantCulture, out var parsed)
            ? parsed.ToString(CultureInfo.InvariantCulture)
            : "0";
    }

    private static string NormalizeBool(string value)
    {
        return ParseBool(value) ? "true" : "false";
    }

    private static string SerializeFoxString(string value)
    {
        return "\"" + value.Replace("\"", "\"\"") + "\"";
    }

    public AttributeCollection GetAttributes() => AttributeCollection.Empty;

    public string? GetClassName() => nameof(CopperfinDesignerSelection);

    public string? GetComponentName() => nameof(CopperfinDesignerSelection);

    public TypeConverter? GetConverter() => null;

    public EventDescriptor? GetDefaultEvent() => null;

    public PropertyDescriptor? GetDefaultProperty() => null;

    public object? GetEditor(Type editorBaseType) => null;

    public EventDescriptorCollection GetEvents(Attribute[]? attributes) => EventDescriptorCollection.Empty;

    public EventDescriptorCollection GetEvents() => EventDescriptorCollection.Empty;

    public PropertyDescriptorCollection GetProperties(Attribute[]? attributes) => GetProperties();

    public PropertyDescriptorCollection GetProperties()
    {
        return new PropertyDescriptorCollection(fields.Select(field => new SelectionPropertyDescriptor(field)).ToArray(), true);
    }

    public object GetPropertyOwner(PropertyDescriptor? pd) => this;
}
