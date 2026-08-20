// prg_engine_variables.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        using VariableStorageIdentity = std::pair<const Frame *, std::string>;

        VariableStorageIdentity resolve_stacked_variable_storage_identity(
            std::size_t frame_index,
            const std::string &name,
            bool include_declared_locals) const
        {
            std::string current_name = normalize_memory_variable_identifier(name);
            while (true)
            {
                const Frame *current_frame = &stack[frame_index];
                const auto binding = current_frame->parameter_reference_bindings.find(current_name);
                if (binding == current_frame->parameter_reference_bindings.end())
                {
                    if (current_frame->locals.contains(current_name) ||
                        (include_declared_locals && current_frame->local_names.contains(current_name)))
                    {
                        return {current_frame, std::move(current_name)};
                    }
                    return {nullptr, std::move(current_name)};
                }

                current_name = normalize_memory_variable_identifier(binding->second);
                if (frame_index == 0U)
                {
                    return {nullptr, std::move(current_name)};
                }
                --frame_index;
            }
        }

        VariableStorageIdentity resolve_variable_storage_identity(
            const Frame &frame,
            const std::string &name,
            bool include_declared_locals) const
        {
            for (std::size_t index = 0U; index < stack.size(); ++index)
            {
                if (&stack[index] == &frame)
                {
                    return resolve_stacked_variable_storage_identity(
                        index,
                        name,
                        include_declared_locals);
                }
            }

            const std::string normalized = normalize_memory_variable_identifier(name);
            if (frame.locals.contains(normalized) ||
                (include_declared_locals && frame.local_names.contains(normalized)))
            {
                return {&frame, normalized};
            }
            return {nullptr, normalized};
        }

        void assign_variable(Frame &frame, const std::string &name, const PrgValue &value)
        {
            const std::string normalized = normalize_memory_variable_identifier(name);
            const VariableStorageIdentity target =
                resolve_variable_storage_identity(frame, normalized, true);
            if (target.first != nullptr)
            {
                const_cast<Frame *>(target.first)->locals[target.second] = value;
            }
            else
            {
                const bool is_top_level_program_frame =
                    stack.size() == 1U &&
                    &stack.front() == &frame &&
                    frame.routine_name == "main";
                if (!is_top_level_program_frame &&
                    !globals.contains(target.second) &&
                    !public_names.contains(target.second) &&
                    !frame.private_saved_values.contains(target.second))
                {
                    frame.private_saved_values.emplace(target.second, std::nullopt);
                }
                globals[target.second] = value;
            }

            for (std::size_t frame_index = 0U; frame_index < stack.size(); ++frame_index)
            {
                Frame &active_frame = stack[frame_index];
                for (const auto &peer_binding : active_frame.parameter_reference_bindings)
                {
                    const VariableStorageIdentity peer =
                        resolve_stacked_variable_storage_identity(frame_index, peer_binding.first, true);
                    if (peer == target)
                    {
                        active_frame.locals[peer_binding.first] = value;
                    }
                }
            }
        }

        bool public_declaration_conflicts(
            const std::string &name,
            bool declaring_array) const
        {
            for (const auto &active_frame : stack)
            {
                if (active_frame.locals.contains(name) ||
                    active_frame.local_names.contains(name) ||
                    active_frame.local_arrays.contains(name) ||
                    active_frame.private_saved_values.contains(name) ||
                    active_frame.private_saved_arrays.contains(name))
                {
                    return true;
                }
            }

            if (public_names.contains(name))
            {
                return declaring_array ? globals.contains(name) : arrays.contains(name);
            }
            return globals.contains(name) || arrays.contains(name);
        }

        const PrgValue *find_variable(const Frame &frame, const std::string &name) const
        {
            const VariableStorageIdentity target =
                resolve_variable_storage_identity(frame, name, false);
            if (target.first != nullptr)
            {
                const auto local = target.first->locals.find(target.second);
                return local == target.first->locals.end() ? nullptr : &local->second;
            }
            if (const auto global = globals.find(target.second); global != globals.end())
            {
                return &global->second;
            }
            return nullptr;
        }

        PrgValue lookup_variable(const Frame &frame, const std::string &name) const
        {
            const PrgValue *value = find_variable(frame, name);
            return value == nullptr ? PrgValue{} : *value;
        }

        std::string canonical_array_name(const std::string &name, const Frame &frame) const
        {
            const std::string normalized = normalize_memory_variable_identifier(name);
            const auto binding = frame.array_reference_bindings.find(normalized);
            return binding == frame.array_reference_bindings.end() ? normalized : binding->second;
        }

        std::string canonical_array_name(const std::string &name) const
        {
            return stack.empty()
                       ? normalize_memory_variable_identifier(name)
                       : canonical_array_name(name, stack.back());
        }

        std::string make_array_copy_reference(const std::string &source_name) const
        {
            return "__copperfin_array_copy__:" + source_name;
        }

        bool is_array_copy_reference(const std::string &reference) const
        {
            return reference.rfind("__copperfin_array_copy__:", 0U) == 0U;
        }

        std::string array_copy_source_name(const std::string &reference) const
        {
            constexpr std::size_t prefix_length = sizeof("__copperfin_array_copy__:") - 1U;
            return reference.size() > prefix_length ? reference.substr(prefix_length) : std::string{};
        }

        std::optional<std::pair<int, std::string>> find_native_object_array_reference(
            const std::string &name,
            const Frame &frame) const
        {
            const std::string reference = trim_copy(name);
            if (reference.find('.') == std::string::npos)
            {
                return std::nullopt;
            }

            std::vector<std::string> segments;
            std::size_t segment_start = 0U;
            while (segment_start < reference.size())
            {
                const std::size_t separator = reference.find('.', segment_start);
                const std::string segment = trim_copy(reference.substr(
                    segment_start,
                    separator == std::string::npos ? std::string::npos : separator - segment_start));
                if (!is_bare_identifier_text(segment))
                {
                    return std::nullopt;
                }
                segments.push_back(segment);
                if (separator == std::string::npos)
                {
                    break;
                }
                segment_start = separator + 1U;
            }
            if (segments.size() < 2U)
            {
                return std::nullopt;
            }

            PrgValue object_reference = lookup_variable(frame, segments.front());
            int object_handle = 0;
            std::string object_prog_id;
            if (!parse_object_handle_reference(object_reference, object_handle, object_prog_id))
            {
                return std::nullopt;
            }

            for (std::size_t index = 1U; index + 1U < segments.size(); ++index)
            {
                const auto object = ole_objects.find(object_handle);
                if (object == ole_objects.end())
                {
                    return std::nullopt;
                }
                const auto property = object->second.properties.find(normalize_identifier(segments[index]));
                if (property == object->second.properties.end() ||
                    !parse_object_handle_reference(property->second, object_handle, object_prog_id))
                {
                    return std::nullopt;
                }
            }

            const std::string property_name = normalize_identifier(segments.back());
            const auto object_arrays = native_object_arrays.find(object_handle);
            if (property_name.empty() || object_arrays == native_object_arrays.end() ||
                !object_arrays->second.contains(property_name))
            {
                return std::nullopt;
            }
            return std::pair<int, std::string>{object_handle, property_name};
        }

        RuntimeArray *find_array(const std::string &name)
        {
            if (stack.empty())
            {
                const auto found = arrays.find(normalize_memory_variable_identifier(name));
                return found == arrays.end() ? nullptr : &found->second;
            }
            return const_cast<RuntimeArray *>(
                static_cast<const Impl *>(this)->find_array(name, stack.back()));
        }

        const RuntimeArray *find_array(const std::string &name) const
        {
            if (stack.empty())
            {
                const auto found = arrays.find(normalize_memory_variable_identifier(name));
                return found == arrays.end() ? nullptr : &found->second;
            }
            return find_array(name, stack.back());
        }

        const RuntimeArray *find_array(const std::string &name, const Frame &frame) const
        {
            if (const auto native_array = find_native_object_array_reference(name, frame); native_array.has_value())
            {
                return &native_object_arrays.at(native_array->first).at(native_array->second);
            }
            const std::string normalized = normalize_memory_variable_identifier(name);
            const auto binding = frame.array_reference_bindings.find(normalized);
            if (binding != frame.array_reference_bindings.end())
            {
                std::size_t frame_index = stack.size();
                for (std::size_t index = 0U; index < stack.size(); ++index)
                {
                    if (&stack[index] == &frame)
                    {
                        frame_index = index;
                        break;
                    }
                }

                const std::string source_name = binding->second;
                while (frame_index > 0U)
                {
                    --frame_index;
                    const auto source = stack[frame_index].local_arrays.find(source_name);
                    if (source != stack[frame_index].local_arrays.end())
                    {
                        return &source->second;
                    }
                }

                const auto global = arrays.find(source_name);
                return global == arrays.end() ? nullptr : &global->second;
            }
            else
            {
                const auto local = frame.local_arrays.find(normalized);
                if (local != frame.local_arrays.end())
                {
                    return &local->second;
                }
            }
            const auto found = arrays.find(normalized);
            return found == arrays.end() ? nullptr : &found->second;
        }

        bool has_array(const std::string &name) const
        {
            return find_array(name) != nullptr;
        }

        bool has_array(const std::string &name, const Frame &frame) const
        {
            return find_array(name, frame) != nullptr;
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

        std::size_t array_length(const std::string &name, int dimension, const Frame &frame) const
        {
            const RuntimeArray *array = find_array(name, frame);
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

        PrgValue array_value(
            const std::string &name,
            std::size_t row,
            std::size_t column,
            const Frame &frame) const
        {
            const RuntimeArray *array = find_array(name, frame);
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
            if (!stack.empty())
            {
                if (const auto native_array = find_native_object_array_reference(name, stack.back()); native_array.has_value())
                {
                    native_object_arrays[native_array->first][native_array->second] = std::move(array);
                    return;
                }
                const std::string normalized = normalize_memory_variable_identifier(name);
                if (!stack.back().array_reference_bindings.contains(normalized))
                {
                    const auto local = stack.back().local_arrays.find(normalized);
                    if (local != stack.back().local_arrays.end())
                    {
                        local->second = std::move(array);
                        return;
                    }
                    if (stack.back().local_names.contains(normalized) || stack.back().locals.contains(normalized))
                    {
                        stack.back().locals.erase(normalized);
                        stack.back().local_arrays[normalized] = std::move(array);
                        return;
                    }
                }
            }
            arrays[canonical_array_name(name)] = std::move(array);
        }

        bool assign_array_copy(const std::string &target_name, const std::string &source_name, const Frame &frame)
        {
            const RuntimeArray *source = find_array(source_name, frame);
            if (source == nullptr)
            {
                return false;
            }

            const std::vector<PrgValue> values = source->values;
            assign_array(target_name, values, source->columns);
            return true;
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
            if (target == nullptr)
            {
                const std::size_t required_rows = (required_elements + target_columns - 1U) / target_columns;
                resize_array(target_name, required_rows, target_columns);
                target = find_array(target_name);
            }
            if (target == nullptr || target_start > target->values.size())
            {
                return make_number_value(0.0);
            }
            const std::size_t target_capacity = target->values.size() - target_start + 1U;
            const std::size_t actual_copy_count = std::min(copy_count, target_capacity);
            if (actual_copy_count == 0U)
            {
                return make_number_value(0.0);
            }

            std::vector<PrgValue> snapshot;
            snapshot.reserve(actual_copy_count);
            for (std::size_t offset = 0U; offset < actual_copy_count; ++offset)
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
            const std::string &attribute_filter,
            int display_flag)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }

            namespace fs = std::filesystem;
            fs::path pattern_path = skeleton.empty()
                ? fs::path("*.*")
                : copperfin::platform::path_from_utf8_string(skeleton);
            if (pattern_path.is_relative())
            {
                pattern_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                    pattern_path;
            }
            const fs::path directory = pattern_path.has_parent_path()
                ? pattern_path.parent_path()
                : copperfin::platform::path_from_utf8_string(current_default_directory());
            const std::string pattern = copperfin::platform::path_to_utf8_string(pattern_path.filename()).empty()
                ? "*.*"
                : copperfin::platform::path_to_utf8_string(pattern_path.filename());
            const std::string requested_attributes = uppercase_copy(attribute_filter);
            const bool include_directories = requested_attributes.find('D') != std::string::npos;
            const bool include_hidden = requested_attributes.find('H') != std::string::npos;
            const bool include_system = requested_attributes.find('S') != std::string::npos;

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
                    if (copperfin::platform::path_is_hidden(entry.path()) && !include_hidden)
                    {
                        continue;
                    }
                    if (copperfin::platform::path_is_system(entry.path()) && !include_system)
                    {
                        continue;
                    }
                    if (!wildcard_match_insensitive(
                            pattern, copperfin::platform::path_to_utf8_string(entry.path().filename())))
                    {
                        continue;
                    }
                    entries.push_back(entry);
                }
            }
            std::sort(entries.begin(), entries.end(), [](const fs::directory_entry &left, const fs::directory_entry &right)
                      { return lowercase_copy(copperfin::platform::path_to_utf8_string(left.path().filename())) <
                               lowercase_copy(copperfin::platform::path_to_utf8_string(right.path().filename())); });

            std::vector<PrgValue> values;
            if (entries.empty())
            {
                // VFP9 leaves a pre-existing target array unchanged when no
                // matching entry is found, and does not create a new array.
                return make_number_value(0.0);
            }
            values.reserve(entries.size() * 5U);
            for (const auto &entry : entries)
            {
                const auto last_write = entry.last_write_time(ignored);
                const bool is_directory = entry.is_directory(ignored);
                std::string name = copperfin::platform::path_to_utf8_string(entry.path().filename());
                if (display_flag == 0)
                {
                    name = uppercase_copy(std::move(name));
                }
                else if (display_flag == 2)
                {
                    name = copperfin::platform::path_dos_8dot3_filename(entry.path());
                }
                values.push_back(make_string_value(std::move(name)));
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
        // Scan the host font directories when possible and retain a deterministic
        // fallback for headless hosts. VFP returns a 1- or 2-column array depending
        // on whether cFontName is supplied.
        PrgValue populate_font_array(const std::string &target_name,
                                     const std::string &font_name_filter,
                                     int size_filter)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }

            const auto trim_font_display_name = [](std::string value) {
                for (char &ch : value)
                {
                    if (ch == '_' || ch == '-')
                    {
                        ch = ' ';
                    }
                }

                std::string normalized;
                normalized.reserve(value.size());
                bool previous_was_space = true;
                for (const unsigned char ch : value)
                {
                    if (std::isspace(ch) != 0)
                    {
                        if (!previous_was_space)
                        {
                            normalized.push_back(' ');
                        }
                        previous_was_space = true;
                        continue;
                    }

                    normalized.push_back(static_cast<char>(ch));
                    previous_was_space = false;
                }

                return trim_copy(normalized);
            };

            const auto fallback_font_names = []() {
                return std::vector<std::string>{
                    "Arial",
                    "Courier New",
                    "Helvetica",
                    "Lucida Console",
                    "Times New Roman",
                };
            };

            const auto looks_like_font_extension = [](const std::filesystem::path &path) {
                const std::string extension = lowercase_copy(
                    copperfin::platform::path_to_utf8_string(path.extension()));
                return extension == ".ttf" ||
                       extension == ".ttc" ||
                       extension == ".otf" ||
                       extension == ".otc" ||
                       extension == ".fon" ||
                       extension == ".fnt" ||
                       extension == ".pfb" ||
                       extension == ".pfm";
            };

            const auto collect_host_font_names = [&]() {
                namespace fs = std::filesystem;
                const std::vector<fs::path> search_roots =
                    platform::font_search_directories();

                std::vector<std::string> names;
                std::set<std::string> seen;
                std::error_code ignored;
                for (const fs::path &root : search_roots)
                {
                    if (root.empty() || !fs::exists(root, ignored))
                    {
                        continue;
                    }

                    for (const fs::directory_entry &entry : fs::recursive_directory_iterator(
                             root,
                             fs::directory_options::skip_permission_denied,
                             ignored))
                    {
                        if (ignored)
                        {
                            ignored.clear();
                            continue;
                        }
                        if (!entry.is_regular_file(ignored) || !looks_like_font_extension(entry.path()))
                        {
                            continue;
                        }

                        const std::string display_name = trim_font_display_name(
                            copperfin::platform::path_to_utf8_string(entry.path().stem()));
                        if (display_name.empty())
                        {
                            continue;
                        }

                        const std::string key = lowercase_copy(display_name);
                        if (seen.insert(key).second)
                        {
                            names.push_back(display_name);
                        }
                    }
                }

                std::sort(names.begin(), names.end(), [](const std::string &left, const std::string &right) {
                    const std::string normalized_left = lowercase_copy(left);
                    const std::string normalized_right = lowercase_copy(right);
                    if (normalized_left == normalized_right)
                    {
                        return left < right;
                    }
                    return normalized_left < normalized_right;
                });
                return names;
            };

            std::vector<std::string> available_fonts = collect_host_font_names();
            if (available_fonts.empty())
            {
                available_fonts = fallback_font_names();
            }

            if (font_name_filter.empty())
            {
                std::vector<PrgValue> values;
                values.reserve(available_fonts.size());
                for (const auto &name : available_fonts)
                {
                    values.push_back(make_string_value(name));
                }
                assign_array(target_name, std::move(values), 1U);
                return make_number_value(static_cast<double>(available_fonts.size()));
            }

            const std::string upper_filter = uppercase_copy(font_name_filter);
            bool font_found = false;
            for (const auto &name : available_fonts)
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
                // MVP contract: if the host exposes the family name, treat positive
                // sizes as accepted and reserve the explicit list below for size enumeration.
                return make_number_value(1.0);
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
            std::vector<std::string> printer_names = options.printer_enumeration_callback
                ? options.printer_enumeration_callback()
                : copperfin::platform::enumerate_printer_names();

            std::vector<PrgValue> values;
            if (printer_names.empty())
            {
                values.push_back(make_string_value("(none)"));
            }
            else
            {
                values.reserve(printer_names.size());
                for (const std::string &printer_name : printer_names)
                {
                    values.push_back(make_string_value(printer_name));
                }
            }
            const std::size_t printer_count = values.size();
            assign_array(target_name, std::move(values), 1U);
            return make_number_value(static_cast<double>(printer_count));
        }


        // AGETFILEVERSION(aName, cFilename) — fill aName with file-version info.
        // Returns the number of elements filled (7) or 0 on failure.
        // VFP fills 7 rows. We extract available version metadata where practical and
        // fall back to stable row-shape defaults when the file has no readable version block.
        PrgValue populate_file_version_array(const std::string &target_name,
                                             const std::string &filepath)
        {
            if (trim_copy(target_name).empty() || trim_copy(filepath).empty())
            {
                return make_number_value(0.0);
            }
            namespace fs = std::filesystem;
            std::error_code ignored;
            const fs::path path = copperfin::platform::path_from_utf8_string(filepath).lexically_normal();
            fs::path snapshot_root;
            const auto metadata_path = materialize_verified_file_snapshot(
                path,
                snapshot_root,
                "Runtime.Prg.Database.Error.VerifiedBytesUnavailable",
                false,
                true);
            if (!metadata_path.has_value())
            {
                return make_number_value(0.0);
            }

            const auto cleanup_snapshot = [&]()
            {
                if (!snapshot_root.empty())
                {
                    fs::remove_all(snapshot_root, ignored);
                    snapshot_root.clear();
                }
            };
            if (!fs::exists(*metadata_path, ignored))
            {
                cleanup_snapshot();
                return make_number_value(0.0);
            }

            const copperfin::platform::FileVersionMetadata metadata =
                copperfin::platform::read_file_version_metadata(*metadata_path);
            cleanup_snapshot();
            static const std::size_t version_row_count = 7U;
            std::vector<PrgValue> values;
            values.reserve(version_row_count);
            values.push_back(make_string_value(metadata.full_version));
            values.push_back(make_string_value(metadata.file_description));
            values.push_back(make_string_value(metadata.company_name));
            values.push_back(make_string_value(metadata.file_version));
            values.push_back(make_string_value(metadata.product_name));
            values.push_back(make_string_value(metadata.product_version));
            values.push_back(make_string_value(metadata.trademark_or_copyright));
            assign_array(target_name, std::move(values), 1U);
            return make_number_value(static_cast<double>(version_row_count));
        }
