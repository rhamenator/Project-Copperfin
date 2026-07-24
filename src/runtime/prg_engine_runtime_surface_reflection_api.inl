// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

bool is_native_identity_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_identity_member_name_matches(runtime_object, normalized_member_name);
}
bool is_native_olecontrol_creation_time_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_olecontrol_creation_time_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_olecontrol_object_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_olecontrol_object_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_olecontrol_inspection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_olecontrol_inspection_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_olecontrol_conflict_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_olecontrol_conflict_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_child_parent_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_child_parent_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_controlcount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_controlcount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_pagecount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_pagecount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_activepage_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_activepage_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_alwaysontop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_alwaysontop_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_showwindow_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_showwindow_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_windowtype_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_windowtype_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_windowstate_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_windowstate_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_scalemode_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_scalemode_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_drawmode_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_drawmode_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_backstyle_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_backstyle_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_specialeffect_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_specialeffect_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_drawstyle_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_drawstyle_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fillstyle_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fillstyle_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fillcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fillcolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_borderwidth_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_borderwidth_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_bordercolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_bordercolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_borderstyle_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_borderstyle_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_drawwidth_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_drawwidth_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_borderstyle_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_borderstyle_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_titlebar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_titlebar_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_desktop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_desktop_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_scrollbars_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_scrollbars_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_lockscreen_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_lockscreen_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_controlbox_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_controlbox_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_closable_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_closable_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_minbutton_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_minbutton_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_maxbutton_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_maxbutton_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_autocenter_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_autocenter_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_enabled_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_enabled_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fontname_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fontname_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_dynamicfontname_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_dynamicfontname_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_dynamicfontsize_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_dynamicfontsize_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_dynamicfontshadow_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_dynamicfontshadow_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_dynamicfontoutline_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_dynamicfontoutline_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fontsize_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fontsize_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fontbold_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fontbold_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fontitalic_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fontitalic_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fontunderline_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fontunderline_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fontstrikethru_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fontstrikethru_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fontoutline_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fontoutline_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_fontshadow_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_fontshadow_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_visible_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_visible_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_controltiptext_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_controltiptext_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_helpcontextid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_helpcontextid_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_whatsthishelpid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_whatsthishelpid_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_tag_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_tag_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_caption_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_caption_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_picture_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_picture_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_dragmode_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_dragmode_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_dragicon_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_dragicon_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_downpicture_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_downpicture_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_disabledpicture_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_disabledpicture_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_autosize_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_autosize_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_alignment_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_alignment_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_editbox_scrollbars_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_editbox_scrollbars_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_inputmask_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_inputmask_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_dynamicinputmask_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_dynamicinputmask_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_column_dynamicalignment_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_column_dynamicalignment_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_column_sparse_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_column_sparse_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_format_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_format_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_passwordchar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_passwordchar_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_maxlength_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_maxlength_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_specialeffect_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_specialeffect_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_borderstyle_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_borderstyle_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_hideselection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_hideselection_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_autocomplete_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_autocomplete_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_enablehyperlinks_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_enablehyperlinks_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_tooltiptext_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_tooltiptext_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_margin_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_margin_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_mouseicon_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_mouseicon_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_disabledbackcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_disabledbackcolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_disabledforecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_disabledforecolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_list_control_disableditembackcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_list_control_disableditembackcolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_list_control_disableditemforecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_list_control_disableditemforecolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_list_control_itembackcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_list_control_itembackcolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_list_control_itemforecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_list_control_itemforecolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_list_control_selecteditembackcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_list_control_selecteditembackcolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_list_control_selecteditemforecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_list_control_selecteditemforecolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_statusbartext_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_statusbartext_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_strictdateentry_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_strictdateentry_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_themes_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_themes_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_selectedbackcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_selectedbackcolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_selectedforecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_selectedforecolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_dateformat_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_dateformat_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_century_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_century_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_datemark_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_datemark_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_hours_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_hours_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_seconds_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_seconds_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_selection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_selection_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_textbox_text_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_textbox_text_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_backcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_backcolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_forecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_forecolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_geometry_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_geometry_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_tabindex_runtime_object(const RuntimeOleObjectState& runtime_object)
{
    return native_tabindex_runtime_object_matches(runtime_object);
}

bool is_native_tabindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_tabindex_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_tabstop_runtime_object(const RuntimeOleObjectState& runtime_object)
{
    return native_tabstop_runtime_object_matches(runtime_object);
}

bool is_native_tabstop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_tabstop_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_control_readonly_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_control_readonly_member_name_matches(runtime_object, normalized_member_name);
}

bool native_combobox_readonly_assignment_blocked(const RuntimeOleObjectState& runtime_object, const PrgValue& assigned_value)
{
    return native_combobox_is_drop_down_list_style(runtime_object) &&
           value_as_bool(assigned_value);
}

void normalize_native_pageframe_activepage_invariant(RuntimeOleObjectState& runtime_object)
{
    if (normalize_identifier(trim_copy(runtime_object.base_class_name)) != "pageframe") {
        return;
    }

    const auto pagecount = runtime_object.properties.find("pagecount");
    const long long available_pages =
        pagecount == runtime_object.properties.end()
            ? 0LL
            : std::max(0LL, std::llround(value_as_number(pagecount->second)));

    auto activepage = runtime_object.properties.find("activepage");
    if (activepage == runtime_object.properties.end()) {
        runtime_object.properties["activepage"] =
            make_number_value(available_pages > 0LL ? 1.0 : 0.0);
        return;
    }

    if (available_pages <= 0LL) {
        activepage->second = make_number_value(0.0);
        return;
    }

    long long requested_page = std::llround(value_as_number(activepage->second));
    if (requested_page < 1LL) {
        requested_page = 1LL;
    } else if (requested_page > available_pages) {
        requested_page = available_pages;
    }
    activepage->second = make_number_value(static_cast<double>(requested_page));
}

void normalize_native_combobox_readonly_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_combobox_is_drop_down_list_style(runtime_object)) {
        return;
    }

    const auto readonly = runtime_object.properties.find("readonly");
    if (readonly != runtime_object.properties.end() &&
        value_as_bool(readonly->second)) {
        readonly->second = make_boolean_value(false);
    }
}

void normalize_native_list_control_sorted_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto sorted = runtime_object.properties.find("sorted");
    if (sorted == runtime_object.properties.end()) {
        return;
    }

    sorted->second = make_boolean_value(value_as_bool(sorted->second));
    sort_native_list_control_rows_if_needed(runtime_object);
}

void normalize_native_listbox_multiselect_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_listbox_runtime_object(runtime_object)) {
        return;
    }

    const auto multiselect = runtime_object.properties.find("multiselect");
    if (multiselect == runtime_object.properties.end()) {
        return;
    }

    multiselect->second = make_boolean_value(value_as_bool(multiselect->second));
    sync_native_list_control_selected_state_from_listindex(runtime_object);
}

void normalize_native_listbox_moverbars_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_listbox_runtime_object(runtime_object)) {
        return;
    }

    const auto moverbars = runtime_object.properties.find("moverbars");
    if (moverbars == runtime_object.properties.end()) {
        return;
    }

    moverbars->second = make_boolean_value(
        native_list_control_rowsourcetype_supports_additem(runtime_object) &&
        value_as_bool(moverbars->second));
}

bool native_listbox_moverbars_row_source_supported(const RuntimeOleObjectState& runtime_object)
{
    return is_native_listbox_runtime_object(runtime_object) &&
           native_list_control_rowsourcetype_supports_additem(runtime_object);
}

void normalize_native_listbox_autohidescrollbar_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_listbox_runtime_object(runtime_object)) {
        return;
    }

    const auto autohide = runtime_object.properties.find("autohidescrollbar");
    if (autohide == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(autohide->second);
    autohide->second = make_number_value(
        std::isfinite(value) && std::llround(value) == 1LL ? 1.0 : 0.0);
}

void normalize_native_visual_mousepointer_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_visual_runtime_object(runtime_object)) {
        return;
    }

    const auto mouse_pointer = runtime_object.properties.find("mousepointer");
    if (mouse_pointer == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(mouse_pointer->second);
    mouse_pointer->second = make_number_value(
        std::isfinite(value) && value >= 0.0 ? static_cast<double>(std::llround(value)) : 0.0);
}

void normalize_native_visual_dragmode_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_drag_runtime_object(runtime_object)) {
        return;
    }

    const auto drag_mode = runtime_object.properties.find("dragmode");
    if (drag_mode == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(drag_mode->second);
    drag_mode->second = make_number_value(
        std::isfinite(value) && value >= 0.0 ? static_cast<double>(std::llround(value)) : 0.0);
}

void normalize_native_list_control_array_range_invariants(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto normalize_positive_integer = [](double value, long long fallback) {
        if (!std::isfinite(value) || value <= 0.0) {
            return fallback;
        }
        if (value >= static_cast<double>(std::numeric_limits<long long>::max())) {
            return std::numeric_limits<long long>::max();
        }
        return std::llround(value);
    };

    if (const auto first_element = runtime_object.properties.find("firstelement");
        first_element != runtime_object.properties.end())
    {
        first_element->second = make_number_value(static_cast<double>(normalize_positive_integer(
            value_as_number(first_element->second),
            1LL)));
    }

    if (const auto number_of_elements = runtime_object.properties.find("numberofelements");
        number_of_elements != runtime_object.properties.end())
    {
        const double value = value_as_number(number_of_elements->second);
        const long long normalized =
            !std::isfinite(value) || value <= 0.0
                ? 0LL
                : value >= static_cast<double>(std::numeric_limits<long long>::max())
                    ? std::numeric_limits<long long>::max()
                    : std::llround(value);
        number_of_elements->second = make_number_value(static_cast<double>(normalized));
    }
}

void normalize_native_combobox_displaycount_invariant(RuntimeOleObjectState& runtime_object)
{
    if (normalize_identifier(trim_copy(runtime_object.base_class_name)) != "combobox") {
        return;
    }

    const auto display_count = runtime_object.properties.find("displaycount");
    if (display_count == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(display_count->second);
    const long long normalized =
        !std::isfinite(value) || value <= 0.0
            ? 0LL
            : value >= static_cast<double>(std::numeric_limits<long long>::max())
                ? std::numeric_limits<long long>::max()
                : std::llround(value);
    display_count->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_list_control_nulldisplay_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto null_display = runtime_object.properties.find("nulldisplay");
    if (null_display == runtime_object.properties.end()) {
        return;
    }

    null_display->second = make_string_value(value_as_string(null_display->second));
}

void normalize_native_list_control_columnlines_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto column_lines = runtime_object.properties.find("columnlines");
    if (column_lines == runtime_object.properties.end()) {
        return;
    }

    column_lines->second = make_boolean_value(value_as_bool(column_lines->second));
}

void normalize_native_list_control_columnwidths_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto column_widths = runtime_object.properties.find("columnwidths");
    if (column_widths == runtime_object.properties.end()) {
        return;
    }

    column_widths->second = make_string_value(value_as_string(column_widths->second));
}

void normalize_native_list_control_itemtips_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto item_tips = runtime_object.properties.find("itemtips");
    if (item_tips == runtime_object.properties.end()) {
        return;
    }

    item_tips->second = make_boolean_value(value_as_bool(item_tips->second));
}

void normalize_native_list_control_incrementalsearch_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_list_control_runtime_object(runtime_object))
    {
        return;
    }

    const auto incremental_search = runtime_object.properties.find("incrementalsearch");
    if (incremental_search == runtime_object.properties.end())
    {
        return;
    }

    incremental_search->second = make_boolean_value(value_as_bool(incremental_search->second));
}

void normalize_native_visual_alignment_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_alignment_runtime_object(runtime_object)) {
        return;
    }

    const auto alignment = runtime_object.properties.find("alignment");
    if (alignment == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(alignment->second);
    const long long normalized =
        !std::isfinite(value) ? 0LL : std::clamp(std::llround(value), 0LL, 2LL);
    alignment->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_form_scalemode_invariant(RuntimeOleObjectState& runtime_object)
{
    if (normalize_identifier(trim_copy(runtime_object.base_class_name)) != "form") {
        return;
    }

    const auto scale_mode = runtime_object.properties.find("scalemode");
    if (scale_mode == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(scale_mode->second);
    const long long normalized =
        !std::isfinite(value) ? 0LL : std::clamp(std::llround(value), 0LL, 3LL);
    scale_mode->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_visual_drawmode_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_drawmode_runtime_object(runtime_object)) {
        return;
    }

    const auto draw_mode = runtime_object.properties.find("drawmode");
    if (draw_mode == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(draw_mode->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : 13LL;
    const long long normalized = rounded >= 1LL && rounded <= 16LL ? rounded : 13LL;
    draw_mode->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_visual_backstyle_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_backstyle_runtime_object(runtime_object)) {
        return;
    }

    const auto back_style = runtime_object.properties.find("backstyle");
    if (back_style == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(back_style->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : 1LL;
    const long long normalized = rounded == 0LL || rounded == 1LL ? rounded : 1LL;
    back_style->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_form_drawstyle_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_form_drawstyle_runtime_object(runtime_object)) {
        return;
    }

    const auto draw_style = runtime_object.properties.find("drawstyle");
    if (draw_style == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(draw_style->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : 0LL;
    const long long normalized = rounded >= 0LL && rounded <= 6LL ? rounded : 0LL;
    draw_style->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_visual_fillstyle_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_fillstyle_runtime_object(runtime_object)) {
        return;
    }

    const auto fill_style = runtime_object.properties.find("fillstyle");
    if (fill_style == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(fill_style->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : 1LL;
    const long long normalized = rounded >= 0LL && rounded <= 7LL ? rounded : 1LL;
    fill_style->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_visual_specialeffect_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_specialeffect_runtime_object(runtime_object)) {
        return;
    }

    const auto special_effect = runtime_object.properties.find("specialeffect");
    if (special_effect == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(special_effect->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : 0LL;
    const long long normalized = std::clamp(rounded, 0LL, 2LL);
    special_effect->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_visual_fillcolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_fillcolor_runtime_object(runtime_object)) {
        return;
    }

    const auto fill_color = runtime_object.properties.find("fillcolor");
    if (fill_color == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(fill_color->second);
    fill_color->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_visual_borderwidth_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_borderwidth_runtime_object(runtime_object)) {
        return;
    }

    const auto border_width = runtime_object.properties.find("borderwidth");
    if (border_width == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(border_width->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : 1LL;
    const long long normalized = rounded >= 0LL && rounded <= 8192LL ? rounded : 1LL;
    border_width->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_visual_bordercolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_bordercolor_runtime_object(runtime_object)) {
        return;
    }

    const auto border_color = runtime_object.properties.find("bordercolor");
    if (border_color == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(border_color->second);
    border_color->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_visual_borderstyle_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_borderstyle_runtime_object(runtime_object)) {
        return;
    }

    const auto border_style = runtime_object.properties.find("borderstyle");
    if (border_style == runtime_object.properties.end()) {
        return;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    const bool is_line_style = normalized_base_class == "line" ||
                               normalized_base_class == "shape";
    const long long default_value = is_line_style ||
                                            (normalized_base_class != "image" &&
                                             normalized_base_class != "label")
                                        ? 1LL
                                        : 0LL;
    const long long maximum = is_line_style ? 6LL : 1LL;
    const double value = value_as_number(border_style->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : default_value;
    const long long normalized = rounded >= 0LL && rounded <= maximum ? rounded : default_value;
    border_style->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_form_drawwidth_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_form_drawwidth_runtime_object(runtime_object)) {
        return;
    }

    const auto draw_width = runtime_object.properties.find("drawwidth");
    if (draw_width == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(draw_width->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : 1LL;
    const long long normalized = rounded >= 1LL && rounded <= 32767LL ? rounded : 1LL;
    draw_width->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_grid_rowheight_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_grid_rowheight_runtime_object(runtime_object)) {
        return;
    }

    const auto row_height = runtime_object.properties.find("rowheight");
    if (row_height == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(row_height->second);
    row_height->second = make_number_value(
        std::isfinite(value) && value >= -1.0
            ? static_cast<double>(std::llround(value))
            : -1.0);
}

void normalize_native_grid_headerheight_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_grid_headerheight_runtime_object(runtime_object)) {
        return;
    }

    const auto header_height = runtime_object.properties.find("headerheight");
    if (header_height == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(header_height->second);
    header_height->second = make_number_value(
        std::isfinite(value) && value >= 0.0
            ? static_cast<double>(std::llround(value))
            : 0.0);
}

void normalize_native_grid_allowheadersizing_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_grid_allowheadersizing_runtime_object(runtime_object)) {
        return;
    }

    const auto allow_header_sizing = runtime_object.properties.find("allowheadersizing");
    if (allow_header_sizing == runtime_object.properties.end()) {
        return;
    }

    allow_header_sizing->second = make_boolean_value(value_as_bool(allow_header_sizing->second));
}

void normalize_native_grid_allowrowsizing_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_grid_allowrowsizing_runtime_object(runtime_object)) {
        return;
    }

    const auto allow_row_sizing = runtime_object.properties.find("allowrowsizing");
    if (allow_row_sizing == runtime_object.properties.end()) {
        return;
    }

    allow_row_sizing->second = make_boolean_value(value_as_bool(allow_row_sizing->second));
}

void normalize_native_grid_allowautocolumnfit_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_grid_allowautocolumnfit_runtime_object(runtime_object)) {
        return;
    }

    const auto allow_auto_column_fit = runtime_object.properties.find("allowautocolumnfit");
    if (allow_auto_column_fit == runtime_object.properties.end()) {
        return;
    }

    allow_auto_column_fit->second = make_boolean_value(value_as_bool(allow_auto_column_fit->second));
}

void normalize_native_editbox_scrollbars_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_editbox_scrollbars_runtime_object(runtime_object)) {
        return;
    }

    const auto scrollbars = runtime_object.properties.find("scrollbars");
    if (scrollbars == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(scrollbars->second);
    const long long normalized =
        !std::isfinite(value) ? 0LL : std::clamp(std::llround(value), 0LL, 3LL);
    scrollbars->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_inputmask_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_inputmask_runtime_object(runtime_object)) {
        return;
    }

    const auto input_mask = runtime_object.properties.find("inputmask");
    if (input_mask == runtime_object.properties.end()) {
        return;
    }

    input_mask->second = make_string_value(value_as_string(input_mask->second));
}

void normalize_native_textbox_dynamicinputmask_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_dynamicinputmask_runtime_object(runtime_object)) {
        return;
    }

    const auto dynamic_input_mask = runtime_object.properties.find("dynamicinputmask");
    if (dynamic_input_mask == runtime_object.properties.end()) {
        return;
    }

    dynamic_input_mask->second = make_string_value(value_as_string(dynamic_input_mask->second));
}

void normalize_native_column_dynamicalignment_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_column_dynamicalignment_runtime_object(runtime_object)) {
        return;
    }

    const auto dynamic_alignment = runtime_object.properties.find("dynamicalignment");
    if (dynamic_alignment == runtime_object.properties.end()) {
        return;
    }

    dynamic_alignment->second = make_string_value(value_as_string(dynamic_alignment->second));
}

void normalize_native_column_sparse_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_column_sparse_runtime_object(runtime_object)) {
        return;
    }

    const auto sparse = runtime_object.properties.find("sparse");
    if (sparse == runtime_object.properties.end()) {
        return;
    }

    sparse->second = make_boolean_value(value_as_bool(sparse->second));
}

void normalize_native_textbox_format_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_format_runtime_object(runtime_object)) {
        return;
    }

    const auto format = runtime_object.properties.find("format");
    if (format == runtime_object.properties.end()) {
        return;
    }

    format->second = make_string_value(value_as_string(format->second));
}

void normalize_native_textbox_passwordchar_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_passwordchar_runtime_object(runtime_object)) {
        return;
    }

    const auto password_char = runtime_object.properties.find("passwordchar");
    if (password_char == runtime_object.properties.end()) {
        return;
    }

    password_char->second = make_string_value(value_as_string(password_char->second));
}

void normalize_native_textbox_maxlength_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_maxlength_runtime_object(runtime_object)) {
        return;
    }

    const auto max_length = runtime_object.properties.find("maxlength");
    if (max_length == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(max_length->second);
    max_length->second = make_number_value(
        std::isfinite(value) && value >= 0.0
            ? static_cast<double>(std::llround(value))
            : 0.0);
}

void normalize_native_textbox_specialeffect_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_specialeffect_runtime_object(runtime_object)) {
        return;
    }

    const auto special_effect = runtime_object.properties.find("specialeffect");
    if (special_effect == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(special_effect->second);
    const long long normalized =
        !std::isfinite(value) ? 0LL : std::clamp(std::llround(value), 0LL, 2LL);
    special_effect->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_borderstyle_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_borderstyle_runtime_object(runtime_object)) {
        return;
    }

    const auto border_style = runtime_object.properties.find("borderstyle");
    if (border_style == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(border_style->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : -1LL;
    const long long normalized = rounded == 0LL || rounded == 1LL ? rounded : 1LL;
    border_style->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_hideselection_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_hideselection_runtime_object(runtime_object)) {
        return;
    }

    const auto hide_selection = runtime_object.properties.find("hideselection");
    if (hide_selection == runtime_object.properties.end()) {
        return;
    }

    hide_selection->second = make_boolean_value(value_as_bool(hide_selection->second));
}

void normalize_native_textbox_autocomplete_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_autocomplete_runtime_object(runtime_object)) {
        return;
    }

    const auto auto_complete = runtime_object.properties.find("autocomplete");
    if (auto_complete == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(auto_complete->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : -1LL;
    const long long normalized = rounded >= 0LL && rounded <= 4LL ? rounded : 0LL;
    auto_complete->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_enablehyperlinks_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_enablehyperlinks_runtime_object(runtime_object)) {
        return;
    }

    const auto enable_hyperlinks = runtime_object.properties.find("enablehyperlinks");
    if (enable_hyperlinks == runtime_object.properties.end()) {
        return;
    }

    enable_hyperlinks->second = make_boolean_value(value_as_bool(enable_hyperlinks->second));
}

void normalize_native_textbox_tooltiptext_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_tooltiptext_runtime_object(runtime_object)) {
        return;
    }

    const auto tooltip_text = runtime_object.properties.find("tooltiptext");
    if (tooltip_text == runtime_object.properties.end()) {
        return;
    }

    tooltip_text->second = make_string_value(value_as_string(tooltip_text->second));
}

void normalize_native_textbox_margin_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_margin_runtime_object(runtime_object)) {
        return;
    }

    const auto margin = runtime_object.properties.find("margin");
    if (margin == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(margin->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : -1LL;
    const long long normalized = rounded >= 0LL ? rounded : 0LL;
    margin->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_mouseicon_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_mouseicon_runtime_object(runtime_object)) {
        return;
    }

    const auto mouse_icon = runtime_object.properties.find("mouseicon");
    if (mouse_icon == runtime_object.properties.end()) {
        return;
    }

    mouse_icon->second = make_string_value(value_as_string(mouse_icon->second));
}

void normalize_native_textbox_disabledbackcolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_disabledbackcolor_runtime_object(runtime_object)) {
        return;
    }

    const auto disabled_backcolor = runtime_object.properties.find("disabledbackcolor");
    if (disabled_backcolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(disabled_backcolor->second);
    disabled_backcolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_textbox_disabledforecolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_disabledforecolor_runtime_object(runtime_object)) {
        return;
    }

    const auto disabled_forecolor = runtime_object.properties.find("disabledforecolor");
    if (disabled_forecolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(disabled_forecolor->second);
    disabled_forecolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_list_control_disableditembackcolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_list_control_disableditembackcolor_runtime_object(runtime_object)) {
        return;
    }

    const auto disabled_item_backcolor = runtime_object.properties.find("disableditembackcolor");
    if (disabled_item_backcolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(disabled_item_backcolor->second);
    disabled_item_backcolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_list_control_disableditemforecolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_list_control_disableditemforecolor_runtime_object(runtime_object)) {
        return;
    }

    const auto disabled_item_forecolor = runtime_object.properties.find("disableditemforecolor");
    if (disabled_item_forecolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(disabled_item_forecolor->second);
    disabled_item_forecolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_list_control_itembackcolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_list_control_itembackcolor_runtime_object(runtime_object)) {
        return;
    }

    const auto item_backcolor = runtime_object.properties.find("itembackcolor");
    if (item_backcolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(item_backcolor->second);
    item_backcolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_list_control_itemforecolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_list_control_itemforecolor_runtime_object(runtime_object)) {
        return;
    }

    const auto item_forecolor = runtime_object.properties.find("itemforecolor");
    if (item_forecolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(item_forecolor->second);
    item_forecolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_list_control_selecteditembackcolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_list_control_selecteditembackcolor_runtime_object(runtime_object)) {
        return;
    }

    const auto selected_item_backcolor = runtime_object.properties.find("selecteditembackcolor");
    if (selected_item_backcolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(selected_item_backcolor->second);
    selected_item_backcolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_list_control_selecteditemforecolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_list_control_selecteditemforecolor_runtime_object(runtime_object)) {
        return;
    }

    const auto selected_item_forecolor = runtime_object.properties.find("selecteditemforecolor");
    if (selected_item_forecolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(selected_item_forecolor->second);
    selected_item_forecolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_textbox_statusbartext_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_statusbartext_runtime_object(runtime_object)) {
        return;
    }

    const auto statusbar_text = runtime_object.properties.find("statusbartext");
    if (statusbar_text == runtime_object.properties.end()) {
        return;
    }

    statusbar_text->second = make_string_value(value_as_string(statusbar_text->second));
}

void normalize_native_visual_helpcontextid_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_helpcontextid_runtime_object(runtime_object)) {
        return;
    }

    const auto help_context_id = runtime_object.properties.find("helpcontextid");
    if (help_context_id == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(help_context_id->second);
    const long long normalized = std::isfinite(value) && value >= 0.0
        ? std::llround(value)
        : 0LL;
    help_context_id->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_visual_whatsthishelpid_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_visual_whatsthishelpid_runtime_object(runtime_object)) {
        return;
    }

    const auto whats_this_help_id = runtime_object.properties.find("whatsthishelpid");
    if (whats_this_help_id == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(whats_this_help_id->second);
    const long long normalized = std::isfinite(value) && value >= 0.0
        ? std::llround(value)
        : 0LL;
    whats_this_help_id->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_strictdateentry_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_strictdateentry_runtime_object(runtime_object)) {
        return;
    }

    const auto strict_date_entry = runtime_object.properties.find("strictdateentry");
    if (strict_date_entry == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(strict_date_entry->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : -1LL;
    const long long normalized = rounded >= 0LL && rounded <= 1LL ? rounded : 1LL;
    strict_date_entry->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_themes_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_themes_runtime_object(runtime_object)) {
        return;
    }

    const auto themes = runtime_object.properties.find("themes");
    if (themes == runtime_object.properties.end()) {
        return;
    }

    themes->second = make_boolean_value(value_as_bool(themes->second));
}

void normalize_native_textbox_selectedbackcolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_selectedbackcolor_runtime_object(runtime_object)) {
        return;
    }

    const auto selected_backcolor = runtime_object.properties.find("selectedbackcolor");
    if (selected_backcolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(selected_backcolor->second);
    selected_backcolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_textbox_selectedforecolor_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_selectedforecolor_runtime_object(runtime_object)) {
        return;
    }

    const auto selected_forecolor = runtime_object.properties.find("selectedforecolor");
    if (selected_forecolor == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(selected_forecolor->second);
    selected_forecolor->second = make_number_value(std::isfinite(value) ? std::trunc(value) : 0.0);
}

void normalize_native_textbox_dateformat_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_dateformat_runtime_object(runtime_object)) {
        return;
    }

    const auto date_format = runtime_object.properties.find("dateformat");
    if (date_format == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(date_format->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : -1LL;
    const long long normalized = rounded >= 0LL && rounded <= 14LL ? rounded : 0LL;
    date_format->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_century_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_century_runtime_object(runtime_object)) {
        return;
    }

    const auto century = runtime_object.properties.find("century");
    if (century == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(century->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : -1LL;
    const long long normalized = rounded >= 0LL && rounded <= 2LL ? rounded : 1LL;
    century->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_datemark_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_datemark_runtime_object(runtime_object)) {
        return;
    }

    const auto date_mark = runtime_object.properties.find("datemark");
    if (date_mark == runtime_object.properties.end()) {
        return;
    }

    date_mark->second = make_string_value(value_as_string(date_mark->second));
}

void normalize_native_textbox_hours_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_hours_runtime_object(runtime_object)) {
        return;
    }

    const auto hours = runtime_object.properties.find("hours");
    if (hours == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(hours->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : -1LL;
    const long long normalized = rounded == 12LL || rounded == 24LL ? rounded : 0LL;
    hours->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_seconds_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_textbox_seconds_runtime_object(runtime_object)) {
        return;
    }

    const auto seconds = runtime_object.properties.find("seconds");
    if (seconds == runtime_object.properties.end()) {
        return;
    }

    const double value = value_as_number(seconds->second);
    const long long rounded = std::isfinite(value) ? std::llround(value) : -1LL;
    const long long normalized = rounded >= 0LL && rounded <= 2LL ? rounded : 2LL;
    seconds->second = make_number_value(static_cast<double>(normalized));
}

void normalize_native_textbox_selection_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_textbox_selection_member_name(runtime_object, "selstart")) {
        return;
    }

    auto value = runtime_object.properties.find("value");
    auto selection_start = runtime_object.properties.find("selstart");
    auto selection_length = runtime_object.properties.find("sellength");
    auto selected_text = runtime_object.properties.find("seltext");
    if (value == runtime_object.properties.end() ||
        selection_start == runtime_object.properties.end() ||
        selection_length == runtime_object.properties.end() ||
        selected_text == runtime_object.properties.end()) {
        return;
    }

    const std::string text = value_as_string(value->second);
    const long long text_length = static_cast<long long>(text.size());
    const double raw_start = value_as_number(selection_start->second);
    const double raw_length = value_as_number(selection_length->second);
    const long long requested_start =
        std::isfinite(raw_start) ? std::llround(raw_start) : 0LL;
    const long long requested_length =
        std::isfinite(raw_length) ? std::llround(raw_length) : 0LL;
    const long long normalized_start = std::clamp(requested_start, 0LL, text_length);
    const long long normalized_length = std::clamp(
        requested_length,
        0LL,
        text_length - normalized_start);

    selection_start->second = make_number_value(static_cast<double>(normalized_start));
    selection_length->second = make_number_value(static_cast<double>(normalized_length));
    selected_text->second = make_string_value(
        text.substr(
            static_cast<std::size_t>(normalized_start),
            static_cast<std::size_t>(normalized_length)));
}

void normalize_native_textbox_text_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_textbox_text_member_name(runtime_object, "text")) {
        return;
    }

    const auto value = runtime_object.properties.find("value");
    const auto text = runtime_object.properties.find("text");
    if (value == runtime_object.properties.end() ||
        text == runtime_object.properties.end()) {
        return;
    }

    text->second = make_string_value(value_as_string(value->second));
}

bool write_native_textbox_selection_property(
    RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name,
    const PrgValue& assigned_value)
{
    if (!is_native_textbox_selection_member_name(runtime_object, normalized_member_name)) {
        return false;
    }

    normalize_native_textbox_selection_invariant(runtime_object);
    auto value = runtime_object.properties.find("value");
    auto selection_start = runtime_object.properties.find("selstart");
    auto selection_length = runtime_object.properties.find("sellength");
    if (value == runtime_object.properties.end() ||
        selection_start == runtime_object.properties.end() ||
        selection_length == runtime_object.properties.end()) {
        return false;
    }

    std::string text = value_as_string(value->second);
    const long long text_length = static_cast<long long>(text.size());
    const long long start = std::clamp(
        static_cast<long long>(std::llround(value_as_number(selection_start->second))),
        0LL,
        text_length);
    const long long length = std::clamp(
        static_cast<long long>(std::llround(value_as_number(selection_length->second))),
        0LL,
        text_length - start);

    if (normalized_member_name == "selstart") {
        const double raw_start = value_as_number(assigned_value);
        if (!std::isfinite(raw_start) || raw_start < 0.0) {
            return false;
        }
        selection_start->second = make_number_value(
            static_cast<double>(std::clamp(std::llround(raw_start), 0LL, text_length)));
        selection_length->second = make_number_value(0.0);
        normalize_native_textbox_selection_invariant(runtime_object);
        return true;
    }

    if (normalized_member_name == "sellength") {
        const double raw_length = value_as_number(assigned_value);
        if (!std::isfinite(raw_length) || raw_length < 0.0) {
            return false;
        }
        selection_length->second = make_number_value(
            static_cast<double>(std::clamp(std::llround(raw_length), 0LL, text_length - start)));
        normalize_native_textbox_selection_invariant(runtime_object);
        return true;
    }

    const std::string replacement = value_as_string(assigned_value);
    text.replace(
        static_cast<std::size_t>(start),
        static_cast<std::size_t>(length),
        replacement);
    value->second = make_string_value(text);
    selection_start->second = make_number_value(
        static_cast<double>(start + static_cast<long long>(replacement.size())));
    selection_length->second = make_number_value(0.0);
    normalize_native_textbox_selection_invariant(runtime_object);
    return true;
}

bool is_native_combobox_style_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_combobox_style_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_string_control_value_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_string_control_value_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_selectonentry_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_selectonentry_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_resizable_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_resizable_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_controlsource_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_controlsource_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_recordsource_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_recordsource_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_allowaddnew_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_allowaddnew_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_grid_rowheight_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_grid_rowheight_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_grid_headerheight_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_grid_headerheight_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_grid_allowheadersizing_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_grid_allowheadersizing_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_grid_allowrowsizing_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_grid_allowrowsizing_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_grid_allowautocolumnfit_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_grid_allowautocolumnfit_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_grid_activecolumn_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_grid_activecolumn_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_allowcellselection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_allowcellselection_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_gridlines_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_gridlines_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_highlight_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_highlight_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_highlightrow_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_highlightrow_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_deletemark_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_deletemark_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_splitbar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_splitbar_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_leftcolumn_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_leftcolumn_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_columnorder_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_columnorder_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_child_collection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_child_collection_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_recordmark_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_recordmark_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_currentcontrol_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_currentcontrol_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_dynamiccurrentcontrol_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_dynamiccurrentcontrol_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_recordsourcetype_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_recordsourcetype_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_rowsource_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_rowsource_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_rowsourcetype_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_rowsourcetype_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_listindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_listindex_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_displayvalue_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_displayvalue_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_listcount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_listcount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_sorted_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_sorted_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_multiselect_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_multiselect_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_moverbars_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_moverbars_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_autohidescrollbar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_autohidescrollbar_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_firstelement_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_firstelement_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_numberofelements_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_numberofelements_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_displaycount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_displaycount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_nulldisplay_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_nulldisplay_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_columnlines_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_columnlines_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_itemtips_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_itemtips_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_incrementalsearch_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_incrementalsearch_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_integralheight_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_integralheight_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_boundto_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_boundto_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_newindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_newindex_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_newitemid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_newitemid_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_listitemid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_listitemid_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_itemdata_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_itemdata_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_topitemid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_topitemid_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_topindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_topindex_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_boundcolumn_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_boundcolumn_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_columncount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_columncount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_column_bound_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_column_bound_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_columnwidths_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_columnwidths_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_name_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_name_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_collection_object(const RuntimeOleObjectState& runtime_object)
{
    return normalize_identifier(runtime_object.base_class_name) == "collection" ||
           normalize_identifier(runtime_object.prog_id) == "collection";
}

bool is_native_collection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    if (!is_native_collection_object(runtime_object)) {
        return false;
    }
    return normalized_member_name == "count" ||
           normalized_member_name == "item" ||
           (!runtime_object.read_only_collection_surface &&
            (normalized_member_name == "add" ||
             normalized_member_name == "remove" ||
             normalized_member_name == "removeall"));
}

bool is_native_collection_readonly_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return is_native_collection_object(runtime_object) && normalized_member_name == "count";
}
