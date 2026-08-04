// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "visual_asset_editor_support.h"

namespace copperfin::vfp {
VisualAssetEditResult set_visual_object_picture(const VisualObjectPictureRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "Picture",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Picture"),
        request.picture);
}

VisualAssetEditResult set_visual_object_down_picture(const VisualObjectDownPictureRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DownPicture",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DownPicture"),
        request.down_picture);
}

VisualAssetEditResult set_visual_object_disabled_picture(const VisualObjectDisabledPictureRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DisabledPicture",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DisabledPicture"),
        request.disabled_picture);
}

VisualAssetEditResult set_visual_object_ole_drag_picture(const VisualObjectOleDragPictureRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "OLEDragPicture",
        visual_asset_text("VisualAssetEditor.PropertyLabel.OLEDragPicture"),
        request.ole_drag_picture);
}

VisualAssetEditResult set_visual_object_mouse_icon(const VisualObjectMouseIconRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "MouseIcon",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MouseIcon"),
        request.mouse_icon);
}

VisualAssetEditResult set_visual_object_drag_icon(const VisualObjectDragIconRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DragIcon",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DragIcon"),
        request.drag_icon);
}

VisualAssetEditResult set_visual_object_drag_mode(const VisualObjectDragModeRequest& request) {
    if (request.drag_mode < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DragMode")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DragMode",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DragMode"),
        std::to_string(request.drag_mode));
}

VisualAssetEditResult set_visual_object_ole_drag_mode(const VisualObjectOleDragModeRequest& request) {
    if (request.ole_drag_mode < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("OLEDragMode")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "OLEDragMode",
        visual_asset_text("VisualAssetEditor.PropertyLabel.OLEDragMode"),
        std::to_string(request.ole_drag_mode));
}

VisualAssetEditResult set_visual_object_ole_drop_mode(const VisualObjectOleDropModeRequest& request) {
    if (request.ole_drop_mode < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("OLEDropMode")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "OLEDropMode",
        visual_asset_text("VisualAssetEditor.PropertyLabel.OLEDropMode"),
        std::to_string(request.ole_drop_mode));
}

VisualAssetEditResult set_visual_object_ole_drop_effects(const VisualObjectOleDropEffectsRequest& request) {
    if (request.ole_drop_effects < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("OLEDropEffects")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "OLEDropEffects",
        visual_asset_text("VisualAssetEditor.PropertyLabel.OLEDropEffects"),
        std::to_string(request.ole_drop_effects));
}

VisualAssetEditResult set_visual_object_ole_drop_text_insertion(
    const VisualObjectOleDropTextInsertionRequest& request) {
    if (request.ole_drop_text_insertion < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("OLEDropTextInsertion")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "OLEDropTextInsertion",
        visual_asset_text("VisualAssetEditor.PropertyLabel.OLEDropTextInsertion"),
        std::to_string(request.ole_drop_text_insertion));
}

VisualAssetEditResult set_visual_object_back_style(const VisualObjectBackStyleRequest& request) {
    if (request.back_style < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("BackStyle")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BackStyle",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BackStyle"),
        std::to_string(request.back_style));
}

VisualAssetEditResult set_visual_object_border_style(const VisualObjectBorderStyleRequest& request) {
    if (request.border_style < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("BorderStyle")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BorderStyle",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BorderStyle"),
        std::to_string(request.border_style));
}

VisualAssetEditResult set_visual_object_border_width(const VisualObjectBorderWidthRequest& request) {
    if (request.border_width < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("BorderWidth")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BorderWidth",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BorderWidth"),
        std::to_string(request.border_width));
}

VisualAssetEditResult set_visual_object_border_color(const VisualObjectBorderColorRequest& request) {
    if (request.border_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("BorderColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BorderColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BorderColor"),
        std::to_string(request.border_color));
}

VisualAssetEditResult set_visual_object_grid_line_color(const VisualObjectGridLineColorRequest& request) {
    if (request.grid_line_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("GridLineColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "GridLineColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.GridLineColor"),
        std::to_string(request.grid_line_color));
}

VisualAssetEditResult set_visual_object_grid_line_width(const VisualObjectGridLineWidthRequest& request) {
    if (request.grid_line_width < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("GridLineWidth")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "GridLineWidth",
        visual_asset_text("VisualAssetEditor.PropertyLabel.GridLineWidth"),
        std::to_string(request.grid_line_width));
}

VisualAssetEditResult set_visual_object_grid_lines(const VisualObjectGridLinesRequest& request) {
    if (request.grid_lines < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("GridLines")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "GridLines",
        visual_asset_text("VisualAssetEditor.PropertyLabel.GridLines"),
        std::to_string(request.grid_lines));
}

VisualAssetEditResult set_visual_object_highlight_row_line_width(
    const VisualObjectHighlightRowLineWidthRequest& request) {
    if (request.highlight_row_line_width < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("HighlightRowLineWidth")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HighlightRowLineWidth",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HighlightRowLineWidth"),
        std::to_string(request.highlight_row_line_width));
}

VisualAssetEditResult set_visual_object_highlight_style(const VisualObjectHighlightStyleRequest& request) {
    if (request.highlight_style < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("HighlightStyle")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HighlightStyle",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HighlightStyle"),
        std::to_string(request.highlight_style));
}

VisualAssetEditResult set_visual_object_header_height(const VisualObjectHeaderHeightRequest& request) {
    if (request.header_height < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("HeaderHeight")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HeaderHeight",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HeaderHeight"),
        std::to_string(request.header_height));
}

VisualAssetEditResult set_visual_object_row_height(const VisualObjectRowHeightRequest& request) {
    if (request.row_height < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("RowHeight")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "RowHeight",
        visual_asset_text("VisualAssetEditor.PropertyLabel.RowHeight"),
        std::to_string(request.row_height));
}

VisualAssetEditResult set_visual_object_special_effect(const VisualObjectSpecialEffectRequest& request) {
    if (request.special_effect < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("SpecialEffect")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SpecialEffect",
        visual_asset_text("VisualAssetEditor.PropertyLabel.SpecialEffect"),
        std::to_string(request.special_effect));
}

VisualAssetEditResult set_visual_object_curvature(const VisualObjectCurvatureRequest& request) {
    if (request.curvature < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("Curvature")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Curvature",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Curvature"),
        std::to_string(request.curvature));
}

VisualAssetEditResult set_visual_object_draw_mode(const VisualObjectDrawModeRequest& request) {
    if (request.draw_mode < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DrawMode")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DrawMode",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DrawMode"),
        std::to_string(request.draw_mode));
}

VisualAssetEditResult set_visual_object_draw_style(const VisualObjectDrawStyleRequest& request) {
    if (request.draw_style < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DrawStyle")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DrawStyle",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DrawStyle"),
        std::to_string(request.draw_style));
}

VisualAssetEditResult set_visual_object_draw_width(const VisualObjectDrawWidthRequest& request) {
    if (request.draw_width < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DrawWidth")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DrawWidth",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DrawWidth"),
        std::to_string(request.draw_width));
}

VisualAssetEditResult set_visual_object_fill_color(const VisualObjectFillColorRequest& request) {
    if (request.fill_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("FillColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FillColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FillColor"),
        std::to_string(request.fill_color));
}

VisualAssetEditResult set_visual_object_fill_style(const VisualObjectFillStyleRequest& request) {
    if (request.fill_style < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("FillStyle")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FillStyle",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FillStyle"),
        std::to_string(request.fill_style));
}

VisualAssetEditResult set_visual_object_buffer_mode(const VisualObjectBufferModeRequest& request) {
    if (request.buffer_mode < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("BufferMode")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BufferMode",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BufferMode"),
        std::to_string(request.buffer_mode));
}

VisualAssetEditResult set_visual_object_buffer_mode_override(
    const VisualObjectBufferModeOverrideRequest& request) {
    if (request.buffer_mode_override < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("BufferModeOverride")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BufferModeOverride",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BufferModeOverride"),
        std::to_string(request.buffer_mode_override));
}

VisualAssetEditResult set_visual_object_scale_mode(const VisualObjectScaleModeRequest& request) {
    if (request.scale_mode < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ScaleMode")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ScaleMode",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ScaleMode"),
        std::to_string(request.scale_mode));
}

VisualAssetEditResult set_visual_object_scroll_bars(const VisualObjectScrollBarsRequest& request) {
    if (request.scroll_bars < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ScrollBars")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ScrollBars",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ScrollBars"),
        std::to_string(request.scroll_bars));
}

VisualAssetEditResult set_visual_object_window_state(const VisualObjectWindowStateRequest& request) {
    if (request.window_state < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("WindowState")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "WindowState",
        visual_asset_text("VisualAssetEditor.PropertyLabel.WindowState"),
        std::to_string(request.window_state));
}

VisualAssetEditResult set_visual_object_show_window(const VisualObjectShowWindowRequest& request) {
    if (request.show_window < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ShowWindow")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ShowWindow",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ShowWindow"),
        std::to_string(request.show_window));
}

VisualAssetEditResult set_visual_object_title_bar(const VisualObjectTitleBarRequest& request) {
    if (request.title_bar < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("TitleBar")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "TitleBar",
        visual_asset_text("VisualAssetEditor.PropertyLabel.TitleBar"),
        std::to_string(request.title_bar));
}

VisualAssetEditResult set_visual_object_mouse_pointer(const VisualObjectMousePointerRequest& request) {
    if (request.mouse_pointer < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("MousePointer")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MousePointer",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MousePointer"),
        std::to_string(request.mouse_pointer));
}

VisualAssetEditResult set_visual_object_picture_margin(const VisualObjectPictureMarginRequest& request) {
    if (request.picture_margin < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("PictureMargin")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PictureMargin",
        visual_asset_text("VisualAssetEditor.PropertyLabel.PictureMargin"),
        std::to_string(request.picture_margin));
}

VisualAssetEditResult set_visual_object_picture_position(const VisualObjectPicturePositionRequest& request) {
    if (request.picture_position < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("PicturePosition")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PicturePosition",
        visual_asset_text("VisualAssetEditor.PropertyLabel.PicturePosition"),
        std::to_string(request.picture_position));
}

VisualAssetEditResult set_visual_object_picture_spacing(const VisualObjectPictureSpacingRequest& request) {
    if (request.picture_spacing < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("PictureSpacing")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PictureSpacing",
        visual_asset_text("VisualAssetEditor.PropertyLabel.PictureSpacing"),
        std::to_string(request.picture_spacing));
}

VisualAssetEditResult set_visual_object_picture_selection_display(
    const VisualObjectPictureSelectionDisplayRequest& request) {
    if (request.picture_selection_display < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("PictureSelectionDisplay")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PictureSelectionDisplay",
        visual_asset_text("VisualAssetEditor.PropertyLabel.PictureSelectionDisplay"),
        std::to_string(request.picture_selection_display));
}

VisualAssetEditResult set_visual_object_input_mask(const VisualObjectInputMaskRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "InputMask",
        visual_asset_text("VisualAssetEditor.PropertyLabel.InputMask"),
        request.input_mask);
}

VisualAssetEditResult set_visual_object_dynamic_input_mask(const VisualObjectDynamicInputMaskRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicInputMask",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicInputMask"),
        request.dynamic_input_mask);
}

VisualAssetEditResult set_visual_object_dynamic_line_height(const VisualObjectDynamicLineHeightRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicLineHeight",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicLineHeight"),
        request.dynamic_line_height);
}

VisualAssetEditResult set_visual_object_format(const VisualObjectFormatRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "Format",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Format"),
        request.format);
}

VisualAssetEditResult set_visual_object_font_name(const VisualObjectFontNameRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "FontName",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FontName"),
        request.font_name);
}

VisualAssetEditResult set_visual_object_font_size(const VisualObjectFontSizeRequest& request) {
    if (!std::isfinite(request.font_size) || request.font_size < 0.0) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.FontSizeFiniteNonNegativeRequired")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontSize",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FontSize"),
        format_visual_geometry_number(request.font_size));
}

VisualAssetEditResult set_visual_object_font_bold(const VisualObjectFontBoldRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontBold",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FontBold"),
        request.font_bold ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_italic(const VisualObjectFontItalicRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontItalic",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FontItalic"),
        request.font_italic ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_underline(const VisualObjectFontUnderlineRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontUnderline",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FontUnderline"),
        request.font_underline ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_strikethru(const VisualObjectFontStrikethruRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontStrikethru",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FontStrikethru"),
        request.font_strikethru ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_outline(const VisualObjectFontOutlineRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontOutline",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FontOutline"),
        request.font_outline ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_shadow(const VisualObjectFontShadowRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontShadow",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FontShadow"),
        request.font_shadow ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_dynamic_alignment(const VisualObjectDynamicAlignmentRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicAlignment",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicAlignment"),
        request.dynamic_alignment);
}

VisualAssetEditResult set_visual_object_dynamic_current_control(
    const VisualObjectDynamicCurrentControlRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicCurrentControl",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicCurrentControl"),
        request.dynamic_current_control);
}

VisualAssetEditResult set_visual_object_dynamic_font_name(const VisualObjectDynamicFontNameRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontName",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicFontName"),
        request.dynamic_font_name);
}

VisualAssetEditResult set_visual_object_dynamic_font_size(const VisualObjectDynamicFontSizeRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontSize",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicFontSize"),
        request.dynamic_font_size);
}

VisualAssetEditResult set_visual_object_dynamic_font_bold(const VisualObjectDynamicFontBoldRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontBold",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicFontBold"),
        request.dynamic_font_bold);
}

VisualAssetEditResult set_visual_object_dynamic_font_italic(const VisualObjectDynamicFontItalicRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontItalic",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicFontItalic"),
        request.dynamic_font_italic);
}

VisualAssetEditResult set_visual_object_dynamic_font_underline(const VisualObjectDynamicFontUnderlineRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontUnderline",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicFontUnderline"),
        request.dynamic_font_underline);
}

VisualAssetEditResult set_visual_object_dynamic_font_strikethru(const VisualObjectDynamicFontStrikethruRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontStrikethru",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicFontStrikethru"),
        request.dynamic_font_strikethru);
}

VisualAssetEditResult set_visual_object_dynamic_font_outline(const VisualObjectDynamicFontOutlineRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontOutline",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicFontOutline"),
        request.dynamic_font_outline);
}

VisualAssetEditResult set_visual_object_dynamic_font_shadow(const VisualObjectDynamicFontShadowRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontShadow",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicFontShadow"),
        request.dynamic_font_shadow);
}

VisualAssetEditResult set_visual_object_selected_back_color(const VisualObjectSelectedBackColorRequest& request) {
    if (request.selected_back_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("SelectedBackColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SelectedBackColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.SelectedBackColor"),
        std::to_string(request.selected_back_color));
}

VisualAssetEditResult set_visual_object_selected_fore_color(const VisualObjectSelectedForeColorRequest& request) {
    if (request.selected_fore_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("SelectedForeColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SelectedForeColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.SelectedForeColor"),
        std::to_string(request.selected_fore_color));
}

VisualAssetEditResult set_visual_object_selected_item_back_color(
    const VisualObjectSelectedItemBackColorRequest& request) {
    if (request.selected_item_back_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("SelectedItemBackColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SelectedItemBackColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.SelectedItemBackColor"),
        std::to_string(request.selected_item_back_color));
}

VisualAssetEditResult set_visual_object_selected_item_fore_color(
    const VisualObjectSelectedItemForeColorRequest& request) {
    if (request.selected_item_fore_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("SelectedItemForeColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SelectedItemForeColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.SelectedItemForeColor"),
        std::to_string(request.selected_item_fore_color));
}

VisualAssetEditResult set_visual_object_disabled_item_back_color(
    const VisualObjectDisabledItemBackColorRequest& request) {
    if (request.disabled_item_back_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DisabledItemBackColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisabledItemBackColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DisabledItemBackColor"),
        std::to_string(request.disabled_item_back_color));
}

VisualAssetEditResult set_visual_object_disabled_item_fore_color(
    const VisualObjectDisabledItemForeColorRequest& request) {
    if (request.disabled_item_fore_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DisabledItemForeColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisabledItemForeColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DisabledItemForeColor"),
        std::to_string(request.disabled_item_fore_color));
}

VisualAssetEditResult set_visual_object_item_back_color(
    const VisualObjectItemBackColorRequest& request) {
    if (request.item_back_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ItemBackColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ItemBackColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ItemBackColor"),
        std::to_string(request.item_back_color));
}

VisualAssetEditResult set_visual_object_item_fore_color(
    const VisualObjectItemForeColorRequest& request) {
    if (request.item_fore_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ItemForeColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ItemForeColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ItemForeColor"),
        std::to_string(request.item_fore_color));
}

VisualAssetEditResult set_visual_object_highlight_back_color(
    const VisualObjectHighlightBackColorRequest& request) {
    if (request.highlight_back_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("HighlightBackColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HighlightBackColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HighlightBackColor"),
        std::to_string(request.highlight_back_color));
}

VisualAssetEditResult set_visual_object_highlight_fore_color(
    const VisualObjectHighlightForeColorRequest& request) {
    if (request.highlight_fore_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("HighlightForeColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HighlightForeColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HighlightForeColor"),
        std::to_string(request.highlight_fore_color));
}

VisualAssetEditResult set_visual_object_back_color(
    const VisualObjectBackColorRequest& request) {
    if (request.back_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("BackColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BackColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BackColor"),
        std::to_string(request.back_color));
}

VisualAssetEditResult set_visual_object_fore_color(
    const VisualObjectForeColorRequest& request) {
    if (request.fore_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ForeColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ForeColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ForeColor"),
        std::to_string(request.fore_color));
}

VisualAssetEditResult set_visual_object_disabled_back_color(
    const VisualObjectDisabledBackColorRequest& request) {
    if (request.disabled_back_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DisabledBackColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisabledBackColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DisabledBackColor"),
        std::to_string(request.disabled_back_color));
}

VisualAssetEditResult set_visual_object_disabled_fore_color(
    const VisualObjectDisabledForeColorRequest& request) {
    if (request.disabled_fore_color < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DisabledForeColor")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisabledForeColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DisabledForeColor"),
        std::to_string(request.disabled_fore_color));
}

VisualAssetEditResult set_visual_object_dynamic_back_color(
    const VisualObjectDynamicBackColorRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicBackColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicBackColor"),
        request.dynamic_back_color);
}

VisualAssetEditResult set_visual_object_dynamic_fore_color(
    const VisualObjectDynamicForeColorRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicForeColor",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DynamicForeColor"),
        request.dynamic_fore_color);
}

}  // namespace copperfin::vfp
