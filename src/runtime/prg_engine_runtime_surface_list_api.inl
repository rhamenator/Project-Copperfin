// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

std::optional<std::size_t> parse_native_list_control_selected_member_slot(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_selected_member_slot_impl(runtime_object, member_name);
}

std::optional<long long> parse_native_list_control_selectedid_member_item_id(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_selectedid_member_item_id_impl(runtime_object, member_name);
}

std::optional<std::size_t> parse_native_list_control_itemdata_member_slot(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_itemdata_member_slot_impl(runtime_object, member_name);
}

std::optional<std::size_t> parse_native_list_control_indextoitemid_member_slot(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_indextoitemid_member_slot_impl(runtime_object, member_name);
}

std::optional<long long> parse_native_list_control_itemidtoindex_member_item_id(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_itemidtoindex_member_item_id_impl(runtime_object, member_name);
}

void refresh_native_list_control_controlsource_value_kind_hint(
    RuntimeOleObjectState& runtime_object,
    const RuntimeControlSourceValueResolver& resolver)
{
    runtime_object.controlsource_value_kind_hint.reset();
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto controlsource = runtime_object.properties.find("controlsource");
    if (controlsource == runtime_object.properties.end()) {
        return;
    }

    const std::string controlsource_text = trim_copy(value_as_string(controlsource->second));
    if (controlsource_text.empty() || !resolver) {
        return;
    }

    const auto resolved_value = resolver(controlsource_text);
    if (!resolved_value.has_value()) {
        return;
    }

    runtime_object.controlsource_value_kind_hint = resolved_value->kind;
}

void sync_native_list_control_displayvalue_from_selection(RuntimeOleObjectState& runtime_object)
{
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
}

void sync_native_list_control_count(RuntimeOleObjectState& runtime_object)
{
    sync_native_list_control_count_impl(runtime_object);
}

void sync_native_list_control_top_item_id(RuntimeOleObjectState& runtime_object)
{
    sync_native_list_control_top_item_id_impl(runtime_object);
}

bool write_native_list_control_item_id(RuntimeOleObjectState& runtime_object, const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);

    const long long requested_item_id = std::llround(value_as_number(assigned_value));
    auto listindex = runtime_object.properties.find("listindex");
    if (listindex == runtime_object.properties.end()) {
        runtime_object.properties["listindex"] = make_number_value(0.0);
        listindex = runtime_object.properties.find("listindex");
    }

    if (requested_item_id == 0LL) {
        listindex->second = make_number_value(0.0);
        sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
        return true;
    }

    if (requested_item_id < 0LL) {
        return false;
    }

    const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
    if (!slot.has_value()) {
        return false;
    }

    listindex->second = make_number_value(static_cast<double>(*slot + 1U));
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    return true;
}

bool write_native_list_control_cell(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot,
    std::size_t column_slot,
    const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);
    if (row_slot >= runtime_object.list_rows.size()) {
        return false;
    }

    auto& row = runtime_object.list_rows[row_slot];
    if (row.size() <= column_slot) {
        row.resize(column_slot + 1U, make_string_value(""));
    }
    row[column_slot] = make_string_value(value_as_string(assigned_value));
    sync_native_list_control_primary_state_from_rows(runtime_object);
    if (column_slot == 0U) {
        sort_native_list_control_rows_if_needed(runtime_object);
    }
    sync_native_list_control_count_impl(runtime_object);
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    return true;
}

bool write_native_list_control_item_cell(
    RuntimeOleObjectState& runtime_object,
    long long item_id,
    std::size_t column_slot,
    const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);
    const auto row_slot = find_native_list_control_row_by_item_id(runtime_object, item_id);
    if (!row_slot.has_value()) {
        return false;
    }

    return write_native_list_control_cell(runtime_object, *row_slot, column_slot, assigned_value);
}

bool write_native_list_control_item_data(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot,
    const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }
    materialize_native_list_control_rows(runtime_object);
    sync_native_list_control_selected_state_size(runtime_object);
    if (row_slot >= runtime_object.list_item_data.size()) {
        return false;
    }
    runtime_object.list_item_data[row_slot] = make_number_value(value_as_number(assigned_value));
    return true;
}

bool write_native_list_control_top_item_id(
    RuntimeOleObjectState& runtime_object,
    const PrgValue& assigned_value)
{
    if (!is_native_listbox_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);
    const long long requested_item_id = std::llround(value_as_number(assigned_value));
    if (requested_item_id < 1LL ||
        !find_native_list_control_row_by_item_id(runtime_object, requested_item_id).has_value()) {
        return false;
    }

    runtime_object.properties["topitemid"] = make_number_value(
        static_cast<double>(requested_item_id));
    return true;
}

bool write_native_list_control_selected_slot(
    RuntimeOleObjectState& runtime_object,
    std::size_t slot,
    const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);
    sync_native_list_control_selected_state_size(runtime_object);
    if (slot >= runtime_object.list_selected.size()) {
        return false;
    }

    const bool selected = value_as_bool(assigned_value);
    if (selected && !native_list_control_allows_multiple_selection(runtime_object)) {
        std::fill(runtime_object.list_selected.begin(), runtime_object.list_selected.end(), false);
    }
    runtime_object.list_selected[slot] = selected;

    auto listindex = runtime_object.properties.find("listindex");
    if (listindex == runtime_object.properties.end()) {
        runtime_object.properties["listindex"] = make_number_value(0.0);
        listindex = runtime_object.properties.find("listindex");
    }

    if (selected) {
        listindex->second = make_number_value(static_cast<double>(slot + 1U));
    } else if (std::llround(value_as_number(listindex->second)) == static_cast<long long>(slot + 1U)) {
        // In a multi-select ListBox, ListIndex is the focused row even when
        // that row is no longer selected. Single-selection controls retain
        // the existing deterministic fallback to another selected row.
        if (!native_list_control_allows_multiple_selection(runtime_object)) {
            if (const auto replacement = find_last_native_list_control_selected_slot(runtime_object);
                replacement.has_value()) {
                listindex->second = make_number_value(static_cast<double>(*replacement + 1U));
            } else {
                listindex->second = make_number_value(0.0);
            }
        }
    }

    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    if (!selected && native_list_control_allows_multiple_selection(runtime_object) &&
        slot < runtime_object.list_selected.size()) {
        // Display synchronization keeps the focused row's derived value
        // current; it must not undo an explicit multi-select deselection.
        runtime_object.list_selected[slot] = false;
    }
    return true;
}

bool write_native_list_control_selected_item_id(
    RuntimeOleObjectState& runtime_object,
    long long requested_item_id,
    const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object) ||
        requested_item_id < 1LL) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);

    const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
    if (!slot.has_value()) {
        return false;
    }

    return write_native_list_control_selected_slot(runtime_object, *slot, assigned_value);
}
