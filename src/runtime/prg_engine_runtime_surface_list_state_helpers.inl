// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

bool object_has_member(const std::vector<std::string>& members, const std::string& normalized_member_name) {
    return std::find_if(members.begin(), members.end(), [&](const std::string& member) {
               return normalize_identifier(member) == normalized_member_name;
           }) != members.end();
}

bool is_native_visual_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "commandgroup" ||
           normalized_base_class == "container" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "form" ||
           normalized_base_class == "grid" ||
           normalized_base_class == "image" ||
           normalized_base_class == "label" ||
           normalized_base_class == "line" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "olecontrol" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "optiongroup" ||
           normalized_base_class == "page" ||
           normalized_base_class == "pageframe" ||
           normalized_base_class == "separator" ||
           normalized_base_class == "shape" ||
           normalized_base_class == "spinner" ||
           normalized_base_class == "textbox" ||
           normalized_base_class == "toolbar";
}

bool native_visual_helpcontextid_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_visual_runtime_object(runtime_object);
}

bool native_visual_whatsthishelpid_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_visual_runtime_object(runtime_object);
}

bool native_visual_caption_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "commandgroup" ||
           normalized_base_class == "form" ||
           normalized_base_class == "label" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "optiongroup" ||
           normalized_base_class == "page" ||
           normalized_base_class == "pageframe";
}

bool native_visual_picture_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "commandgroup" ||
           normalized_base_class == "form" ||
           normalized_base_class == "image" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "optiongroup" ||
           normalized_base_class == "toolbar";
}

bool native_visual_drag_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_visual_runtime_object(runtime_object);
}

bool native_visual_button_state_picture_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "commandgroup" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "optiongroup";
}

bool native_visual_autosize_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "commandgroup" ||
           normalized_base_class == "label" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "optiongroup";
}

bool native_visual_drawmode_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" ||
           normalized_base_class == "line" ||
           normalized_base_class == "shape";
}

bool native_visual_backstyle_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "commandgroup" ||
           normalized_base_class == "container" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "image" ||
           normalized_base_class == "label" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "optiongroup" ||
           normalized_base_class == "page" ||
           normalized_base_class == "shape" ||
           normalized_base_class == "textbox";
}

bool native_form_drawstyle_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "form";
}

bool native_visual_fillstyle_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" ||
           normalized_base_class == "shape";
}

bool native_visual_alignment_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "editbox" ||
           normalized_base_class == "label" ||
           normalized_base_class == "textbox";
}

bool native_grid_rowheight_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_grid_headerheight_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_grid_allowheadersizing_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_grid_allowrowsizing_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_editbox_scrollbars_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "editbox";
}

bool native_textbox_inputmask_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "textbox";
}

bool native_textbox_format_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_passwordchar_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_maxlength_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "editbox" || normalized_base_class == "textbox";
}

bool native_textbox_specialeffect_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_borderstyle_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_hideselection_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_autocomplete_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_enablehyperlinks_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_tooltiptext_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_margin_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_mouseicon_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_disabledbackcolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_disabledforecolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_statusbartext_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_strictdateentry_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_themes_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_selectedbackcolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_selectedforecolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_dateformat_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_century_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_datemark_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_hours_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_textbox_seconds_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_textbox_inputmask_runtime_object(runtime_object);
}

bool native_tabindex_runtime_object_matches(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "commandgroup" ||
           normalized_base_class == "container" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "form" ||
           normalized_base_class == "grid" ||
           normalized_base_class == "label" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "olecontrol" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "optiongroup" ||
           normalized_base_class == "pageframe" ||
           normalized_base_class == "spinner" ||
           normalized_base_class == "textbox";
}

bool native_tabstop_runtime_object_matches(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "container" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "form" ||
           normalized_base_class == "grid" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "olecontrol" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "page" ||
           normalized_base_class == "pageframe" ||
           normalized_base_class == "spinner" ||
           normalized_base_class == "textbox";
}

bool is_native_focusable_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "form" ||
           normalized_base_class == "grid" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "olecontrol" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "page" ||
           normalized_base_class == "spinner" ||
           normalized_base_class == "textbox";
}

bool is_builtin_native_runtime_method_name(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if ((normalized_member_name == "readexpression" ||
         normalized_member_name == "writeexpression" ||
         normalized_member_name == "readmethod" ||
         normalized_member_name == "writemethod") &&
        (!runtime_object.class_hierarchy.empty() || !runtime_object.source.empty())) {
        return true;
    }

    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    if (normalized_member_name == "release" &&
        !runtime_object.source.empty()) {
        return true;
    }

    if (normalized_member_name == "refresh" ||
        normalized_member_name == "resettodefault") {
        return true;
    }

    if (normalized_member_name == "move" &&
        is_native_visual_runtime_object(runtime_object)) {
        return true;
    }

    if ((normalized_member_name == "show" || normalized_member_name == "hide") &&
        is_native_visual_runtime_object(runtime_object)) {
        return true;
    }

    if (normalized_member_name == "setfocus" &&
        is_native_focusable_runtime_object(runtime_object)) {
        return true;
    }

    if (normalized_member_name == "additem" ||
        normalized_member_name == "addlistitem" ||
        normalized_member_name == "clear" ||
        normalized_member_name == "indextoitemid" ||
        normalized_member_name == "itemidtoindex" ||
        normalized_member_name == "moveitem" ||
        normalized_member_name == "removeitem" ||
        normalized_member_name == "removelistitem") {
        const std::string normalized_base_class =
            normalize_identifier(trim_copy(runtime_object.base_class_name));
        return normalized_base_class == "combobox" ||
               normalized_base_class == "listbox";
    }

    return false;
}

bool is_scripting_dictionary_object(const RuntimeOleObjectState& runtime_object) {
    return normalize_identifier(runtime_object.prog_id) == "scripting.dictionary";
}

bool is_native_list_control_runtime_object(const RuntimeOleObjectState& runtime_object) {
    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_list_control_disableditembackcolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_list_control_runtime_object(runtime_object);
}

bool native_list_control_disableditemforecolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_list_control_runtime_object(runtime_object);
}

bool native_list_control_itembackcolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_list_control_runtime_object(runtime_object);
}

bool native_list_control_itemforecolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_list_control_runtime_object(runtime_object);
}

bool native_list_control_selecteditembackcolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_list_control_runtime_object(runtime_object);
}

bool native_list_control_selecteditemforecolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return is_native_list_control_runtime_object(runtime_object);
}

bool is_native_listbox_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "listbox";
}

bool native_list_control_allows_multiple_selection(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_listbox_runtime_object(runtime_object)) {
        return false;
    }

    const auto multiselect = runtime_object.properties.find("multiselect");
    return multiselect != runtime_object.properties.end() &&
           value_as_bool(multiselect->second);
}

bool native_list_control_rowsourcetype_supports_additem(const RuntimeOleObjectState& runtime_object);
bool native_list_control_rowsourcetype_supports_clear(const RuntimeOleObjectState& runtime_object);

bool native_list_control_sorted_enabled(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object) ||
        !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
        return false;
    }

    const auto sorted = runtime_object.properties.find("sorted");
    return sorted != runtime_object.properties.end() &&
           value_as_bool(sorted->second);
}

std::optional<std::size_t> native_list_control_selected_slot(const RuntimeOleObjectState& runtime_object);

bool native_list_control_rowsourcetype_supports_additem(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    const auto rowsourcetype = runtime_object.properties.find("rowsourcetype");
    if (rowsourcetype == runtime_object.properties.end()) {
        return true;
    }

    const long long value = std::llround(value_as_number(rowsourcetype->second));
    return value == 0LL || value == 1LL;
}

bool native_list_control_rowsourcetype_supports_clear(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    const auto rowsourcetype = runtime_object.properties.find("rowsourcetype");
    if (rowsourcetype == runtime_object.properties.end()) {
        return true;
    }

    const long long value = std::llround(value_as_number(rowsourcetype->second));
    return value == 0LL;
}

void sync_native_list_control_selected_state_size(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const std::size_t row_count =
        runtime_object.list_rows.empty() ? runtime_object.collection_items.size()
                                         : runtime_object.list_rows.size();
    runtime_object.list_item_data.resize(row_count, make_number_value(0.0));
    runtime_object.list_selected.resize(row_count, false);
}

std::optional<std::size_t> find_last_native_list_control_selected_slot(
    const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    for (std::size_t index = runtime_object.list_selected.size(); index > 0U; --index) {
        if (runtime_object.list_selected[index - 1U]) {
            return index - 1U;
        }
    }
    return std::nullopt;
}

void sync_native_list_control_selected_state_from_listindex(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    sync_native_list_control_selected_state_size(runtime_object);

    if (!native_list_control_allows_multiple_selection(runtime_object)) {
        std::fill(runtime_object.list_selected.begin(), runtime_object.list_selected.end(), false);
    }

    if (const auto selected_slot = native_list_control_selected_slot(runtime_object);
        selected_slot.has_value() && *selected_slot < runtime_object.list_selected.size()) {
        runtime_object.list_selected[*selected_slot] = true;
    }
}

std::optional<std::size_t> native_list_control_selected_slot(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const auto listindex = runtime_object.properties.find("listindex");
    if (listindex == runtime_object.properties.end()) {
        return std::nullopt;
    }

    const long long selected_index = std::llround(value_as_number(listindex->second));
    if (selected_index < 1LL ||
        static_cast<std::size_t>(selected_index) > runtime_object.collection_items.size()) {
        return std::nullopt;
    }

    return static_cast<std::size_t>(selected_index - 1LL);
}

std::size_t native_list_control_bound_column(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return 1U;
    }

    const auto boundcolumn = runtime_object.properties.find("boundcolumn");
    if (boundcolumn == runtime_object.properties.end()) {
        return 1U;
    }

    const long long requested_column = std::llround(value_as_number(boundcolumn->second));
    return requested_column < 1LL ? 1U : static_cast<std::size_t>(requested_column);
}

bool native_list_control_boundto_enabled_impl(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    const auto boundto = runtime_object.properties.find("boundto");
    return boundto != runtime_object.properties.end() &&
           value_as_bool(boundto->second);
}

void update_native_list_control_boundto_index_value_mode_impl(
    RuntimeOleObjectState& runtime_object,
    bool was_boundto) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const bool boundto = native_list_control_boundto_enabled_impl(runtime_object);
    if (boundto) {
        runtime_object.boundto_index_value_mode = false;
    } else if (was_boundto) {
        runtime_object.boundto_index_value_mode = true;
    }
}

bool prg_value_kind_prefers_listindex(PrgValueKind kind) {
    return kind == PrgValueKind::number ||
           kind == PrgValueKind::int64 ||
           kind == PrgValueKind::uint64 ||
           kind == PrgValueKind::currency;
}

bool native_list_control_prefers_index_value(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object) ||
        native_list_control_boundto_enabled_impl(runtime_object)) {
        return false;
    }

    if (runtime_object.boundto_index_value_mode) {
        return true;
    }

    if (runtime_object.controlsource_value_kind_hint.has_value()) {
        return prg_value_kind_prefers_listindex(
            *runtime_object.controlsource_value_kind_hint);
    }

    const auto value = runtime_object.properties.find("value");
    if (value == runtime_object.properties.end()) {
        return false;
    }

    return prg_value_kind_prefers_listindex(value->second.kind);
}

PrgValue native_list_control_index_value_for_slot(const PrgValue& previous_value, std::size_t slot) {
    const auto index = static_cast<std::int64_t>(slot + 1U);
    if (previous_value.kind == PrgValueKind::uint64) {
        return make_uint64_value(static_cast<std::uint64_t>(index));
    }
    if (previous_value.kind == PrgValueKind::int64) {
        return make_int64_value(index);
    }
    if (previous_value.kind == PrgValueKind::currency) {
        return make_currency_value(index * 10000);
    }
    return make_number_value(static_cast<double>(index));
}

PrgValue native_list_control_empty_index_value(const PrgValue& previous_value) {
    if (previous_value.kind == PrgValueKind::uint64) {
        return make_uint64_value(0U);
    }
    if (previous_value.kind == PrgValueKind::int64) {
        return make_int64_value(0LL);
    }
    if (previous_value.kind == PrgValueKind::currency) {
        return make_currency_value(0);
    }
    return make_number_value(0.0);
}

void sync_native_list_control_displayvalue_from_selection_impl(RuntimeOleObjectState& runtime_object) {
    sync_native_list_control_selected_state_from_listindex(runtime_object);
    const auto previous_value = runtime_object.properties.find("value");
    const PrgValue previous_value_snapshot =
        previous_value == runtime_object.properties.end()
            ? make_string_value("")
            : previous_value->second;
    const bool prefer_index_value = native_list_control_prefers_index_value(runtime_object);

    if (const auto selected_slot = native_list_control_selected_slot(runtime_object);
        selected_slot.has_value()) {
        std::string display_text = value_as_string(runtime_object.collection_items[*selected_slot]);
        if (runtime_object.collection_items[*selected_slot].is_null) {
            if (const auto null_display = runtime_object.properties.find("nulldisplay");
                null_display != runtime_object.properties.end()) {
                display_text = value_as_string(null_display->second);
            }
        }
        runtime_object.properties["displayvalue"] =
            make_string_value(display_text);
        const std::size_t bound_column = native_list_control_bound_column(runtime_object);
        if (*selected_slot < runtime_object.list_rows.size() &&
            bound_column >= 1U &&
            bound_column <= runtime_object.list_rows[*selected_slot].size()) {
            runtime_object.properties["value"] = prefer_index_value
                                                     ? native_list_control_index_value_for_slot(
                                                           previous_value_snapshot,
                                                           *selected_slot)
                                                     : runtime_object.list_rows[*selected_slot][bound_column - 1U];
        } else {
            runtime_object.properties["value"] = prefer_index_value
                                                     ? native_list_control_empty_index_value(
                                                           previous_value_snapshot)
                                                     : make_string_value("");
        }
        if (*selected_slot < runtime_object.collection_item_keys.size()) {
            try {
                runtime_object.properties["listitemid"] = make_number_value(
                    static_cast<double>(std::stoll(runtime_object.collection_item_keys[*selected_slot])));
            } catch (const std::exception&) {
                runtime_object.properties["listitemid"] = make_number_value(0.0);
            }
        } else {
            runtime_object.properties["listitemid"] = make_number_value(0.0);
        }
        return;
    }

    runtime_object.properties["displayvalue"] = make_string_value("");
    runtime_object.properties["listitemid"] = make_number_value(0.0);
    if (prefer_index_value) {
        runtime_object.properties["value"] =
            native_list_control_empty_index_value(previous_value_snapshot);
    } else if (is_native_listbox_runtime_object(runtime_object)) {
        runtime_object.properties["value"] = make_string_value("");
    }
}

void sync_native_list_control_top_item_id_impl(RuntimeOleObjectState& runtime_object);

void sync_native_list_control_count_impl(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    runtime_object.properties["listcount"] =
        make_number_value(static_cast<double>(
            runtime_object.list_rows.empty() ? runtime_object.collection_items.size()
                                             : runtime_object.list_rows.size()));
}

void sync_native_list_control_primary_state_from_rows(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    runtime_object.collection_items.clear();
    runtime_object.collection_items.reserve(runtime_object.list_rows.size());
    for (const auto& row : runtime_object.list_rows) {
        runtime_object.collection_items.push_back(
            row.empty() ? make_string_value("") : row.front());
    }

    if (runtime_object.collection_item_keys.size() > runtime_object.list_rows.size()) {
        runtime_object.collection_item_keys.resize(runtime_object.list_rows.size());
    }
    sync_native_list_control_selected_state_size(runtime_object);
    sync_native_list_control_top_item_id_impl(runtime_object);
}

void materialize_native_list_control_rows(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    if (!runtime_object.list_rows.empty()) {
        sync_native_list_control_primary_state_from_rows(runtime_object);
        return;
    }

    runtime_object.list_rows.reserve(runtime_object.collection_items.size());
    for (const auto& item : runtime_object.collection_items) {
        runtime_object.list_rows.push_back({item});
    }
    sync_native_list_control_selected_state_size(runtime_object);
    sync_native_list_control_top_item_id_impl(runtime_object);
}

std::optional<std::size_t> find_native_list_control_row_by_item_id(
    const RuntimeOleObjectState& runtime_object,
    long long item_id) {
    if (!is_native_list_control_runtime_object(runtime_object) ||
        item_id < 1LL) {
        return std::nullopt;
    }

    const std::string expected_key = std::to_string(item_id);
    const auto found = std::find(runtime_object.collection_item_keys.begin(),
                                 runtime_object.collection_item_keys.end(),
                                 expected_key);
    if (found == runtime_object.collection_item_keys.end()) {
        return std::nullopt;
    }

    return static_cast<std::size_t>(
        std::distance(runtime_object.collection_item_keys.begin(), found));
}

void sync_native_list_control_top_item_id_impl(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    auto top_item_id = runtime_object.properties.find("topitemid");
    if (top_item_id == runtime_object.properties.end()) {
        top_item_id = runtime_object.properties.emplace(
            "topitemid",
            make_number_value(0.0)).first;
    }
    auto top_index = runtime_object.properties.find("topindex");
    if (top_index == runtime_object.properties.end()) {
        top_index = runtime_object.properties.emplace(
            "topindex",
            make_number_value(0.0)).first;
    }

    if (runtime_object.collection_item_keys.empty()) {
        top_item_id->second = make_number_value(0.0);
        top_index->second = make_number_value(0.0);
        return;
    }

    const long long current_item_id = std::llround(value_as_number(top_item_id->second));
    std::optional<std::size_t> top_slot;
    if (current_item_id >= 1LL) {
        top_slot = find_native_list_control_row_by_item_id(runtime_object, current_item_id);
    }
    if (!top_slot.has_value()) {
        try {
            const long long first_item_id = std::stoll(runtime_object.collection_item_keys.front());
            if (first_item_id >= 1LL) {
                top_item_id->second = make_number_value(static_cast<double>(first_item_id));
                top_slot = 0U;
            }
        } catch (const std::exception&) {
        }
    }
    if (top_slot.has_value()) {
        top_index->second = make_number_value(static_cast<double>(*top_slot + 1U));
    } else {
        top_item_id->second = make_number_value(0.0);
        top_index->second = make_number_value(0.0);
    }
}

std::int64_t next_native_list_control_item_id(const RuntimeOleObjectState& runtime_object) {
    std::int64_t next_id = 1;
    for (const std::string& key : runtime_object.collection_item_keys) {
        if (key.empty()) {
            continue;
        }
        try {
            next_id = std::max(next_id, static_cast<std::int64_t>(std::stoll(key) + 1LL));
        } catch (const std::exception&) {
        }
    }
    return next_id;
}

void sort_native_list_control_rows_if_needed(RuntimeOleObjectState& runtime_object) {
    if (!native_list_control_sorted_enabled(runtime_object)) {
        return;
    }

    materialize_native_list_control_rows(runtime_object);
    sync_native_list_control_selected_state_size(runtime_object);

    const auto active_slot = native_list_control_selected_slot(runtime_object);
    std::optional<std::size_t> latest_added_slot;
    if (const auto newindex = runtime_object.properties.find("newindex");
        newindex != runtime_object.properties.end()) {
        const long long requested_index = std::llround(value_as_number(newindex->second));
        if (requested_index >= 1LL &&
            static_cast<std::size_t>(requested_index) <= runtime_object.list_rows.size()) {
            latest_added_slot = static_cast<std::size_t>(requested_index - 1LL);
        }
    }

    struct NativeListControlSortEntry {
        std::vector<PrgValue> row;
        std::string item_key;
        PrgValue item_data = make_number_value(0.0);
        bool selected = false;
        bool active = false;
        bool latest_added = false;
    };

    std::vector<NativeListControlSortEntry> entries;
    entries.reserve(runtime_object.list_rows.size());
    for (std::size_t index = 0U; index < runtime_object.list_rows.size(); ++index) {
        entries.push_back(NativeListControlSortEntry{
            .row = runtime_object.list_rows[index],
            .item_key = index < runtime_object.collection_item_keys.size()
                            ? runtime_object.collection_item_keys[index]
                            : std::string{},
            .item_data = index < runtime_object.list_item_data.size()
                             ? runtime_object.list_item_data[index]
                             : make_number_value(0.0),
            .selected = index < runtime_object.list_selected.size() &&
                        runtime_object.list_selected[index],
            .active = active_slot.has_value() && *active_slot == index,
            .latest_added = latest_added_slot.has_value() && *latest_added_slot == index});
    }

    std::stable_sort(
        entries.begin(),
        entries.end(),
        [](const NativeListControlSortEntry& left, const NativeListControlSortEntry& right) {
            const std::string left_key =
                left.row.empty() ? std::string{} : value_as_string(left.row.front());
            const std::string right_key =
                right.row.empty() ? std::string{} : value_as_string(right.row.front());
            return left_key < right_key;
        });

    runtime_object.list_rows.clear();
    runtime_object.collection_item_keys.clear();
    runtime_object.list_item_data.clear();
    runtime_object.list_selected.clear();
    runtime_object.list_rows.reserve(entries.size());
    runtime_object.collection_item_keys.reserve(entries.size());
    runtime_object.list_selected.reserve(entries.size());

    bool active_found = false;
    bool latest_added_found = false;
    for (std::size_t index = 0U; index < entries.size(); ++index) {
        auto& entry = entries[index];
        runtime_object.list_rows.push_back(std::move(entry.row));
        runtime_object.collection_item_keys.push_back(std::move(entry.item_key));
        runtime_object.list_item_data.push_back(std::move(entry.item_data));
        runtime_object.list_selected.push_back(entry.selected);
        if (entry.active) {
            runtime_object.properties["listindex"] =
                make_number_value(static_cast<double>(index + 1U));
            active_found = true;
        }
        if (entry.latest_added) {
            runtime_object.properties["newindex"] =
                make_number_value(static_cast<double>(index + 1U));
            latest_added_found = true;
        }
    }

    if (!active_found) {
        runtime_object.properties["listindex"] = make_number_value(0.0);
    }
    if (!latest_added_found) {
        runtime_object.properties["newindex"] = make_number_value(0.0);
    }

    sync_native_list_control_primary_state_from_rows(runtime_object);
    sync_native_list_control_count_impl(runtime_object);
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
}

bool remove_native_list_control_slot(
    RuntimeOleObjectState& runtime_object,
    std::size_t slot) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);
    if (slot >= runtime_object.collection_items.size()) {
        return false;
    }

    if (slot < runtime_object.list_rows.size()) {
        runtime_object.list_rows.erase(
            runtime_object.list_rows.begin() + static_cast<std::ptrdiff_t>(slot));
    }
    if (slot < runtime_object.collection_item_keys.size()) {
        runtime_object.collection_item_keys.erase(
            runtime_object.collection_item_keys.begin() + static_cast<std::ptrdiff_t>(slot));
    }
    if (slot < runtime_object.list_item_data.size()) {
        runtime_object.list_item_data.erase(
            runtime_object.list_item_data.begin() + static_cast<std::ptrdiff_t>(slot));
    }
    if (slot < runtime_object.list_selected.size()) {
        runtime_object.list_selected.erase(
            runtime_object.list_selected.begin() + static_cast<std::ptrdiff_t>(slot));
    }
    sync_native_list_control_primary_state_from_rows(runtime_object);
    sync_native_list_control_count_impl(runtime_object);

    const auto listindex = runtime_object.properties.find("listindex");
    if (listindex != runtime_object.properties.end()) {
        long long selected_index = std::llround(value_as_number(listindex->second));
        const long long removed_index = static_cast<long long>(slot + 1U);
        const long long new_count = static_cast<long long>(runtime_object.collection_items.size());
        if (selected_index == removed_index) {
            if (new_count <= 0LL) {
                selected_index = 0LL;
            } else if (selected_index > new_count) {
                selected_index = new_count;
            }
        } else if (selected_index > removed_index) {
            --selected_index;
        } else if (selected_index > new_count) {
            selected_index = std::max(0LL, new_count);
        }
        listindex->second = make_number_value(static_cast<double>(selected_index));
    }

    const auto newindex = runtime_object.properties.find("newindex");
    if (newindex != runtime_object.properties.end()) {
        long long last_added_index = std::llround(value_as_number(newindex->second));
        const long long removed_index = static_cast<long long>(slot + 1U);
        const long long new_count = static_cast<long long>(runtime_object.collection_items.size());
        if (last_added_index == removed_index) {
            last_added_index = 0LL;
        } else if (last_added_index > removed_index) {
            --last_added_index;
        } else if (last_added_index > new_count) {
            last_added_index = std::max(0LL, new_count);
        }
        newindex->second = make_number_value(static_cast<double>(last_added_index));
    }

    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    return true;
}

void clear_native_list_control_rows(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto previous_value = runtime_object.properties.find("value");
    const PrgValue previous_value_snapshot =
        previous_value == runtime_object.properties.end()
            ? make_string_value("")
            : previous_value->second;
    const bool prefer_index_value = native_list_control_prefers_index_value(runtime_object);

    runtime_object.list_rows.clear();
    runtime_object.collection_items.clear();
    runtime_object.collection_item_keys.clear();
    runtime_object.list_item_data.clear();
    runtime_object.list_selected.clear();
    runtime_object.properties["listindex"] = make_number_value(0.0);
    runtime_object.properties["newindex"] = make_number_value(0.0);
    runtime_object.properties["newitemid"] = make_number_value(0.0);
    runtime_object.properties["topitemid"] = make_number_value(0.0);
    runtime_object.properties["topindex"] = make_number_value(0.0);
    sync_native_list_control_count_impl(runtime_object);
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    runtime_object.properties["value"] = prefer_index_value
                                             ? native_list_control_empty_index_value(
                                                   previous_value_snapshot)
                                             : make_string_value("");
}
