// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_usage_exposes_selected_execution_catalogs(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_execution_catalog_usage_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(studio_host_path, {}, temp_root);

    expect(process.exit_code == 2, "#1409: no-argument studio host invocation should return usage failure");
    expect_contains(process.stdout_text,
        "Usage: copperfin_studio_host --path <asset>",
        "#2394: default studio host usage should preserve en-US CLI text");
    expect_contains(process.stdout_text,
        "   or: copperfin_studio_host --list-subsystems [--json]",
        "#2576: default studio host usage should preserve en-US alternate usage prose");
    expect_contains(process.stdout_text,
        "Selection context tokens: visual_object, visual_method, container_object, class_designer, report_expression, label_expression, menu_item, project_item, data_environment",
        "#2576: default studio host usage should preserve en-US selection-context token prose");
    expect_contains(process.stdout_text,
        "Selected-back-color object:",
        "#2570: default studio host usage should preserve en-US selected-back-color labels");
    expect_contains(process.stdout_text,
        "Dynamic-fore-color object:",
        "#2570: default studio host usage should preserve en-US dynamic-fore-color labels");
    expect_contains(process.stdout_text,
        "Desktop object:",
        "#2571: default studio host usage should preserve en-US desktop labels");
    expect_contains(process.stdout_text,
        "Picture-selection-display object:",
        "#2571: default studio host usage should preserve en-US picture-selection-display labels");
    expect_contains(process.stdout_text,
        "Dynamic-input-mask object:",
        "#2572: default studio host usage should preserve en-US dynamic-input-mask labels");
    expect_contains(process.stdout_text,
        "Font-name object:",
        "#2572: default studio host usage should preserve en-US font-name labels");
    expect_contains(process.stdout_text,
        "Max-top object:",
        "#2572: default studio host usage should preserve en-US max-top labels");
    expect_contains(process.stdout_text,
        "Button-count object:",
        "#2573: default studio host usage should preserve en-US button-count labels");
    expect_contains(process.stdout_text,
        "Buffer-mode-override object:",
        "#2573: default studio host usage should preserve en-US buffer-mode-override labels");
    expect_contains(process.stdout_text,
        "Header-height object:",
        "#2573: default studio host usage should preserve en-US header-height labels");
    expect_contains(process.stdout_text,
        "Row-height object:",
        "#2574: default studio host usage should preserve en-US row-height labels");
    expect_contains(process.stdout_text,
        "Grid-line-width object:",
        "#2574: default studio host usage should preserve en-US grid-line-width labels");
    expect_contains(process.stdout_text,
        "Partition object:",
        "#2574: default studio host usage should preserve en-US partition labels");
    expect_contains(process.stdout_text,
        "Record-source-type object:",
        "#2575: default studio host usage should preserve en-US record-source-type labels");
    expect_contains(process.stdout_text,
        "Fill-color object:",
        "#2575: default studio host usage should preserve en-US fill-color labels");
    expect_contains(process.stdout_text,
        "Record-source object:",
        "#2575: default studio host usage should preserve en-US record-source labels");
    expect_contains(process.stdout_text,
        "OLE drop-mode object:",
        "#2569: default studio host usage should preserve localized OLE drop-mode labels");
    expect_contains(process.stdout_text,
        "WhatsThis help ID object:",
        "#2569: default studio host usage should preserve localized WhatsThis help ID labels");
    expect_contains(process.stdout_text,
        "--selection-builder-dispatch-execution-catalog",
        "#1409: studio host usage should advertise selected builder execution catalog");
    expect_contains(process.stdout_text,
        "--selection-toolbox-dispatch-execution-catalog",
        "#1409: studio host usage should advertise selected toolbox execution catalog");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(studio_host_path, {}, temp_root);

    expect(process.exit_code == 2,
        "#2394: pseudo-localized no-argument studio host invocation should preserve usage failure");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2394: pseudo-localized studio host usage should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "copperfin_studio_host",
        "#2394: pseudo-localized studio host usage should preserve command name");
    expect_contains(process.stdout_text,
        "--path",
        "#2394: pseudo-localized studio host usage should preserve CLI flags");
    expect_contains(process.stdout_text,
        "<true|false>",
        "#2394: pseudo-localized studio host usage should preserve invariant value placeholders");
    expect_contains(process.stdout_text,
        "--selection-toolbox-dispatch-execution-catalog",
        "#2394: pseudo-localized studio host usage should preserve catalog command tokens");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "qps-ploc");
    const auto spanish_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "pt-BR");
    const std::vector<std::string_view> object_command_keys = {
        "StudioHost.LaunchParse.ObjectCommand.Rename",
        "StudioHost.LaunchParse.ObjectCommand.RenameRequiredOptions",
        "StudioHost.LaunchParse.ObjectCommand.Reorder",
        "StudioHost.LaunchParse.ObjectCommand.Reparent",
        "StudioHost.LaunchParse.ObjectCommand.ReparentRequiredOptions"};
    const std::vector<std::string_view> object_action_keys = {
        "StudioHost.LaunchParse.ObjectAction.Alignment",
        "StudioHost.LaunchParse.ObjectAction.AlignmentTitle",
        "StudioHost.LaunchParse.ObjectAction.Distribution",
        "StudioHost.LaunchParse.ObjectAction.DistributionTitle",
        "StudioHost.LaunchParse.ObjectAction.Nudge",
        "StudioHost.LaunchParse.ObjectAction.NudgeTitle",
        "StudioHost.LaunchParse.ObjectAction.Resize",
        "StudioHost.LaunchParse.ObjectAction.ResizeTitle",
        "StudioHost.LaunchParse.ObjectAction.Snap",
        "StudioHost.LaunchParse.ObjectAction.SnapTitle"};
    const std::vector<std::string_view> object_assignment_layout_window_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.AddLineFeeds",
        "StudioHost.LaunchParse.ObjectAssignment.AddLineFeedsTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AllowCellSelection",
        "StudioHost.LaunchParse.ObjectAssignment.AllowCellSelectionTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AllowHeaderSizing",
        "StudioHost.LaunchParse.ObjectAssignment.AllowHeaderSizingTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AllowOutput",
        "StudioHost.LaunchParse.ObjectAssignment.AllowOutputTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AllowRowSizing",
        "StudioHost.LaunchParse.ObjectAssignment.AllowRowSizingTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AlwaysOnBottom",
        "StudioHost.LaunchParse.ObjectAssignment.AlwaysOnBottomTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AlwaysOnTop",
        "StudioHost.LaunchParse.ObjectAssignment.AlwaysOnTopTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AutoCenter",
        "StudioHost.LaunchParse.ObjectAssignment.AutoCenterTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AutoRelease",
        "StudioHost.LaunchParse.ObjectAssignment.AutoReleaseTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AutoSize",
        "StudioHost.LaunchParse.ObjectAssignment.AutoSizeTitle",
        "StudioHost.LaunchParse.ObjectAssignment.AutoVerbMenu"};
    const std::vector<std::string_view> object_assignment_control_display_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.ContinuousScroll",
        "StudioHost.LaunchParse.ObjectAssignment.ContinuousScrollTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ControlBox",
        "StudioHost.LaunchParse.ObjectAssignment.ControlBoxTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ControlSource",
        "StudioHost.LaunchParse.ObjectAssignment.ControlSourceTitle",
        "StudioHost.LaunchParse.ObjectAssignment.CurrentControl",
        "StudioHost.LaunchParse.ObjectAssignment.CurrentControlTitle",
        "StudioHost.LaunchParse.ObjectAssignment.Curvature",
        "StudioHost.LaunchParse.ObjectAssignment.DataSession",
        "StudioHost.LaunchParse.ObjectAssignment.DefaultFilePath",
        "StudioHost.LaunchParse.ObjectAssignment.DeleteMark",
        "StudioHost.LaunchParse.ObjectAssignment.DeleteMarkTitle",
        "StudioHost.LaunchParse.ObjectAssignment.Desktop",
        "StudioHost.LaunchParse.ObjectAssignment.DisabledBackColor",
        "StudioHost.LaunchParse.ObjectAssignment.DisabledForeColor",
        "StudioHost.LaunchParse.ObjectAssignment.DisabledItemBackColor",
        "StudioHost.LaunchParse.ObjectAssignment.DisabledItemForeColor",
        "StudioHost.LaunchParse.ObjectAssignment.DisabledPicture",
        "StudioHost.LaunchParse.ObjectAssignment.DisabledPictureTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DisplayOrientation",
        "StudioHost.LaunchParse.ObjectAssignment.DisplayValue",
        "StudioHost.LaunchParse.ObjectAssignment.Dockable",
        "StudioHost.LaunchParse.ObjectAssignment.DockableTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DownPicture"};
    const std::vector<std::string_view> object_assignment_drag_dynamic_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.DownPictureTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DragIcon",
        "StudioHost.LaunchParse.ObjectAssignment.DragIconTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DragMode",
        "StudioHost.LaunchParse.ObjectAssignment.DragModeTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DrawMode",
        "StudioHost.LaunchParse.ObjectAssignment.DrawStyle",
        "StudioHost.LaunchParse.ObjectAssignment.DrawWidth",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicAlignment",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicBackColor",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicCurrentControl",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicCurrentControlTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontBold",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontBoldTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontItalic",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontItalicTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontName",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontNameTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontOutline",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontOutlineTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontShadow",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontShadowTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontSize",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontSizeTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontStrikethru"};
    const std::vector<std::string_view> object_assignment_font_fill_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontStrikethruTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontUnderline",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicFontUnderlineTitle",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicForeColor",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicInputMask",
        "StudioHost.LaunchParse.ObjectAssignment.DynamicLineHeight",
        "StudioHost.LaunchParse.ObjectAssignment.Enabled",
        "StudioHost.LaunchParse.ObjectAssignment.EnabledTitle",
        "StudioHost.LaunchParse.ObjectAssignment.FillColor",
        "StudioHost.LaunchParse.ObjectAssignment.FillStyle",
        "StudioHost.LaunchParse.ObjectAssignment.FontBold",
        "StudioHost.LaunchParse.ObjectAssignment.FontItalic",
        "StudioHost.LaunchParse.ObjectAssignment.FontName",
        "StudioHost.LaunchParse.ObjectAssignment.FontOutline",
        "StudioHost.LaunchParse.ObjectAssignment.FontShadow",
        "StudioHost.LaunchParse.ObjectAssignment.FontSize",
        "StudioHost.LaunchParse.ObjectAssignment.FontStrikethru",
        "StudioHost.LaunchParse.ObjectAssignment.FontUnderline",
        "StudioHost.LaunchParse.ObjectAssignment.ForeColor",
        "StudioHost.LaunchParse.ObjectAssignment.FormSetClass",
        "StudioHost.LaunchParse.ObjectAssignment.Format",
        "StudioHost.LaunchParse.ObjectAssignment.FormatTitle",
        "StudioHost.LaunchParse.ObjectAssignment.GridLineColor",
        "StudioHost.LaunchParse.ObjectAssignment.GridLineWidth",
        "StudioHost.LaunchParse.ObjectAssignment.GridLines"};
    const std::vector<std::string_view> object_assignment_base_layout_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.BackColor",
        "StudioHost.LaunchParse.ObjectAssignment.BackStyle",
        "StudioHost.LaunchParse.ObjectAssignment.BindControls",
        "StudioHost.LaunchParse.ObjectAssignment.BorderColor",
        "StudioHost.LaunchParse.ObjectAssignment.BorderStyle",
        "StudioHost.LaunchParse.ObjectAssignment.BorderWidth",
        "StudioHost.LaunchParse.ObjectAssignment.BoundColumn",
        "StudioHost.LaunchParse.ObjectAssignment.BoundColumnTitle",
        "StudioHost.LaunchParse.ObjectAssignment.BufferMode",
        "StudioHost.LaunchParse.ObjectAssignment.BufferModeOverride",
        "StudioHost.LaunchParse.ObjectAssignment.ButtonCount",
        "StudioHost.LaunchParse.ObjectAssignment.Caption",
        "StudioHost.LaunchParse.ObjectAssignment.CaptionTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ChildOrder",
        "StudioHost.LaunchParse.ObjectAssignment.ClipControls",
        "StudioHost.LaunchParse.ObjectAssignment.ClipControlsTitle",
        "StudioHost.LaunchParse.ObjectAssignment.Closable",
        "StudioHost.LaunchParse.ObjectAssignment.ClosableTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ColumnCount",
        "StudioHost.LaunchParse.ObjectAssignment.ColumnCountTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ColumnLines",
        "StudioHost.LaunchParse.ObjectAssignment.ColumnLinesTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ColumnOrder",
        "StudioHost.LaunchParse.ObjectAssignment.ColumnWidths",
        "StudioHost.LaunchParse.ObjectAssignment.ColumnWidthsTitle"};
    const std::vector<std::string_view> object_assignment_selection_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.HalfHeightCaption",
        "StudioHost.LaunchParse.ObjectAssignment.HeaderHeight",
        "StudioHost.LaunchParse.ObjectAssignment.HelpContextId",
        "StudioHost.LaunchParse.ObjectAssignment.HideSelection",
        "StudioHost.LaunchParse.ObjectAssignment.HighlightBackColor",
        "StudioHost.LaunchParse.ObjectAssignment.HighlightForeColor",
        "StudioHost.LaunchParse.ObjectAssignment.HighlightRow",
        "StudioHost.LaunchParse.ObjectAssignment.HighlightRowLineWidth",
        "StudioHost.LaunchParse.ObjectAssignment.HighlightRowTitle",
        "StudioHost.LaunchParse.ObjectAssignment.HighlightStyle",
        "StudioHost.LaunchParse.ObjectAssignment.IncrementalSearch",
        "StudioHost.LaunchParse.ObjectAssignment.IncrementalSearchTitle",
        "StudioHost.LaunchParse.ObjectAssignment.InitialSelectedAlias",
        "StudioHost.LaunchParse.ObjectAssignment.InputMask",
        "StudioHost.LaunchParse.ObjectAssignment.InputMaskTitle",
        "StudioHost.LaunchParse.ObjectAssignment.IntegralHeight",
        "StudioHost.LaunchParse.ObjectAssignment.IntegralHeightTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ItemBackColor",
        "StudioHost.LaunchParse.ObjectAssignment.ItemForeColor",
        "StudioHost.LaunchParse.ObjectAssignment.KeyPreview",
        "StudioHost.LaunchParse.ObjectAssignment.LeftColumn",
        "StudioHost.LaunchParse.ObjectAssignment.LeftColumnTitle",
        "StudioHost.LaunchParse.ObjectAssignment.LinkMaster",
        "StudioHost.LaunchParse.ObjectAssignment.ListIndex",
        "StudioHost.LaunchParse.ObjectAssignment.ListIndexTitle"};
    const std::vector<std::string_view> object_assignment_lock_window_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.ListItemId",
        "StudioHost.LaunchParse.ObjectAssignment.LockColumns",
        "StudioHost.LaunchParse.ObjectAssignment.LockColumnsLeft",
        "StudioHost.LaunchParse.ObjectAssignment.LockScreen",
        "StudioHost.LaunchParse.ObjectAssignment.LockScreenTitle",
        "StudioHost.LaunchParse.ObjectAssignment.Locked",
        "StudioHost.LaunchParse.ObjectAssignment.LockedTitle",
        "StudioHost.LaunchParse.ObjectAssignment.MacDesktop",
        "StudioHost.LaunchParse.ObjectAssignment.MaxButton",
        "StudioHost.LaunchParse.ObjectAssignment.MaxHeight",
        "StudioHost.LaunchParse.ObjectAssignment.MaxLeft",
        "StudioHost.LaunchParse.ObjectAssignment.MaxTop",
        "StudioHost.LaunchParse.ObjectAssignment.MaxWidth",
        "StudioHost.LaunchParse.ObjectAssignment.MdiForm",
        "StudioHost.LaunchParse.ObjectAssignment.MinButton",
        "StudioHost.LaunchParse.ObjectAssignment.MinHeight",
        "StudioHost.LaunchParse.ObjectAssignment.MinWidth",
        "StudioHost.LaunchParse.ObjectAssignment.MouseIcon",
        "StudioHost.LaunchParse.ObjectAssignment.MouseIconTitle",
        "StudioHost.LaunchParse.ObjectAssignment.MousePointer",
        "StudioHost.LaunchParse.ObjectAssignment.Movable",
        "StudioHost.LaunchParse.ObjectAssignment.MultiSelect",
        "StudioHost.LaunchParse.ObjectAssignment.MultiSelectTitle",
        "StudioHost.LaunchParse.ObjectAssignment.OleDragMode",
        "StudioHost.LaunchParse.ObjectAssignment.OleDragModeTitle"};
    const std::vector<std::string_view> object_assignment_picture_record_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.OleDragPicture",
        "StudioHost.LaunchParse.ObjectAssignment.OleDragPictureTitle",
        "StudioHost.LaunchParse.ObjectAssignment.OleDropEffectsTitle",
        "StudioHost.LaunchParse.ObjectAssignment.OleDropModeTitle",
        "StudioHost.LaunchParse.ObjectAssignment.OleDropTextInsertionTitle",
        "StudioHost.LaunchParse.ObjectAssignment.PanelLink",
        "StudioHost.LaunchParse.ObjectAssignment.PanelLinkTitle",
        "StudioHost.LaunchParse.ObjectAssignment.Partition",
        "StudioHost.LaunchParse.ObjectAssignment.Picture",
        "StudioHost.LaunchParse.ObjectAssignment.PictureMargin",
        "StudioHost.LaunchParse.ObjectAssignment.PicturePosition",
        "StudioHost.LaunchParse.ObjectAssignment.PictureSelectionDisplay",
        "StudioHost.LaunchParse.ObjectAssignment.PictureSpacing",
        "StudioHost.LaunchParse.ObjectAssignment.PictureTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ReadOnly",
        "StudioHost.LaunchParse.ObjectAssignment.ReadOnlyTitle",
        "StudioHost.LaunchParse.ObjectAssignment.RecordMark",
        "StudioHost.LaunchParse.ObjectAssignment.RecordMarkTitle",
        "StudioHost.LaunchParse.ObjectAssignment.RecordSource",
        "StudioHost.LaunchParse.ObjectAssignment.RecordSourceType",
        "StudioHost.LaunchParse.ObjectAssignment.Resizable",
        "StudioHost.LaunchParse.ObjectAssignment.ResizableTitle",
        "StudioHost.LaunchParse.ObjectAssignment.RowHeight",
        "StudioHost.LaunchParse.ObjectAssignment.RowSource",
        "StudioHost.LaunchParse.ObjectAssignment.RowSourceTitle"};
    const std::vector<std::string_view> object_assignment_selection_state_tab_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.RowSourceType",
        "StudioHost.LaunchParse.ObjectAssignment.RowSourceTypeTitle",
        "StudioHost.LaunchParse.ObjectAssignment.ScaleMode",
        "StudioHost.LaunchParse.ObjectAssignment.ScrollBars",
        "StudioHost.LaunchParse.ObjectAssignment.SelectedBackColor",
        "StudioHost.LaunchParse.ObjectAssignment.SelectedForeColor",
        "StudioHost.LaunchParse.ObjectAssignment.SelectedItemBackColor",
        "StudioHost.LaunchParse.ObjectAssignment.SelectedItemForeColor",
        "StudioHost.LaunchParse.ObjectAssignment.ShowWindow",
        "StudioHost.LaunchParse.ObjectAssignment.Sparse",
        "StudioHost.LaunchParse.ObjectAssignment.SparseTitle",
        "StudioHost.LaunchParse.ObjectAssignment.SpecialEffect",
        "StudioHost.LaunchParse.ObjectAssignment.SplitBar",
        "StudioHost.LaunchParse.ObjectAssignment.SplitBarTitle",
        "StudioHost.LaunchParse.ObjectAssignment.StartingTabIndex",
        "StudioHost.LaunchParse.ObjectAssignment.StatusBarText",
        "StudioHost.LaunchParse.ObjectAssignment.StatusBarTextTitle",
        "StudioHost.LaunchParse.ObjectAssignment.Style",
        "StudioHost.LaunchParse.ObjectAssignment.StyleTitle",
        "StudioHost.LaunchParse.ObjectAssignment.TabOrder",
        "StudioHost.LaunchParse.ObjectAssignment.TabOrderTitle",
        "StudioHost.LaunchParse.ObjectAssignment.TabOrientation",
        "StudioHost.LaunchParse.ObjectAssignment.TabStop",
        "StudioHost.LaunchParse.ObjectAssignment.TabStopTitle",
        "StudioHost.LaunchParse.ObjectAssignment.TitleBar"};
    const std::vector<std::string_view> host_tail_parse_keys = {
        "StudioHost.LaunchParse.ObjectAssignment.TooltipText",
        "StudioHost.LaunchParse.ObjectAssignment.TooltipTextTitle",
        "StudioHost.LaunchParse.ObjectAssignment.Visibility",
        "StudioHost.LaunchParse.ObjectAssignment.VisibilityTitle",
        "StudioHost.LaunchParse.ObjectAssignment.WhatsThisButtonTitle",
        "StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelpIdTitle",
        "StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelpTitle",
        "StudioHost.LaunchParse.ObjectAssignment.WindowState",
        "StudioHost.LaunchParse.PropertyCommand.Clear",
        "StudioHost.LaunchParse.PropertyCommand.Rename",
        "StudioHost.LaunchParse.PropertyCommand.Update",
        "StudioHost.LaunchParse.Request.DeletedStateTargetTitle",
        "StudioHost.LaunchParse.Request.DeletedStates",
        "StudioHost.LaunchParse.Request.SubtreeDeletedState",
        "StudioHost.LaunchParse.Request.SubtreeDeletedStateTitle",
        "StudioHost.LaunchParse.SelectionContextAllowedValues",
        "StudioHost.LaunchParse.Selector.Root",
        "StudioHost.LaunchParse.Selector.Target",
        "StudioHost.LaunchParse.Value.Value",
        "StudioHost.VisualMethodParse.Error.BooleanValue",
        "StudioHost.VisualMethodParse.Error.CopyBatchItemRequiresMethodName",
        "StudioHost.VisualMethodParse.Error.DeleteBatchItemRequiresMethodName",
        "StudioHost.VisualMethodParse.Error.MissingValue",
        "StudioHost.VisualMethodParse.Error.MoveBatchItemRequiresMethodName",
        "StudioHost.VisualMethodParse.Error.NoAssetPath"};
    const std::vector<std::string_view> visual_method_object_error_keys = {
        "StudioHost.VisualMethodParse.Error.NoMethodCopies",
        "StudioHost.VisualMethodParse.Error.NoMethodDeletes",
        "StudioHost.VisualMethodParse.Error.NoMethodKind",
        "StudioHost.VisualMethodParse.Error.NoMethodMoves",
        "StudioHost.VisualMethodParse.Error.NoMethodName",
        "StudioHost.VisualMethodParse.Error.NoMethodPlacement",
        "StudioHost.VisualMethodParse.Error.NoMethodRenames",
        "StudioHost.VisualMethodParse.Error.NoMethodReorders",
        "StudioHost.VisualMethodParse.Error.NoMethodSource",
        "StudioHost.VisualMethodParse.Error.NoTargetMethodName",
        "StudioHost.VisualMethodParse.Error.NonNegativeInteger",
        "StudioHost.VisualMethodParse.Error.RenameBatchItemRequiresMethodName",
        "StudioHost.VisualMethodParse.Error.ReorderBatchItemRequiresMethodName",
        "StudioHost.VisualMethodParse.Error.UnknownOption",
        "StudioHost.VisualObjectParse.Error.DuplicateBatchItemRequiresSelectedObject",
        "StudioHost.VisualObjectParse.Error.MissingValue",
        "StudioHost.VisualObjectParse.Error.NoAssetPath",
        "StudioHost.VisualObjectParse.Error.NoDuplicateOperations",
        "StudioHost.VisualObjectParse.Error.NoObjectPlacement",
        "StudioHost.VisualObjectParse.Error.NoRenameOperations",
        "StudioHost.VisualObjectParse.Error.NoReorderOperations",
        "StudioHost.VisualObjectParse.Error.NoReparentOperations",
        "StudioHost.VisualObjectParse.Error.NoRootObjectSelector",
        "StudioHost.VisualObjectParse.Error.NoSubtreeReplacementIdentities",
        "StudioHost.VisualObjectParse.Error.NoUpdateOperations"};
    const std::vector<std::string_view> visual_object_property_error_keys = {
        "StudioHost.VisualObjectParse.Error.NonNegativeInteger",
        "StudioHost.VisualObjectParse.Error.RenameBatchItemRequiresSelectedObject",
        "StudioHost.VisualObjectParse.Error.ReorderBatchItemRequiresSelectedObject",
        "StudioHost.VisualObjectParse.Error.ReparentBatchItemRequiresSelectedObject",
        "StudioHost.VisualObjectParse.Error.SubtreeReplacementRequiresSourceUniqueId",
        "StudioHost.VisualObjectParse.Error.UnknownOption",
        "StudioHost.VisualObjectParse.Error.UpdateBatchPropertyOptionsRequireSelectedObject",
        "StudioHost.VisualObjectParse.Error.UpdateBatchPropertyValuesRequirePropertyName",
        "StudioHost.VisualPropertyParse.Error.BooleanValueRequired",
        "StudioHost.VisualPropertyParse.Error.ClearBatchItemRequiresPropertyName",
        "StudioHost.VisualPropertyParse.Error.CopyBatchItemRequiresPropertyName",
        "StudioHost.VisualPropertyParse.Error.MissingValue",
        "StudioHost.VisualPropertyParse.Error.MoveBatchItemRequiresPropertyName",
        "StudioHost.VisualPropertyParse.Error.NoAssetPath",
        "StudioHost.VisualPropertyParse.Error.NoPropertyChanges",
        "StudioHost.VisualPropertyParse.Error.NoPropertyClears",
        "StudioHost.VisualPropertyParse.Error.NoPropertyCopies",
        "StudioHost.VisualPropertyParse.Error.NoPropertyMoves",
        "StudioHost.VisualPropertyParse.Error.NoPropertyName",
        "StudioHost.VisualPropertyParse.Error.NoPropertyPlacement",
        "StudioHost.VisualPropertyParse.Error.NoPropertyRenames",
        "StudioHost.VisualPropertyParse.Error.NoPropertyReorders",
        "StudioHost.VisualPropertyParse.Error.NoTargetPropertyName",
        "StudioHost.VisualPropertyParse.Error.NonNegativeInteger",
        "StudioHost.VisualPropertyParse.Error.RenameBatchItemRequiresPropertyName"};
    const std::vector<std::string_view> visual_property_asset_editor_keys = {
        "StudioHost.VisualPropertyParse.Error.ReorderBatchItemRequiresPropertyName",
        "StudioHost.VisualPropertyParse.Error.UnknownOption",
        "StudioHost.VisualPropertyParse.Error.UpdateBatchItemRequiresPropertyName",
        "VisualAssetEditor.Field.NameRequired",
        "VisualAssetEditor.Field.NotFound",
        "VisualAssetEditor.Field.TargetNotFound",
        "VisualAssetEditor.Geometry.DistributionCoordinateNotNumeric",
        "VisualAssetEditor.Geometry.DistributionCoordinatesMissing",
        "VisualAssetEditor.Geometry.DistributionDistinctEndpointsRequired",
        "VisualAssetEditor.Geometry.DistributionTargetCountRequired",
        "VisualAssetEditor.Geometry.GridHeightPositiveRequired",
        "VisualAssetEditor.Geometry.GridWidthPositiveRequired",
        "VisualAssetEditor.Geometry.HorizontalNudgeDeltaRequired",
        "VisualAssetEditor.Geometry.ObjectGeometryNotNumeric",
        "VisualAssetEditor.Geometry.RequiredFieldsMissing",
        "VisualAssetEditor.Geometry.VerticalNudgeDeltaRequired",
        "VisualAssetEditor.Identity.CopiedRowFieldRequired",
        "VisualAssetEditor.Identity.FieldMissing",
        "VisualAssetEditor.Identity.FieldsRequired",
        "VisualAssetEditor.Identity.ReplacementDuplicatedInSubtree",
        "VisualAssetEditor.Identity.ReplacementExists",
        "VisualAssetEditor.Identity.ReplacementFieldMissing",
        "VisualAssetEditor.Identity.SubtreeReplacementBatchRequired",
        "VisualAssetEditor.Identity.SubtreeReplacementDataMissing",
        "VisualAssetEditor.Method.Ambiguous"};
    const std::vector<std::string_view> asset_editor_method_object_keys = {
        "VisualAssetEditor.Identity.SubtreeReplacementMissingOrAmbiguous",
        "VisualAssetEditor.Identity.ValueExists",
        "VisualAssetEditor.Method.CopyBatchRequired",
        "VisualAssetEditor.Method.DeclarationParseFailed",
        "VisualAssetEditor.Method.DeleteBatchRequired",
        "VisualAssetEditor.Method.MoveBatchRequired",
        "VisualAssetEditor.Method.NameRequired",
        "VisualAssetEditor.Method.NamesCannotBeEmpty",
        "VisualAssetEditor.Method.NotFound",
        "VisualAssetEditor.Method.PlacementUnsupported",
        "VisualAssetEditor.Method.RelativeAmbiguous",
        "VisualAssetEditor.Method.RelativeNameRequired",
        "VisualAssetEditor.Method.RelativeNotFound",
        "VisualAssetEditor.Method.RenameBatchRequired",
        "VisualAssetEditor.Method.ReorderBatchRequired",
        "VisualAssetEditor.Method.SourceMoveToSelf",
        "VisualAssetEditor.Method.SourceNotFound",
        "VisualAssetEditor.Method.SourceRelativeToSelf",
        "VisualAssetEditor.Method.TargetExists",
        "VisualAssetEditor.Method.TargetNameRequired",
        "VisualAssetEditor.Method.TargetObjectAlreadyHasMethod",
        "VisualAssetEditor.Object.AlignmentModeUnsupported",
        "VisualAssetEditor.Object.AlignmentTargetsRequired",
        "VisualAssetEditor.Object.ContainerRecordUnavailable",
        "VisualAssetEditor.Object.CopiedRootRecordUnavailable"};
    const std::vector<std::string_view> asset_editor_object_batch_keys = {
        "VisualAssetEditor.Object.CreateBatchRequired",
        "VisualAssetEditor.Object.CreatedBatchRecordUnavailable",
        "VisualAssetEditor.Object.CreatedRecordUnavailable",
        "VisualAssetEditor.Object.DeletedStateBatchRequired",
        "VisualAssetEditor.Object.DistributionModeUnsupported",
        "VisualAssetEditor.Object.DuplicateBatchRequired",
        "VisualAssetEditor.Object.DuplicatedRecordUnavailable",
        "VisualAssetEditor.Object.EditBatchRequired",
        "VisualAssetEditor.Object.EnabledSelectionDuplicate",
        "VisualAssetEditor.Object.EnabledSelectionRequired",
        "VisualAssetEditor.Object.FieldValuesRequired",
        "VisualAssetEditor.Object.GridSnappingModeUnsupported",
        "VisualAssetEditor.Object.GroupContainerFieldsRequired",
        "VisualAssetEditor.Object.GroupContainerNameMissing",
        "VisualAssetEditor.Object.GroupContainerUnavailable",
        "VisualAssetEditor.Object.GroupSelectionRequired",
        "VisualAssetEditor.Object.LockedSelectionDuplicate",
        "VisualAssetEditor.Object.LockedSelectionRequired",
        "VisualAssetEditor.Object.MemoFieldMissing",
        "VisualAssetEditor.Object.NameAmbiguous",
        "VisualAssetEditor.Object.NameNotFound",
        "VisualAssetEditor.Object.NameRequired",
        "VisualAssetEditor.Object.NudgeModeUnsupported",
        "VisualAssetEditor.Object.NudgeSelectionRequired",
        "VisualAssetEditor.Object.ParentChainCycle"};
    const std::vector<std::string_view> asset_editor_object_selection_keys = {
        "VisualAssetEditor.Object.ParentNameAmbiguous",
        "VisualAssetEditor.Object.ParentNameMissing",
        "VisualAssetEditor.Object.ParentObjectChainCycle",
        "VisualAssetEditor.Object.ParentRecordUnavailable",
        "VisualAssetEditor.Object.ParentSelectorRequired",
        "VisualAssetEditor.Object.PlacementUnsupported",
        "VisualAssetEditor.Object.PropertyAssignmentDuplicate",
        "VisualAssetEditor.Object.PropertyAssignmentSelectionRequired",
        "VisualAssetEditor.Object.ReadOnlySelectionDuplicate",
        "VisualAssetEditor.Object.ReadOnlySelectionRequired",
        "VisualAssetEditor.Object.RecordUnavailable",
        "VisualAssetEditor.Object.RenameBatchRequired",
        "VisualAssetEditor.Object.ReorderBatchRequired",
        "VisualAssetEditor.Object.ReorderRelativeToSelf",
        "VisualAssetEditor.Object.ReparentBatchRequired",
        "VisualAssetEditor.Object.ReparentDescendantUnsupported",
        "VisualAssetEditor.Object.ReparentSelfUnsupported",
        "VisualAssetEditor.Object.ResizeModeUnsupported",
        "VisualAssetEditor.Object.ResizeTargetsRequired",
        "VisualAssetEditor.Object.SelectedFieldMissing",
        "VisualAssetEditor.Object.SelectedFieldOrPropertyMissing",
        "VisualAssetEditor.Object.SelectedMemoFieldMissing",
        "VisualAssetEditor.Object.SelectedContainerChildrenRequired",
        "VisualAssetEditor.Object.SelectedContainerNameMissing",
        "VisualAssetEditor.Object.SnapSelectionRequired"};
    const std::vector<std::string_view> asset_editor_object_property_keys = {
        "VisualAssetEditor.Object.TabOrderSelectionDuplicate",
        "VisualAssetEditor.Object.TabOrderSelectionRequired",
        "VisualAssetEditor.Object.TabStopSelectionDuplicate",
        "VisualAssetEditor.Object.TabStopSelectionRequired",
        "VisualAssetEditor.Object.TargetRecordUnavailable",
        "VisualAssetEditor.Object.TargetSelectorRequired",
        "VisualAssetEditor.Object.UniqueIdAmbiguous",
        "VisualAssetEditor.Object.UniqueIdNotFound",
        "VisualAssetEditor.Object.VisibilitySelectionDuplicate",
        "VisualAssetEditor.Object.VisibilitySelectionRequired",
        "VisualAssetEditor.Operation.AssetPathRequired",
        "VisualAssetEditor.Operation.RollbackFailed",
        "VisualAssetEditor.Operation.TargetRollbackFailed",
        "VisualAssetEditor.Property.Ambiguous",
        "VisualAssetEditor.Property.ChangeBatchRequired",
        "VisualAssetEditor.Property.ClearBatchRequired",
        "VisualAssetEditor.Property.CopyBatchRequired",
        "VisualAssetEditor.Property.DirectFieldRenameUnsupported",
        "VisualAssetEditor.Property.DirectFieldReorderUnsupported",
        "VisualAssetEditor.Property.FontSizeFiniteNonNegativeRequired",
        "VisualAssetEditor.Property.MoveBatchRequired",
        "VisualAssetEditor.Property.NameRequired",
        "VisualAssetEditor.Property.NonNegativeRequired",
        "VisualAssetEditor.Property.NotFound",
        "VisualAssetEditor.Property.NotRenameableMemo"};
    const std::vector<std::string_view> asset_editor_property_storage_keys = {
        "VisualAssetEditor.Property.NotReorderableMemo",
        "VisualAssetEditor.Property.NotWritableField",
        "VisualAssetEditor.Property.PlacementUnsupported",
        "VisualAssetEditor.Property.ReadFailed",
        "VisualAssetEditor.Property.RelativeAmbiguous",
        "VisualAssetEditor.Property.RelativeNameRequired",
        "VisualAssetEditor.Property.RelativeNotFound",
        "VisualAssetEditor.Property.RenameBatchRequired",
        "VisualAssetEditor.Property.ReorderBatchRequired",
        "VisualAssetEditor.Property.SourceAmbiguousInObject",
        "VisualAssetEditor.Property.SourceMoveToSelf",
        "VisualAssetEditor.Property.SourceNotFound",
        "VisualAssetEditor.Property.SourceRelativeToSelf",
        "VisualAssetEditor.Property.SourceRenameToSelf",
        "VisualAssetEditor.Property.StartingTabIndexNonNegativeRequired",
        "VisualAssetEditor.Property.TargetExistsInObject",
        "VisualAssetEditor.Property.TargetNameRequired",
        "VisualAssetEditor.Property.TargetObjectAlreadyHasProperty",
        "VisualAssetEditor.Storage.CharacterValueTooLarge",
        "VisualAssetEditor.Storage.DirectFieldUpdateUnsupported",
        "VisualAssetEditor.Storage.LogicalValueRequired",
        "VisualAssetEditor.Storage.MemoSidecarBlockSizeInvalid",
        "VisualAssetEditor.Storage.MemoSidecarNextFreeBlockInvalid",
        "VisualAssetEditor.Storage.MemoSidecarOpenFailed",
        "VisualAssetEditor.Storage.MemoSidecarPathMissing"};
    const std::vector<std::string_view> asset_editor_property_label_batch1_keys = {
        "VisualAssetEditor.PropertyLabel.AddLineFeeds",
        "VisualAssetEditor.PropertyLabel.AllowCellSelection",
        "VisualAssetEditor.PropertyLabel.AllowHeaderSizing",
        "VisualAssetEditor.PropertyLabel.AllowOutput",
        "VisualAssetEditor.PropertyLabel.AllowRowSizing",
        "VisualAssetEditor.PropertyLabel.AlwaysOnBottom",
        "VisualAssetEditor.PropertyLabel.AlwaysOnTop",
        "VisualAssetEditor.PropertyLabel.AutoCenter",
        "VisualAssetEditor.PropertyLabel.AutoRelease",
        "VisualAssetEditor.PropertyLabel.AutoSize",
        "VisualAssetEditor.PropertyLabel.AutoVerbMenu",
        "VisualAssetEditor.PropertyLabel.BackColor",
        "VisualAssetEditor.PropertyLabel.BackStyle",
        "VisualAssetEditor.PropertyLabel.BindControls",
        "VisualAssetEditor.PropertyLabel.BorderColor",
        "VisualAssetEditor.PropertyLabel.BorderStyle",
        "VisualAssetEditor.PropertyLabel.BorderWidth",
        "VisualAssetEditor.PropertyLabel.BoundColumn",
        "VisualAssetEditor.PropertyLabel.BufferMode",
        "VisualAssetEditor.PropertyLabel.BufferModeOverride",
        "VisualAssetEditor.PropertyLabel.ButtonCount",
        "VisualAssetEditor.PropertyLabel.Caption",
        "VisualAssetEditor.PropertyLabel.ChildOrder",
        "VisualAssetEditor.PropertyLabel.ClipControls",
        "VisualAssetEditor.PropertyLabel.Closable"};
    const std::vector<std::string_view> asset_editor_property_label_batch2_keys = {
        "VisualAssetEditor.PropertyLabel.ColumnCount",
        "VisualAssetEditor.PropertyLabel.ColumnLines",
        "VisualAssetEditor.PropertyLabel.ColumnOrder",
        "VisualAssetEditor.PropertyLabel.ColumnWidths",
        "VisualAssetEditor.PropertyLabel.ContinuousScroll",
        "VisualAssetEditor.PropertyLabel.ControlBox",
        "VisualAssetEditor.PropertyLabel.ControlSource",
        "VisualAssetEditor.PropertyLabel.CurrentControl",
        "VisualAssetEditor.PropertyLabel.Curvature",
        "VisualAssetEditor.PropertyLabel.DataSession",
        "VisualAssetEditor.PropertyLabel.DefaultFilePath",
        "VisualAssetEditor.PropertyLabel.DeleteMark",
        "VisualAssetEditor.PropertyLabel.Desktop",
        "VisualAssetEditor.PropertyLabel.DisabledBackColor",
        "VisualAssetEditor.PropertyLabel.DisabledForeColor",
        "VisualAssetEditor.PropertyLabel.DisabledItemBackColor",
        "VisualAssetEditor.PropertyLabel.DisabledItemForeColor",
        "VisualAssetEditor.PropertyLabel.DisabledPicture",
        "VisualAssetEditor.PropertyLabel.DisplayOrientation",
        "VisualAssetEditor.PropertyLabel.DisplayValue",
        "VisualAssetEditor.PropertyLabel.Dockable",
        "VisualAssetEditor.PropertyLabel.DownPicture",
        "VisualAssetEditor.PropertyLabel.DragIcon",
        "VisualAssetEditor.PropertyLabel.DragMode",
        "VisualAssetEditor.PropertyLabel.DrawMode"};
    const std::vector<std::string_view> asset_editor_property_label_batch3_keys = {
        "VisualAssetEditor.PropertyLabel.DrawStyle",
        "VisualAssetEditor.PropertyLabel.DrawWidth",
        "VisualAssetEditor.PropertyLabel.DynamicAlignment",
        "VisualAssetEditor.PropertyLabel.DynamicBackColor",
        "VisualAssetEditor.PropertyLabel.DynamicCurrentControl",
        "VisualAssetEditor.PropertyLabel.DynamicFontBold",
        "VisualAssetEditor.PropertyLabel.DynamicFontItalic",
        "VisualAssetEditor.PropertyLabel.DynamicFontName",
        "VisualAssetEditor.PropertyLabel.DynamicFontOutline",
        "VisualAssetEditor.PropertyLabel.DynamicFontShadow",
        "VisualAssetEditor.PropertyLabel.DynamicFontSize",
        "VisualAssetEditor.PropertyLabel.DynamicFontStrikethru",
        "VisualAssetEditor.PropertyLabel.DynamicFontUnderline",
        "VisualAssetEditor.PropertyLabel.DynamicForeColor",
        "VisualAssetEditor.PropertyLabel.DynamicInputMask",
        "VisualAssetEditor.PropertyLabel.DynamicLineHeight",
        "VisualAssetEditor.PropertyLabel.FillColor",
        "VisualAssetEditor.PropertyLabel.FillStyle",
        "VisualAssetEditor.PropertyLabel.FontBold",
        "VisualAssetEditor.PropertyLabel.FontItalic",
        "VisualAssetEditor.PropertyLabel.FontName",
        "VisualAssetEditor.PropertyLabel.FontOutline",
        "VisualAssetEditor.PropertyLabel.FontShadow",
        "VisualAssetEditor.PropertyLabel.FontSize",
        "VisualAssetEditor.PropertyLabel.FontStrikethru"};
    const std::vector<std::string_view> asset_editor_property_label_batch4_keys = {
        "VisualAssetEditor.PropertyLabel.FontUnderline",
        "VisualAssetEditor.PropertyLabel.ForeColor",
        "VisualAssetEditor.PropertyLabel.FormSetClass",
        "VisualAssetEditor.PropertyLabel.Format",
        "VisualAssetEditor.PropertyLabel.GridLineColor",
        "VisualAssetEditor.PropertyLabel.GridLineWidth",
        "VisualAssetEditor.PropertyLabel.GridLines",
        "VisualAssetEditor.PropertyLabel.HalfHeightCaption",
        "VisualAssetEditor.PropertyLabel.HeaderHeight",
        "VisualAssetEditor.PropertyLabel.HelpContextID",
        "VisualAssetEditor.PropertyLabel.HideSelection",
        "VisualAssetEditor.PropertyLabel.HighlightBackColor",
        "VisualAssetEditor.PropertyLabel.HighlightForeColor",
        "VisualAssetEditor.PropertyLabel.HighlightRow",
        "VisualAssetEditor.PropertyLabel.HighlightRowLineWidth",
        "VisualAssetEditor.PropertyLabel.HighlightStyle",
        "VisualAssetEditor.PropertyLabel.IncrementalSearch",
        "VisualAssetEditor.PropertyLabel.InitialSelectedAlias",
        "VisualAssetEditor.PropertyLabel.InputMask",
        "VisualAssetEditor.PropertyLabel.IntegralHeight",
        "VisualAssetEditor.PropertyLabel.ItemBackColor",
        "VisualAssetEditor.PropertyLabel.ItemForeColor",
        "VisualAssetEditor.PropertyLabel.KeyPreview",
        "VisualAssetEditor.PropertyLabel.LeftColumn",
        "VisualAssetEditor.PropertyLabel.LinkMaster"};
    const std::vector<std::string_view> asset_editor_property_label_batch5_keys = {
        "VisualAssetEditor.PropertyLabel.ListIndex",
        "VisualAssetEditor.PropertyLabel.ListItemID",
        "VisualAssetEditor.PropertyLabel.LockColumns",
        "VisualAssetEditor.PropertyLabel.LockColumnsLeft",
        "VisualAssetEditor.PropertyLabel.LockScreen",
        "VisualAssetEditor.PropertyLabel.MDIForm",
        "VisualAssetEditor.PropertyLabel.MacDesktop",
        "VisualAssetEditor.PropertyLabel.MaxButton",
        "VisualAssetEditor.PropertyLabel.MaxHeight",
        "VisualAssetEditor.PropertyLabel.MaxLeft",
        "VisualAssetEditor.PropertyLabel.MaxTop",
        "VisualAssetEditor.PropertyLabel.MaxWidth",
        "VisualAssetEditor.PropertyLabel.MinButton",
        "VisualAssetEditor.PropertyLabel.MinHeight",
        "VisualAssetEditor.PropertyLabel.MinWidth",
        "VisualAssetEditor.PropertyLabel.MouseIcon",
        "VisualAssetEditor.PropertyLabel.MousePointer",
        "VisualAssetEditor.PropertyLabel.Movable",
        "VisualAssetEditor.PropertyLabel.MultiSelect",
        "VisualAssetEditor.PropertyLabel.OLEDragMode",
        "VisualAssetEditor.PropertyLabel.OLEDragPicture",
        "VisualAssetEditor.PropertyLabel.OLEDropEffects",
        "VisualAssetEditor.PropertyLabel.OLEDropMode",
        "VisualAssetEditor.PropertyLabel.OLEDropTextInsertion",
        "VisualAssetEditor.PropertyLabel.PanelLink"};
    const std::vector<std::string_view> asset_editor_property_label_batch6_keys = {
        "VisualAssetEditor.PropertyLabel.Partition",
        "VisualAssetEditor.PropertyLabel.Picture",
        "VisualAssetEditor.PropertyLabel.PictureMargin",
        "VisualAssetEditor.PropertyLabel.PicturePosition",
        "VisualAssetEditor.PropertyLabel.PictureSelectionDisplay",
        "VisualAssetEditor.PropertyLabel.PictureSpacing",
        "VisualAssetEditor.PropertyLabel.RecordMark",
        "VisualAssetEditor.PropertyLabel.RecordSource",
        "VisualAssetEditor.PropertyLabel.RecordSourceType",
        "VisualAssetEditor.PropertyLabel.Resizable",
        "VisualAssetEditor.PropertyLabel.RowHeight",
        "VisualAssetEditor.PropertyLabel.RowSource",
        "VisualAssetEditor.PropertyLabel.RowSourceType",
        "VisualAssetEditor.PropertyLabel.ScaleMode",
        "VisualAssetEditor.PropertyLabel.ScrollBars",
        "VisualAssetEditor.PropertyLabel.SelectedBackColor",
        "VisualAssetEditor.PropertyLabel.SelectedForeColor",
        "VisualAssetEditor.PropertyLabel.SelectedItemBackColor",
        "VisualAssetEditor.PropertyLabel.SelectedItemForeColor",
        "VisualAssetEditor.PropertyLabel.ShowWindow",
        "VisualAssetEditor.PropertyLabel.Sparse",
        "VisualAssetEditor.PropertyLabel.SpecialEffect",
        "VisualAssetEditor.PropertyLabel.SplitBar",
        "VisualAssetEditor.PropertyLabel.StatusBarText",
        "VisualAssetEditor.PropertyLabel.Style",
        "VisualAssetEditor.PropertyLabel.TabOrientation",
        "VisualAssetEditor.PropertyLabel.TitleBar",
        "VisualAssetEditor.PropertyLabel.ToolTipText",
        "VisualAssetEditor.PropertyLabel.WhatsThisButton",
        "VisualAssetEditor.PropertyLabel.WhatsThisHelp",
        "VisualAssetEditor.PropertyLabel.WhatsThisHelpID",
        "VisualAssetEditor.PropertyLabel.WindowState"};
    const std::vector<std::string_view> asset_editor_storage_undo_keys = {
        "VisualAssetEditor.Storage.MemoSidecarWriteFailed",
        "VisualAssetEditor.Storage.NumericValueTooLarge",
        "VisualAssetEditor.Storage.RecordDataTruncated",
        "VisualAssetEditor.Storage.RecordIndexOutOfRange",
        "VisualAssetEditor.Storage.TableOpenFailed",
        "VisualAssetEditor.Storage.TableWriteFailed",
        "VisualAssetEditor.Storage.TargetFieldMemoRequired",
        "VisualAssetEditor.Undo.CreateJournalFailed",
        "VisualAssetEditor.Undo.CurrentPropertyReadFailed",
        "VisualAssetEditor.Undo.HistoryUnavailable",
        "VisualAssetEditor.Undo.JournalReadFailed",
        "VisualAssetEditor.Undo.PersistJournalFailed",
        "VisualAssetEditor.Undo.PropertyLabel",
        "VisualAssetEditor.Undo.PropertyLookupMismatch",
        "VisualAssetEditor.Undo.RenamePropertyLabel"};
    const std::vector<std::string_view> pseudo_builder_parse_error_keys = {
        "StudioHost.BuilderParse.Error.BooleanValueRequired",
        "StudioHost.BuilderParse.Error.ContextConflict",
        "StudioHost.BuilderParse.Error.MissingValue",
        "StudioHost.BuilderParse.Error.NoBuilderContext",
        "StudioHost.BuilderParse.Error.NoBuilderId",
        "StudioHost.BuilderParse.Error.NoBuilderLaunchCommand",
        "StudioHost.BuilderParse.Error.NoBuilderOrSelectionContext",
        "StudioHost.BuilderParse.Error.NoSelectionContext",
        "StudioHost.BuilderParse.Error.RecordNonNegativeInteger",
        "StudioHost.BuilderParse.Error.UnknownBuilderContextToken",
        "StudioHost.BuilderParse.Error.UnknownOption",
        "StudioHost.BuilderParse.Error.UnknownSelectionContextToken"};
    expect_contains(process.stdout_text,
        pseudo_catalog.translate(
            "StudioHost.Usage.Alternate",
            {{"usageTemplate", "copperfin_studio_host --list-subsystems [--json]"}}),
        "#2576: pseudo-localized studio host usage should route alternate usage prose through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate(
            "StudioHost.Usage.ObjectEntry",
            {
                {"objectName", pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSourceTitle")},
                {"usageTemplate", "--record-source-object --record-source <value> [--record-source-target-object-name <name>] [--record-source-target-unique-id <id>]"}
            }),
        "#2576: pseudo-localized studio host usage should route object wrapper prose through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate(
            "StudioHost.Usage.SelectionContextTokens",
            {{"selectionContextTokens", "visual_object, visual_method, container_object, class_designer, report_expression, label_expression, menu_item, project_item, data_environment"}}),
        "#2576: pseudo-localized studio host usage should route selection-context token prose through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropMode"),
        "#2569: pseudo-localized studio host usage should route OLE drop-mode labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedBackColorTitle"),
        "#2570: pseudo-localized studio host usage should route selected-back-color labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicForeColorTitle"),
        "#2570: pseudo-localized studio host usage should route dynamic-fore-color labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DesktopTitle"),
        "#2571: pseudo-localized studio host usage should route desktop labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureSelectionDisplayTitle"),
        "#2571: pseudo-localized studio host usage should route picture-selection-display labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicInputMaskTitle"),
        "#2572: pseudo-localized studio host usage should route dynamic-input-mask labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontNameTitle"),
        "#2572: pseudo-localized studio host usage should route font-name labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxTopTitle"),
        "#2572: pseudo-localized studio host usage should route max-top labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ButtonCountTitle"),
        "#2573: pseudo-localized studio host usage should route button-count labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BufferModeOverrideTitle"),
        "#2573: pseudo-localized studio host usage should route buffer-mode-override labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HeaderHeightTitle"),
        "#2573: pseudo-localized studio host usage should route header-height labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowHeightTitle"),
        "#2574: pseudo-localized studio host usage should route row-height labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLineWidthTitle"),
        "#2574: pseudo-localized studio host usage should route grid-line-width labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PartitionTitle"),
        "#2574: pseudo-localized studio host usage should route partition labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSourceTypeTitle"),
        "#2575: pseudo-localized studio host usage should route record-source-type labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FillColorTitle"),
        "#2575: pseudo-localized studio host usage should route fill-color labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSourceTitle"),
        "#2575: pseudo-localized studio host usage should route record-source labels through localization");
    expect_contains(process.stdout_text,
        pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelpId"),
        "#2569: pseudo-localized studio host usage should route WhatsThis help ID labels through localization");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectCommand.Rename") == "renombrar" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectCommand.Reorder") == "reordenar" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectCommand.Rename") == "renomear" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectCommand.Reparent") == "reparentar" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectCommand.Reorder") ==
                copperfin::localization::pseudo_localize("reorder"),
        "#2628: host object-command labels should resolve through locale catalogs without changing CLI option placeholders");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAction.Alignment") == "alineacion" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAction.ResizeTitle") == "Redimensionar" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAction.Alignment") == "alinhamento" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAction.SnapTitle") == "Encaixar" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAction.Nudge") ==
                copperfin::localization::pseudo_localize("nudge"),
        "#2629: host object-action labels should resolve through locale catalogs without changing CLI option placeholders");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoCenter") == "centrado automatico" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowOutputTitle") ==
                "Permitir salida" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoVerbMenu") ==
                "menu automatico de verbos" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AlwaysOnTopTitle") ==
                "Sempre no topo" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AddLineFeeds") ==
                copperfin::localization::pseudo_localize("add-line-feeds"),
        "#2654: host object-assignment layout/window labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.CurrentControl") == "control actual" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledPictureTitle") ==
                "Imagen deshabilitada" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DefaultFilePath") ==
                "caminho de arquivo padrao" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DockableTitle") ==
                "Acoplavel" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledBackColor") ==
                copperfin::localization::pseudo_localize("disabled back-color"),
        "#2655: host object-assignment control/display labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DragModeTitle") == "Modo de arrastre" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontName") ==
                "nombre de fuente dinamica" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawWidth") ==
                "largura de desenho" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicCurrentControlTitle") ==
                "Controle atual dinamico" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicBackColor") ==
                copperfin::localization::pseudo_localize("dynamic-back-color"),
        "#2656: host object-assignment drag and dynamic-font labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FillColor") == "color de relleno" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontUnderlineTitle") ==
                "Fuente dinamica subrayada" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontName") ==
                "nome da fonte" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.EnabledTitle") ==
                "Habilitado" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ForeColor") ==
                copperfin::localization::pseudo_localize("fore-color"),
        "#2657: host object-assignment font/fill labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BackColor") == "color de fondo" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnWidthsTitle") ==
                "Anchos de columna" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BufferModeOverride") ==
                "substituicao do modo de buffer" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ClosableTitle") ==
                "Fechavel" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Caption") ==
                copperfin::localization::pseudo_localize("caption"),
        "#2658: host object-assignment base layout/column labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightBackColor") ==
                "color de fondo del resaltado" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LeftColumnTitle") ==
                "Columna izquierda" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HelpContextId") ==
                "ID do contexto de ajuda" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ListIndexTitle") ==
                "Indice da lista" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemForeColor") ==
                copperfin::localization::pseudo_localize("item-fore-color"),
        "#2659: host object-assignment selection and highlight labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockScreenTitle") ==
                "Bloquear pantalla" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxWidth") ==
                "ancho maximo" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MouseIcon") ==
                "icone do mouse" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MultiSelectTitle") ==
                "Selecao multipla" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDragMode") ==
                copperfin::localization::pseudo_localize("OLE-drag-mode"),
        "#2660: host object-assignment lock and window-state labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDragPictureTitle") ==
                "Imagen de arrastre OLE" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ReadOnly") ==
                "solo lectura" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PanelLinkTitle") ==
                "Vinculo de painel" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowSourceTitle") ==
                "Origem da linha" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSource") ==
                copperfin::localization::pseudo_localize("record-source"),
        "#2661: host object-assignment picture and record labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ScaleMode") ==
                "modo de escala" &&
            spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.StatusBarTextTitle") ==
                "Texto de barra de estado" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SparseTitle") ==
                "Esparso" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabOrder") ==
                "ordem de tabulacao" &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemForeColor") ==
                copperfin::localization::pseudo_localize("selected-item-fore-color"),
        "#2662: host object-assignment selection state and tab labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TooltipTextTitle") ==
                "Texto de informacion sobre herramientas" &&
            spanish_catalog.translate("StudioHost.LaunchParse.PropertyCommand.Clear") == "borrar" &&
            portuguese_catalog.translate("StudioHost.LaunchParse.Request.SubtreeDeletedStateTitle") ==
                "Estado excluido da subarvore" &&
            portuguese_catalog.translate("StudioHost.VisualMethodParse.Error.NoAssetPath") ==
                "Nenhum caminho de ativo foi fornecido." &&
            pseudo_catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WindowState") ==
                copperfin::localization::pseudo_localize("window-state"),
        "#2663: host tooltip, selector, and visual-method parse labels should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.VisualMethodParse.Error.NoMethodKind") ==
                "No se proporciono ningun tipo de metodo." &&
            spanish_catalog.translate("StudioHost.VisualObjectParse.Error.NoRootObjectSelector") ==
                "No se proporciono ningun selector de objeto raiz." &&
            portuguese_catalog.translate("StudioHost.VisualMethodParse.Error.NonNegativeInteger") ==
                "O valor de {option} deve ser um inteiro nao negativo." &&
            portuguese_catalog.translate("StudioHost.VisualObjectParse.Error.NoUpdateOperations") ==
                "Nenhuma operacao de atualizacao foi fornecida." &&
            pseudo_catalog.translate("StudioHost.VisualMethodParse.Error.UnknownOption") ==
                copperfin::localization::pseudo_localize("Unknown {commandName} option: {argument}"),
        "#2664: host visual method and visual object parse errors should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.VisualObjectParse.Error.UpdateBatchPropertyValuesRequirePropertyName") ==
                "Los valores de propiedad del lote de actualizacion de objeto visual requieren un {propertyNameOption} anterior." &&
            spanish_catalog.translate("StudioHost.VisualPropertyParse.Error.NoPropertyChanges") ==
                "No se proporcionaron cambios de propiedad." &&
            portuguese_catalog.translate("StudioHost.VisualObjectParse.Error.ReparentBatchItemRequiresSelectedObject") ==
                "As opcoes do item do lote de reparentamento de objeto visual exigem um seletor de objeto selecionado anterior." &&
            portuguese_catalog.translate("StudioHost.VisualPropertyParse.Error.RenameBatchItemRequiresPropertyName") ==
                "As opcoes do item do lote de renomeacao de propriedade visual exigem um {propertyNameOption} anterior." &&
            pseudo_catalog.translate("StudioHost.VisualPropertyParse.Error.NoPropertyCopies") ==
                copperfin::localization::pseudo_localize("No property copies were provided."),
        "#2665: host visual object and visual property parse errors should resolve through locale catalogs without changing host usage tokens");
    expect(
        spanish_catalog.translate("StudioHost.VisualPropertyParse.Error.UpdateBatchItemRequiresPropertyName") ==
                "Las opciones del elemento del lote de actualizacion de propiedad visual requieren un {propertyNameOption} anterior." &&
            spanish_catalog.translate("VisualAssetEditor.Geometry.GridWidthPositiveRequired") ==
                "El ancho de la cuadrícula debe ser positivo para el ajuste horizontal." &&
            portuguese_catalog.translate("VisualAssetEditor.Field.NameRequired") ==
                "Os nomes de campo nao podem estar vazios." &&
            portuguese_catalog.translate("VisualAssetEditor.Identity.SubtreeReplacementBatchRequired") ==
                "Nenhuma substituicao de subarvore foi fornecida." &&
            pseudo_catalog.translate("VisualAssetEditor.Method.Ambiguous") ==
                copperfin::localization::pseudo_localize("Method match is ambiguous."),
        "#2666: host visual property tail and asset-editor field geometry labels should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.Method.DeclarationParseFailed") ==
                "No se pudo analizar la declaracion del metodo solicitado." &&
            spanish_catalog.translate("VisualAssetEditor.Object.AlignmentModeUnsupported") ==
                "Modo de alineacion de objeto visual no compatible." &&
            portuguese_catalog.translate("VisualAssetEditor.Method.TargetObjectAlreadyHasMethod") ==
                "O objeto de destino ja possui um metodo com o nome solicitado." &&
            portuguese_catalog.translate("VisualAssetEditor.Identity.ValueExists") ==
                "O valor de identidade solicitado ja existe no ativo." &&
            pseudo_catalog.translate("VisualAssetEditor.Method.SourceNotFound") ==
                copperfin::localization::pseudo_localize("The source method was not found."),
        "#2667: asset-editor method and object operation labels should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.Object.CreateBatchRequired") ==
                "No se proporcionaron creaciones de objeto visual." &&
            spanish_catalog.translate("VisualAssetEditor.Object.GroupSelectionRequired") ==
                "No se seleccionaron objetos visuales para agrupar." &&
            portuguese_catalog.translate("VisualAssetEditor.Object.LockedSelectionDuplicate") ==
                "O mesmo objeto visual foi selecionado mais de uma vez para a atribuicao de estado bloqueado." &&
            portuguese_catalog.translate("VisualAssetEditor.Object.ParentChainCycle") ==
                "A cadeia de pais do objeto selecionado contem um ciclo." &&
            pseudo_catalog.translate("VisualAssetEditor.Object.MemoFieldMissing") ==
                copperfin::localization::pseudo_localize("The object does not expose a {fieldName} memo field."),
        "#2668: asset-editor object batch and grouping labels should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.Object.ParentSelectorRequired") ==
                "No se proporciono ningun selector de objeto padre." &&
            spanish_catalog.translate("VisualAssetEditor.Object.SelectedContainerChildrenRequired") ==
                "El contenedor seleccionado no tiene objetos secundarios para desagrupar." &&
            portuguese_catalog.translate("VisualAssetEditor.Object.ReparentDescendantUnsupported") ==
                "Um objeto visual nao pode ser reparentado para um de seus descendentes." &&
            portuguese_catalog.translate("VisualAssetEditor.Object.SnapSelectionRequired") ==
                "Nenhum objeto visual foi selecionado para o encaixe em grade." &&
            pseudo_catalog.translate("VisualAssetEditor.Object.PropertyAssignmentDuplicate") ==
                copperfin::localization::pseudo_localize("The same visual object was selected more than once for {propertyLabel} assignment."),
        "#2669: asset-editor object placement and selection labels should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.Object.TargetSelectorRequired") ==
                "No se proporciono ningun selector de objeto de destino." &&
            spanish_catalog.translate("VisualAssetEditor.Property.DirectFieldRenameUnsupported") ==
                "Los campos directos con respaldo DBF no pueden renombrarse por objeto." &&
            portuguese_catalog.translate("VisualAssetEditor.Operation.RollbackFailed") ==
                "{error} Falha na reversao: {rollbackError}" &&
            portuguese_catalog.translate("VisualAssetEditor.Property.NotRenameableMemo") ==
                "A propriedade solicitada nao e exposta como uma propriedade baseada em memo renomeavel neste ativo." &&
            pseudo_catalog.translate("VisualAssetEditor.Object.VisibilitySelectionDuplicate") ==
                copperfin::localization::pseudo_localize("The same visual object was selected more than once for visibility assignment."),
        "#2670: asset-editor object target and property labels should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.Property.SourceRelativeToSelf") ==
                "La propiedad de origen no puede posicionarse en relacion consigo misma." &&
            spanish_catalog.translate("VisualAssetEditor.Storage.MemoSidecarNextFreeBlockInvalid") ==
                "El puntero al siguiente bloque libre del sidecar memo no es valido." &&
            portuguese_catalog.translate("VisualAssetEditor.Property.NotWritableField") ==
                "A propriedade solicitada nao e exposta como um campo gravavel neste ativo." &&
            portuguese_catalog.translate("VisualAssetEditor.Storage.MemoSidecarPathMissing") ==
                "Nenhum caminho de sidecar memo pode ser inferido para o ativo." &&
            pseudo_catalog.translate("VisualAssetEditor.Property.SourceMoveToSelf") ==
                copperfin::localization::pseudo_localize("The source property cannot be moved onto itself."),
        "#2671: asset-editor property relative and storage validation labels should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.PropertyLabel.BufferModeOverride") ==
                "reemplazo del modo de bufer" &&
            spanish_catalog.translate("VisualAssetEditor.PropertyLabel.Closable") == "cerrable" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.AllowHeaderSizing") ==
                "permitir redimensionamento do cabecalho" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.BorderWidth") == "largura da borda" &&
            pseudo_catalog.translate("VisualAssetEditor.PropertyLabel.AutoVerbMenu") ==
                copperfin::localization::pseudo_localize("auto-verb-menu"),
        "#2672: asset-editor property labels batch 1 should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.PropertyLabel.ColumnWidths") == "anchos de columna" &&
            spanish_catalog.translate("VisualAssetEditor.PropertyLabel.DisabledPicture") == "imagen deshabilitada" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.ControlSource") == "origem do controle" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.DrawMode") == "modo de desenho" &&
            pseudo_catalog.translate("VisualAssetEditor.PropertyLabel.DisplayOrientation") ==
                copperfin::localization::pseudo_localize("display orientation"),
        "#2673: asset-editor property labels batch 2 should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.PropertyLabel.DynamicFontSize") == "tamano de fuente dinamica" &&
            spanish_catalog.translate("VisualAssetEditor.PropertyLabel.FillStyle") == "estilo de relleno" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.DrawWidth") == "largura de desenho" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.FontShadow") == "sombra da fonte" &&
            pseudo_catalog.translate("VisualAssetEditor.PropertyLabel.DynamicForeColor") ==
                copperfin::localization::pseudo_localize("dynamic fore-color"),
        "#2674: asset-editor property labels batch 3 should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.PropertyLabel.HighlightStyle") == "estilo de resaltado" &&
            spanish_catalog.translate("VisualAssetEditor.PropertyLabel.KeyPreview") == "vista previa de teclas" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.GridLineWidth") == "largura da linha da grade" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.LinkMaster") == "vinculo mestre" &&
            pseudo_catalog.translate("VisualAssetEditor.PropertyLabel.HelpContextID") ==
                copperfin::localization::pseudo_localize("help-context ID"),
        "#2675: asset-editor property labels batch 4 should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.PropertyLabel.LockColumnsLeft") == "bloquear columnas a la izquierda" &&
            spanish_catalog.translate("VisualAssetEditor.PropertyLabel.OLEDragPicture") == "imagen de arrastre OLE" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.MaxWidth") == "largura maxima" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.PanelLink") == "vinculo de painel" &&
            pseudo_catalog.translate("VisualAssetEditor.PropertyLabel.ListItemID") ==
                copperfin::localization::pseudo_localize("list-item ID"),
        "#2676: asset-editor property labels batch 5 should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.PropertyLabel.RecordSourceType") == "tipo de origen del registro" &&
            spanish_catalog.translate("VisualAssetEditor.PropertyLabel.WhatsThisHelpID") == "ID de ayuda WhatsThis" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.ScrollBars") == "barras de rolagem" &&
            portuguese_catalog.translate("VisualAssetEditor.PropertyLabel.ToolTipText") == "texto da dica de ferramenta" &&
            pseudo_catalog.translate("VisualAssetEditor.PropertyLabel.WindowState") ==
                copperfin::localization::pseudo_localize("window-state"),
        "#2677: asset-editor property labels batch 6 should resolve through locale catalogs without changing machine contracts");
    expect(
        spanish_catalog.translate("VisualAssetEditor.Storage.TableWriteFailed") ==
                "No se pudo escribir la tabla del recurso visual." &&
            spanish_catalog.translate("VisualAssetEditor.Undo.PropertyLabel") == "Propiedad {propertyName}" &&
            portuguese_catalog.translate("VisualAssetEditor.Storage.TargetFieldMemoRequired") ==
                "O campo de destino nao e um campo baseado em memo." &&
            portuguese_catalog.translate("VisualAssetEditor.Undo.RenamePropertyLabel") ==
                "Renomear propriedade {propertyName}" &&
            pseudo_catalog.translate("VisualAssetEditor.Undo.HistoryUnavailable") ==
                copperfin::localization::pseudo_localize("No visual asset undo history is available."),
        "#2678: asset-editor storage and undo labels should resolve through locale catalogs without changing machine contracts");
    expect(
        pseudo_catalog.translate("StudioHost.BuilderParse.Error.NoBuilderLaunchCommand") ==
                copperfin::localization::pseudo_localize("No builder launch command was provided.") &&
            pseudo_catalog.translate("StudioHost.BuilderParse.Error.UnknownOption") ==
                copperfin::localization::pseudo_localize("Unknown {commandName} option: {argument}"),
        "#2679: qps-ploc builder-parse diagnostics should pseudo-localize the remaining builder parse error prose without changing placeholders");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_command_keys) == 0U,
        "#2628: es-419 should define every remaining StudioHost.LaunchParse.ObjectCommand localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_command_keys) == 0U,
        "#2628: pt-BR should define every remaining StudioHost.LaunchParse.ObjectCommand localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_command_keys) == 0U,
        "#2628: qps-ploc should define every remaining StudioHost.LaunchParse.ObjectCommand localization key");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_action_keys) == 0U,
        "#2629: es-419 should define every remaining StudioHost.LaunchParse.ObjectAction localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_action_keys) == 0U,
        "#2629: pt-BR should define every remaining StudioHost.LaunchParse.ObjectAction localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_action_keys) == 0U,
        "#2629: qps-ploc should define every remaining StudioHost.LaunchParse.ObjectAction localization key");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_layout_window_keys) == 0U,
        "#2654: es-419 should define every remaining layout/window object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_layout_window_keys) == 0U,
        "#2654: pt-BR should define every remaining layout/window object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_layout_window_keys) == 0U,
        "#2654: qps-ploc should define every remaining layout/window object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_control_display_keys) == 0U,
        "#2655: es-419 should define every remaining control/display object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_control_display_keys) == 0U,
        "#2655: pt-BR should define every remaining control/display object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_control_display_keys) == 0U,
        "#2655: qps-ploc should define every remaining control/display object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_drag_dynamic_keys) == 0U,
        "#2656: es-419 should define every remaining drag/dynamic object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_drag_dynamic_keys) == 0U,
        "#2656: pt-BR should define every remaining drag/dynamic object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_drag_dynamic_keys) == 0U,
        "#2656: qps-ploc should define every remaining drag/dynamic object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_font_fill_keys) == 0U,
        "#2657: es-419 should define every remaining font/fill object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_font_fill_keys) == 0U,
        "#2657: pt-BR should define every remaining font/fill object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_font_fill_keys) == 0U,
        "#2657: qps-ploc should define every remaining font/fill object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_base_layout_keys) == 0U,
        "#2658: es-419 should define every remaining base layout/column object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_base_layout_keys) == 0U,
        "#2658: pt-BR should define every remaining base layout/column object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_base_layout_keys) == 0U,
        "#2658: qps-ploc should define every remaining base layout/column object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_selection_keys) == 0U,
        "#2659: es-419 should define every remaining selection/highlight object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_selection_keys) == 0U,
        "#2659: pt-BR should define every remaining selection/highlight object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_selection_keys) == 0U,
        "#2659: qps-ploc should define every remaining selection/highlight object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_lock_window_keys) == 0U,
        "#2660: es-419 should define every remaining lock/window object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_lock_window_keys) == 0U,
        "#2660: pt-BR should define every remaining lock/window object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_lock_window_keys) == 0U,
        "#2660: qps-ploc should define every remaining lock/window object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_picture_record_keys) == 0U,
        "#2661: es-419 should define every remaining picture/record object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_picture_record_keys) == 0U,
        "#2661: pt-BR should define every remaining picture/record object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_picture_record_keys) == 0U,
        "#2661: qps-ploc should define every remaining picture/record object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", object_assignment_selection_state_tab_keys) == 0U,
        "#2662: es-419 should define every remaining selection-state/tab object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", object_assignment_selection_state_tab_keys) == 0U,
        "#2662: pt-BR should define every remaining selection-state/tab object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", object_assignment_selection_state_tab_keys) == 0U,
        "#2662: qps-ploc should define every remaining selection-state/tab object-assignment localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", host_tail_parse_keys) == 0U,
        "#2663: es-419 should define every remaining host tail parse localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", host_tail_parse_keys) == 0U,
        "#2663: pt-BR should define every remaining host tail parse localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", host_tail_parse_keys) == 0U,
        "#2663: qps-ploc should define every remaining host tail parse localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", visual_method_object_error_keys) == 0U,
        "#2664: es-419 should define every remaining visual-method/object parse localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", visual_method_object_error_keys) == 0U,
        "#2664: pt-BR should define every remaining visual-method/object parse localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", visual_method_object_error_keys) == 0U,
        "#2664: qps-ploc should define every remaining visual-method/object parse localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", visual_object_property_error_keys) == 0U,
        "#2665: es-419 should define every remaining visual-object/property parse localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", visual_object_property_error_keys) == 0U,
        "#2665: pt-BR should define every remaining visual-object/property parse localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", visual_object_property_error_keys) == 0U,
        "#2665: qps-ploc should define every remaining visual-object/property parse localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", visual_property_asset_editor_keys) == 0U,
        "#2666: es-419 should define every remaining visual-property/asset-editor localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", visual_property_asset_editor_keys) == 0U,
        "#2666: pt-BR should define every remaining visual-property/asset-editor localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", visual_property_asset_editor_keys) == 0U,
        "#2666: qps-ploc should define every remaining visual-property/asset-editor localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_method_object_keys) == 0U,
        "#2667: es-419 should define every remaining asset-editor method/object localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_method_object_keys) == 0U,
        "#2667: pt-BR should define every remaining asset-editor method/object localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_method_object_keys) == 0U,
        "#2667: qps-ploc should define every remaining asset-editor method/object localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_object_batch_keys) == 0U,
        "#2668: es-419 should define every remaining asset-editor object batch/grouping localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_object_batch_keys) == 0U,
        "#2668: pt-BR should define every remaining asset-editor object batch/grouping localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_object_batch_keys) == 0U,
        "#2668: qps-ploc should define every remaining asset-editor object batch/grouping localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_object_selection_keys) == 0U,
        "#2669: es-419 should define every remaining asset-editor object placement/selection localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_object_selection_keys) == 0U,
        "#2669: pt-BR should define every remaining asset-editor object placement/selection localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_object_selection_keys) == 0U,
        "#2669: qps-ploc should define every remaining asset-editor object placement/selection localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_object_property_keys) == 0U,
        "#2670: es-419 should define every remaining asset-editor object target/property localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_object_property_keys) == 0U,
        "#2670: pt-BR should define every remaining asset-editor object target/property localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_object_property_keys) == 0U,
        "#2670: qps-ploc should define every remaining asset-editor object target/property localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_property_storage_keys) == 0U,
        "#2671: es-419 should define every remaining asset-editor property/storage localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_property_storage_keys) == 0U,
        "#2671: pt-BR should define every remaining asset-editor property/storage localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_property_storage_keys) == 0U,
        "#2671: qps-ploc should define every remaining asset-editor property/storage localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_property_label_batch1_keys) == 0U,
        "#2672: es-419 should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_property_label_batch1_keys) == 0U,
        "#2672: pt-BR should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_property_label_batch1_keys) == 0U,
        "#2672: qps-ploc should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_property_label_batch2_keys) == 0U,
        "#2673: es-419 should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_property_label_batch2_keys) == 0U,
        "#2673: pt-BR should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_property_label_batch2_keys) == 0U,
        "#2673: qps-ploc should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_property_label_batch3_keys) == 0U,
        "#2674: es-419 should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_property_label_batch3_keys) == 0U,
        "#2674: pt-BR should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_property_label_batch3_keys) == 0U,
        "#2674: qps-ploc should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_property_label_batch4_keys) == 0U,
        "#2675: es-419 should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_property_label_batch4_keys) == 0U,
        "#2675: pt-BR should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_property_label_batch4_keys) == 0U,
        "#2675: qps-ploc should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_property_label_batch5_keys) == 0U,
        "#2676: es-419 should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_property_label_batch5_keys) == 0U,
        "#2676: pt-BR should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_property_label_batch5_keys) == 0U,
        "#2676: qps-ploc should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_property_label_batch6_keys) == 0U,
        "#2677: es-419 should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_property_label_batch6_keys) == 0U,
        "#2677: pt-BR should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_property_label_batch6_keys) == 0U,
        "#2677: qps-ploc should define every remaining asset-editor property-label localization key in this slice");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", asset_editor_storage_undo_keys) == 0U,
        "#2678: es-419 should define every remaining asset-editor storage/undo localization key in this slice");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", asset_editor_storage_undo_keys) == 0U,
        "#2678: pt-BR should define every remaining asset-editor storage/undo localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", asset_editor_storage_undo_keys) == 0U,
        "#2678: qps-ploc should define every remaining asset-editor storage/undo localization key in this slice");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", pseudo_builder_parse_error_keys) == 0U,
        "#2679: qps-ploc should define every remaining builder-parse error localization key in this slice");
    expect_not_contains(process.stdout_text,
        "Usage: copperfin_studio_host --path <asset>",
        "#2576: pseudo-localized studio host usage should not fall back to raw English primary usage prose");
    expect_not_contains(process.stdout_text,
        "   or: copperfin_studio_host --list-subsystems [--json]",
        "#2576: pseudo-localized studio host usage should not fall back to raw English alternate usage prose");
    expect_not_contains(process.stdout_text,
        "Selection context tokens: visual_object, visual_method, container_object, class_designer, report_expression, label_expression, menu_item, project_item, data_environment",
        "#2576: pseudo-localized studio host usage should not fall back to raw English selection-context token prose");
    expect_not_contains(process.stdout_text,
        "Selected-back-color object:",
        "#2570: pseudo-localized studio host usage should not fall back to the raw English selected-back-color label");
    expect_not_contains(process.stdout_text,
        "Dynamic-fore-color object:",
        "#2570: pseudo-localized studio host usage should not fall back to the raw English dynamic-fore-color label");
    expect_not_contains(process.stdout_text,
        "Record-source-type object:",
        "#2575: pseudo-localized studio host usage should not fall back to the raw English record-source-type label");
    expect_not_contains(process.stdout_text,
        "Fill-color object:",
        "#2575: pseudo-localized studio host usage should not fall back to the raw English fill-color label");
    expect_not_contains(process.stdout_text,
        "Record-source object:",
        "#2575: pseudo-localized studio host usage should not fall back to the raw English record-source label");
    expect_not_contains(process.stdout_text,
        "Desktop object:",
        "#2571: pseudo-localized studio host usage should not fall back to the raw English desktop label");
    expect_not_contains(process.stdout_text,
        "Picture-selection-display object:",
        "#2571: pseudo-localized studio host usage should not fall back to the raw English picture-selection-display label");
    expect_not_contains(process.stdout_text,
        "Dynamic-input-mask object:",
        "#2572: pseudo-localized studio host usage should not fall back to the raw English dynamic-input-mask label");
    expect_not_contains(process.stdout_text,
        "Font-name object:",
        "#2572: pseudo-localized studio host usage should not fall back to the raw English font-name label");
    expect_not_contains(process.stdout_text,
        "Max-top object:",
        "#2572: pseudo-localized studio host usage should not fall back to the raw English max-top label");
    expect_not_contains(process.stdout_text,
        "Button-count object:",
        "#2573: pseudo-localized studio host usage should not fall back to the raw English button-count label");
    expect_not_contains(process.stdout_text,
        "Buffer-mode-override object:",
        "#2573: pseudo-localized studio host usage should not fall back to the raw English buffer-mode-override label");
    expect_not_contains(process.stdout_text,
        "Header-height object:",
        "#2573: pseudo-localized studio host usage should not fall back to the raw English header-height label");
    expect_not_contains(process.stdout_text,
        "Row-height object:",
        "#2574: pseudo-localized studio host usage should not fall back to the raw English row-height label");
    expect_not_contains(process.stdout_text,
        "Grid-line-width object:",
        "#2574: pseudo-localized studio host usage should not fall back to the raw English grid-line-width label");
    expect_not_contains(process.stdout_text,
        "Partition object:",
        "#2574: pseudo-localized studio host usage should not fall back to the raw English partition label");
    expect_not_contains(process.stdout_text,
        "OLE drop-mode object:",
        "#2569: pseudo-localized studio host usage should not fall back to the raw English OLE drop-mode label");
    expect_not_contains(process.stdout_text,
        "WhatsThis help ID object:",
        "#2569: pseudo-localized studio host usage should not fall back to the raw English WhatsThis help ID label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_list_subsystems_localizes_descriptor_text(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_list_subsystems_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(studio_host_path, {"--list-subsystems"}, temp_root);

    expect(process.exit_code == 0,
        "#2395: list-subsystems text output should succeed");
    expect_contains(process.stdout_text,
        "title: Report Designer",
        "#2395: default list-subsystems text output should preserve en-US titles");
    expect_contains(process.stdout_text,
        "current_status: implemented",
        "#2395: default list-subsystems text output should preserve invariant status values");
    expect_contains(process.stdout_text,
        "vfp9_equivalent: FRX/FRT designer, ReportBuilder.app, ReportPreview.app, ReportOutput.app",
        "#2395: default list-subsystems text output should preserve VFP-equivalent identifiers");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(studio_host_path, {"--list-subsystems", "--json"}, temp_root);

    expect(process.exit_code == 0,
        "#2395: pseudo-localized list-subsystems JSON output should succeed");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2395: pseudo-localized list-subsystems JSON should decorate descriptor prose");
    expect_contains(process.stdout_text,
        "\"id\": \"report-designer\"",
        "#2395: pseudo-localized list-subsystems JSON should preserve subsystem ids");
    expect_contains(process.stdout_text,
        "\"currentStatus\": \"implemented\"",
        "#2395: pseudo-localized list-subsystems JSON should preserve status values");
    expect_contains(process.stdout_text,
        "\"vfp9Equivalent\": \"FRX/FRT designer, ReportBuilder.app, ReportPreview.app, ReportOutput.app\"",
        "#2395: pseudo-localized list-subsystems JSON should preserve VFP-equivalent identifiers");
    expect_contains(process.stdout_text,
        "\"vfp9EquivalentDisplay\": \"[!! ",
        "#4246: pseudo-localized list-subsystems JSON should expose localized VFP-equivalent display text");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_builder_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_builder_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2396: default builder parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2396: default builder parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2396: default builder parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Unknown builder context token: unknown",
        "#2396: default builder parser diagnostics should preserve en-US prose");

    const std::string control_option = std::string("--unknown") + static_cast<char>(0x1f) + "value";
    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-catalog", "--builder-context", "control", "--json", control_option},
        temp_root);
    expect(process.exit_code == 2,
        "#4873: control-character unknown options should preserve parse-failure exit status");
    expect_contains(process.stdout_text, "--unknown\\u001fvalue",
        "#4873: Studio-host JSON should canonically escape control bytes");
    expect(process.stdout_text.find(static_cast<char>(0x1f)) == std::string::npos,
        "#4873: Studio-host JSON should not emit raw control bytes");

    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown"},
        temp_root);

    expect(process.exit_code == 2,
        "#2567: default text-mode builder diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "status: error",
        "#2567: default text-mode builder diagnostics should preserve machine-readable status");
    expect_contains(process.stdout_text,
        "error: Unknown builder context token: unknown",
        "#2567: default text-mode builder diagnostics should preserve the en-US prefixed error line");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2396: pseudo-localized unknown-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2396: pseudo-localized unknown-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2396: pseudo-localized unknown-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2396: pseudo-localized unknown-context diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2396: pseudo-localized unknown-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown builder context token: unknown",
        "#2396: pseudo-localized unknown-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown"},
        temp_root);

    const std::string pseudo_error_prefix =
        copperfin::localization::load_catalogs(
            copperfin::localization::resolve_catalog_root(),
            "qps-ploc").translate("StudioHost.Prefix.Error");
    expect(process.exit_code == 2,
        "#2567: pseudo-localized text-mode builder diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "status: error",
        "#2567: pseudo-localized text-mode builder diagnostics should preserve machine-readable status");
    expect_contains(process.stdout_text,
        pseudo_error_prefix,
        "#2567: pseudo-localized text-mode builder diagnostics should route the error prefix through localization");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2567: pseudo-localized text-mode builder diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2567: pseudo-localized text-mode builder diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "error: Unknown builder context token: unknown",
        "#2567: pseudo-localized text-mode builder diagnostics should not fall back to the raw English prefixed error");

    process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    const auto pseudo_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "qps-ploc");
    const std::string pseudo_launch_plan_request_name =
        pseudo_catalog.translate("StudioHost.BuilderParse.RequestName.LaunchPlan");
    expect(process.exit_code == 2,
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        pseudo_launch_plan_request_name,
        "#2568: pseudo-localized ambiguous builder-context diagnostics should route request labels through localization");
    expect_contains(process.stdout_text,
        "--builder-context",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "--selection-context",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Builder launch-plan requests cannot provide both --builder-context and --selection-context.",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should not fall back to raw English request labels");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2577: es-419 ambiguous builder-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2577: es-419 ambiguous builder-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2577: es-419 ambiguous builder-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Las solicitudes de plan de lanzamiento del builder no pueden proporcionar --builder-context y --selection-context al mismo tiempo.",
        "#2577: es-419 ambiguous builder-context diagnostics should localize builder request conflict prose");
    expect_not_contains(process.stdout_text,
        "Builder launch-plan requests cannot provide both --builder-context and --selection-context.",
        "#2577: es-419 ambiguous builder-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2577: pt-BR ambiguous builder-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2577: pt-BR ambiguous builder-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2577: pt-BR ambiguous builder-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Solicitacoes de plano de lancamento do builder nao podem fornecer --builder-context e --selection-context ao mesmo tempo.",
        "#2577: pt-BR ambiguous builder-context diagnostics should localize builder request conflict prose");
    expect_not_contains(process.stdout_text,
        "Builder launch-plan requests cannot provide both --builder-context and --selection-context.",
        "#2577: pt-BR ambiguous builder-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2578: es-419 unknown builder-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Token de contexto del builder desconocido: unknown",
        "#2578: es-419 unknown builder-context diagnostics should localize builder-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown builder context token: unknown",
        "#2578: es-419 unknown builder-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "form-builder", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2578: pt-BR missing-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Nenhum contexto do builder ou de selecao foi fornecido.",
        "#2578: pt-BR missing-context diagnostics should localize missing builder-or-selection context prose");
    expect_not_contains(process.stdout_text,
        "No builder or selection context was provided.",
        "#2578: pt-BR missing-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2578: es-419 invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "El valor de --admit-ui-launch debe ser true o false.",
        "#2578: es-419 invalid-boolean diagnostics should localize boolean validation prose");
    expect_contains(process.stdout_text,
        "true",
        "#2578: es-419 invalid-boolean diagnostics should preserve invariant true tokens");
    expect_contains(process.stdout_text,
        "false",
        "#2578: es-419 invalid-boolean diagnostics should preserve invariant false tokens");
    expect_not_contains(process.stdout_text,
        "The --admit-ui-launch value must be true or false.",
        "#2578: es-419 invalid-boolean diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2396: pseudo-localized invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2396: pseudo-localized invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--admit-ui-launch",
        "#2396: pseudo-localized invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2396: pseudo-localized invalid-boolean diagnostics should preserve true boolean values");
    expect_contains(process.stdout_text,
        "false",
        "#2396: pseudo-localized invalid-boolean diagnostics should preserve false boolean values");
    expect_not_contains(process.stdout_text,
        "The --admit-ui-launch value must be true or false.",
        "#2396: pseudo-localized invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-catalog", "--builder-context", "control", "--admit-ui-launch", "true", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2396: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2396: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "builder-launch-catalog",
        "#2396: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--admit-ui-launch",
        "#2396: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown builder-launch-catalog option: --admit-ui-launch",
        "#2396: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_editor_action_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--editor-action-launch-plan", "show-property-grid", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2397: default editor-action parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2397: default editor-action parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"editorActionLaunchPlan\": null",
        "#2397: default editor-action parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2397: default editor-action parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--editor-action-launch-plan", "show-property-grid", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2397: pseudo-localized unknown-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2397: pseudo-localized unknown-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"editorActionLaunchPlan\": null",
        "#2397: pseudo-localized unknown-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2397: pseudo-localized unknown-context diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2397: pseudo-localized unknown-context diagnostics should preserve selection-context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2397: pseudo-localized unknown-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2397: pseudo-localized invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2397: pseudo-localized invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--admit-editor-invocation",
        "#2397: pseudo-localized invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2397: pseudo-localized invalid-boolean diagnostics should preserve true boolean values");
    expect_contains(process.stdout_text,
        "false",
        "#2397: pseudo-localized invalid-boolean diagnostics should preserve false boolean values");
    expect_not_contains(process.stdout_text,
        "The --admit-editor-invocation value must be true or false.",
        "#2397: pseudo-localized invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2397: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2397: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "editor-action-launch-catalog",
        "#2397: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--admit-editor-invocation",
        "#2397: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown editor-action-launch-catalog option: --admit-editor-invocation",
        "#2397: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--editor-action-launch-plan", "show-property-grid", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2580: es-419 unknown-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2580: es-419 unknown-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"editorActionLaunchPlan\": null",
        "#2580: es-419 unknown-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Token de contexto de seleccion desconocido: unknown",
        "#2580: es-419 unknown-context diagnostics should localize selection-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2580: es-419 unknown-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2580: pt-BR invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "O valor de --admit-editor-invocation deve ser true ou false.",
        "#2580: pt-BR invalid-boolean diagnostics should localize boolean validation prose");
    expect_contains(process.stdout_text,
        "true",
        "#2580: pt-BR invalid-boolean diagnostics should preserve invariant true tokens");
    expect_contains(process.stdout_text,
        "false",
        "#2580: pt-BR invalid-boolean diagnostics should preserve invariant false tokens");
    expect_not_contains(process.stdout_text,
        "The --admit-editor-invocation value must be true or false.",
        "#2580: pt-BR invalid-boolean diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2580: es-419 unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Opcion desconocida de editor-action-launch-catalog: --admit-editor-invocation",
        "#2580: es-419 unknown-option diagnostics should localize command-option prose");
    expect_not_contains(process.stdout_text,
        "Unknown editor-action-launch-catalog option: --admit-editor-invocation",
        "#2580: es-419 unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_designer_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surfaces", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2400: default designer parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2400: default designer parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"designerLaunchSurfaces\": null",
        "#2400: default designer parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2400: default designer parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surfaces", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2400: pseudo-localized unknown-selection diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2400: pseudo-localized unknown-selection diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"designerLaunchSurfaces\": null",
        "#2400: pseudo-localized unknown-selection diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2400: pseudo-localized unknown-selection diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2400: pseudo-localized unknown-selection diagnostics should preserve selection-context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2400: pseudo-localized unknown-selection diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2400: pseudo-localized invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2400: pseudo-localized invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--admit-editor-invocations",
        "#2400: pseudo-localized invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2400: pseudo-localized invalid-boolean diagnostics should preserve true boolean values");
    expect_contains(process.stdout_text,
        "false",
        "#2400: pseudo-localized invalid-boolean diagnostics should preserve false boolean values");
    expect_not_contains(process.stdout_text,
        "The --admit-editor-invocations value must be true or false.",
        "#2400: pseudo-localized invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surface-catalog", "--selection-context", "visual_object", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2400: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2400: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "designer-launch-surface-catalog",
        "#2400: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--selection-context",
        "#2400: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown designer-launch-surface-catalog option: --selection-context",
        "#2400: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2400: pseudo-localized missing-launch-command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2400: pseudo-localized missing-launch-command diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No designer editor action launch command was provided.",
        "#2400: pseudo-localized missing-launch-command diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surfaces", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2581: es-419 unknown-selection diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2581: es-419 unknown-selection diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"designerLaunchSurfaces\": null",
        "#2581: es-419 unknown-selection diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Token de contexto de seleccion desconocido: unknown",
        "#2581: es-419 unknown-selection diagnostics should localize selection-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2581: es-419 unknown-selection diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2581: pt-BR invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "O valor de --admit-editor-invocations deve ser true ou false.",
        "#2581: pt-BR invalid-boolean diagnostics should localize boolean validation prose");
    expect_contains(process.stdout_text,
        "true",
        "#2581: pt-BR invalid-boolean diagnostics should preserve invariant true tokens");
    expect_contains(process.stdout_text,
        "false",
        "#2581: pt-BR invalid-boolean diagnostics should preserve invariant false tokens");
    expect_not_contains(process.stdout_text,
        "The --admit-editor-invocations value must be true or false.",
        "#2581: pt-BR invalid-boolean diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surface-catalog", "--selection-context", "visual_object", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2581: es-419 unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Opcion desconocida de designer-launch-surface-catalog: --selection-context",
        "#2581: es-419 unknown-option diagnostics should localize command-option prose");
    expect_not_contains(process.stdout_text,
        "Unknown designer-launch-surface-catalog option: --selection-context",
        "#2581: es-419 unknown-option diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2581: pt-BR missing-launch-command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Nenhum comando de lancamento da acao do editor do designer foi fornecido.",
        "#2581: pt-BR missing-launch-command diagnostics should localize designer editor-action launch-command prose");
    expect_not_contains(process.stdout_text,
        "No designer editor action launch command was provided.",
        "#2581: pt-BR missing-launch-command diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_execution_fallback_errors_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_execution_fallback_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    auto process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "true",
            "--builder-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(process.exit_code == 4,
        "#2560: pseudo-localized builder execution failures should preserve nonzero exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2560: pseudo-localized builder execution failures should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "\"observedExitCode\": 1",
        "#2560: pseudo-localized builder execution failures should preserve child exit codes");
    expect_not_contains(process.stdout_text,
        "Builder launch command returned a non-zero exit code.",
        "#2560: pseudo-localized builder execution failures should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(process.exit_code == 4,
        "#2560: pseudo-localized editor-action execution failures should preserve nonzero exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2560: pseudo-localized editor-action execution failures should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "\"observedExitCode\": 1",
        "#2560: pseudo-localized editor-action execution failures should preserve child exit codes");
    expect_not_contains(process.stdout_text,
        "Editor action launch command returned a non-zero exit code.",
        "#2560: pseudo-localized editor-action execution failures should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--toolbox-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(process.exit_code == 4,
        "#2560: pseudo-localized toolbox execution failures should preserve nonzero exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2560: pseudo-localized toolbox execution failures should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "\"observedExitCode\": 1",
        "#2560: pseudo-localized toolbox execution failures should preserve child exit codes");
    expect_not_contains(process.stdout_text,
        "Toolbox launch command returned a non-zero exit code.",
        "#2560: pseudo-localized toolbox execution failures should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(process.exit_code == 4,
        "#2560: pseudo-localized designer execution failures should preserve nonzero exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2560: pseudo-localized designer execution failures should decorate aggregate and child prose");
    expect_contains(process.stdout_text,
        "\"error\": \"[!! ",
        "#2560: pseudo-localized designer execution failures should localize the aggregate error field");
    expect_contains(process.stdout_text,
        "\"executionBlockedErrors\": [\"[!! ",
        "#2560: pseudo-localized designer execution failures should localize blocked child errors");
    expect_not_contains(process.stdout_text,
        "Designer builder launch command returned a non-zero exit code.",
        "#2560: pseudo-localized designer execution failures should not fall back to raw English child prose");
    expect_not_contains(process.stdout_text,
        "One or more designer child executions failed.",
        "#2560: pseudo-localized designer execution failures should not fall back to the raw English aggregate error");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
