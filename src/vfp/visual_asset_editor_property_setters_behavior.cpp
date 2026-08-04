// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "visual_asset_editor_support.h"

namespace copperfin::vfp {
VisualAssetEditResult set_visual_object_tab_order(const VisualObjectTabOrderRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TabOrderSelectionRequired")};
    }
    if (request.starting_tab_index < 0) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.StartingTabIndexNonNegativeRequired")};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (std::size_t index = 0U; index < request.objects.size(); ++index) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = request.objects[index].record_index,
            .object_name = request.objects[index].object_name,
            .unique_id = request.objects[index].unique_id,
            .property_name = "TABINDEX"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedFieldMissing", {{"fieldName", "TABINDEX"}})};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TabOrderSelectionDuplicate")};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = request.objects[index].record_index,
            .object_name = request.objects[index].object_name,
            .unique_id = request.objects[index].unique_id,
            .properties = {
                {
                    .property_name = "TABINDEX",
                    .property_value = std::to_string(request.starting_tab_index + static_cast<int>(index))
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_tab_stop(const VisualObjectTabStopRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TabStopSelectionRequired")};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "TABSTOP"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedFieldOrPropertyMissing", {{"fieldName", "TABSTOP"}})};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TabStopSelectionDuplicate")};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "TABSTOP",
                    .property_value = request.tab_stop ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_visibility(const VisualObjectVisibilityRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.VisibilitySelectionRequired")};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "VISIBLE"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedFieldOrPropertyMissing", {{"fieldName", "VISIBLE"}})};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.VisibilitySelectionDuplicate")};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "VISIBLE",
                    .property_value = request.visible ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_enabled(const VisualObjectEnabledRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.EnabledSelectionRequired")};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "ENABLED"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedFieldOrPropertyMissing", {{"fieldName", "ENABLED"}})};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.EnabledSelectionDuplicate")};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "ENABLED",
                    .property_value = request.enabled ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_read_only(const VisualObjectReadOnlyRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ReadOnlySelectionRequired")};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "READONLY"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedFieldOrPropertyMissing", {{"fieldName", "READONLY"}})};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ReadOnlySelectionDuplicate")};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "READONLY",
                    .property_value = request.read_only ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_locked(const VisualObjectLockedRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.LockedSelectionRequired")};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "LOCKED"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedFieldOrPropertyMissing", {{"fieldName", "LOCKED"}})};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.LockedSelectionDuplicate")};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "LOCKED",
                    .property_value = request.locked ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_caption(const VisualObjectCaptionRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "Caption",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Caption"),
        request.caption);
}

VisualAssetEditResult set_visual_object_tooltip_text(const VisualObjectToolTipTextRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "ToolTipText",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ToolTipText"),
        request.tooltip_text);
}

VisualAssetEditResult set_visual_object_status_bar_text(const VisualObjectStatusBarTextRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "StatusBarText",
        visual_asset_text("VisualAssetEditor.PropertyLabel.StatusBarText"),
        request.status_bar_text);
}

VisualAssetEditResult set_visual_object_control_source(const VisualObjectControlSourceRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "ControlSource",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ControlSource"),
        request.control_source);
}

VisualAssetEditResult set_visual_object_current_control(const VisualObjectCurrentControlRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "CurrentControl",
        visual_asset_text("VisualAssetEditor.PropertyLabel.CurrentControl"),
        request.current_control);
}

VisualAssetEditResult set_visual_object_closable(const VisualObjectClosableRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Closable",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Closable"),
        request.closable ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_control_box(const VisualObjectControlBoxRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ControlBox",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ControlBox"),
        request.control_box ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_allow_output(const VisualObjectAllowOutputRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AllowOutput",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AllowOutput"),
        request.allow_output ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_auto_center(const VisualObjectAutoCenterRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AutoCenter",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AutoCenter"),
        request.auto_center ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_auto_size(const VisualObjectAutoSizeRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AutoSize",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AutoSize"),
        request.auto_size ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_auto_release(const VisualObjectAutoReleaseRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AutoRelease",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AutoRelease"),
        request.auto_release ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_auto_verb_menu(const VisualObjectAutoVerbMenuRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AutoVerbMenu",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AutoVerbMenu"),
        request.auto_verb_menu ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_bind_controls(const VisualObjectBindControlsRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BindControls",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BindControls"),
        request.bind_controls ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_clip_controls(const VisualObjectClipControlsRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ClipControls",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ClipControls"),
        request.clip_controls ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_dockable(const VisualObjectDockableRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Dockable",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Dockable"),
        request.dockable ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_continuous_scroll(const VisualObjectContinuousScrollRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ContinuousScroll",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ContinuousScroll"),
        request.continuous_scroll ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_desktop(const VisualObjectDesktopRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Desktop",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Desktop"),
        request.desktop ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_key_preview(const VisualObjectKeyPreviewRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "KeyPreview",
        visual_asset_text("VisualAssetEditor.PropertyLabel.KeyPreview"),
        request.key_preview ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_mac_desktop(const VisualObjectMacDesktopRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MacDesktop",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MacDesktop"),
        request.mac_desktop ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_max_button(const VisualObjectMaxButtonRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MaxButton",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MaxButton"),
        request.max_button ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_max_height(const VisualObjectMaxHeightRequest& request) {
    if (request.max_height < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("MaxHeight")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MaxHeight",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MaxHeight"),
        std::to_string(request.max_height));
}

VisualAssetEditResult set_visual_object_max_width(const VisualObjectMaxWidthRequest& request) {
    if (request.max_width < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("MaxWidth")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MaxWidth",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MaxWidth"),
        std::to_string(request.max_width));
}

VisualAssetEditResult set_visual_object_max_left(const VisualObjectMaxLeftRequest& request) {
    if (request.max_left < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("MaxLeft")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MaxLeft",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MaxLeft"),
        std::to_string(request.max_left));
}

VisualAssetEditResult set_visual_object_max_top(const VisualObjectMaxTopRequest& request) {
    if (request.max_top < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("MaxTop")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MaxTop",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MaxTop"),
        std::to_string(request.max_top));
}

VisualAssetEditResult set_visual_object_min_button(const VisualObjectMinButtonRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MinButton",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MinButton"),
        request.min_button ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_min_height(const VisualObjectMinHeightRequest& request) {
    if (request.min_height < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("MinHeight")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MinHeight",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MinHeight"),
        std::to_string(request.min_height));
}

VisualAssetEditResult set_visual_object_min_width(const VisualObjectMinWidthRequest& request) {
    if (request.min_width < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("MinWidth")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MinWidth",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MinWidth"),
        std::to_string(request.min_width));
}

VisualAssetEditResult set_visual_object_movable(const VisualObjectMovableRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Movable",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Movable"),
        request.movable ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_half_height_caption(
    const VisualObjectHalfHeightCaptionRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HalfHeightCaption",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HalfHeightCaption"),
        request.half_height_caption ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_mdi_form(const VisualObjectMdiFormRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MDIForm",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MDIForm"),
        request.mdi_form ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_whats_this_button(const VisualObjectWhatsThisButtonRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "WhatsThisButton",
        visual_asset_text("VisualAssetEditor.PropertyLabel.WhatsThisButton"),
        request.whats_this_button ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_whats_this_help(const VisualObjectWhatsThisHelpRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "WhatsThisHelp",
        visual_asset_text("VisualAssetEditor.PropertyLabel.WhatsThisHelp"),
        request.whats_this_help ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_whats_this_help_id(const VisualObjectWhatsThisHelpIdRequest& request) {
    if (request.whats_this_help_id < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("WhatsThisHelpID")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "WhatsThisHelpID",
        visual_asset_text("VisualAssetEditor.PropertyLabel.WhatsThisHelpID"),
        std::to_string(request.whats_this_help_id));
}

VisualAssetEditResult set_visual_object_help_context_id(const VisualObjectHelpContextIdRequest& request) {
    if (request.help_context_id < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("HelpContextID")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HelpContextID",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HelpContextID"),
        std::to_string(request.help_context_id));
}

VisualAssetEditResult set_visual_object_display_orientation(const VisualObjectDisplayOrientationRequest& request) {
    if (request.display_orientation < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DisplayOrientation")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisplayOrientation",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DisplayOrientation"),
        std::to_string(request.display_orientation));
}

VisualAssetEditResult set_visual_object_tab_orientation(const VisualObjectTabOrientationRequest& request) {
    if (request.tab_orientation < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("TabOrientation")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "TabOrientation",
        visual_asset_text("VisualAssetEditor.PropertyLabel.TabOrientation"),
        std::to_string(request.tab_orientation));
}

VisualAssetEditResult set_visual_object_list_item_id(const VisualObjectListItemIdRequest& request) {
    if (request.list_item_id < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ListItemID")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ListItemID",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ListItemID"),
        std::to_string(request.list_item_id));
}

VisualAssetEditResult set_visual_object_lock_screen(const VisualObjectLockScreenRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "LockScreen",
        visual_asset_text("VisualAssetEditor.PropertyLabel.LockScreen"),
        request.lock_screen ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_hide_selection(const VisualObjectHideSelectionRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HideSelection",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HideSelection"),
        request.hide_selection ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_allow_cell_selection(const VisualObjectAllowCellSelectionRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AllowCellSelection",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AllowCellSelection"),
        request.allow_cell_selection ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_delete_mark(const VisualObjectDeleteMarkRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DeleteMark",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DeleteMark"),
        request.delete_mark ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_record_mark(const VisualObjectRecordMarkRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "RecordMark",
        visual_asset_text("VisualAssetEditor.PropertyLabel.RecordMark"),
        request.record_mark ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_split_bar(const VisualObjectSplitBarRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SplitBar",
        visual_asset_text("VisualAssetEditor.PropertyLabel.SplitBar"),
        request.split_bar ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_highlight_row(const VisualObjectHighlightRowRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HighlightRow",
        visual_asset_text("VisualAssetEditor.PropertyLabel.HighlightRow"),
        request.highlight_row ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_panel_link(const VisualObjectPanelLinkRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PanelLink",
        visual_asset_text("VisualAssetEditor.PropertyLabel.PanelLink"),
        request.panel_link ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_allow_header_sizing(const VisualObjectAllowHeaderSizingRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AllowHeaderSizing",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AllowHeaderSizing"),
        request.allow_header_sizing ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_allow_row_sizing(const VisualObjectAllowRowSizingRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AllowRowSizing",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AllowRowSizing"),
        request.allow_row_sizing ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_resizable(const VisualObjectResizableRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Resizable",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Resizable"),
        request.resizable ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_sparse(const VisualObjectSparseRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Sparse",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Sparse"),
        request.sparse ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_add_line_feeds(const VisualObjectAddLineFeedsRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AddLineFeeds",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AddLineFeeds"),
        request.add_line_feeds ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_always_on_top(const VisualObjectAlwaysOnTopRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AlwaysOnTop",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AlwaysOnTop"),
        request.always_on_top ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_always_on_bottom(const VisualObjectAlwaysOnBottomRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AlwaysOnBottom",
        visual_asset_text("VisualAssetEditor.PropertyLabel.AlwaysOnBottom"),
        request.always_on_bottom ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_style(const VisualObjectStyleRequest& request) {
    if (request.style < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("Style")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Style",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Style"),
        std::to_string(request.style));
}

}  // namespace copperfin::vfp
