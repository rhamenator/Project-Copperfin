// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

std::string xml_escape(std::string value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '&':
                escaped += "&amp;";
                break;
            case '<':
                escaped += "&lt;";
                break;
            case '>':
                escaped += "&gt;";
                break;
            case '"':
                escaped += "&quot;";
                break;
            case '\'':
                escaped += "&apos;";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }
    return escaped;
}

std::string xml_unescape(std::string value) {
    const auto replace_all = [&](const std::string& token, const std::string& replacement) {
        std::size_t position = 0U;
        while ((position = value.find(token, position)) != std::string::npos) {
            value.replace(position, token.size(), replacement);
            position += replacement.size();
        }
    };
    replace_all("&lt;", "<");
    replace_all("&gt;", ">");
    replace_all("&quot;", "\"");
    replace_all("&apos;", "'");
    replace_all("&amp;", "&");
    return value;
}

std::string xml_attribute(const std::string& tag_text, const std::string& name) {
    const std::string needle = name + "=\"";
    const std::size_t start = tag_text.find(needle);
    if (start == std::string::npos) {
        return {};
    }
    const std::size_t value_start = start + needle.size();
    const std::size_t value_end = tag_text.find('"', value_start);
    if (value_end == std::string::npos) {
        return {};
    }
    return xml_unescape(tag_text.substr(value_start, value_end - value_start));
}

std::string serialize_cursor_snapshot_xml(const RuntimeSurfaceCursorSnapshot& snapshot) {
    std::ostringstream xml;
    xml.imbue(std::locale::classic());
    xml << "<CopperfinCursor alias=\"" << xml_escape(snapshot.alias) << "\">\n";
    xml << "  <Fields>\n";
    for (const auto& field : snapshot.fields) {
        xml << "    <Field name=\"" << xml_escape(field.name)
            << "\" type=\"" << xml_escape(std::string(1U, field.type))
            << "\" width=\"" << field.width
            << "\" decimals=\"" << field.decimals
            << "\" />\n";
    }
    xml << "  </Fields>\n";
    xml << "  <Rows>\n";
    for (const auto& row : snapshot.rows) {
        xml << "    <Row>";
        for (const auto& value : row.values) {
            xml << "<Col>" << xml_escape(value) << "</Col>";
        }
        xml << "</Row>\n";
    }
    xml << "  </Rows>\n";
    xml << "</CopperfinCursor>\n";
    return xml.str();
}

std::optional<RuntimeSurfaceCursorSnapshot> parse_cursor_snapshot_xml(const std::string& xml_text) {
    RuntimeSurfaceCursorSnapshot snapshot;

    const std::size_t root_start = xml_text.find("<CopperfinCursor");
    if (root_start == std::string::npos) {
        return std::nullopt;
    }
    const std::size_t root_tag_end = xml_text.find('>', root_start);
    const std::size_t root_end = xml_text.find("</CopperfinCursor>", root_tag_end == std::string::npos ? 0U : root_tag_end);
    if (root_tag_end == std::string::npos || root_end == std::string::npos) {
        return std::nullopt;
    }
    snapshot.alias = xml_attribute(xml_text.substr(root_start, root_tag_end - root_start + 1U), "alias");

    const std::size_t fields_start = xml_text.find("<Fields>", root_tag_end);
    const std::size_t fields_end = xml_text.find("</Fields>", fields_start == std::string::npos ? 0U : fields_start);
    if (fields_start == std::string::npos || fields_end == std::string::npos) {
        return std::nullopt;
    }

    std::size_t scan = fields_start;
    while (true) {
        const std::size_t field_start = xml_text.find("<Field ", scan);
        if (field_start == std::string::npos || field_start >= fields_end) {
            break;
        }
        const std::size_t field_end = xml_text.find("/>", field_start);
        if (field_end == std::string::npos || field_end > fields_end) {
            return std::nullopt;
        }
        const std::string field_tag = xml_text.substr(field_start, field_end - field_start + 2U);
        RuntimeSurfaceCursorField field;
        field.name = xml_attribute(field_tag, "name");
        const std::string type_text = xml_attribute(field_tag, "type");
        field.type = type_text.empty() ? 'C' : type_text.front();
        const auto width = copperfin::platform::try_parse_invariant_integer<std::size_t>(
            xml_attribute(field_tag, "width"));
        const auto decimals = copperfin::platform::try_parse_invariant_integer<std::size_t>(
            xml_attribute(field_tag, "decimals"));
        if (field.name.empty() || !width.has_value() || !decimals.has_value()) {
            return std::nullopt;
        }
        field.width = *width;
        field.decimals = *decimals;
        snapshot.fields.push_back(std::move(field));
        scan = field_end + 2U;
    }

    const std::size_t rows_start = xml_text.find("<Rows>", fields_end);
    const std::size_t rows_end = xml_text.find("</Rows>", rows_start == std::string::npos ? 0U : rows_start);
    if (rows_start == std::string::npos || rows_end == std::string::npos) {
        return std::nullopt;
    }

    scan = rows_start;
    while (true) {
        const std::size_t row_start = xml_text.find("<Row>", scan);
        if (row_start == std::string::npos || row_start >= rows_end) {
            break;
        }
        const std::size_t row_end = xml_text.find("</Row>", row_start);
        if (row_end == std::string::npos || row_end > rows_end) {
            return std::nullopt;
        }

        RuntimeSurfaceCursorRow row;
        std::size_t col_scan = row_start;
        while (true) {
            const std::size_t col_start = xml_text.find("<Col>", col_scan);
            if (col_start == std::string::npos || col_start >= row_end) {
                break;
            }
            const std::size_t col_value_start = col_start + 5U;
            const std::size_t col_end = xml_text.find("</Col>", col_value_start);
            if (col_end == std::string::npos || col_end > row_end) {
                return std::nullopt;
            }
            row.values.push_back(xml_unescape(xml_text.substr(col_value_start, col_end - col_value_start)));
            col_scan = col_end + 6U;
        }
        if (row.values.size() != snapshot.fields.size()) {
            return std::nullopt;
        }
        snapshot.rows.push_back(std::move(row));
        scan = row_end + 6U;
    }

    return snapshot;
}

std::vector<std::string> collect_object_member_names(
    const RuntimeOleObjectState& runtime_object,
    int flags,
    const std::optional<NativeMemberVisibility>& visibility_filter = std::nullopt) {
    const bool include_all = flags == 0;
    const bool include_properties = include_all || ((flags & 1) != 0);
    const bool include_methods = include_all || ((flags & 2) != 0);
    const bool include_events = include_all || ((flags & 4) != 0);

    std::set<std::string> unique_members;
    if (include_properties) {
        for (const auto& [name, value] : runtime_object.properties) {
            (void)value;
            if (native_name_member_name_matches(runtime_object, normalize_identifier(name))) {
                continue;
            }
            unique_members.insert(normalize_identifier(name));
        }
        if (is_native_collection_object(runtime_object)) {
            unique_members.insert("count");
        }
        if (native_listitem_member_name_matches(runtime_object, "listitem")) {
            unique_members.insert("listitem");
        }
        if (native_itemdata_member_name_matches(runtime_object, "itemdata")) {
            unique_members.insert("itemdata");
        }
        for (const auto& metadata_name : collect_native_identity_member_names(runtime_object)) {
            unique_members.insert(metadata_name);
        }
        for (const auto& method_name : runtime_object.methods) {
            std::string stem;
            if ((method_ends_with_suffix(method_name, "access", &stem) ||
                 method_ends_with_suffix(method_name, "assign", &stem)) &&
                !stem.empty()) {
                unique_members.insert(stem);
            }
        }
    }
    if (include_methods) {
        for (const auto& method_name : runtime_object.methods) {
            unique_members.insert(normalize_identifier(method_name));
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "readexpression")) {
            unique_members.insert("readexpression");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "writeexpression")) {
            unique_members.insert("writeexpression");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "readmethod")) {
            unique_members.insert("readmethod");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "writemethod")) {
            unique_members.insert("writemethod");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "release")) {
            unique_members.insert("release");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "refresh")) {
            unique_members.insert("refresh");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "move")) {
            unique_members.insert("move");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "show")) {
            unique_members.insert("show");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "hide")) {
            unique_members.insert("hide");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "setfocus")) {
            unique_members.insert("setfocus");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "resettodefault")) {
            unique_members.insert("resettodefault");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "clear")) {
            unique_members.insert("clear");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "additem")) {
            unique_members.insert("additem");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "addlistitem")) {
            unique_members.insert("addlistitem");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "indextoitemid")) {
            unique_members.insert("indextoitemid");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "itemidtoindex")) {
            unique_members.insert("itemidtoindex");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "removeitem")) {
            unique_members.insert("removeitem");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "removelistitem")) {
            unique_members.insert("removelistitem");
        }
        if (is_native_collection_object(runtime_object)) {
            unique_members.insert("item");
            if (!runtime_object.read_only_collection_surface) {
                unique_members.insert("add");
                unique_members.insert("remove");
                unique_members.insert("removeall");
            }
        }
    }
    if (include_events) {
        for (const auto& event_name : runtime_object.events) {
            unique_members.insert(normalize_identifier(event_name));
        }
    }

    std::vector<std::string> members;
    members.reserve(unique_members.size());
    for (const std::string& member_name : unique_members) {
        if (visibility_filter.has_value()) {
            const auto visibility = runtime_object.member_visibility.find(normalize_identifier(member_name));
            const NativeMemberVisibility effective_visibility =
                visibility == runtime_object.member_visibility.end()
                    ? NativeMemberVisibility::public_member
                    : visibility->second;
            if (effective_visibility != *visibility_filter) {
                continue;
            }
        }
        members.push_back(uppercase_copy(member_name));
    }

    std::sort(members.begin(), members.end(), [](const std::string& left, const std::string& right) {
        const std::string normalized_left = lowercase_copy(left);
        const std::string normalized_right = lowercase_copy(right);
        if (normalized_left == normalized_right) {
            return left < right;
        }
        return normalized_left < normalized_right;
    });
    return members;
}
