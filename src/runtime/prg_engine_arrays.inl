// prg_engine_arrays.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        PrgValue mutate_array_function(
            const std::string &function,
            const std::vector<std::string> &raw_arguments,
            const std::vector<PrgValue> &arguments)
        {
            if (raw_arguments.empty())
            {
                return make_number_value(0.0);
            }
            const std::string array_name = raw_arguments[0];
            const std::string normalized_function = normalize_identifier(function);
            RuntimeArray *array = find_array(array_name);

            if (normalized_function == "alines" && arguments.size() >= 2U)
            {
                const int flags = arguments.size() >= 3U ? static_cast<int>(std::llround(value_as_number(arguments[2]))) : 0;
                std::vector<std::string> parse_tokens;
                for (std::size_t index = 3U; index < arguments.size(); ++index)
                {
                    parse_tokens.push_back(value_as_string(arguments[index]));
                }
                return populate_lines_array(array_name, value_as_string(arguments[1]), flags, parse_tokens);
            }

            if (normalized_function == "adir")
            {
                const std::string skeleton = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{"*.*"};
                const std::string attributes = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                return populate_directory_array(array_name, skeleton, attributes);
            }

            if (normalized_function == "afields")
            {
                const std::string designator = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                return populate_fields_array(array_name, designator);
            }

            if (normalized_function == "asize")
            {
                const std::size_t rows = arguments.size() >= 2U
                                             ? static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[1])))
                                             : 0U;
                const std::size_t columns = arguments.size() >= 3U
                                                ? static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[2])))
                                                : 1U;
                return resize_array(array_name, rows, columns);
            }

            if (normalized_function == "acopy" && raw_arguments.size() >= 2U)
            {
                const std::size_t source_start = arguments.size() >= 3U
                                                     ? static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[2])))
                                                     : 1U;
                const std::size_t count = arguments.size() >= 4U
                                              ? static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[3])))
                                              : 0U;
                const std::size_t target_start = arguments.size() >= 5U
                                                     ? static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[4])))
                                                     : 1U;
                return copy_array_values(array_name, raw_arguments[1], source_start, count, target_start);
            }

            if (array == nullptr)
            {
                return make_number_value(0.0);
            }
            if (normalized_function == "aelement" && arguments.size() >= 2U)
            {
                const std::size_t row = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[1])));
                const std::size_t column = arguments.size() >= 3U
                                               ? static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[2])))
                                               : 1U;
                return make_number_value(static_cast<double>(array_linear_index(*array, row, column)));
            }
            if (normalized_function == "asubscript" && arguments.size() >= 3U)
            {
                const std::size_t element = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[1])));
                const int dimension = static_cast<int>(std::llround(value_as_number(arguments[2])));
                return make_number_value(static_cast<double>(array_subscript(*array, element, dimension)));
            }
            if (normalized_function == "ascan" && arguments.size() >= 2U)
            {
                const double raw_start = arguments.size() >= 3U ? value_as_number(arguments[2]) : 1.0;
                const double raw_count = arguments.size() >= 4U ? value_as_number(arguments[3]) : -1.0;
                const int search_column = arguments.size() >= 5U ? static_cast<int>(std::llround(value_as_number(arguments[4]))) : -1;
                const int flags = arguments.size() >= 6U ? static_cast<int>(std::llround(value_as_number(arguments[5]))) : 0;
                const bool case_insensitive = (flags & 1) != 0;
                const bool predicate_search = (flags & 16) != 0;
                const bool exact_match = (flags & 4) != 0
                                             ? (flags & 2) != 0
                                             : is_set_enabled("exact");
                const std::string predicate_text = predicate_search ? trim_copy(value_as_string(arguments[1])) : std::string{};
                const auto array_value_matches = [&](const PrgValue &left, const PrgValue &right)
                {
                    if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                    {
                        std::string left_value = value_as_string(left);
                        std::string right_value = value_as_string(right);
                        if (case_insensitive)
                        {
                            left_value = uppercase_copy(std::move(left_value));
                            right_value = uppercase_copy(std::move(right_value));
                        }
                        return exact_match
                                   ? left_value == right_value
                                   : left_value.rfind(right_value, 0U) == 0U;
                    }
                    if (left.kind == PrgValueKind::boolean || right.kind == PrgValueKind::boolean)
                    {
                        return value_as_bool(left) == value_as_bool(right);
                    }
                    return std::abs(value_as_number(left) - value_as_number(right)) < 0.000001;
                };
                auto parse_predicate_block = [](const std::string &text)
                {
                    struct PredicateBlock
                    {
                        std::string parameter;
                        std::string expression;
                    };
                    PredicateBlock block{std::string{}, trim_copy(text)};
                    if (text.size() >= 5U && text[0] == '{' && text[1] == '|')
                    {
                        const std::size_t parameter_end = text.find('|', 2U);
                        if (parameter_end != std::string::npos)
                        {
                            const std::size_t close = text.rfind('}');
                            const std::size_t expression_end = close == std::string::npos || close <= parameter_end
                                                                   ? text.size()
                                                                   : close;
                            block.parameter = trim_copy(text.substr(2U, parameter_end - 2U));
                            block.expression = trim_copy(text.substr(parameter_end + 1U, expression_end - parameter_end - 1U));
                        }
                    }
                    return block;
                };
                const auto predicate_block = parse_predicate_block(predicate_text);
                const std::array<std::string, 4U> predicate_metadata_names = {
                    normalize_memory_variable_identifier("_ASCANVALUE"),
                    normalize_memory_variable_identifier("_ASCANINDEX"),
                    normalize_memory_variable_identifier("_ASCANROW"),
                    normalize_memory_variable_identifier("_ASCANCOLUMN")};
                const std::string predicate_parameter_name = normalize_memory_variable_identifier(predicate_block.parameter);
                std::map<std::string, std::optional<PrgValue>> saved_globals;
                std::map<std::string, std::optional<PrgValue>> saved_locals;
                auto snapshot_predicate_binding = [&](Frame &frame, const std::string &name)
                {
                    if (name.empty() || saved_globals.contains(name))
                    {
                        return;
                    }
                    if (const auto global = globals.find(name); global != globals.end())
                    {
                        saved_globals[name] = global->second;
                    }
                    else
                    {
                        saved_globals[name] = std::nullopt;
                    }
                    if (const auto local = frame.locals.find(name); local != frame.locals.end())
                    {
                        saved_locals[name] = local->second;
                    }
                    else
                    {
                        saved_locals[name] = std::nullopt;
                    }
                };
                if (predicate_search && !stack.empty())
                {
                    Frame &frame = stack.back();
                    for (const std::string &name : predicate_metadata_names)
                    {
                        snapshot_predicate_binding(frame, name);
                    }
                    snapshot_predicate_binding(frame, predicate_parameter_name);
                }
                auto restore_predicate_bindings = [&]()
                {
                    if (stack.empty())
                    {
                        return;
                    }
                    Frame &frame = stack.back();
                    for (const auto &[name, value] : saved_globals)
                    {
                        if (value)
                        {
                            globals[name] = *value;
                        }
                        else
                        {
                            globals.erase(name);
                        }
                    }
                    for (const auto &[name, value] : saved_locals)
                    {
                        if (value)
                        {
                            frame.locals[name] = *value;
                        }
                        else
                        {
                            frame.locals.erase(name);
                        }
                    }
                };
                const auto predicate_value_matches = [&](const PrgValue &value, std::size_t linear_index)
                {
                    if (!predicate_search)
                    {
                        return false;
                    }
                    if (predicate_block.expression.empty() || stack.empty())
                    {
                        return false;
                    }
                    Frame &frame = stack.back();
                    const std::size_t row = array->columns > 0U ? (linear_index / array->columns) + 1U : linear_index + 1U;
                    const std::size_t column = array->columns > 0U ? (linear_index % array->columns) + 1U : 1U;
                    assign_variable(frame, "_ASCANVALUE", value);
                    assign_variable(frame, "_ASCANINDEX", make_number_value(static_cast<double>(linear_index + 1U)));
                    assign_variable(frame, "_ASCANROW", make_number_value(static_cast<double>(row)));
                    assign_variable(frame, "_ASCANCOLUMN", make_number_value(static_cast<double>(column)));
                    if (!predicate_block.parameter.empty())
                    {
                        assign_variable(frame, predicate_block.parameter, value);
                    }
                    return value_as_bool(evaluate_expression(predicate_block.expression, frame));
                };
                const std::size_t start = raw_start <= 0.0
                                              ? 1U
                                              : static_cast<std::size_t>(raw_start);
                const std::size_t count = raw_count <= 0.0
                                              ? 0U
                                              : static_cast<std::size_t>(raw_count);
                if (start == 0U || start > array->values.size())
                {
                    restore_predicate_bindings();
                    return make_number_value(0.0);
                }
                if (search_column > 0 && array->columns > 1U)
                {
                    const std::size_t column = static_cast<std::size_t>(search_column);
                    if (column > array->columns)
                    {
                        restore_predicate_bindings();
                        return make_number_value(0.0);
                    }
                    const std::size_t start_row = (start - 1U) / array->columns;
                    const std::size_t available_rows = array->rows > start_row ? array->rows - start_row : 0U;
                    const std::size_t rows_to_scan = count == 0U ? available_rows : std::min(count, available_rows);
                    for (std::size_t row = start_row; row < start_row + rows_to_scan; ++row)
                    {
                        const std::size_t index = (row * array->columns) + (column - 1U);
                        if (index < array->values.size() &&
                            (predicate_value_matches(array->values[index], index) ||
                             (!predicate_search && array_value_matches(array->values[index], arguments[1]))))
                        {
                            const PrgValue result = make_number_value((flags & 8) != 0
                                                                          ? static_cast<double>(row + 1U)
                                                                          : static_cast<double>(index + 1U));
                            restore_predicate_bindings();
                            return result;
                        }
                    }
                    restore_predicate_bindings();
                    return make_number_value(0.0);
                }
                const std::size_t begin_index = start - 1U;
                const std::size_t available = array->values.size() - begin_index;
                const std::size_t scan_count = count == 0U ? available : std::min(count, available);
                const std::size_t end_index = begin_index + scan_count;
                for (std::size_t index = begin_index; index < end_index; ++index)
                {
                    if (predicate_value_matches(array->values[index], index) ||
                        (!predicate_search && array_value_matches(array->values[index], arguments[1])))
                    {
                        const PrgValue result = make_number_value((flags & 8) != 0 && array->columns > 1U
                                                                      ? static_cast<double>((index / array->columns) + 1U)
                                                                      : static_cast<double>(index + 1U));
                        restore_predicate_bindings();
                        return result;
                    }
                }
                restore_predicate_bindings();
                return make_number_value(0.0);
            }
            if (normalized_function == "adel" && arguments.size() >= 2U)
            {
                const std::size_t position = static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[1])));
                const int row_or_column = arguments.size() >= 3U ? static_cast<int>(std::llround(value_as_number(arguments[2]))) : 1;
                if (array->columns > 1U)
                {
                    if (row_or_column == 2)
                    {
                        if (position == 0U || position > array->columns)
                        {
                            return make_number_value(0.0);
                        }
                        for (std::size_t row = 0U; row < array->rows; ++row)
                        {
                            for (std::size_t column = position - 1U; column + 1U < array->columns; ++column)
                            {
                                array->values[(row * array->columns) + column] =
                                    array->values[(row * array->columns) + column + 1U];
                            }
                            array->values[(row * array->columns) + array->columns - 1U] = make_boolean_value(false);
                        }
                    }
                    else
                    {
                        if (position == 0U || position > array->rows)
                        {
                            return make_number_value(0.0);
                        }
                        for (std::size_t row = position - 1U; row + 1U < array->rows; ++row)
                        {
                            for (std::size_t column = 0U; column < array->columns; ++column)
                            {
                                array->values[(row * array->columns) + column] =
                                    array->values[((row + 1U) * array->columns) + column];
                            }
                        }
                        const std::size_t last_row = array->rows - 1U;
                        for (std::size_t column = 0U; column < array->columns; ++column)
                        {
                            array->values[(last_row * array->columns) + column] = make_boolean_value(false);
                        }
                    }
                }
                else
                {
                    if (position == 0U || position > array->values.size())
                    {
                        return make_number_value(0.0);
                    }
                    for (std::size_t index = position - 1U; index + 1U < array->values.size(); ++index)
                    {
                        array->values[index] = array->values[index + 1U];
                    }
                    if (!array->values.empty())
                    {
                        array->values.back() = make_boolean_value(false);
                    }
                }
                return make_number_value(1.0);
            }
            if (normalized_function == "ains" && arguments.size() >= 2U)
            {
                const std::size_t position = static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[1])));
                const int row_or_column = arguments.size() >= 3U ? static_cast<int>(std::llround(value_as_number(arguments[2]))) : 1;
                if (array->columns > 1U)
                {
                    if (row_or_column == 2)
                    {
                        if (position == 0U || position > array->columns)
                        {
                            return make_number_value(0.0);
                        }
                        for (std::size_t row = 0U; row < array->rows; ++row)
                        {
                            for (std::size_t column = array->columns - 1U; column > position - 1U; --column)
                            {
                                array->values[(row * array->columns) + column] =
                                    array->values[(row * array->columns) + column - 1U];
                            }
                            array->values[(row * array->columns) + position - 1U] = make_boolean_value(false);
                        }
                    }
                    else
                    {
                        if (position == 0U || position > array->rows)
                        {
                            return make_number_value(0.0);
                        }
                        for (std::size_t row = array->rows - 1U; row > position - 1U; --row)
                        {
                            for (std::size_t column = 0U; column < array->columns; ++column)
                            {
                                array->values[(row * array->columns) + column] =
                                    array->values[((row - 1U) * array->columns) + column];
                            }
                        }
                        for (std::size_t column = 0U; column < array->columns; ++column)
                        {
                            array->values[((position - 1U) * array->columns) + column] = make_boolean_value(false);
                        }
                    }
                }
                else
                {
                    if (position == 0U || position > array->values.size())
                    {
                        return make_number_value(0.0);
                    }
                    for (std::size_t index = array->values.size() - 1U; index > position - 1U; --index)
                    {
                        array->values[index] = array->values[index - 1U];
                    }
                    array->values[position - 1U] = make_boolean_value(false);
                }
                return make_number_value(1.0);
            }
            if (normalized_function == "asort")
            {
                const double raw_start = arguments.size() >= 2U ? value_as_number(arguments[1]) : 1.0;
                const double raw_count = arguments.size() >= 3U ? value_as_number(arguments[2]) : -1.0;
                const double raw_order = arguments.size() >= 4U ? value_as_number(arguments[3]) : 0.0;
                const int flags = arguments.size() >= 5U ? static_cast<int>(std::llround(value_as_number(arguments[4]))) : 0;
                const bool descending = raw_order > 0.0;
                const bool case_insensitive = (flags & 1) != 0;
                const auto is_numeric_array_value = [](const PrgValue &value)
                {
                    return value.kind == PrgValueKind::number ||
                           value.kind == PrgValueKind::int64 ||
                           value.kind == PrgValueKind::uint64;
                };
                const auto sort_key = [&](const PrgValue &value)
                {
                    std::string key = value_as_string(value);
                    return case_insensitive ? uppercase_copy(std::move(key)) : key;
                };
                const auto value_less = [&](const PrgValue &left, const PrgValue &right)
                {
                    if (is_numeric_array_value(left) && is_numeric_array_value(right))
                    {
                        const double left_number = value_as_number(left);
                        const double right_number = value_as_number(right);
                        return descending ? right_number < left_number : left_number < right_number;
                    }
                    const std::string left_key = sort_key(left);
                    const std::string right_key = sort_key(right);
                    return descending ? right_key < left_key : left_key < right_key;
                };
                const std::size_t start = raw_start <= 0.0
                                              ? 1U
                                              : static_cast<std::size_t>(raw_start);
                if (start == 0U || start > array->values.size())
                {
                    return make_number_value(-1.0);
                }
                if (array->columns <= 1U)
                {
                    const std::size_t begin_index = start - 1U;
                    const std::size_t available = array->values.size() - begin_index;
                    const std::size_t count = raw_count <= 0.0
                                                  ? available
                                                  : std::min(static_cast<std::size_t>(raw_count), available);
                    std::sort(array->values.begin() + static_cast<std::ptrdiff_t>(begin_index),
                              array->values.begin() + static_cast<std::ptrdiff_t>(begin_index + count),
                              value_less);
                    return make_number_value(1.0);
                }
                const std::size_t start_index = start - 1U;
                const std::size_t start_row = start_index / array->columns;
                const std::size_t sort_column = start_index % array->columns;
                const std::size_t available_rows = array->rows > start_row ? array->rows - start_row : 0U;
                const std::size_t rows_to_sort = raw_count <= 0.0
                                                     ? available_rows
                                                     : std::min(static_cast<std::size_t>(raw_count), available_rows);
                std::vector<std::vector<PrgValue>> rows;
                rows.reserve(rows_to_sort);
                for (std::size_t row = start_row; row < start_row + rows_to_sort; ++row)
                {
                    const auto row_begin = array->values.begin() + static_cast<std::ptrdiff_t>(row * array->columns);
                    rows.emplace_back(row_begin, row_begin + static_cast<std::ptrdiff_t>(array->columns));
                }
                std::sort(rows.begin(), rows.end(), [&](const auto &left, const auto &right)
                          { return value_less(left[sort_column], right[sort_column]); });
                for (std::size_t offset = 0U; offset < rows.size(); ++offset)
                {
                    const std::size_t row = start_row + offset;
                    std::copy(rows[offset].begin(), rows[offset].end(),
                              array->values.begin() + static_cast<std::ptrdiff_t>(row * array->columns));
                }
                return make_number_value(1.0);
            }
            return make_number_value(0.0);
        }

        int populate_error_array(const std::string &name)
        {
            if (trim_copy(name).empty())
            {
                return 0;
            }
            if (last_error_code == 0 && last_error_message.empty())
            {
                return 0;
            }
            const int effective_error_code = last_error_code == 0
                                                 ? classify_runtime_error_code(last_error_message)
                                                 : last_error_code;
            const std::string error_parameter = runtime_error_parameter(last_error_message);
            if (effective_error_code == 1526)
            {
                std::vector<PrgValue> values{
                    make_number_value(1526.0),
                    make_string_value(last_error_message),
                    make_string_value(error_parameter),
                    make_string_value("HY000"),
                    make_number_value(-1.0),
                    make_number_value(0.0),
                    make_empty_value()};
                assign_array(name, std::move(values), 7U);
                return 1;
            }
            if (effective_error_code == 1429)
            {
                std::vector<PrgValue> values{
                    make_number_value(1429.0),
                    make_string_value(last_error_message),
                    make_string_value(error_parameter),
                    make_string_value("Copperfin OLE"),
                    make_empty_value(),
                    make_empty_value(),
                    make_number_value(1429.0)};
                assign_array(name, std::move(values), 7U);
                return 1;
            }
            std::vector<PrgValue> values{
                make_number_value(static_cast<double>(effective_error_code)),
                make_string_value(last_error_message),
                make_string_value(error_parameter == last_error_message ? std::string{} : error_parameter),
                make_number_value(static_cast<double>(current_selected_work_area())),
                make_empty_value(),
                make_empty_value(),
                make_empty_value()};
            assign_array(name, std::move(values), 7U);
            return 1;
        }

