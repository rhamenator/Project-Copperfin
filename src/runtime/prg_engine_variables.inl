// prg_engine_variables.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        void assign_variable(Frame &frame, const std::string &name, const PrgValue &value)
        {
            const std::string normalized = normalize_memory_variable_identifier(name);
            if (frame.local_names.contains(normalized) || frame.locals.contains(normalized))
            {
                frame.locals[normalized] = value;
                return;
            }
            globals[normalized] = value;
        }

        PrgValue lookup_variable(const Frame &frame, const std::string &name) const
        {
            const std::string normalized = normalize_memory_variable_identifier(name);
            if (const auto local = frame.locals.find(normalized); local != frame.locals.end())
            {
                return local->second;
            }
            if (const auto global = globals.find(normalized); global != globals.end())
            {
                return global->second;
            }
            return {};
        }

        RuntimeArray *find_array(const std::string &name)
        {
            const auto found = arrays.find(normalize_memory_variable_identifier(name));
            return found == arrays.end() ? nullptr : &found->second;
        }

        const RuntimeArray *find_array(const std::string &name) const
        {
            const auto found = arrays.find(normalize_memory_variable_identifier(name));
            return found == arrays.end() ? nullptr : &found->second;
        }

        bool has_array(const std::string &name) const
        {
            return find_array(name) != nullptr;
        }

        std::size_t array_length(const std::string &name, int dimension) const
        {
            const RuntimeArray *array = find_array(name);
            if (array == nullptr)
            {
                return 0U;
            }
            if (dimension == 1)
            {
                return array->rows;
            }
            if (dimension == 2)
            {
                return array->columns;
            }
            return array->values.size();
        }

        PrgValue array_value(const std::string &name, std::size_t row, std::size_t column = 1U) const
        {
            const RuntimeArray *array = find_array(name);
            if (array == nullptr || row == 0U || column == 0U || row > array->rows || column > array->columns)
            {
                return make_empty_value();
            }
            const std::size_t index = ((row - 1U) * array->columns) + (column - 1U);
            return index < array->values.size() ? array->values[index] : make_empty_value();
        }

        std::size_t array_linear_index(const RuntimeArray &array, std::size_t row, std::size_t column) const
        {
            if (row == 0U || column == 0U || row > array.rows || column > array.columns)
            {
                return 0U;
            }
            const std::size_t index = ((row - 1U) * array.columns) + column;
            return index <= array.values.size() ? index : 0U;
        }

        std::size_t array_subscript(const RuntimeArray &array, std::size_t element, int dimension) const
        {
            if (element == 0U || element > array.values.size())
            {
                return 0U;
            }
            if (dimension == 1)
            {
                return ((element - 1U) / array.columns) + 1U;
            }
            if (dimension == 2)
            {
                return ((element - 1U) % array.columns) + 1U;
            }
            return 0U;
        }

        void assign_array(const std::string &name, std::vector<PrgValue> values, std::size_t columns = 1U)
        {
            columns = std::max<std::size_t>(1U, columns);
            RuntimeArray array;
            array.columns = columns;
            array.rows = values.empty() ? 0U : ((values.size() + columns - 1U) / columns);
            array.values = std::move(values);
            array.values.resize(array.rows * array.columns);
            arrays[normalize_memory_variable_identifier(name)] = std::move(array);
        }

        bool parse_array_reference(
            const std::string &reference,
            const Frame &frame,
            std::string &array_name,
            std::size_t &row,
            std::size_t &column)
        {
            const std::string trimmed = trim_copy(reference);
            if (trimmed.empty())
            {
                return false;
            }

            const std::size_t bracket_open = trimmed.find('[');
            const std::size_t paren_open = trimmed.find('(');
            const bool uses_brackets = bracket_open != std::string::npos;
            const std::size_t open = uses_brackets ? bracket_open : paren_open;
            if (open == std::string::npos || open == 0U)
            {
                return false;
            }

            const char close_char = uses_brackets ? ']' : ')';
            if (trimmed.back() != close_char)
            {
                return false;
            }

            array_name = trim_copy(trimmed.substr(0U, open));
            if (array_name.empty())
            {
                return false;
            }
            const std::string indexes_text = trimmed.substr(open + 1U, trimmed.size() - open - 2U);
            const std::vector<std::string> parts = split_csv_like(indexes_text);
            if (parts.empty())
            {
                return false;
            }
            row = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(evaluate_expression(parts[0], frame))));
            column = parts.size() >= 2U
                         ? static_cast<std::size_t>(std::max<double>(0.0, value_as_number(evaluate_expression(parts[1], frame))))
                         : 1U;
            return row > 0U && column > 0U;
        }

        bool assign_array_element(const std::string &reference, const Frame &frame, const PrgValue &value)
        {
            std::string array_name;
            std::size_t row = 0U;
            std::size_t column = 1U;
            if (!parse_array_reference(reference, frame, array_name, row, column))
            {
                return false;
            }

            RuntimeArray *array = find_array(array_name);
            if (array == nullptr || row > array->rows || column > array->columns)
            {
                const std::size_t new_rows = array == nullptr ? row : std::max(row, array->rows);
                const std::size_t new_columns = array == nullptr ? column : std::max(column, array->columns);
                resize_array(array_name, new_rows, new_columns);
                array = find_array(array_name);
            }
            if (array == nullptr || row == 0U || column == 0U || row > array->rows || column > array->columns)
            {
                return false;
            }
            array->values[((row - 1U) * array->columns) + (column - 1U)] = value;
            return true;
        }

        bool declare_array(const std::string &declaration, const Frame &frame)
        {
            std::string array_name;
            std::size_t rows = 0U;
            std::size_t columns = 1U;
            if (!parse_array_reference(declaration, frame, array_name, rows, columns))
            {
                return false;
            }
            resize_array(array_name, rows, columns);
            return true;
        }

        PrgValue resize_array(const std::string &name, std::size_t rows, std::size_t columns = 1U)
        {
            columns = std::max<std::size_t>(1U, columns);
            RuntimeArray *array = find_array(name);
            if (array == nullptr)
            {
                assign_array(name, {}, columns);
                array = find_array(name);
            }
            if (array == nullptr)
            {
                return make_number_value(0.0);
            }
            std::vector<PrgValue> new_values(rows * columns);
            const std::size_t copy_rows = std::min(rows, array->rows);
            const std::size_t copy_columns = std::min(columns, array->columns);
            for (std::size_t row = 0U; row < copy_rows; ++row)
            {
                for (std::size_t column = 0U; column < copy_columns; ++column)
                {
                    new_values[(row * columns) + column] = array->values[(row * array->columns) + column];
                }
            }
            array->rows = rows;
            array->columns = columns;
            array->values = std::move(new_values);
            return make_number_value(static_cast<double>(array->values.size()));
        }

        PrgValue copy_array_values(
            const std::string &source_name,
            const std::string &target_name,
            std::size_t source_start,
            std::size_t count,
            std::size_t target_start)
        {
            RuntimeArray *source = find_array(source_name);
            if (source == nullptr || source->values.empty() || trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }
            source_start = std::max<std::size_t>(1U, source_start);
            target_start = std::max<std::size_t>(1U, target_start);
            if (source_start > source->values.size())
            {
                return make_number_value(0.0);
            }

            const std::size_t available = source->values.size() - source_start + 1U;
            const std::size_t copy_count = count == 0U ? available : std::min(count, available);
            if (copy_count == 0U)
            {
                return make_number_value(0.0);
            }

            RuntimeArray *target = find_array(target_name);
            const std::size_t target_columns = target == nullptr ? 1U : std::max<std::size_t>(1U, target->columns);
            const std::size_t required_elements = target_start + copy_count - 1U;
            if (target == nullptr || required_elements > target->values.size())
            {
                const std::size_t required_rows = (required_elements + target_columns - 1U) / target_columns;
                resize_array(target_name, required_rows, target_columns);
                target = find_array(target_name);
            }
            if (target == nullptr || required_elements > target->values.size())
            {
                return make_number_value(0.0);
            }

            std::vector<PrgValue> snapshot;
            snapshot.reserve(copy_count);
            for (std::size_t offset = 0U; offset < copy_count; ++offset)
            {
                snapshot.push_back(source->values[source_start - 1U + offset]);
            }
            for (std::size_t offset = 0U; offset < snapshot.size(); ++offset)
            {
                target->values[target_start - 1U + offset] = snapshot[offset];
            }
            return make_number_value(static_cast<double>(snapshot.size()));
        }

        PrgValue populate_lines_array(
            const std::string &target_name,
            const std::string &text,
            int flags = 0,
            const std::vector<std::string> &parse_tokens = {})
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }

            std::vector<std::string> lines;
            if (!parse_tokens.empty())
            {
                std::string current = text;
                lines.push_back(current);
                for (const std::string &token : parse_tokens)
                {
                    if (token.empty())
                    {
                        continue;
                    }
                    std::vector<std::string> next;
                    for (const std::string &part : lines)
                    {
                        std::size_t start = 0U;
                        while (true)
                        {
                            const std::size_t found = part.find(token, start);
                            if (found == std::string::npos)
                            {
                                next.push_back(part.substr(start));
                                break;
                            }
                            next.push_back(part.substr(start, found - start));
                            start = found + token.size();
                        }
                    }
                    lines = std::move(next);
                }
            }
            else
            {
                lines = split_text_lines(text);
            }

            const bool trim_lines = (flags & 1) != 0;
            const bool omit_empty = (flags & 2) != 0;
            std::vector<PrgValue> values;
            values.reserve(lines.size());
            for (std::string line : lines)
            {
                if (trim_lines)
                {
                    line = trim_copy(line);
                }
                if (omit_empty && line.empty())
                {
                    continue;
                }
                values.push_back(make_string_value(std::move(line)));
            }
            assign_array(target_name, std::move(values), 1U);
            return make_number_value(static_cast<double>(array_length(target_name, 0)));
        }

        PrgValue populate_directory_array(
            const std::string &target_name,
            const std::string &skeleton,
            const std::string &attribute_filter)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }

            namespace fs = std::filesystem;
            fs::path pattern_path = skeleton.empty() ? fs::path("*.*") : fs::path(skeleton);
            if (pattern_path.is_relative())
            {
                pattern_path = fs::path(current_default_directory()) / pattern_path;
            }
            const fs::path directory = pattern_path.has_parent_path() ? pattern_path.parent_path() : fs::path(current_default_directory());
            const std::string pattern = pattern_path.filename().string().empty() ? "*.*" : pattern_path.filename().string();
            const bool include_directories = normalize_identifier(attribute_filter).find('d') != std::string::npos;

            std::vector<fs::directory_entry> entries;
            std::error_code ignored;
            if (fs::exists(directory, ignored))
            {
                for (const auto &entry : fs::directory_iterator(directory, ignored))
                {
                    const bool is_directory = entry.is_directory(ignored);
                    if (is_directory && !include_directories)
                    {
                        continue;
                    }
                    if (!wildcard_match_insensitive(pattern, entry.path().filename().string()))
                    {
                        continue;
                    }
                    entries.push_back(entry);
                }
            }
            std::sort(entries.begin(), entries.end(), [](const fs::directory_entry &left, const fs::directory_entry &right)
                      { return lowercase_copy(left.path().filename().string()) < lowercase_copy(right.path().filename().string()); });

            std::vector<PrgValue> values;
            values.reserve(entries.size() * 5U);
            for (const auto &entry : entries)
            {
                const auto last_write = entry.last_write_time(ignored);
                const bool is_directory = entry.is_directory(ignored);
                values.push_back(make_string_value(entry.path().filename().string()));
                values.push_back(make_number_value(is_directory ? 0.0 : static_cast<double>(entry.file_size(ignored))));
                values.push_back(make_string_value(format_file_time_part(last_write, true)));
                values.push_back(make_string_value(format_file_time_part(last_write, false)));
                values.push_back(make_string_value(file_attributes_for_adir(entry)));
            }
            assign_array(target_name, std::move(values), 5U);
            return make_number_value(static_cast<double>(entries.size()));
        }

        PrgValue populate_fields_array(const std::string &target_name, const std::string &designator)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }
            CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr)
            {
                assign_array(target_name, {}, 16U);
                return make_number_value(0.0);
            }
            const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(*cursor);

            std::vector<PrgValue> values;
            values.reserve(fields.size() * 16U);
            for (const auto &field : fields)
            {
                values.push_back(make_string_value(field.name));
                values.push_back(make_string_value(std::string(1U, static_cast<char>(std::toupper(static_cast<unsigned char>(field.type))))));
                values.push_back(make_number_value(static_cast<double>(field.length)));
                values.push_back(make_number_value(static_cast<double>(field.decimal_count)));
                values.push_back(make_boolean_value(false));
                values.push_back(make_boolean_value(false));
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
            }
            assign_array(target_name, std::move(values), 16U);
            return make_number_value(static_cast<double>(fields.size()));
        }

        PrgValue populate_used_aliases_array(const std::string &target_name)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }

            std::vector<PrgValue> values;
            values.reserve(current_session_state().cursors.size() * 2U);
            for (const auto &[work_area, cursor] : current_session_state().cursors)
            {
                if (cursor.alias.empty())
                {
                    continue;
                }
                values.push_back(make_string_value(cursor.alias));
                values.push_back(make_number_value(static_cast<double>(work_area)));
            }

            const std::size_t rows = values.size() / 2U;
            assign_array(target_name, std::move(values), 2U);
            return make_number_value(static_cast<double>(rows));
        }

        // ASESSIONS(aName) — populate a 1-column array with active data session IDs.
        // VFP always returns at least 1 (the default session).
        PrgValue populate_sessions_array(const std::string &target_name)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }
            std::vector<PrgValue> values;
            // Always include session 1 (default).  Add any private sessions that
            // have been created (keys > 1 in data_sessions map).
            std::set<int> seen;
            seen.insert(1);
            for (const auto &[session_id, _] : data_sessions)
            {
                seen.insert(session_id);
            }
            for (const int sid : seen)
            {
                values.push_back(make_number_value(static_cast<double>(sid)));
            }
            assign_array(target_name, std::move(values), 1U);
            return make_number_value(static_cast<double>(seen.size()));
        }

        // AFONT(aName [, cFontName [, nSize]]) — populate with available font names.
        // In a headless runtime we return a fixed stub set.  VFP returns a 1- or
        // 2-column array depending on whether cFontName is supplied.
        PrgValue populate_font_array(const std::string &target_name,
                                     const std::string &font_name_filter,
                                     int size_filter)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }
            static const std::vector<std::string> stub_fonts = {
                "Arial", "Courier New", "Times New Roman",
                "Helvetica", "Lucida Console"};

            if (font_name_filter.empty())
            {
                // No filter — return all font names in a 1-column array.
                std::vector<PrgValue> values;
                values.reserve(stub_fonts.size());
                for (const auto &name : stub_fonts)
                {
                    values.push_back(make_string_value(name));
                }
                assign_array(target_name, std::move(values), 1U);
                return make_number_value(static_cast<double>(stub_fonts.size()));
            }

            // Font name supplied — return available sizes (stub: common point sizes).
            // If size_filter > 0 VFP returns .T./.F. for a single size; we model
            // the size-list variant (size_filter == 0).
            const std::string upper_filter = uppercase_copy(font_name_filter);
            bool font_found = false;
            for (const auto &name : stub_fonts)
            {
                if (uppercase_copy(name) == upper_filter)
                {
                    font_found = true;
                    break;
                }
            }
            if (!font_found)
            {
                assign_array(target_name, {}, 1U);
                return make_number_value(0.0);
            }
            static const std::vector<int> common_sizes = {8, 9, 10, 11, 12, 14, 16, 18, 20, 24, 36, 48, 72};
            if (size_filter > 0)
            {
                // Return 1 if the size is in our stub list, 0 otherwise.
                for (int sz : common_sizes)
                {
                    if (sz == size_filter)
                    {
                        return make_number_value(1.0);
                    }
                }
                return make_number_value(0.0);
            }
            std::vector<PrgValue> values;
            values.reserve(common_sizes.size());
            for (int sz : common_sizes)
            {
                values.push_back(make_number_value(static_cast<double>(sz)));
            }
            assign_array(target_name, std::move(values), 1U);
            return make_number_value(static_cast<double>(common_sizes.size()));
        }

        // APRINTERS(aName) — populate with available printer names.
        // Headless runtime: return a single "(none)" entry so PRG code can always
        // inspect the array without crashing.
        PrgValue populate_printers_array(const std::string &target_name)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }
            std::vector<PrgValue> values;
#if defined(__linux__)
            // Attempt a lightweight enumeration via /dev/lp* and cups if available;
            // fall back to the stub entry on any failure.
            std::error_code ignored;
            namespace fs = std::filesystem;
            bool found_any = false;
            if (fs::exists("/dev", ignored))
            {
                for (const auto &entry : fs::directory_iterator("/dev", ignored))
                {
                    const std::string filename = entry.path().filename().string();
                    if (filename.rfind("lp", 0U) == 0U)
                    {
                        values.push_back(make_string_value(entry.path().string()));
                        found_any = true;
                    }
                }
            }
            if (!found_any)
            {
                values.push_back(make_string_value("(none)"));
            }
#else
            values.push_back(make_string_value("(none)"));
#endif
            const std::size_t printer_count = values.size();
            assign_array(target_name, std::move(values), 1U);
            return make_number_value(static_cast<double>(printer_count));
        }

        // AGETFILEVERSION(aName, cFilename) — fill aName with file-version info.
        // Returns the number of elements filled (7) or 0 on failure.
        // VFP fills 7 rows; for non-PE / non-Windows files we return stub metadata
        // if the file exists, or 0 if it does not.
        PrgValue populate_file_version_array(const std::string &target_name,
                                             const std::string &filepath)
        {
            if (trim_copy(target_name).empty() || trim_copy(filepath).empty())
            {
                return make_number_value(0.0);
            }
            namespace fs = std::filesystem;
            std::error_code ignored;
            const fs::path path = fs::path(filepath);
            if (!fs::exists(path, ignored))
            {
                return make_number_value(0.0);
            }
            // Stub version row-shape compatible with VFP AGETFILEVERSION():
            //   [1] Full version string   "0.0.0.0"
            //   [2] File description      (filename stem)
            //   [3] Company name          ""
            //   [4] File version          "0.0.0.0"
            //   [5] Product name          ""
            //   [6] Product version       "0.0.0.0"
            //   [7] Trademark/copyright   ""
            const std::string stem = path.filename().string();
            static const std::size_t version_row_count = 7U;
            std::vector<PrgValue> values;
            values.reserve(version_row_count);
            values.push_back(make_string_value("0.0.0.0"));
            values.push_back(make_string_value(stem));
            values.push_back(make_string_value(""));
            values.push_back(make_string_value("0.0.0.0"));
            values.push_back(make_string_value(""));
            values.push_back(make_string_value("0.0.0.0"));
            values.push_back(make_string_value(""));
            assign_array(target_name, std::move(values), 1U);
            return make_number_value(static_cast<double>(version_row_count));
        }
