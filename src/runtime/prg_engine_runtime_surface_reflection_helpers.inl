// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

void sync_native_collection_count(RuntimeOleObjectState& runtime_object) {
    runtime_object.properties["count"] = make_number_value(static_cast<double>(runtime_object.collection_items.size()));
}

std::optional<std::size_t> resolve_native_collection_slot(
    const RuntimeOleObjectState& runtime_object,
    const PrgValue& selector) {
    switch (selector.kind) {
        case PrgValueKind::number: {
            const long long index = std::llround(selector.number_value);
            if (index >= 1LL && static_cast<std::size_t>(index) <= runtime_object.collection_items.size()) {
                return static_cast<std::size_t>(index - 1LL);
            }
            return std::nullopt;
        }
        case PrgValueKind::currency: {
            const long long index = std::llround(value_as_number(selector));
            if (index >= 1LL && static_cast<std::size_t>(index) <= runtime_object.collection_items.size()) {
                return static_cast<std::size_t>(index - 1LL);
            }
            return std::nullopt;
        }
        case PrgValueKind::int64:
            if (selector.int64_value >= 1LL &&
                static_cast<std::size_t>(selector.int64_value) <= runtime_object.collection_items.size()) {
                return static_cast<std::size_t>(selector.int64_value - 1LL);
            }
            return std::nullopt;
        case PrgValueKind::uint64:
            if (selector.uint64_value >= 1ULL &&
                static_cast<std::size_t>(selector.uint64_value) <= runtime_object.collection_items.size()) {
                return static_cast<std::size_t>(selector.uint64_value - 1ULL);
            }
            return std::nullopt;
        case PrgValueKind::boolean:
        case PrgValueKind::string:
        case PrgValueKind::empty:
            break;
    }

    const std::string key = normalize_identifier(trim_copy(value_as_string(selector)));
    if (key.empty()) {
        return std::nullopt;
    }

    const auto found = std::find(runtime_object.collection_item_keys.begin(),
                                 runtime_object.collection_item_keys.end(),
                                 key);
    if (found == runtime_object.collection_item_keys.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(std::distance(runtime_object.collection_item_keys.begin(), found));
}

std::optional<PrgValue> get_native_identity_reflection_metadata(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name == "hwnd" && runtime_object.native_hwnd.has_value()) {
        return make_int64_value(static_cast<std::int64_t>(*runtime_object.native_hwnd));
    }
    if (runtime_object.class_hierarchy.empty()) {
        return std::nullopt;
    }
    if (normalized_member_name == "class" && !trim_copy(runtime_object.prog_id).empty()) {
        return make_string_value(runtime_object.prog_id);
    }
    if (normalized_member_name == "baseclass" && !trim_copy(runtime_object.base_class_name).empty()) {
        return make_string_value(runtime_object.base_class_name);
    }
    if (normalized_member_name == "parentclass" && !trim_copy(runtime_object.base_class_name).empty()) {
        return make_string_value(runtime_object.base_class_name);
    }
    if (normalized_member_name == "classlibrary" && !trim_copy(runtime_object.class_library).empty()) {
        return make_string_value(runtime_object.class_library);
    }
    return std::nullopt;
}

bool native_identity_member_name_matches(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name) {
    if (normalized_member_name == "hwnd" && runtime_object.native_hwnd.has_value()) {
        return true;
    }
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }
    return normalized_member_name == "class" ||
           normalized_member_name == "baseclass" ||
           normalized_member_name == "parentclass" ||
           normalized_member_name == "classlibrary";
}

bool native_olecontrol_creation_time_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    if (!is_olecontrol) {
        return false;
    }
    return (normalized_member_name == "oleclass" ||
            normalized_member_name == "documentfile" ||
            normalized_member_name == "oletypeallowed") &&
           runtime_object.properties.contains(normalized_member_name);
}

bool native_olecontrol_object_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return is_olecontrol &&
           normalized_member_name == "object" &&
           runtime_object.properties.contains("object");
}

bool native_olecontrol_inspection_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    if (!is_olecontrol) {
        return false;
    }
    return (normalized_member_name == "objectverbscount" ||
            normalized_member_name == "objectverbs") &&
           (normalized_member_name == "objectverbs" ||
           runtime_object.properties.contains("objectverbscount"));
}

bool native_olecontrol_conflict_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return is_olecontrol &&
           normalized_member_name == "application" &&
           runtime_object.properties.contains("application");
}

bool native_child_parent_member_name_matches(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name) {
    if (normalized_member_name != "parent") {
        return false;
    }

    const auto parent = runtime_object.properties.find("parent");
    if (parent == runtime_object.properties.end()) {
        return false;
    }

    int handle = 0;
    std::string prog_id;
    return parse_object_handle_reference(parent->second, handle, prog_id);
}

bool native_controlcount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "controlcount" &&
           runtime_object.properties.contains("controlcount");
}

bool native_pagecount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "pagecount" ||
        !runtime_object.properties.contains("pagecount")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "pageframe";
}

bool native_activepage_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "activepage" ||
        !runtime_object.properties.contains("activepage")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "pageframe";
}

bool native_form_alwaysontop_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "alwaysontop") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("alwaysontop");
}

bool native_form_showwindow_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "showwindow") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("showwindow");
}

bool native_form_windowtype_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "windowtype") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("windowtype");
}

bool native_form_windowstate_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "windowstate") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("windowstate");
}

bool native_form_scalemode_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "scalemode") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("scalemode");
}

bool native_form_drawstyle_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "drawstyle") {
        return false;
    }

    return native_form_drawstyle_runtime_object(runtime_object) &&
           runtime_object.properties.contains("drawstyle");
}

bool native_visual_fillstyle_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fillstyle" &&
           native_visual_fillstyle_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fillstyle");
}

bool native_visual_specialeffect_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "specialeffect" &&
           native_visual_specialeffect_runtime_object(runtime_object) &&
           runtime_object.properties.contains("specialeffect");
}

bool native_visual_fillcolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fillcolor" &&
           native_visual_fillcolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fillcolor");
}

bool native_visual_borderwidth_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "borderwidth" &&
           native_visual_borderwidth_runtime_object(runtime_object) &&
           runtime_object.properties.contains("borderwidth");
}

bool native_visual_bordercolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "bordercolor" &&
           native_visual_bordercolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("bordercolor");
}

bool native_visual_borderstyle_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "borderstyle" &&
           native_visual_borderstyle_runtime_object(runtime_object) &&
           runtime_object.properties.contains("borderstyle");
}

bool native_form_drawwidth_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "drawwidth" &&
           native_form_drawwidth_runtime_object(runtime_object) &&
           runtime_object.properties.contains("drawwidth");
}

bool native_form_borderstyle_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "borderstyle") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("borderstyle");
}

bool native_form_titlebar_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "titlebar") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("titlebar");
}

bool native_form_desktop_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "desktop") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("desktop");
}

bool native_form_scrollbars_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "scrollbars") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("scrollbars");
}

bool native_form_lockscreen_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "lockscreen") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("lockscreen");
}

bool native_form_controlbox_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "controlbox") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("controlbox");
}

bool native_form_closable_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "closable") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("closable");
}

bool native_form_minbutton_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "minbutton") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("minbutton");
}

bool native_form_maxbutton_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "maxbutton") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("maxbutton");
}

bool native_form_autocenter_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "autocenter") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("autocenter");
}

bool native_visual_enabled_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "enabled" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("enabled");
}

bool native_visual_fontname_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fontname" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fontname");
}

bool native_visual_fontsize_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fontsize" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fontsize");
}

bool native_visual_fontbold_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fontbold" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fontbold");
}

bool native_visual_fontitalic_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fontitalic" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fontitalic");
}

bool native_visual_fontunderline_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fontunderline" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fontunderline");
}

bool native_visual_fontstrikethru_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fontstrikethru" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fontstrikethru");
}

bool native_visual_fontoutline_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fontoutline" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fontoutline");
}

bool native_visual_fontshadow_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "fontshadow" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("fontshadow");
}

bool native_visual_visible_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "visible" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("visible");
}

bool native_controltiptext_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "controltiptext" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("controltiptext");
}

bool native_visual_helpcontextid_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "helpcontextid" &&
           native_visual_helpcontextid_runtime_object(runtime_object) &&
           runtime_object.properties.contains("helpcontextid");
}

bool native_visual_whatsthishelpid_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "whatsthishelpid" &&
           native_visual_whatsthishelpid_runtime_object(runtime_object) &&
           runtime_object.properties.contains("whatsthishelpid");
}

bool native_visual_tag_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "tag" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("tag");
}

bool native_visual_caption_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "caption" &&
           native_visual_caption_runtime_object(runtime_object) &&
           runtime_object.properties.contains("caption");
}

bool native_visual_picture_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "picture" &&
           native_visual_picture_runtime_object(runtime_object) &&
           runtime_object.properties.contains("picture");
}

bool native_visual_dragmode_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "dragmode" &&
           native_visual_drag_runtime_object(runtime_object) &&
           runtime_object.properties.contains("dragmode");
}

bool native_visual_dragicon_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "dragicon" &&
           native_visual_drag_runtime_object(runtime_object) &&
           runtime_object.properties.contains("dragicon");
}

bool native_visual_downpicture_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "downpicture" &&
           native_visual_button_state_picture_runtime_object(runtime_object) &&
           runtime_object.properties.contains("downpicture");
}

bool native_visual_disabledpicture_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "disabledpicture" &&
           native_visual_button_state_picture_runtime_object(runtime_object) &&
           runtime_object.properties.contains("disabledpicture");
}

bool native_visual_autosize_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "autosize" &&
           native_visual_autosize_runtime_object(runtime_object) &&
           runtime_object.properties.contains("autosize");
}

bool native_visual_drawmode_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "drawmode" &&
           native_visual_drawmode_runtime_object(runtime_object) &&
           runtime_object.properties.contains("drawmode");
}

bool native_visual_backstyle_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "backstyle" &&
           native_visual_backstyle_runtime_object(runtime_object) &&
           runtime_object.properties.contains("backstyle");
}

bool native_visual_alignment_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "alignment" &&
           native_visual_alignment_runtime_object(runtime_object) &&
           runtime_object.properties.contains("alignment");
}

bool native_editbox_scrollbars_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "scrollbars" &&
           native_editbox_scrollbars_runtime_object(runtime_object) &&
           runtime_object.properties.contains("scrollbars");
}

bool native_textbox_inputmask_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "inputmask" &&
           native_textbox_inputmask_runtime_object(runtime_object) &&
           runtime_object.properties.contains("inputmask");
}

bool native_textbox_format_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "format" &&
           native_textbox_format_runtime_object(runtime_object) &&
           runtime_object.properties.contains("format");
}

bool native_textbox_passwordchar_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "passwordchar" &&
           native_textbox_passwordchar_runtime_object(runtime_object) &&
           runtime_object.properties.contains("passwordchar");
}

bool native_textbox_maxlength_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "maxlength" &&
           native_textbox_maxlength_runtime_object(runtime_object) &&
           runtime_object.properties.contains("maxlength");
}

bool native_textbox_specialeffect_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "specialeffect" &&
           native_textbox_specialeffect_runtime_object(runtime_object) &&
           runtime_object.properties.contains("specialeffect");
}

bool native_textbox_borderstyle_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "borderstyle" &&
           native_textbox_borderstyle_runtime_object(runtime_object) &&
           runtime_object.properties.contains("borderstyle");
}

bool native_textbox_hideselection_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "hideselection" &&
           native_textbox_hideselection_runtime_object(runtime_object) &&
           runtime_object.properties.contains("hideselection");
}

bool native_textbox_autocomplete_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "autocomplete" &&
           native_textbox_autocomplete_runtime_object(runtime_object) &&
           runtime_object.properties.contains("autocomplete");
}

bool native_textbox_enablehyperlinks_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "enablehyperlinks" &&
           native_textbox_enablehyperlinks_runtime_object(runtime_object) &&
           runtime_object.properties.contains("enablehyperlinks");
}

bool native_textbox_tooltiptext_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "tooltiptext" &&
           native_textbox_tooltiptext_runtime_object(runtime_object) &&
           runtime_object.properties.contains("tooltiptext");
}

bool native_textbox_margin_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "margin" &&
           native_textbox_margin_runtime_object(runtime_object) &&
           runtime_object.properties.contains("margin");
}

bool native_textbox_mouseicon_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "mouseicon" &&
           native_textbox_mouseicon_runtime_object(runtime_object) &&
           runtime_object.properties.contains("mouseicon");
}

bool native_textbox_disabledbackcolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "disabledbackcolor" &&
           native_textbox_disabledbackcolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("disabledbackcolor");
}

bool native_textbox_disabledforecolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "disabledforecolor" &&
           native_textbox_disabledforecolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("disabledforecolor");
}

bool native_list_control_disableditembackcolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "disableditembackcolor" &&
           native_list_control_disableditembackcolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("disableditembackcolor");
}

bool native_list_control_disableditemforecolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "disableditemforecolor" &&
           native_list_control_disableditemforecolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("disableditemforecolor");
}

bool native_list_control_itembackcolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "itembackcolor" &&
           native_list_control_itembackcolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("itembackcolor");
}

bool native_list_control_itemforecolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "itemforecolor" &&
           native_list_control_itemforecolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("itemforecolor");
}

bool native_list_control_selecteditembackcolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "selecteditembackcolor" &&
           native_list_control_selecteditembackcolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("selecteditembackcolor");
}

bool native_list_control_selecteditemforecolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "selecteditemforecolor" &&
           native_list_control_selecteditemforecolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("selecteditemforecolor");
}

bool native_textbox_statusbartext_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "statusbartext" &&
           native_textbox_statusbartext_runtime_object(runtime_object) &&
           runtime_object.properties.contains("statusbartext");
}

bool native_textbox_strictdateentry_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "strictdateentry" &&
           native_textbox_strictdateentry_runtime_object(runtime_object) &&
           runtime_object.properties.contains("strictdateentry");
}

bool native_textbox_themes_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "themes" &&
           native_textbox_themes_runtime_object(runtime_object) &&
           runtime_object.properties.contains("themes");
}

bool native_textbox_selectedbackcolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "selectedbackcolor" &&
           native_textbox_selectedbackcolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("selectedbackcolor");
}

bool native_textbox_selectedforecolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "selectedforecolor" &&
           native_textbox_selectedforecolor_runtime_object(runtime_object) &&
           runtime_object.properties.contains("selectedforecolor");
}

bool native_textbox_dateformat_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "dateformat" &&
           native_textbox_dateformat_runtime_object(runtime_object) &&
           runtime_object.properties.contains("dateformat");
}

bool native_textbox_century_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "century" &&
           native_textbox_century_runtime_object(runtime_object) &&
           runtime_object.properties.contains("century");
}

bool native_textbox_datemark_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "datemark" &&
           native_textbox_datemark_runtime_object(runtime_object) &&
           runtime_object.properties.contains("datemark");
}

bool native_textbox_hours_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "hours" &&
           native_textbox_hours_runtime_object(runtime_object) &&
           runtime_object.properties.contains("hours");
}

bool native_textbox_seconds_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "seconds" &&
           native_textbox_seconds_runtime_object(runtime_object) &&
           runtime_object.properties.contains("seconds");
}

bool native_textbox_selection_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "selstart" &&
        normalized_member_name != "sellength" &&
        normalized_member_name != "seltext") {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return (normalized_base_class == "textbox" ||
            normalized_base_class == "editbox") &&
           runtime_object.properties.contains("value") &&
           runtime_object.properties.contains("selstart") &&
           runtime_object.properties.contains("sellength") &&
           runtime_object.properties.contains("seltext");
}

bool native_textbox_text_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "text" ||
        !runtime_object.properties.contains("text") ||
        !runtime_object.properties.contains("value")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "textbox" ||
           normalized_base_class == "editbox";
}

bool native_visual_backcolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return normalized_member_name == "backcolor" &&
           is_native_visual_runtime_object(runtime_object) &&
           !is_olecontrol &&
           runtime_object.properties.contains("backcolor");
}

bool native_visual_forecolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return normalized_member_name == "forecolor" &&
           is_native_visual_runtime_object(runtime_object) &&
           !is_olecontrol &&
           runtime_object.properties.contains("forecolor");
}

bool native_visual_geometry_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "left" &&
        normalized_member_name != "top" &&
        normalized_member_name != "width" &&
        normalized_member_name != "height") {
        return false;
    }

    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return is_native_visual_runtime_object(runtime_object) &&
           !is_olecontrol &&
           runtime_object.properties.contains(normalized_member_name);
}

bool native_tabindex_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "tabindex" &&
           native_tabindex_runtime_object_matches(runtime_object) &&
           runtime_object.properties.contains("tabindex");
}

bool native_tabstop_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "tabstop" &&
           native_tabstop_runtime_object_matches(runtime_object) &&
           runtime_object.properties.contains("tabstop");
}

bool native_string_control_value_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "value" ||
        !runtime_object.properties.contains("value")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "textbox" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_selectonentry_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "selectonentry" ||
        !runtime_object.properties.contains("selectonentry")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "textbox" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "column";
}

bool native_resizable_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "resizable" ||
        !runtime_object.properties.contains("resizable")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_controlsource_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "controlsource" ||
        !runtime_object.properties.contains("controlsource")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "textbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "column" ||
           normalized_base_class == "checkbox" ||
           normalized_base_class == "spinner";
}

bool native_recordsource_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "recordsource" ||
        !runtime_object.properties.contains("recordsource")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_allowaddnew_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "allowaddnew" ||
        !runtime_object.properties.contains("allowaddnew")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_grid_rowheight_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "rowheight" ||
        !runtime_object.properties.contains("rowheight")) {
        return false;
    }

    return native_grid_rowheight_runtime_object(runtime_object);
}

bool native_grid_headerheight_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "headerheight" ||
        !runtime_object.properties.contains("headerheight")) {
        return false;
    }

    return native_grid_headerheight_runtime_object(runtime_object);
}

bool native_grid_allowheadersizing_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "allowheadersizing" ||
        !runtime_object.properties.contains("allowheadersizing")) {
        return false;
    }

    return native_grid_allowheadersizing_runtime_object(runtime_object);
}

bool native_grid_allowrowsizing_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "allowrowsizing" ||
        !runtime_object.properties.contains("allowrowsizing")) {
        return false;
    }

    return native_grid_allowrowsizing_runtime_object(runtime_object);
}

bool native_allowcellselection_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "allowcellselection" ||
        !runtime_object.properties.contains("allowcellselection")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_gridlines_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "gridlines" ||
        !runtime_object.properties.contains("gridlines")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_highlight_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "highlight" ||
        !runtime_object.properties.contains("highlight")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_highlightrow_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "highlightrow" ||
        !runtime_object.properties.contains("highlightrow")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_deletemark_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "deletemark" ||
        !runtime_object.properties.contains("deletemark")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_splitbar_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "splitbar" ||
        !runtime_object.properties.contains("splitbar")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_leftcolumn_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "leftcolumn" ||
        !runtime_object.properties.contains("leftcolumn")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_child_collection_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return (normalized_member_name == "objects" ||
            normalized_member_name == "controls" ||
            normalized_member_name == "columns" ||
            normalized_member_name == "pages") &&
           runtime_object.properties.contains(normalized_member_name);
}

bool native_columnorder_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "columnorder" ||
        !runtime_object.properties.contains("columnorder")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_recordmark_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "recordmark" ||
        !runtime_object.properties.contains("recordmark")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_currentcontrol_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "currentcontrol" ||
        !runtime_object.properties.contains("currentcontrol")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_dynamiccurrentcontrol_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "dynamiccurrentcontrol" ||
        !runtime_object.properties.contains("dynamiccurrentcontrol")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_recordsourcetype_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "recordsourcetype" ||
        !runtime_object.properties.contains("recordsourcetype")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_rowsource_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "rowsource" ||
        !runtime_object.properties.contains("rowsource")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_rowsourcetype_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "rowsourcetype" ||
        !runtime_object.properties.contains("rowsourcetype")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_listindex_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "listindex" ||
        !runtime_object.properties.contains("listindex")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_displayvalue_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "displayvalue" ||
        !runtime_object.properties.contains("displayvalue")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_listcount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "listcount" ||
        !runtime_object.properties.contains("listcount")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_sorted_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "sorted" ||
        !runtime_object.properties.contains("sorted")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_multiselect_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "multiselect" ||
        !runtime_object.properties.contains("multiselect")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "listbox";
}

bool native_moverbars_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "moverbars" ||
        !runtime_object.properties.contains("moverbars")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "listbox";
}

bool native_autohidescrollbar_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "autohidescrollbar" ||
        !runtime_object.properties.contains("autohidescrollbar")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "listbox";
}

bool native_firstelement_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "firstelement" ||
        !runtime_object.properties.contains("firstelement")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_numberofelements_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "numberofelements" ||
        !runtime_object.properties.contains("numberofelements")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_displaycount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "displaycount" ||
        !runtime_object.properties.contains("displaycount")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "combobox";
}

bool native_nulldisplay_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "nulldisplay" ||
        !runtime_object.properties.contains("nulldisplay")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_columnlines_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "columnlines" ||
        !runtime_object.properties.contains("columnlines")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_itemtips_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "itemtips" ||
        !runtime_object.properties.contains("itemtips")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_incrementalsearch_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "incrementalsearch" ||
        !runtime_object.properties.contains("incrementalsearch")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_integralheight_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "integralheight" ||
        !runtime_object.properties.contains("integralheight")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "listbox" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "textbox";
}

bool native_boundto_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "boundto" ||
        !runtime_object.properties.contains("boundto")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_newindex_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "newindex" ||
        !runtime_object.properties.contains("newindex")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_newitemid_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "newitemid" ||
        !runtime_object.properties.contains("newitemid")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_listitemid_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "listitemid" ||
        !runtime_object.properties.contains("listitemid")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_listitem_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "listitem") {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_itemdata_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "itemdata") {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_topitemid_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "topitemid") {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_topindex_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "topindex") {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_boundcolumn_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "boundcolumn" ||
        !runtime_object.properties.contains("boundcolumn")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_columncount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "columncount" ||
        !runtime_object.properties.contains("columncount")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "grid";
}

bool native_column_bound_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "bound" ||
        !runtime_object.properties.contains("bound")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_columnwidths_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "columnwidths" ||
        !runtime_object.properties.contains("columnwidths")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_combobox_style_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "style" ||
        !runtime_object.properties.contains("style")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox";
}

bool native_combobox_is_drop_down_list_style(const RuntimeOleObjectState& runtime_object) {
    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    if (normalized_base_class != "combobox") {
        return false;
    }

    const auto style = runtime_object.properties.find("style");
    if (style == runtime_object.properties.end()) {
        return false;
    }

    return std::llround(value_as_number(style->second)) == 2LL;
}

bool native_control_readonly_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "readonly" ||
        !runtime_object.properties.contains("readonly")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "textbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "grid" ||
           normalized_base_class == "column" ||
           normalized_base_class == "checkbox" ||
           normalized_base_class == "spinner";
}

bool native_name_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "name" &&
           !runtime_object.class_hierarchy.empty() &&
           runtime_object.properties.contains("name");
}

std::vector<std::string> collect_native_identity_member_names(const RuntimeOleObjectState& runtime_object) {
    std::vector<std::string> members;
    if (get_native_identity_reflection_metadata(runtime_object, "hwnd").has_value()) {
        members.push_back("hwnd");
    }
    if (get_native_identity_reflection_metadata(runtime_object, "class").has_value()) {
        members.push_back("class");
    }
    if (get_native_identity_reflection_metadata(runtime_object, "baseclass").has_value()) {
        members.push_back("baseclass");
    }
    if (get_native_identity_reflection_metadata(runtime_object, "classlibrary").has_value()) {
        members.push_back("classlibrary");
    }
    if (get_native_identity_reflection_metadata(runtime_object, "parentclass").has_value()) {
        members.push_back("parentclass");
    }
    return members;
}

bool method_ends_with_suffix(
    const std::string& method_name,
    const std::string& suffix,
    std::string* stem = nullptr) {
    const std::string normalized_method = normalize_identifier(method_name);
    if (normalized_method.size() <= suffix.size() ||
        normalized_method.compare(normalized_method.size() - suffix.size(), suffix.size(), suffix) != 0) {
        return false;
    }

    if (normalized_method[normalized_method.size() - suffix.size() - 1U] != '_') {
        return false;
    }

    if (stem != nullptr) {
        *stem = normalized_method.substr(0U, normalized_method.size() - suffix.size() - 1U);
    }
    return true;
}

bool object_has_accessor_property(const RuntimeOleObjectState& runtime_object, const std::string& normalized_property_name) {
    return std::any_of(runtime_object.methods.begin(), runtime_object.methods.end(), [&](const std::string& method_name) {
        std::string stem;
        return method_ends_with_suffix(method_name, "access", &stem) && stem == normalized_property_name;
    });
}

bool object_has_assigner_property(const RuntimeOleObjectState& runtime_object, const std::string& normalized_property_name) {
    return std::any_of(runtime_object.methods.begin(), runtime_object.methods.end(), [&](const std::string& method_name) {
        std::string stem;
        return method_ends_with_suffix(method_name, "assign", &stem) && stem == normalized_property_name;
    });
}

bool looks_like_file_path(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.empty()) {
        return false;
    }
    if (trimmed.find('/') != std::string::npos || trimmed.find('\\') != std::string::npos) {
        return true;
    }
    if (trimmed.size() >= 2U && std::isalpha(static_cast<unsigned char>(trimmed[0])) != 0 && trimmed[1] == ':') {
        return true;
    }
    const std::string lower = lowercase_copy(trimmed);
    const auto has_suffix = [&](const std::string& suffix) {
        return lower.size() >= suffix.size() && lower.compare(lower.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    return has_suffix(".xml") || has_suffix(".txt");
}
