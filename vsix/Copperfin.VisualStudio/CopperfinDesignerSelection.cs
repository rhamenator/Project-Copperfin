// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.Linq;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinDesignerSelection : ICustomTypeDescriptor
{
    private sealed class ReportSettingDescriptor
    {
        public string Name { get; set; } = string.Empty;
        public string LocalizationKey { get; set; } = string.Empty;
        public bool Numeric { get; set; }
        public bool Logical { get; set; }
        public bool MaterializeWhenMissing { get; set; }
        public bool RequiresSnapshotPropertyPresence { get; set; }
    }

    private sealed class SelectionField
    {
        public string Name { get; set; } = string.Empty;
        public string TargetName { get; set; } = string.Empty;
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

        public override object? GetValue(object? component) {
            return component is CopperfinDesignerSelection selection
                ? selection.GetValue(field.Name)
                : null;
        }

        public override void ResetValue(object component) {
        }

        public override void SetValue(object? component, object? value) {
            if (component is not CopperfinDesignerSelection selection)
            {
                return;
            }

            selection.SetValue(field.Name, value);
            OnValueChanged(selection, EventArgs.Empty);
        }

        public override bool ShouldSerializeValue(object component) => false;

        public override string DisplayName => this.field.DisplayName;
    }

    private readonly List<SelectionField> fields = new();
    private readonly Dictionary<string, SelectionField> fieldMap = new(StringComparer.OrdinalIgnoreCase);

    public int RecordIndex { get; private set; }

    public static CopperfinDesignerSelection? FromSnapshot(
        string assetFamily,
        CopperfinStudioSnapshotObject snapshotObject,
        CopperfinLocalization? localization = null,
        bool documentReadOnly = false)
    {
        localization ??= CopperfinLocalization.FromEnvironment();

        var selection = new CopperfinDesignerSelection
        {
            RecordIndex = snapshotObject.RecordIndex
        };

        string L(string key, string fallback)
        {
            var localized = localization.Text(key);
            return string.Equals(localized, key, StringComparison.Ordinal) ? fallback : localized;
        }

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
                if (int.TryParse(
                        selection.Read(snapshotObject, "OBJTYPE"),
                        NumberStyles.Integer,
                        CultureInfo.InvariantCulture,
                        out var expressionObjectType) &&
                    expressionObjectType == 8)
                {
                    selection.AddEditableInt(
                        "RULERLINES",
                        L("AssetEditor.Property.StringTrimming", "String Trimming"),
                        selection.Read(snapshotObject, "RULERLINES"));
                    selection.AddEditableInt(
                        "OFFSET",
                        L("AssetEditor.Property.ExpressionAlignment", "Expression Alignment"),
                        selection.Read(snapshotObject, "OFFSET"));
                    selection.AddEditableString(
                        "FILLCHAR",
                        L("AssetEditor.Property.ExpressionDataType", "Expression Data Type"),
                        selection.Read(snapshotObject, "FILLCHAR"));
                    selection.AddEditableInt(
                        "TOTALTYPE",
                        L("AssetEditor.Property.CalculationType", "Calculation Type"),
                        selection.Read(snapshotObject, "TOTALTYPE"));
                    selection.AddEditableInt(
                        "RESETTOTAL",
                        L("AssetEditor.Property.ResetTotal", "Reset Total"),
                        selection.Read(snapshotObject, "RESETTOTAL"));
                }
                if (int.TryParse(
                        selection.Read(snapshotObject, "OBJTYPE"),
                        NumberStyles.Integer,
                        CultureInfo.InvariantCulture,
                        out var labelObjectType) &&
                    labelObjectType == 5)
                {
                    selection.AddEditableInt(
                        "SPACING",
                        L("AssetEditor.Property.LineSpacing", "Line Spacing"),
                        selection.Read(snapshotObject, "SPACING"));
                }
                if (int.TryParse(
                        selection.Read(snapshotObject, "OBJTYPE"),
                        NumberStyles.Integer,
                        CultureInfo.InvariantCulture,
                        out var imageObjectType) &&
                    imageObjectType == 17)
                {
                    selection.AddEditableInt(
                        "GENERAL",
                        L("AssetEditor.Property.ImageScaleMode", "Image Scale Mode"),
                        selection.Read(snapshotObject, "GENERAL"));
                    selection.AddEditableInt(
                        "OFFSET",
                        L("AssetEditor.Property.ImageSourceMode", "Image Source Mode"),
                        selection.Read(snapshotObject, "OFFSET"));
                }
                if (int.TryParse(
                        selection.Read(snapshotObject, "OBJTYPE"),
                        NumberStyles.Integer,
                        CultureInfo.InvariantCulture,
                        out var tooltipObjectType) &&
                    (tooltipObjectType == 5 || tooltipObjectType == 6 || tooltipObjectType == 7 ||
                     tooltipObjectType == 8 || tooltipObjectType == 10 || tooltipObjectType == 17))
                {
                    selection.AddEditableString(
                        "TAG2",
                        L("AssetEditor.Property.ToolTipText", "ToolTip Text"),
                        selection.Read(snapshotObject, "TAG2"));
                }
                if (int.TryParse(
                        selection.Read(snapshotObject, "OBJTYPE"),
                        NumberStyles.Integer,
                        CultureInfo.InvariantCulture,
                        out var penColorObjectType) &&
                    (penColorObjectType == 5 || penColorObjectType == 8))
                {
                    selection.AddEditableInt("PENRED", L("AssetEditor.Property.PenRed", "Pen Red"), selection.Read(snapshotObject, "PENRED"));
                    selection.AddEditableInt("PENGREEN", L("AssetEditor.Property.PenGreen", "Pen Green"), selection.Read(snapshotObject, "PENGREEN"));
                    selection.AddEditableInt("PENBLUE", L("AssetEditor.Property.PenBlue", "Pen Blue"), selection.Read(snapshotObject, "PENBLUE"));
                }
                if (penColorObjectType == 8)
                {
                    selection.AddEditableInt("FILLRED", L("AssetEditor.Property.FillRed", "Fill Red"), selection.Read(snapshotObject, "FILLRED"));
                    selection.AddEditableInt("FILLGREEN", L("AssetEditor.Property.FillGreen", "Fill Green"), selection.Read(snapshotObject, "FILLGREEN"));
                    selection.AddEditableInt("FILLBLUE", L("AssetEditor.Property.FillBlue", "Fill Blue"), selection.Read(snapshotObject, "FILLBLUE"));
                }
                if (int.TryParse(
                        selection.Read(snapshotObject, "OBJTYPE"),
                        NumberStyles.Integer,
                        CultureInfo.InvariantCulture,
                        out var lineShapeObjectType) &&
                    (lineShapeObjectType == 6 || lineShapeObjectType == 7))
                {
                    selection.AddEditableInt("PENSIZE", L("AssetEditor.Property.PenSize", "Pen Size"), selection.Read(snapshotObject, "PENSIZE"));
                    selection.AddEditableInt("PENPAT", L("AssetEditor.Property.PenPattern", "Pen Pattern"), selection.Read(snapshotObject, "PENPAT"));
                    if (lineShapeObjectType == 7)
                    {
                        selection.AddEditableInt("FILLPAT", L("AssetEditor.Property.FillPattern", "Fill Pattern"), selection.Read(snapshotObject, "FILLPAT"));
                    }
                }
                if (int.TryParse(
                        selection.Read(snapshotObject, "OBJTYPE"),
                        NumberStyles.Integer,
                        CultureInfo.InvariantCulture,
                        out var fontCharsetObjectType) &&
                    (fontCharsetObjectType == 1 || fontCharsetObjectType == 5 ||
                     fontCharsetObjectType == 8 || fontCharsetObjectType == 17))
                {
                    selection.AddEditableBool(
                        "DOUBLE",
                        L("AssetEditor.Property.ExplicitFontCharset", "Explicit Font Charset"),
                        selection.Read(snapshotObject, "DOUBLE"));
                    if (fontCharsetObjectType == 1 || fontCharsetObjectType == 5 || fontCharsetObjectType == 8)
                    {
                        selection.AddEditableInt(
                            "RESOID",
                            L("AssetEditor.Property.FontCharset", "Font Charset"),
                            selection.Read(snapshotObject, "RESOID"));
                    }
                }
                selection.AddEditableString("SUPEXPR", L("AssetEditor.Property.PrintWhen", "Print When"), selection.Read(snapshotObject, "SUPEXPR"));
                selection.AddEditableInt("SUPGROUP", L("AssetEditor.Property.PrintWhenGroup", "When Group Changes"), selection.Read(snapshotObject, "SUPGROUP"));
                selection.AddEditableBool("SUPALWAYS", L("AssetEditor.Property.PrintWhenRepeated", "Print Repeated Values"), selection.Read(snapshotObject, "SUPALWAYS"));
                selection.AddEditableBool("SUPVALCHNG", L("AssetEditor.Property.PrintWhenValueChanges", "Print Only When Value Changes"), selection.Read(snapshotObject, "SUPVALCHNG"));
                selection.AddEditableInt("SUPRPCOL", L("AssetEditor.Property.PrintWhenNewPageColumn", "In First Whole Band of New Page/Column"), selection.Read(snapshotObject, "SUPRPCOL"));
                selection.AddEditableBool("SUPOVFLOW", L("AssetEditor.Property.PrintWhenOverflow", "When Detail Overflows to New Page/Column"), selection.Read(snapshotObject, "SUPOVFLOW"));
                selection.AddEditableBool("BOTTOM", L("AssetEditor.Property.BottomRelativeToBand", "Fix Relative to Bottom of Band"), selection.Read(snapshotObject, "BOTTOM"));
                selection.AddEditableBool("TOP", L("AssetEditor.Property.TopRelativeToBand", "Fix Relative to Top of Band"), selection.Read(snapshotObject, "TOP"));
                if (int.TryParse(
                        selection.Read(snapshotObject, "OBJTYPE"),
                        NumberStyles.Integer,
                        CultureInfo.InvariantCulture,
                        out var objectType) &&
                    (objectType == 5 || objectType == 8 || objectType == 17))
                {
                    selection.AddEditableString(
                        "PICTURE",
                        L("AssetEditor.Property.Picture", "Picture"),
                        selection.Read(snapshotObject, "PICTURE"));
                }
                selection.AddEditableInt("HPOS", L("AssetEditor.Property.Left", "Left"), selection.Read(snapshotObject, "HPOS"));
                selection.AddEditableInt("VPOS", L("AssetEditor.Column.Top", "Top"), selection.Read(snapshotObject, "VPOS"));
                selection.AddEditableInt("WIDTH", L("AssetEditor.Property.Width", "Width"), selection.Read(snapshotObject, "WIDTH"));
                selection.AddEditableInt("HEIGHT", L("AssetEditor.Property.Height", "Height"), selection.Read(snapshotObject, "HEIGHT"));
                selection.AddEditableString("FONTFACE", L("AssetEditor.Property.FontFace", "Font Face"), selection.Read(snapshotObject, "FONTFACE"));
                selection.AddEditableInt("FONTSTYLE", L("AssetEditor.Property.FontStyle", "Font Style"), selection.Read(snapshotObject, "FONTSTYLE"));
                selection.AddEditableInt("FONTSIZE", L("AssetEditor.Property.FontSize", "Font Size"), selection.Read(snapshotObject, "FONTSIZE"));
                selection.AddEditableInt("MODE", L("AssetEditor.Property.ReportMode", "Back Style / Direction Mode"), selection.Read(snapshotObject, "MODE"));
                selection.AddEditableBool("FLOAT", L("AssetEditor.Property.Float", "Float"), selection.Read(snapshotObject, "FLOAT"));
                selection.AddEditableBool("NOREPEAT", L("AssetEditor.Property.NoRepeat", "No Repeat"), selection.Read(snapshotObject, "NOREPEAT"));
                selection.AddEditableBool("STRETCH", L("AssetEditor.Property.Stretch", "Stretch with Overflow"), selection.Read(snapshotObject, "STRETCH"));
                selection.AddEditableBool("STRETCHTOP", L("AssetEditor.Property.StretchTop", "Stretch Relative to Top"), selection.Read(snapshotObject, "STRETCHTOP"));
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

        selection.ApplyDocumentReadOnly(documentReadOnly);
        return selection;
    }

    public static CopperfinDesignerSelection FromReportSection(
        CopperfinStudioReportSection section,
        CopperfinLocalization localization,
        bool documentReadOnly = false)
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
        selection.AddEditableInt(
            "TOP",
            localization.Text("AssetEditor.Column.Top"),
            section.Top.ToString(CultureInfo.InvariantCulture),
            targetName: "VPOS");
        selection.AddEditableInt("HEIGHT", localization.Text("AssetEditor.Property.Height"), section.Height.ToString(CultureInfo.InvariantCulture));
        selection.AddEditableBool(
            "PAGEBREAK",
            localization.Text("AssetEditor.Property.PageBreak"),
            section.PageBreak);
        selection.AddEditableBool(
            "COLBREAK",
            localization.Text("AssetEditor.Property.ColumnBreak"),
            section.ColumnBreak);
        selection.AddEditableBool(
            "RESETPAGE",
            localization.Text("AssetEditor.Property.ResetPage"),
            section.ResetPage);
        selection.AddEditableBool(
            "EJECTBEFOR",
            localization.Text("AssetEditor.Property.EjectBefore"),
            section.EjectBefore);
        selection.AddEditableBool(
            "EJECTAFTER",
            localization.Text("AssetEditor.Property.EjectAfter"),
            section.EjectAfter);
        selection.AddEditableBool(
            "PLAIN",
            localization.Text("AssetEditor.Property.ConstantBandHeight"),
            section.Plain);
        selection.AddEditableString(
            "TAG",
            localization.Text("AssetEditor.Property.OnEntryExpression"),
            section.OnEntryExpression);
        selection.AddEditableString(
            "TAG2",
            localization.Text("AssetEditor.Property.OnExitExpression"),
            section.OnExitExpression);
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

        selection.ApplyDocumentReadOnly(documentReadOnly);
        return selection;
    }

    public static CopperfinDesignerSelection FromReportSettings(
        IReadOnlyList<CopperfinStudioNamedValue> settings,
        CopperfinLocalization localization,
        bool deleted = false,
        CopperfinStudioReportLayout? reportLayout = null,
        IEnumerable<string>? availablePropertyNames = null,
        bool documentReadOnly = false)
    {
        var selection = new CopperfinDesignerSelection
        {
            RecordIndex = settings.FirstOrDefault()?.RecordIndex ?? 0
        };
        var availablePropertyNameSet = availablePropertyNames is null
            ? null
            : new HashSet<string>(availablePropertyNames, StringComparer.OrdinalIgnoreCase);

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
            if (DescriptorForSetting(setting.Name)?.Logical == true)
            {
                selection.AddEditableBool(setting.Name, displayName, setting.Value);
                continue;
            }
            if (IsNumericReportSetting(setting.Name) &&
                int.TryParse(setting.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out _))
            {
                selection.AddEditableInt(setting.Name, displayName, setting.Value);
                continue;
            }

            selection.AddEditableString(setting.Name, displayName, setting.Value);
        }

        foreach (var settingName in GetOptionalReportStringSettingNames())
        {
            if (!seenNames.Add(settingName.Name))
            {
                continue;
            }

            if (settingName.RequiresSnapshotPropertyPresence &&
                availablePropertyNameSet is not null &&
                !availablePropertyNameSet.Contains(settingName.Name))
            {
                continue;
            }

            if (settingName.Logical)
            {
                selection.AddEditableBool(
                    settingName.Name,
                    localization.Text(settingName.LocalizationKey),
                    string.Empty);
                continue;
            }

            if (settingName.Numeric)
            {
                selection.AddEditableOptionalInt(
                    settingName.Name,
                    localization.Text(settingName.LocalizationKey),
                    string.Empty);
                continue;
            }

            selection.AddEditableString(
                settingName.Name,
                localization.Text(settingName.LocalizationKey),
                string.Empty);
        }

        selection.ApplyDocumentReadOnly(documentReadOnly);
        return selection;
    }

    public static CopperfinDesignerSelection FromReportGrouping(
        CopperfinStudioReportGrouping grouping,
        CopperfinLocalization localization,
        bool documentReadOnly = false)
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
        selection.AddEditableString(
            "GROUPINGEXPRESSION",
            localization.Text("AssetEditor.Property.GroupingExpression"),
            grouping.Expression,
            targetName: "EXPR");

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

        selection.ApplyDocumentReadOnly(documentReadOnly);
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
        var descriptor = GetKnownReportSettingDescriptors().FirstOrDefault(candidate =>
            string.Equals(candidate.Name, settingName, StringComparison.OrdinalIgnoreCase));
        return descriptor is null || string.IsNullOrWhiteSpace(descriptor.LocalizationKey)
            ? settingName
            : localization.Text(descriptor.LocalizationKey);
    }

    private static bool IsNumericReportSetting(string settingName)
    {
        return GetKnownReportSettingDescriptors().Any(candidate =>
            candidate.Numeric &&
            string.Equals(candidate.Name, settingName, StringComparison.OrdinalIgnoreCase));
    }

    private static ReportSettingDescriptor? DescriptorForSetting(string settingName)
    {
        return GetKnownReportSettingDescriptors().FirstOrDefault(candidate =>
            string.Equals(candidate.Name, settingName, StringComparison.OrdinalIgnoreCase));
    }

    private static IEnumerable<ReportSettingDescriptor> GetOptionalReportStringSettingNames()
    {
        return GetKnownReportSettingDescriptors().Where(candidate => candidate.MaterializeWhenMissing);
    }

    private static IEnumerable<ReportSettingDescriptor> GetKnownReportSettingDescriptors()
    {
        yield return new ReportSettingDescriptor { Name = "ASCII", LocalizationKey = "AssetEditor.Property.Ascii", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "COLLATE", LocalizationKey = "AssetEditor.Property.Collate", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "COLOR", LocalizationKey = "AssetEditor.Property.Color", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "COPIES", LocalizationKey = "AssetEditor.Property.Copies", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "DRIVER", LocalizationKey = "AssetEditor.Property.PrinterDriver", Numeric = false, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "DEVICE", LocalizationKey = "AssetEditor.Property.PrinterDevice", Numeric = false, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "OUTPUT", LocalizationKey = "AssetEditor.Property.PrinterOutput", Numeric = false, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "DEFAULTSOURCE", LocalizationKey = "AssetEditor.Property.DefaultSource", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "PRINTQUALITY", LocalizationKey = "AssetEditor.Property.PrintQuality", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "YRESOLUTION", LocalizationKey = "AssetEditor.Property.YResolution", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "TTOPTION", LocalizationKey = "AssetEditor.Property.TrueTypeOption", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "ORIENTATION", LocalizationKey = "AssetEditor.Property.Orientation", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "PAPERSIZE", LocalizationKey = "AssetEditor.Property.PaperSize", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "PAPERLENGTH", LocalizationKey = "AssetEditor.Property.PaperLength", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "PAPERWIDTH", LocalizationKey = "AssetEditor.Property.PaperWidth", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "TOPMARGIN", LocalizationKey = "AssetEditor.Property.TopMargin", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "BOTMARGIN", LocalizationKey = "AssetEditor.Property.BottomMargin", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "LEFTMARGIN", LocalizationKey = "AssetEditor.Property.LeftMargin", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "RIGHTMARGIN", LocalizationKey = "AssetEditor.Property.RightMargin", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "COLS", LocalizationKey = "AssetEditor.Property.Columns", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "COLWIDTH", LocalizationKey = "AssetEditor.Property.ColumnWidth", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "COLSPACING", LocalizationKey = "AssetEditor.Property.ColumnSpacing", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "GRIDV", LocalizationKey = "AssetEditor.Property.VerticalGrid", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "GRIDH", LocalizationKey = "AssetEditor.Property.HorizontalGrid", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "GRID", LocalizationKey = "AssetEditor.Property.GridSnapping", Logical = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "RULER", LocalizationKey = "AssetEditor.Property.RulerUnits", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "RULERLINES", LocalizationKey = "AssetEditor.Property.RulerLines", Numeric = true, MaterializeWhenMissing = true };
        yield return new ReportSettingDescriptor { Name = "TAG", LocalizationKey = "AssetEditor.Property.SortExpression", Numeric = false, MaterializeWhenMissing = true };
    }

    private static string BuildStateText(CopperfinLocalization? localization, bool deleted)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        return localization.Text(deleted ? "AssetEditor.State.Deleted" : "AssetEditor.State.Live");
    }

    public bool TryGetUpdate(string propertyName, out string targetName, out string serializedValue)
    {
        targetName = string.Empty;
        serializedValue = string.Empty;

        if (!fieldMap.TryGetValue(propertyName, out var field) || field.IsReadOnly)
        {
            return false;
        }

        targetName = string.IsNullOrWhiteSpace(field.TargetName) ? field.Name : field.TargetName;
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

    private void AddEditableString(
        string name,
        string displayName,
        string value,
        bool requiresFoxStringLiteral = false,
        string? targetName = null)
    {
        AddField(new SelectionField
        {
            Name = name,
            TargetName = string.IsNullOrWhiteSpace(targetName) ? name : targetName!,
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

    private void AddEditableInt(string name, string displayName, string value, string? targetName = null)
    {
        AddField(new SelectionField
        {
            Name = name,
            TargetName = string.IsNullOrWhiteSpace(targetName) ? name : targetName!,
            DisplayName = displayName,
            ValueType = typeof(int),
            IsReadOnly = false,
            CurrentValue = NormalizeInt(value),
            Deserialize = static text => ParseInt(text),
            Serialize = static input => Convert.ToInt32(input ?? 0, CultureInfo.InvariantCulture).ToString(CultureInfo.InvariantCulture),
            Store = static input => Convert.ToInt32(input ?? 0, CultureInfo.InvariantCulture).ToString(CultureInfo.InvariantCulture)
        });
    }

    private void AddEditableOptionalInt(string name, string displayName, string value)
    {
        AddField(new SelectionField
        {
            Name = name,
            DisplayName = displayName,
            ValueType = typeof(string),
            IsReadOnly = false,
            CurrentValue = TryParseNormalizedInt(value, out var parsed)
                ? parsed.ToString(CultureInfo.InvariantCulture)
                : string.Empty,
            Deserialize = static text => text,
            Serialize = static input => ParseInt(input?.ToString() ?? string.Empty).ToString(CultureInfo.InvariantCulture),
            Store = static input => input?.ToString() ?? string.Empty
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
        field.IsReadOnly |= documentReadOnly;
        fields.Add(field);
        fieldMap[field.Name] = field;
    }

    private bool documentReadOnly;

    private void ApplyDocumentReadOnly(bool value)
    {
        documentReadOnly = value;
        if (!value)
        {
            return;
        }

        foreach (var field in fields)
        {
            field.IsReadOnly = true;
        }
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
        return TryParseNormalizedInt(value, out var parsed)
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
        return TryParseNormalizedInt(value, out var parsed)
            ? parsed.ToString(CultureInfo.InvariantCulture)
            : "0";
    }

    private static bool TryParseNormalizedInt(string value, out int parsed)
    {
        if (int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out parsed))
        {
            return true;
        }

        if (decimal.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out var decimalValue))
        {
            var truncated = decimal.Truncate(decimalValue);
            if (truncated >= int.MinValue && truncated <= int.MaxValue)
            {
                parsed = decimal.ToInt32(truncated);
                return true;
            }
        }

        parsed = 0;
        return false;
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
