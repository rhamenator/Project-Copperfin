// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

std::optional<std::size_t> parse_native_list_control_selected_member_slot_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const auto extract_member_selector_text =
        [](const std::string& candidate, const std::string& base_name) -> std::optional<std::string>
    {
        const std::string trimmed = trim_copy(candidate);
        const std::string normalized = lowercase_copy(trimmed);
        const auto extract_for_delimiters =
            [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
        {
            std::string prefix = base_name;
            prefix.push_back(open_delimiter);
            if (!starts_with_insensitive(normalized, prefix) ||
                normalized.empty() ||
                normalized.back() != close_delimiter)
            {
                return std::nullopt;
            }

            const std::size_t open = trimmed.find(open_delimiter);
            const std::size_t close = trimmed.rfind(close_delimiter);
            if (open == std::string::npos ||
                close == std::string::npos ||
                close <= open + 1U)
            {
                return std::nullopt;
            }
            return trim_copy(trimmed.substr(open + 1U, close - open - 1U));
        };

        if (const auto parenthesized = extract_for_delimiters('(', ')');
            parenthesized.has_value())
        {
            return parenthesized;
        }
        return extract_for_delimiters('[', ']');
    };

    const auto selector_text = extract_member_selector_text(member_name, "selected");
    if (!selector_text.has_value() || selector_text->empty()) {
        return std::nullopt;
    }

    try {
        const long long requested_index = std::stoll(*selector_text);
        if (requested_index < 1LL) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(requested_index - 1LL);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}
std::optional<NativeListControlCellReference> parse_native_list_control_list_member_cell_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const auto extract_member_selector_text =
        [](const std::string& candidate, const std::string& base_name) -> std::optional<std::string>
    {
        const std::string trimmed = trim_copy(candidate);
        const std::string normalized = lowercase_copy(trimmed);
        const auto extract_for_delimiters =
            [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
        {
            std::string prefix = base_name;
            prefix.push_back(open_delimiter);
            if (!starts_with_insensitive(normalized, prefix) ||
                normalized.empty() ||
                normalized.back() != close_delimiter)
            {
                return std::nullopt;
            }

            const std::size_t open = trimmed.find(open_delimiter);
            const std::size_t close = trimmed.rfind(close_delimiter);
            if (open == std::string::npos ||
                close == std::string::npos ||
                close <= open + 1U)
            {
                return std::nullopt;
            }
            return trim_copy(trimmed.substr(open + 1U, close - open - 1U));
        };

        if (const auto parenthesized = extract_for_delimiters('(', ')');
            parenthesized.has_value())
        {
            return parenthesized;
        }
        return extract_for_delimiters('[', ']');
    };

    const auto selector_text = extract_member_selector_text(member_name, "list");
    if (!selector_text.has_value() || selector_text->empty()) {
        return std::nullopt;
    }

    const std::size_t comma = selector_text->find(',');
    const std::string row_text = trim_copy(selector_text->substr(0U, comma));
    const std::string column_text =
        comma == std::string::npos ? std::string("1") : trim_copy(selector_text->substr(comma + 1U));
    if (row_text.empty() || column_text.empty()) {
        return std::nullopt;
    }

    try {
        const long long requested_row = std::stoll(row_text);
        const long long requested_column = std::stoll(column_text);
        if (requested_row < 1LL || requested_column < 1LL) {
            return std::nullopt;
        }
        return NativeListControlCellReference{
            .row_slot = static_cast<std::size_t>(requested_row - 1LL),
            .column_slot = static_cast<std::size_t>(requested_column - 1LL)};
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<NativeListControlItemCellReference> parse_native_list_control_listitem_member_cell_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const auto extract_member_selector_text =
        [](const std::string& candidate, const std::string& base_name) -> std::optional<std::string>
    {
        const std::string trimmed = trim_copy(candidate);
        const std::string normalized = lowercase_copy(trimmed);
        const auto extract_for_delimiters =
            [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
        {
            std::string prefix = base_name;
            prefix.push_back(open_delimiter);
            if (!starts_with_insensitive(normalized, prefix) ||
                normalized.empty() ||
                normalized.back() != close_delimiter)
            {
                return std::nullopt;
            }

            const std::size_t open = trimmed.find(open_delimiter);
            const std::size_t close = trimmed.rfind(close_delimiter);
            if (open == std::string::npos ||
                close == std::string::npos ||
                close <= open + 1U)
            {
                return std::nullopt;
            }
            return trim_copy(trimmed.substr(open + 1U, close - open - 1U));
        };

        if (const auto parenthesized = extract_for_delimiters('(', ')');
            parenthesized.has_value())
        {
            return parenthesized;
        }
        return extract_for_delimiters('[', ']');
    };

    const auto selector_text = extract_member_selector_text(member_name, "listitem");
    if (!selector_text.has_value() || selector_text->empty()) {
        return std::nullopt;
    }

    const std::size_t comma = selector_text->find(',');
    const std::string item_id_text = trim_copy(selector_text->substr(0U, comma));
    const std::string column_text =
        comma == std::string::npos ? std::string("1") : trim_copy(selector_text->substr(comma + 1U));
    if (item_id_text.empty() || column_text.empty()) {
        return std::nullopt;
    }

    try {
        const long long requested_item_id = std::stoll(item_id_text);
        const long long requested_column = std::stoll(column_text);
        if (requested_item_id < 1LL || requested_column < 1LL) {
            return std::nullopt;
        }
        return NativeListControlItemCellReference{
            .item_id = requested_item_id,
            .column_slot = static_cast<std::size_t>(requested_column - 1LL)};
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::size_t> parse_native_list_control_itemdata_member_slot_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const std::string trimmed = trim_copy(member_name);
    const std::string normalized = lowercase_copy(trimmed);
    std::optional<std::string> selector_text;
    for (const auto delimiters : {std::pair<char, char>{'(', ')'}, {'[', ']'}}) {
        std::string prefix = "itemdata";
        prefix.push_back(delimiters.first);
        if (!starts_with_insensitive(normalized, prefix) ||
            normalized.empty() || normalized.back() != delimiters.second) {
            continue;
        }
        const std::size_t open = trimmed.find(delimiters.first);
        const std::size_t close = trimmed.rfind(delimiters.second);
        if (open == std::string::npos || close <= open + 1U) {
            return std::nullopt;
        }
        selector_text = trim_copy(trimmed.substr(open + 1U, close - open - 1U));
        break;
    }
    if (!selector_text.has_value() || selector_text->empty()) {
        return std::nullopt;
    }

    try {
        const long long requested_index = std::stoll(*selector_text);
        if (requested_index < 1LL) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(requested_index - 1LL);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<long long> parse_native_list_control_selectedid_member_item_id_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const auto extract_member_selector_text =
        [](const std::string& candidate, const std::string& base_name) -> std::optional<std::string>
    {
        const std::string trimmed = trim_copy(candidate);
        const std::string normalized = lowercase_copy(trimmed);
        const auto extract_for_delimiters =
            [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
        {
            std::string prefix = base_name;
            prefix.push_back(open_delimiter);
            if (!starts_with_insensitive(normalized, prefix) ||
                normalized.empty() ||
                normalized.back() != close_delimiter)
            {
                return std::nullopt;
            }

            const std::size_t open = trimmed.find(open_delimiter);
            const std::size_t close = trimmed.rfind(close_delimiter);
            if (open == std::string::npos ||
                close == std::string::npos ||
                close <= open + 1U)
            {
                return std::nullopt;
            }
            return trim_copy(trimmed.substr(open + 1U, close - open - 1U));
        };

        if (const auto parenthesized = extract_for_delimiters('(', ')');
            parenthesized.has_value())
        {
            return parenthesized;
        }
        return extract_for_delimiters('[', ']');
    };

    const auto selector_text = extract_member_selector_text(member_name, "selectedid");
    if (!selector_text.has_value() || selector_text->empty()) {
        return std::nullopt;
    }

    try {
        const long long requested_item_id = std::stoll(*selector_text);
        if (requested_item_id < 1LL) {
            return std::nullopt;
        }
        return requested_item_id;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::size_t> parse_native_list_control_indextoitemid_member_slot_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const auto extract_member_selector_text =
        [](const std::string& candidate, const std::string& base_name) -> std::optional<std::string>
    {
        const std::string trimmed = trim_copy(candidate);
        const std::string normalized = lowercase_copy(trimmed);
        const auto extract_for_delimiters =
            [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
        {
            std::string prefix = base_name;
            prefix.push_back(open_delimiter);
            if (!starts_with_insensitive(normalized, prefix) ||
                normalized.empty() ||
                normalized.back() != close_delimiter)
            {
                return std::nullopt;
            }

            const std::size_t open = trimmed.find(open_delimiter);
            const std::size_t close = trimmed.rfind(close_delimiter);
            if (open == std::string::npos ||
                close == std::string::npos ||
                close <= open + 1U)
            {
                return std::nullopt;
            }
            return trim_copy(trimmed.substr(open + 1U, close - open - 1U));
        };

        if (const auto parenthesized = extract_for_delimiters('(', ')');
            parenthesized.has_value())
        {
            return parenthesized;
        }
        return extract_for_delimiters('[', ']');
    };

    const auto selector_text = extract_member_selector_text(member_name, "indextoitemid");
    if (!selector_text.has_value() || selector_text->empty()) {
        return std::nullopt;
    }

    try {
        const long long requested_index = std::stoll(*selector_text);
        if (requested_index < 1LL) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(requested_index - 1LL);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<long long> parse_native_list_control_itemidtoindex_member_item_id_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const auto extract_member_selector_text =
        [](const std::string& candidate, const std::string& base_name) -> std::optional<std::string>
    {
        const std::string trimmed = trim_copy(candidate);
        const std::string normalized = lowercase_copy(trimmed);
        const auto extract_for_delimiters =
            [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
        {
            std::string prefix = base_name;
            prefix.push_back(open_delimiter);
            if (!starts_with_insensitive(normalized, prefix) ||
                normalized.empty() ||
                normalized.back() != close_delimiter)
            {
                return std::nullopt;
            }

            const std::size_t open = trimmed.find(open_delimiter);
            const std::size_t close = trimmed.rfind(close_delimiter);
            if (open == std::string::npos ||
                close == std::string::npos ||
                close <= open + 1U)
            {
                return std::nullopt;
            }
            return trim_copy(trimmed.substr(open + 1U, close - open - 1U));
        };

        if (const auto parenthesized = extract_for_delimiters('(', ')');
            parenthesized.has_value())
        {
            return parenthesized;
        }
        return extract_for_delimiters('[', ']');
    };

    const auto selector_text = extract_member_selector_text(member_name, "itemidtoindex");
    if (!selector_text.has_value() || selector_text->empty()) {
        return std::nullopt;
    }

    try {
        const long long requested_item_id = std::stoll(*selector_text);
        if (requested_item_id < 1LL) {
            return std::nullopt;
        }
        return requested_item_id;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::size_t> resolve_native_list_control_insert_slot(
    const RuntimeOleObjectState& runtime_object,
    const std::vector<PrgValue>& arguments) {
    if (arguments.size() < 2U) {
        return runtime_object.collection_items.size();
    }

    const long long requested_index = std::llround(value_as_number(arguments[1]));
    if (requested_index < 1LL) {
        return std::nullopt;
    }
    if (static_cast<std::size_t>(requested_index) > runtime_object.collection_items.size()) {
        return runtime_object.collection_items.size();
    }
    return static_cast<std::size_t>(requested_index - 1LL);
}
