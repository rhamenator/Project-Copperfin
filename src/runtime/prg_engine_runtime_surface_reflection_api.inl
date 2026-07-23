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

bool is_native_visual_visible_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_visible_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_controltiptext_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_controltiptext_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_tag_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_tag_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_caption_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_caption_member_name_matches(runtime_object, normalized_member_name);
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
