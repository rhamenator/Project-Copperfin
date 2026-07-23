// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

std::optional<PrgValue> read_native_collection_member(RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    if (!is_native_collection_object(runtime_object) || normalized_member_name != "count") {
        return std::nullopt;
    }
    sync_native_collection_count(runtime_object);
    return runtime_object.properties["count"];
}

std::optional<PrgValue> invoke_native_collection_method(RuntimeOleObjectState& runtime_object,
                                                        const std::string& normalized_method_name,
                                                        const std::vector<PrgValue>& arguments)
{
    if (!is_native_collection_object(runtime_object)) {
        return std::nullopt;
    }

    if (runtime_object.read_only_collection_surface &&
        normalized_method_name != "item") {
        return make_boolean_value(false);
    }

    if (normalized_method_name == "add" && !arguments.empty()) {
        const std::string key =
            arguments.size() >= 2U ? normalize_identifier(trim_copy(value_as_string(arguments[1]))) : std::string{};
        if (!key.empty()) {
            const auto existing = std::find(runtime_object.collection_item_keys.begin(),
                                            runtime_object.collection_item_keys.end(),
                                            key);
            if (existing != runtime_object.collection_item_keys.end()) {
                throw std::runtime_error(
                    runtime_text("Runtime.Prg.RuntimeSurface.Error.CollectionKeyNotUnique"));
            } else {
                runtime_object.collection_items.push_back(arguments[0]);
                runtime_object.collection_item_keys.push_back(key);
            }
        } else {
            runtime_object.collection_items.push_back(arguments[0]);
            runtime_object.collection_item_keys.push_back({});
        }
        sync_native_collection_count(runtime_object);
        return make_boolean_value(true);
    }

    if (normalized_method_name == "item" && !arguments.empty()) {
        const auto slot = resolve_native_collection_slot(runtime_object, arguments[0]);
        return slot.has_value() ? runtime_object.collection_items[*slot] : make_empty_value();
    }

    if (normalized_method_name == "remove" && !arguments.empty()) {
        const auto slot = resolve_native_collection_slot(runtime_object, arguments[0]);
        if (!slot.has_value()) {
            return make_boolean_value(false);
        }
        runtime_object.collection_items.erase(runtime_object.collection_items.begin() + static_cast<std::ptrdiff_t>(*slot));
        runtime_object.collection_item_keys.erase(runtime_object.collection_item_keys.begin() + static_cast<std::ptrdiff_t>(*slot));
        sync_native_collection_count(runtime_object);
        return make_boolean_value(true);
    }

    if (normalized_method_name == "removeall") {
        runtime_object.collection_items.clear();
        runtime_object.collection_item_keys.clear();
        sync_native_collection_count(runtime_object);
        return make_boolean_value(true);
    }

    return std::nullopt;
}

std::optional<PrgValue> invoke_native_list_control_method(RuntimeOleObjectState& runtime_object,
                                                          const std::string& normalized_method_name,
                                                          const std::vector<PrgValue>& arguments,
                                                          const std::function<bool(const std::vector<PrgValue>&)>& before_move)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    materialize_native_list_control_rows(runtime_object);

    if (normalized_method_name == "list") {
        if (arguments.empty()) {
            return make_string_value("");
        }

        const long long requested_index = std::llround(value_as_number(arguments[0]));
        if (requested_index < 1LL ||
            static_cast<std::size_t>(requested_index) > runtime_object.list_rows.size()) {
            return make_string_value("");
        }

        long long requested_column = 1LL;
        if (arguments.size() >= 2U) {
            requested_column = std::llround(value_as_number(arguments[1]));
            if (requested_column < 1LL) {
                return make_string_value("");
            }
        }

        const auto& row = runtime_object.list_rows[static_cast<std::size_t>(requested_index - 1LL)];
        if (static_cast<std::size_t>(requested_column) > row.size()) {
            return make_string_value("");
        }
        return make_string_value(
            value_as_string(row[static_cast<std::size_t>(requested_column - 1LL)]));
    }

    if (normalized_method_name == "itemdata") {
        if (arguments.empty()) {
            return make_number_value(0.0);
        }
        sync_native_list_control_selected_state_size(runtime_object);
        const long long requested_index = std::llround(value_as_number(arguments[0]));
        if (requested_index < 1LL ||
            static_cast<std::size_t>(requested_index) > runtime_object.list_item_data.size()) {
            return make_number_value(0.0);
        }
        return runtime_object.list_item_data[static_cast<std::size_t>(requested_index - 1LL)];
    }

    if (normalized_method_name == "selected") {
        if (arguments.empty()) {
            return make_boolean_value(false);
        }

        sync_native_list_control_selected_state_size(runtime_object);
        const long long requested_index = std::llround(value_as_number(arguments[0]));
        if (requested_index < 1LL ||
            static_cast<std::size_t>(requested_index) > runtime_object.list_selected.size()) {
            return make_boolean_value(false);
        }

        return make_boolean_value(
            runtime_object.list_selected[static_cast<std::size_t>(requested_index - 1LL)]);
    }

    if (normalized_method_name == "selectedid") {
        if (arguments.empty()) {
            return make_boolean_value(false);
        }

        sync_native_list_control_selected_state_size(runtime_object);
        const long long requested_item_id = std::llround(value_as_number(arguments[0]));
        if (requested_item_id < 1LL) {
            return make_boolean_value(false);
        }

        const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
        if (!slot.has_value()) {
            return make_boolean_value(false);
        }

        return make_boolean_value(runtime_object.list_selected[*slot]);
    }

    if (normalized_method_name == "indextoitemid") {
        if (arguments.empty()) {
            return make_number_value(0.0);
        }

        materialize_native_list_control_rows(runtime_object);
        const long long requested_index = std::llround(value_as_number(arguments[0]));
        if (requested_index < 1LL ||
            static_cast<std::size_t>(requested_index) > runtime_object.collection_item_keys.size()) {
            return make_number_value(0.0);
        }

        const std::string& item_key =
            runtime_object.collection_item_keys[static_cast<std::size_t>(requested_index - 1LL)];
        try {
            return make_number_value(static_cast<double>(std::stoll(item_key)));
        } catch (const std::exception&) {
            return make_number_value(0.0);
        }
    }

    if (normalized_method_name == "itemidtoindex") {
        if (arguments.empty()) {
            return make_number_value(0.0);
        }

        materialize_native_list_control_rows(runtime_object);
        const long long requested_item_id = std::llround(value_as_number(arguments[0]));
        if (requested_item_id < 1LL) {
            return make_number_value(0.0);
        }

        const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
        return make_number_value(
            slot.has_value() ? static_cast<double>(*slot + 1U) : 0.0);
    }

    if (normalized_method_name == "listitem") {
        if (arguments.empty()) {
            return make_string_value("");
        }

        materialize_native_list_control_rows(runtime_object);
        const long long requested_item_id = std::llround(value_as_number(arguments[0]));
        if (requested_item_id < 1LL) {
            return make_string_value("");
        }

        long long requested_column = 1LL;
        if (arguments.size() >= 2U) {
            requested_column = std::llround(value_as_number(arguments[1]));
            if (requested_column < 1LL) {
                return make_string_value("");
            }
        }

        const auto item_value = read_native_list_control_item_cell(
            runtime_object,
            requested_item_id,
            static_cast<std::size_t>(requested_column - 1LL));
        return item_value.value_or(make_string_value(""));
    }

    if (normalized_method_name == "addlistitem") {
        if (arguments.empty() ||
            !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
            return make_number_value(0.0);
        }

        long long requested_column = 1LL;
        if (arguments.size() >= 3U) {
            requested_column = std::llround(value_as_number(arguments[2]));
            if (requested_column < 1LL) {
                return make_number_value(0.0);
            }
        }

        const PrgValue inserted_item = make_string_value(value_as_string(arguments[0]));

        if (requested_column == 1LL) {
            long long item_id = next_native_list_control_item_id(runtime_object);
            if (arguments.size() >= 2U) {
                item_id = std::llround(value_as_number(arguments[1]));
                if (item_id < 1LL ||
                    find_native_list_control_row_by_item_id(runtime_object, item_id).has_value()) {
                    return make_number_value(0.0);
                }
            }

            runtime_object.list_rows.push_back({inserted_item});
            runtime_object.collection_item_keys.push_back(std::to_string(item_id));
            runtime_object.list_selected.push_back(false);
            sync_native_list_control_primary_state_from_rows(runtime_object);
            sync_native_list_control_count_impl(runtime_object);
            runtime_object.properties["newindex"] =
                make_number_value(static_cast<double>(runtime_object.list_rows.size()));
            runtime_object.properties["newitemid"] = make_number_value(static_cast<double>(item_id));
            sort_native_list_control_rows_if_needed(runtime_object);
            return make_number_value(static_cast<double>(item_id));
        }

        if (arguments.size() < 2U) {
            return make_number_value(0.0);
        }

        const long long item_id = std::llround(value_as_number(arguments[1]));
        const auto slot = find_native_list_control_row_by_item_id(runtime_object, item_id);
        if (!slot.has_value()) {
            return make_number_value(0.0);
        }

        auto& row = runtime_object.list_rows[*slot];
        if (row.size() < static_cast<std::size_t>(requested_column)) {
            row.resize(static_cast<std::size_t>(requested_column), make_string_value(""));
        }
        row[static_cast<std::size_t>(requested_column - 1LL)] = inserted_item;
        sync_native_list_control_primary_state_from_rows(runtime_object);
        if (requested_column == 1LL) {
            sort_native_list_control_rows_if_needed(runtime_object);
        }
        sync_native_list_control_count_impl(runtime_object);
        runtime_object.properties["newitemid"] = make_number_value(static_cast<double>(item_id));
        sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
        return make_number_value(static_cast<double>(item_id));
    }

    if (normalized_method_name == "clear") {
        if (!native_list_control_rowsourcetype_supports_clear(runtime_object)) {
            return make_empty_value();
        }

        clear_native_list_control_rows(runtime_object);
        return make_empty_value();
    }

    if (normalized_method_name == "removeitem") {
        if (arguments.empty() ||
            !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
            return make_empty_value();
        }

        const long long requested_index = std::llround(value_as_number(arguments[0]));
        if (requested_index < 1LL ||
            static_cast<std::size_t>(requested_index) > runtime_object.collection_items.size()) {
            return make_empty_value();
        }
        if (!remove_native_list_control_slot(
                runtime_object,
                static_cast<std::size_t>(requested_index - 1LL))) {
            return make_empty_value();
        }
        return make_empty_value();
    }

    if (normalized_method_name == "removelistitem") {
        if (arguments.empty() ||
            !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
            return make_empty_value();
        }

        const long long requested_item_id = std::llround(value_as_number(arguments[0]));
        if (requested_item_id < 1LL) {
            return make_empty_value();
        }

        const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
        if (!slot.has_value() ||
            !remove_native_list_control_slot(runtime_object, *slot)) {
            return make_empty_value();
        }
        return make_empty_value();
    }

    if (normalized_method_name == "moveitem") {
        if (!is_native_listbox_runtime_object(runtime_object) || arguments.size() < 2U) {
            return make_empty_value();
        }

        const std::vector<PrgValue> event_arguments = {
            make_number_value(8.0),
            make_number_value(0.0),
            arguments[0],
            arguments[1]};
        if (before_move && !before_move(event_arguments)) {
            return make_empty_value();
        }
        if (!native_list_control_rowsourcetype_supports_additem(runtime_object)) {
            return make_empty_value();
        }

        materialize_native_list_control_rows(runtime_object);
        runtime_object.collection_item_keys.resize(runtime_object.list_rows.size());
        std::set<long long> used_item_ids;
        for (const std::string& key : runtime_object.collection_item_keys) {
            try {
                const long long item_id = std::stoll(key);
                if (item_id >= 1LL) {
                    used_item_ids.insert(item_id);
                }
            } catch (const std::exception&) {
            }
        }
        long long next_item_id = 1LL;
        for (std::string& key : runtime_object.collection_item_keys) {
            if (!key.empty()) {
                continue;
            }
            while (used_item_ids.contains(next_item_id)) {
                ++next_item_id;
            }
            key = std::to_string(next_item_id);
            used_item_ids.insert(next_item_id);
            ++next_item_id;
        }
        const double raw_index = value_as_number(arguments[0]);
        const double raw_move_by = value_as_number(arguments[1]);
        if (!std::isfinite(raw_index) ||
            !std::isfinite(raw_move_by) ||
            raw_index < 1.0 ||
            raw_index > static_cast<double>(runtime_object.list_rows.size()) ||
            raw_move_by <= static_cast<double>(std::numeric_limits<long long>::min()) ||
            raw_move_by >= static_cast<double>(std::numeric_limits<long long>::max())) {
            return make_empty_value();
        }
        const long long requested_index = std::llround(raw_index);
        const long long move_by = std::llround(raw_move_by);

        const std::size_t source_slot = static_cast<std::size_t>(requested_index - 1LL);
        const long long unclamped_target = static_cast<long long>(source_slot) + move_by;
        const long long last_slot = static_cast<long long>(runtime_object.list_rows.size() - 1U);
        const std::size_t target_slot = static_cast<std::size_t>(std::max(
            0LL,
            std::min(last_slot, unclamped_target)));

        std::optional<std::string> active_item_key;
        if (const auto listindex = runtime_object.properties.find("listindex");
            listindex != runtime_object.properties.end()) {
            const long long active_index = std::llround(value_as_number(listindex->second));
            if (active_index >= 1LL &&
                static_cast<std::size_t>(active_index) <= runtime_object.collection_item_keys.size()) {
                active_item_key = runtime_object.collection_item_keys[static_cast<std::size_t>(active_index - 1LL)];
            }
        }
        std::optional<std::string> new_item_key;
        if (const auto newindex = runtime_object.properties.find("newindex");
            newindex != runtime_object.properties.end()) {
            const long long new_item_index = std::llround(value_as_number(newindex->second));
            if (new_item_index >= 1LL &&
                static_cast<std::size_t>(new_item_index) <= runtime_object.collection_item_keys.size()) {
                new_item_key = runtime_object.collection_item_keys[static_cast<std::size_t>(new_item_index - 1LL)];
            }
        }

        if (source_slot != target_slot) {
            auto move_parallel_entry = [&](auto& values) {
                auto moved = values[source_slot];
                values.erase(values.begin() + static_cast<std::ptrdiff_t>(source_slot));
                values.insert(values.begin() + static_cast<std::ptrdiff_t>(target_slot), std::move(moved));
            };
            move_parallel_entry(runtime_object.list_rows);
            move_parallel_entry(runtime_object.collection_item_keys);
            move_parallel_entry(runtime_object.list_item_data);
            move_parallel_entry(runtime_object.list_selected);
        }

        sync_native_list_control_primary_state_from_rows(runtime_object);
        sync_native_list_control_count_impl(runtime_object);
        const auto set_index_for_key = [&](const std::optional<std::string>& item_key,
                                           const char* property_name) {
            if (!item_key.has_value()) {
                return;
            }
            const auto slot = std::find(
                runtime_object.collection_item_keys.begin(),
                runtime_object.collection_item_keys.end(),
                *item_key);
            if (slot != runtime_object.collection_item_keys.end()) {
                runtime_object.properties[property_name] = make_number_value(
                    static_cast<double>(std::distance(runtime_object.collection_item_keys.begin(), slot) + 1U));
            }
        };
        set_index_for_key(active_item_key, "listindex");
        set_index_for_key(new_item_key, "newindex");
        sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
        return make_empty_value();
    }

    if (normalized_method_name != "additem") {
        return std::nullopt;
    }

    if (arguments.empty() ||
        !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
        return make_number_value(0.0);
    }

    long long requested_column = 1LL;
    if (arguments.size() >= 3U) {
        requested_column = std::llround(value_as_number(arguments[2]));
        if (requested_column < 1LL) {
            return make_number_value(0.0);
        }
    }

    const auto insert_slot = resolve_native_list_control_insert_slot(runtime_object, arguments);
    if (!insert_slot.has_value()) {
        return make_number_value(0.0);
    }

    const PrgValue inserted_item = make_string_value(value_as_string(arguments[0]));
    const std::int64_t item_id = next_native_list_control_item_id(runtime_object);
    std::vector<PrgValue> inserted_row;
    inserted_row.resize(static_cast<std::size_t>(requested_column), make_string_value(""));
    inserted_row[static_cast<std::size_t>(requested_column - 1LL)] = inserted_item;
    runtime_object.list_rows.insert(
        runtime_object.list_rows.begin() + static_cast<std::ptrdiff_t>(*insert_slot),
        std::move(inserted_row));
    runtime_object.collection_item_keys.insert(
        runtime_object.collection_item_keys.begin() + static_cast<std::ptrdiff_t>(*insert_slot),
        std::to_string(item_id));
    runtime_object.list_selected.insert(
        runtime_object.list_selected.begin() + static_cast<std::ptrdiff_t>(*insert_slot),
        false);
    runtime_object.list_item_data.insert(
        runtime_object.list_item_data.begin() + static_cast<std::ptrdiff_t>(*insert_slot),
        make_number_value(0.0));
    sync_native_list_control_primary_state_from_rows(runtime_object);
    sync_native_list_control_count_impl(runtime_object);
    runtime_object.properties["newindex"] =
        make_number_value(static_cast<double>(*insert_slot + 1U));
    runtime_object.properties["newitemid"] = make_number_value(static_cast<double>(item_id));

    const auto listindex = runtime_object.properties.find("listindex");
    if (listindex != runtime_object.properties.end()) {
        const long long selected_index = std::llround(value_as_number(listindex->second));
        if (selected_index >= 1LL &&
            static_cast<std::size_t>(selected_index) >= *insert_slot + 1U) {
            listindex->second = make_number_value(static_cast<double>(selected_index + 1LL));
        }
    }
    sort_native_list_control_rows_if_needed(runtime_object);
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    if (const auto actual_slot = find_native_list_control_row_by_item_id(runtime_object, item_id);
        actual_slot.has_value()) {
        runtime_object.properties["newindex"] =
            make_number_value(static_cast<double>(*actual_slot + 1U));
        return make_number_value(static_cast<double>(*actual_slot + 1U));
    }
    return make_number_value(static_cast<double>(*insert_slot + 1U));
}

std::optional<NativeListControlCellReference> parse_native_list_control_list_member_cell(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_list_member_cell_impl(runtime_object, member_name);
}

std::optional<NativeListControlItemCellReference> parse_native_list_control_listitem_member_cell(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_listitem_member_cell_impl(runtime_object, member_name);
}

std::optional<PrgValue> read_native_list_control_item_data(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }
    materialize_native_list_control_rows(runtime_object);
    sync_native_list_control_selected_state_size(runtime_object);
    if (row_slot >= runtime_object.list_item_data.size()) {
        return make_number_value(0.0);
    }
    return runtime_object.list_item_data[row_slot];
}

std::optional<PrgValue> read_native_list_control_cell(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot,
    std::size_t column_slot)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    materialize_native_list_control_rows(runtime_object);
    if (row_slot >= runtime_object.list_rows.size()) {
        return make_string_value("");
    }

    const auto& row = runtime_object.list_rows[row_slot];
    if (column_slot >= row.size()) {
        return make_string_value("");
    }

    return make_string_value(value_as_string(row[column_slot]));
}

std::optional<PrgValue> read_native_list_control_item_cell(
    RuntimeOleObjectState& runtime_object,
    long long item_id,
    std::size_t column_slot)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    materialize_native_list_control_rows(runtime_object);
    const auto row_slot = find_native_list_control_row_by_item_id(runtime_object, item_id);
    if (!row_slot.has_value()) {
        return make_string_value("");
    }

    return read_native_list_control_cell(runtime_object, *row_slot, column_slot);
}

std::optional<PrgValue> read_native_list_control_item_id_for_slot(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    materialize_native_list_control_rows(runtime_object);
    if (row_slot >= runtime_object.collection_item_keys.size()) {
        return make_number_value(0.0);
    }

    try {
        return make_number_value(
            static_cast<double>(std::stoll(runtime_object.collection_item_keys[row_slot])));
    } catch (const std::exception&) {
        return make_number_value(0.0);
    }
}

std::optional<PrgValue> read_native_list_control_index_for_item_id(
    RuntimeOleObjectState& runtime_object,
    long long item_id)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    materialize_native_list_control_rows(runtime_object);
    if (item_id < 1LL) {
        return make_number_value(0.0);
    }

    const auto row_slot = find_native_list_control_row_by_item_id(runtime_object, item_id);
    return make_number_value(row_slot.has_value() ? static_cast<double>(*row_slot + 1U) : 0.0);
}

std::optional<PrgValue> read_native_identity_metadata(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    // Ordinary dotted reads intentionally trail reflection parity for metadata we have not widened yet.
    if (normalized_member_name != "hwnd" &&
        normalized_member_name != "class" &&
        normalized_member_name != "baseclass" &&
        normalized_member_name != "parentclass" &&
        normalized_member_name != "classlibrary")
    {
        return std::nullopt;
    }
    return get_native_identity_reflection_metadata(runtime_object, normalized_member_name);
}
