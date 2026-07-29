// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

    if (function == "file" && !arguments.empty()) {
        std::error_code ignored;
        const std::filesystem::path path =
            resolve_runtime_file_probe_path(
                value_as_string(arguments[0]), default_directory, set_callback, true);
        const std::filesystem::file_status status = std::filesystem::status(path, ignored);
        return make_boolean_value(
            !ignored && std::filesystem::exists(status) && !std::filesystem::is_directory(status));
    }
    if (function == "sys") {
        const auto format_sys16_frame = [](const std::string& routine_name,
                                           const std::string& file_path,
                                           bool procedure_context) {
            if (!procedure_context || routine_name.empty() || file_path.empty()) {
                return file_path;
            }
            return std::string("PROCEDURE ") + routine_name + " " + file_path;
        };
        if (!arguments.empty()) {
            const long long sys_code = std::llround(value_as_number(arguments[0]));
            if (sys_code == 3) {
                // VFP9 SYS(3) is a legal temporary filename component, not
                // a product/version descriptor. Keep it extension-free so
                // callers can append the file type they need.
                return make_string_value(make_legal_runtime_temp_file_name());
            }
            if (sys_code == 2015) {
                // VFP9 SYS(2015) is a ten-character identifier for generated
                // procedure, alias, cursor, and file names.
                return make_string_value(make_unique_runtime_procedure_name());
            }
            if (sys_code == 2014) {
                // VFP9 SYS(2014) returns a lexical path relative to the
                // current or supplied directory; it does not require a file
                // to exist and must not change the runtime default directory.
                if (arguments.size() < 2U) {
                    return make_string_value({});
                }
                const std::string base_path = arguments.size() >= 3U
                    ? value_as_string(arguments[2])
                    : std::string{};
                return make_string_value(minimum_runtime_path(
                    value_as_string(arguments[1]),
                    base_path,
                    default_directory));
            }
            if (sys_code == 2000) {
                // VFP9 SYS(2000) enumerates regular files in deterministic
                // order; the SET callback owns per-data-session continuation.
                if (arguments.size() < 2U || !set_callback) {
                    return make_string_value({});
                }
                const bool next_match_requested = arguments.size() >= 3U &&
                    safe_int_argument(2U, 0) == 1;
                const std::string request =
                    std::string("__sys2000__\x1f") +
                    (next_match_requested ? "next" : "first") +
                    '\x1f' + value_as_string(arguments[1]);
                return make_string_value(set_callback(request));
            }
            if (sys_code == 2029) {
                // VFP9 SYS(2029) reports the physical DBF table type for the
                // current cursor or the requested alias. Synthetic and
                // remote cursors have no physical header, so they report 0.
                const std::string cursor_designator = arguments.size() >= 2U
                    ? value_as_string(arguments[1])
                    : std::string{};
                const auto snapshot = snapshot_cursor_callback
                    ? snapshot_cursor_callback(cursor_designator)
                    : std::nullopt;
                return make_string_value(format_value(make_number_value(
                    snapshot.has_value() && snapshot->table_type.has_value()
                        ? static_cast<double>(*snapshot->table_type)
                        : 0.0)));
            }
            if (sys_code == 5 || sys_code == 2003 || sys_code == 2004) {
                return make_string_value(default_directory);
            }
            if (sys_code == 7) {
                return make_string_value(host_os_name());
            }
            if (sys_code == 11) {
                return make_string_value("0");
            }
            if (sys_code == 13) {
                return make_string_value("0");
            }
            if (sys_code == 16) {
                if (arguments.size() >= 2U && program_stack_frame_callback) {
                    const long long level = safe_int_argument(1U, 0);
                    if (const auto stack_frame = program_stack_frame_callback(level); stack_frame.has_value()) {
                        return make_string_value(format_sys16_frame(
                            stack_frame->routine_name,
                            stack_frame->file_path,
                            stack_frame->procedure_context));
                    }
                    return make_string_value({});
                }
                return make_string_value(format_sys16_frame(
                    current_program_name,
                    frame_file_path,
                    current_program_name != "main"));
            }
            if (sys_code == 2018) {
                return make_string_value(uppercase_copy(runtime_error_parameter(last_error_message)));
            }
            if (sys_code == 2020) {
                return make_string_value(format_value(make_number_value(available_disk_space({}, default_directory))));
            }
            if (sys_code == 2023) {
                std::error_code ignored;
                return make_string_value(copperfin::platform::path_to_utf8_string(
                    std::filesystem::temp_directory_path(ignored)));
            }
            if (sys_code == 2326 && arguments.size() >= 2U && whandle_from_hwnd_callback) {
                const std::int64_t hwnd = safe_int64_argument(1U, 0);
                if (const auto whandle = whandle_from_hwnd_callback(hwnd); whandle.has_value()) {
                    return make_int64_value(*whandle);
                }
                return make_number_value(0.0);
            }
            if (sys_code == 2327 && arguments.size() >= 2U && hwnd_from_whandle_callback) {
                const std::int64_t whandle = safe_int64_argument(1U, 0);
                if (const auto hwnd = hwnd_from_whandle_callback(whandle); hwnd.has_value()) {
                    return make_int64_value(*hwnd);
                }
                return make_number_value(0.0);
            }
        }
        return make_string_value("0");
    }
    if (function == "home") {
        std::string home = default_directory;
        if (!home.empty() && home.back() != '/' && home.back() != '\\') {
            home += static_cast<char>(std::filesystem::path::preferred_separator);
        }
        return make_string_value(std::move(home));
    }
    if (function == "os") {
        return make_string_value(host_os_name());
    }
    if (function == "diskspace") {
        const std::string path = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        return make_number_value(available_disk_space(path, default_directory));
    }
    if (function == "drivetype") {
        const std::string path = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        return make_number_value(static_cast<double>(drive_type_value(path, default_directory)));
    }
    if (function == "filesize") {
        if (arguments.empty()) {
            return make_number_value(0.0);
        }
        std::error_code ignored;
        const std::filesystem::path path =
            resolve_runtime_file_probe_path(
                value_as_string(arguments[0]), default_directory, set_callback, true);
        if (!std::filesystem::exists(path, ignored)) {
            return make_number_value(0.0);
        }
        const auto size = std::filesystem::file_size(path, ignored);
        return make_number_value(ignored ? 0.0 : static_cast<double>(size));
    }
    if (function == "message") {
        return make_string_value(last_error_message);
    }
    if (function == "prmbar") {
        if (popup_prompt_callback) {
            if (const auto prompt = popup_prompt_callback(arguments); prompt.has_value()) {
                return *prompt;
            }
        }
        return make_string_value({});
    }
    if (function == "cntbar") {
        if (popup_bar_count_callback) {
            if (const auto count = popup_bar_count_callback(arguments); count.has_value()) {
                return *count;
            }
        }
        return make_number_value(0.0);
    }
    if (function == "getbar") {
        if (popup_bar_position_callback) {
            if (const auto bar = popup_bar_position_callback(arguments); bar.has_value()) {
                return *bar;
            }
        }
        return make_number_value(0.0);
    }
    if (function == "skpbar") {
        if (popup_bar_skip_callback) {
            if (const auto skipped = popup_bar_skip_callback(arguments); skipped.has_value()) {
                return *skipped;
            }
        }
        return make_boolean_value(false);
    }
    if (function == "mrkbar") {
        if (popup_bar_mark_callback) {
            if (const auto marked = popup_bar_mark_callback(arguments); marked.has_value()) {
                return *marked;
            }
        }
        return make_boolean_value(false);
    }
    if (function == "aerror" && !raw_arguments.empty()) {
        return make_number_value(static_cast<double>(aerror_callback(raw_arguments[0])));
    }
    if ((function == "eval" || function == "evaluate") && !arguments.empty()) {
        std::string expression_text = value_as_string(arguments[0]);
        std::string last_identifier_text;
        const auto expand_identifier_chain =
            [&](std::string expanded_text) {
                if (expanded_text.empty()) {
                    return expanded_text;
                }
                constexpr std::size_t max_macro_expression_depth = 16U;
                std::vector<std::string> visited_macros;
                visited_macros.reserve(8U);
                for (std::size_t depth = 0U; depth < max_macro_expression_depth; ++depth) {
                    const bool bare_identifier =
                        std::all_of(
                            expanded_text.begin(),
                            expanded_text.end(),
                            [](unsigned char ch) {
                                return std::isalnum(ch) != 0 || ch == '_';
                            });
                    if (!bare_identifier) {
                        break;
                    }
                    const std::string normalized_identifier = normalize_memory_variable_identifier(expanded_text);
                    if (std::find(visited_macros.begin(), visited_macros.end(), normalized_identifier) != visited_macros.end()) {
                        break;
                    }
                    visited_macros.push_back(normalized_identifier);
                    const std::string next_text =
                        trim_copy(value_as_string(eval_expression_callback(expanded_text)));
                    if (next_text.empty() || next_text == expanded_text) {
                        break;
                    }
                    last_identifier_text = expanded_text;
                    expanded_text = next_text;
                }
                return expanded_text;
            };
        if (!raw_arguments.empty()) {
            const std::string raw_text = trim_copy(raw_arguments[0]);
            if (!raw_text.empty() && raw_text.front() == '&') {
                const std::string macro_variable_text = trim_copy(raw_text.substr(1U));
                const bool simple_macro_variable =
                    !macro_variable_text.empty() &&
                    std::all_of(
                        macro_variable_text.begin(),
                        macro_variable_text.end(),
                        [](unsigned char ch) {
                            return std::isalnum(ch) != 0 || ch == '_';
                        });
                if (simple_macro_variable) {
                    expression_text =
                        trim_copy(value_as_string(eval_expression_callback(macro_variable_text)));
                    expression_text = expand_identifier_chain(expression_text);
                }
            } else if (std::all_of(
                           raw_text.begin(),
                           raw_text.end(),
                           [](unsigned char ch) {
                               return std::isalnum(ch) != 0 || ch == '_';
                           })) {
                const std::string expanded_text = expand_identifier_chain(expression_text);
                if (!last_identifier_text.empty()) {
                    expression_text = last_identifier_text;
                } else {
                    expression_text = expanded_text;
                }
            }
        }
        PrgValue evaluated = eval_expression_callback(expression_text);
        const bool empty_string_result =
            evaluated.kind == PrgValueKind::string && value_as_string(evaluated).empty();
        if ((evaluated.kind == PrgValueKind::empty || empty_string_result) &&
            !last_identifier_text.empty() &&
            expression_text != last_identifier_text) {
            evaluated = eval_expression_callback(last_identifier_text);
        }
        return evaluated;
    }
    if (function == "cursortoxml") {
        const std::string cursor_designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        const std::string output_target = arguments.size() >= 2U ? trim_copy(value_as_string(arguments[1])) : std::string{};
        if (!snapshot_cursor_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.UnavailableCallback",
                {
                    {"capability", "cursor snapshot"},
                    {"function", "CURSORTOXML()"}
                }));
            if (!output_target.empty() && looks_like_file_path(output_target)) {
                return make_boolean_value(false);
            }
            return make_string_value(std::string{});
        }

        const std::optional<RuntimeSurfaceCursorSnapshot> snapshot = snapshot_cursor_callback(cursor_designator);
        if (!snapshot.has_value()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.CursorToXmlTargetUnreadable",
                {{"function", "CURSORTOXML()"}}));
            if (!output_target.empty() && looks_like_file_path(output_target)) {
                return make_boolean_value(false);
            }
            return make_string_value(std::string{});
        }

        const std::string xml_payload = serialize_cursor_snapshot_xml(*snapshot);
        if (record_event_callback) {
            record_event_callback(
                "runtime.cursortoxml",
                snapshot->alias + " rows=" + std::to_string(snapshot->rows.size()));
        }

        if (output_target.empty() || !looks_like_file_path(output_target)) {
            return make_string_value(xml_payload);
        }

        std::error_code ignored;
        std::filesystem::path output_path = filesystem_probe_path(output_target, default_directory);
        std::filesystem::create_directories(output_path.parent_path(), ignored);
        std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
        output << xml_payload;
        output.close();
        if (!output.good()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.CursorToXmlWriteFailed",
                {{"function", "CURSORTOXML()"}}));
            return make_boolean_value(false);
        }
        return make_boolean_value(true);
    }
    if (function == "xmltocursor") {
        if (arguments.size() < 2U) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorInputAndAliasRequired",
                {{"function", "XMLTOCURSOR()"}}));
            return make_number_value(0.0);
        }
        const std::string xml_or_path = value_as_string(arguments[0]);
        const std::string destination_alias = trim_copy(value_as_string(arguments[1]));
        if (destination_alias.empty()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorDestinationAliasRequired",
                {{"function", "XMLTOCURSOR()"}}));
            return make_number_value(0.0);
        }
        if (!load_cursor_snapshot_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.UnavailableCallback",
                {
                    {"capability", "cursor load"},
                    {"function", "XMLTOCURSOR()"}
                }));
            return make_number_value(0.0);
        }

        std::string xml_payload = xml_or_path;
        std::error_code ignored;
        if (looks_like_file_path(xml_or_path)) {
            std::filesystem::path probe_path = filesystem_probe_path(xml_or_path, default_directory);
            if (require_verified_file_byte_overrides) {
                const auto verified_payload = read_verified_file_callback
                    ? read_verified_file_callback(probe_path)
                    : std::nullopt;
                if (!verified_payload.has_value()) {
                    record_runtime_warning(runtime_text(
                        "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorVerifiedBytesUnavailable",
                        {{"path", copperfin::platform::path_to_utf8_string(probe_path)}}));
                    return make_number_value(0.0);
                }
                xml_payload = *verified_payload;
            } else if (std::filesystem::exists(probe_path, ignored)) {
                std::ifstream input(probe_path, std::ios::binary);
                std::ostringstream buffer;
                buffer << input.rdbuf();
                xml_payload = buffer.str();
            }
        }

        const std::optional<RuntimeSurfaceCursorSnapshot> parsed = parse_cursor_snapshot_xml(xml_payload);
        if (!parsed.has_value()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorParseFailed",
                {{"function", "XMLTOCURSOR()"}}));
            return make_number_value(0.0);
        }

        std::optional<std::size_t> loaded_count = load_cursor_snapshot_callback(*parsed, destination_alias);
        if (!loaded_count.has_value()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorMaterializeFailed",
                {{"function", "XMLTOCURSOR()"}}));
            return make_number_value(0.0);
        }

        if (record_event_callback) {
            record_event_callback(
                "runtime.xmltocursor",
                destination_alias + " rows=" + std::to_string(*loaded_count));
        }
        return make_number_value(static_cast<double>(*loaded_count));
    }
    if (function == "set" && !arguments.empty()) {
        std::string option_name = value_as_string(arguments[0]);
        if (arguments.size() >= 2U && normalize_identifier(option_name) == "textmerge") {
            const PrgValue &variant = arguments[1];
            std::string variant_text;
            if (variant.kind == PrgValueKind::number ||
                variant.kind == PrgValueKind::int64 ||
                variant.kind == PrgValueKind::uint64 ||
                variant.kind == PrgValueKind::currency) {
                variant_text = std::to_string(static_cast<long long>(std::llround(value_as_number(variant))));
            } else {
                variant_text = trim_copy(value_as_string(variant));
            }

            if (variant_text == "1" || variant_text == "3") {
                option_name += "," + variant_text;
            }
        }
        return make_string_value(set_callback(option_name));
    }
    if ((function == "relation" || function == "target") && !arguments.empty()) {
        const long long relation_number = static_cast<long long>(std::llround(value_as_number(arguments[0])));
        if (relation_number < 1) {
            return make_string_value(std::string{});
        }

        // Relation introspection is resolved by the owning runtime session so the
        // expression evaluator does not need to own cursor or data-session state.
        const std::string designator = arguments.size() >= 2U
            ? value_as_string(arguments[1])
            : std::string{};
        return make_string_value(
            set_callback("__relation_introspection__\x1f" + function + "\x1f" +
                         std::to_string(relation_number) + "\x1f" + designator));
    }
    if (function == "error") {
        return make_number_value(static_cast<double>(last_error_code));
    }
    if (function == "program") {
        if (!arguments.empty() && value_as_number(arguments[0]) == -1.0) {
            return make_number_value(static_cast<double>(program_stack_depth));
        }
        if (arguments.empty() && !current_program_name.empty()) {
            return make_string_value(current_program_name);
        }
        if (!arguments.empty() && program_stack_frame_callback) {
            const long long level = safe_int_argument(0U, -1);
            if (const auto stack_frame = program_stack_frame_callback(level); stack_frame.has_value()) {
                return make_string_value(stack_frame->routine_name);
            }
            return make_string_value({});
        }
        return make_string_value(last_error_procedure);
    }
    if (function == "lineno") {
        return make_number_value(static_cast<double>(last_error_line));
    }
    if (function == "version") {
        return make_number_value(arguments.empty() ? 9.0 : 0.0);
    }
    if (function == "on" && !arguments.empty()) {
        const std::string topic = uppercase_copy(value_as_string(arguments[0]));
        if (topic == "ERROR") {
            return make_string_value(error_handler);
        }
        if (topic == "SHUTDOWN") {
            return make_string_value(shutdown_handler);
        }
        return make_string_value(std::string{});
    }
    if (function == "messagebox" && !arguments.empty()) {
        return make_number_value(1.0);
    }
    if (function == "cast" && !arguments.empty()) {
        std::string type_name;
        if (!raw_arguments.empty()) {
            const std::string raw = uppercase_copy(raw_arguments[0]);
            const auto as_pos = raw.rfind(" AS ");
            if (as_pos != std::string::npos) {
                type_name = trim_copy(raw.substr(as_pos + 4U));
            }
        }

        const PrgValue source = arguments[0];
        if (type_name == "INT64" || type_name == "LONGLONG" || type_name == "BIGINT") {
            return make_int64_value(static_cast<std::int64_t>(value_as_number(source)));
        }
        if (type_name == "UINT64" || type_name == "ULONGLONG" || type_name == "UBIGINT") {
            return make_uint64_value(static_cast<std::uint64_t>(value_as_number(source)));
        }
        if (type_name == "INT" || type_name == "INT32" || type_name == "INTEGER" ||
            type_name == "LONG" || type_name == "INT16" || type_name == "SHORT") {
            return make_int64_value(static_cast<std::int64_t>(std::trunc(value_as_number(source))));
        }
        if (type_name == "BYTE" || type_name == "UINT8") {
            return make_uint64_value(
                static_cast<std::uint64_t>(value_as_number(source)) & 0xFFULL);
        }
        if (type_name == "FLOAT" || type_name == "SINGLE") {
            return make_number_value(
                static_cast<double>(static_cast<float>(value_as_number(source))));
        }
        if (type_name == "DOUBLE" || type_name == "NUMERIC") {
            return make_number_value(value_as_number(source));
        }
        if (type_name == "STRING" || type_name == "CHAR" || type_name == "VARCHAR" ||
            type_name == "CHARACTER") {
            return make_string_value(value_as_string(source));
        }
        if (type_name == "LOGICAL" || type_name == "BOOL" || type_name == "BOOLEAN") {
            return make_boolean_value(value_as_bool(source));
        }
        return source;
    }

    if (function == "bitand" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result &= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitor" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result |= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitxor" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result ^= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitnot" && !arguments.empty()) {
        return make_int64_value(signed_bitwise_result(~bitwise_value(arguments[0])));
    }
    if (function == "bitclear" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_int64_value(signed_bitwise_result(value & ~mask));
    }
    if (function == "bitset" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_int64_value(signed_bitwise_result(value | mask));
    }
    if (function == "bittest" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_boolean_value((value & mask) != 0U);
    }
    if (function == "bitlshift" && arguments.size() >= 2U) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int count = static_cast<int>(value_as_number(arguments[1]));
        return make_int64_value(value << count);
    }
    if (function == "bitrshift" && arguments.size() >= 2U) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int count = static_cast<int>(value_as_number(arguments[1]));
        return make_int64_value(value >> count);
    }

    if (function == "bintoc" && !arguments.empty()) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int width = arguments.size() >= 2U
                              ? static_cast<int>(value_as_number(arguments[1]))
                              : 4;
        std::string result(static_cast<std::size_t>(std::max(width, 0)), '\0');
        std::uint64_t unsigned_value = static_cast<std::uint64_t>(value);
        for (int index = 0; index < width; ++index) {
            result[static_cast<std::size_t>(index)] =
                static_cast<char>(unsigned_value & 0xFFU);
            unsigned_value >>= 8;
        }
        return make_string_value(std::move(result));
    }
    if (function == "ctobin" && !arguments.empty()) {
        const std::string source = value_as_string(arguments[0]);
        const std::string type = arguments.size() >= 2U
                                     ? uppercase_copy(value_as_string(arguments[1]))
                                     : std::string("N");
        std::uint64_t unsigned_value = 0U;
        for (std::size_t index = source.size(); index-- > 0U;) {
            unsigned_value = (unsigned_value << 8) |
                             static_cast<std::uint8_t>(source[index]);
        }
        if (type == "N" || type == "INTEGER" || type == "INT") {
            return make_int64_value(static_cast<std::int64_t>(unsigned_value));
        }
        return make_uint64_value(unsigned_value);
    }

    if (function == "numlock" || function == "capslock" || function == "scrolllock") {
        return make_boolean_value(false);
    }
    if (function == "cursorsetprop" || function == "cursorgetprop") {
        return make_number_value(0.0);
    }
    // NEWID([cDatabase]) — generate a unique identifier string (UUID v4-style, no braces)
    if (function == "newid") {
        static thread_local std::mt19937_64 uuid_gen{std::random_device{}()};
        std::uniform_int_distribution<std::uint64_t> dist(0, std::numeric_limits<std::uint64_t>::max());
        const std::uint64_t hi = dist(uuid_gen);
        const std::uint64_t lo = dist(uuid_gen);
        // Set UUID v4 bits
        const std::uint64_t hi4 = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        const std::uint64_t lo4 = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
        std::ostringstream oss;
        oss << std::uppercase << std::hex << std::setfill('0')
            << std::setw(8) << ((hi4 >> 32) & 0xFFFFFFFFU) << '-'
            << std::setw(4) << ((hi4 >> 16) & 0xFFFFU) << '-'
            << std::setw(4) << (hi4 & 0xFFFFU) << '-'
            << std::setw(4) << ((lo4 >> 48) & 0xFFFFU) << '-'
            << std::setw(12) << (lo4 & 0x0000FFFFFFFFFFFFULL);
        return make_string_value(oss.str());
    }
    // VFP9 help (dv_foxhelp.chm, CPCURRENT() topic):
    // - omitted/0 => configured VFP code page, or current OS code page when no
    //   CODEPAGE config item is in effect
    // - 1 => current OS code page regardless of CODEPAGE config
    // - 2 => underlying OS code page (MS-DOS/OEM on Windows)
    //
    // CODEPAGE is a startup configuration value, not a data-session SET state;
    // omitted and 0 read it back while 1 and 2 remain host/OEM queries.
    if (function == "cpcurrent") {
        const int type_flag = arguments.empty() ? 0 : static_cast<int>(std::llround(value_as_number(arguments[0])));
        if (type_flag == 2) {
            return make_number_value(static_cast<double>(current_host_oem_code_page()));
        }
        if (type_flag == 0) {
            const std::string configured = set_callback("CODEPAGE");
            try {
                return make_number_value(std::stod(configured));
            } catch (const std::exception&) {
                // Preserve the host fallback if a malformed state value is
                // encountered rather than leaking a runtime exception.
            }
        }
        return make_number_value(static_cast<double>(current_host_code_page()));
    }
    // VFP9 help (dv_foxhelp.chm, CPCONVERT() topic): convert a character expression
    // from one explicit code page to another.
    //
    // Copperfin preserves same-codepage calls as identity. For supported VFP code pages,
    // it uses host conversion APIs where available; otherwise it falls back to the
    // original byte sequence unchanged.
    if (function == "cpconvert" && arguments.size() >= 3U) {
        const int source_code_page = safe_int_argument(0, 0);
        const int target_code_page = safe_int_argument(1, 0);
        const std::string input = value_as_string(arguments[2]);
        if (source_code_page == target_code_page) {
            return make_string_value(input);
        }
        if (!is_supported_vfp_code_page(source_code_page) ||
            !is_supported_vfp_code_page(target_code_page)) {
            return make_string_value(input);
        }
        if (const auto converted =
                convert_between_host_code_pages(source_code_page, target_code_page, input);
            converted.has_value()) {
            return make_string_value(*converted);
        }
        return make_string_value(input);
    }
    // VFP9 help (dv_foxhelp.chm, CPDBF() topic):
    // - omitted => code page of the table in the current work area
    // - nWorkArea => code page of that work area, returning 0 when no table is open
    // - cTableAlias => code page of that alias, raising an error when no such alias exists
    //
    // Copperfin projects the DBF header code-page mark when the selected cursor is
    // table-backed. Synthetic/remote cursors and missing header metadata fall back to 0.
    if (function == "cpdbf") {
        const bool explicit_alias = !arguments.empty() &&
            arguments[0].kind == PrgValueKind::string;
        const std::string cursor_designator = arguments.empty()
            ? std::string{}
            : value_as_string(arguments[0]);
        const std::optional<RuntimeSurfaceCursorSnapshot> snapshot =
            snapshot_cursor_callback ? snapshot_cursor_callback(cursor_designator) : std::nullopt;
        if (!snapshot.has_value()) {
            if (explicit_alias) {
                throw std::runtime_error(runtime_text(
                    "Runtime.Prg.RuntimeSurface.Error.CpDbfAliasNotFound",
                    {{"alias", cursor_designator}}));
            }
            return make_number_value(0.0);
        }

        return make_number_value(static_cast<double>(snapshot->code_page.value_or(0)));
    }
    // GETPICT([cTitle [, cFileName]]) — headless contract: emit payload and preserve
    // the current selection when the host does not provide a replacement.
    if (function == "getpict") {
        const std::string title = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        const std::string current_file = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
        if (record_event_callback) {
            std::ostringstream detail;
            detail << "mode=headless";
            detail << " title=" << std::quoted(title);
            detail << " current=" << std::quoted(current_file);
            detail << " result=" << std::quoted(current_file);
            record_event_callback("runtime.getpict", detail.str());
        }
        return make_string_value(current_file);
    }
    // GETCOLOR([nDefaultColor [, cTitle]]) — headless contract: emit payload and
    // preserve the provided default color when the host does not override it.
    if (function == "getcolor") {
        const double default_color = arguments.empty() ? 0.0 : value_as_number(arguments[0]);
        const std::string title = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
        if (record_event_callback) {
            std::ostringstream detail;
            detail << "mode=headless";
            detail << " default=" << std::llround(default_color);
            detail << " title=" << std::quoted(title);
            detail << " result=" << std::llround(default_color);
            record_event_callback("runtime.getcolor", detail.str());
        }
        return make_number_value(default_color);
    }
    // GETFONT([cFontName [, nFontSize [, cFontStyle]]]) — headless contract: emit
    // payload and preserve the provided current font when the host does not override it.
    if (function == "getfont") {
        const std::string font_name = !arguments.empty() ? value_as_string(arguments[0]) : std::string{};
        const long long font_size =
            arguments.size() >= 2U ? std::llround(value_as_number(arguments[1])) : 0LL;
        const std::string font_style = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
        if (record_event_callback) {
            std::ostringstream detail;
            detail << "mode=headless";
            detail << " name=" << std::quoted(font_name);
            if (arguments.size() >= 2U) {
                detail << " size=" << font_size;
            }
            if (arguments.size() >= 3U) {
                detail << " style=" << std::quoted(font_style);
            }
            detail << " result=" << std::quoted(font_name);
            record_event_callback("runtime.getfont", detail.str());
        }
        return make_string_value(font_name);
    }
    // VARREAD() — name of variable currently being read/edited (interactive mode only).
    // Headless contract: report that no interactive read is active and return "".
    if (function == "varread") {
        if (record_event_callback) {
            record_event_callback("runtime.varread", "mode=headless active=false result=\"\"");
        }
        return make_string_value({});
    }
