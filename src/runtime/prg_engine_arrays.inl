// prg_engine_arrays.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        PrgValue mutate_array_function(
            const std::string &function,
            const std::vector<std::string> &raw_arguments,
            const std::vector<PrgValue> &arguments,
            const Frame &frame)
        {
            if (raw_arguments.empty())
            {
                return make_number_value(0.0);
            }
            const auto resolve_array_argument_name = [&](std::size_t index)
            {
                std::string candidate = index < raw_arguments.size() ? trim_copy(raw_arguments[index]) : std::string{};
                if (has_array(candidate, frame))
                {
                    return candidate;
                }
                if (!is_bare_identifier_text(candidate) &&
                    index < arguments.size() &&
                    arguments[index].kind == PrgValueKind::string)
                {
                    const std::string evaluated_name = trim_copy(value_as_string(arguments[index]));
                    if (is_bare_identifier_text(evaluated_name))
                    {
                        candidate = evaluated_name;
                    }
                }
                if (is_bare_identifier_text(candidate))
                {
                    constexpr std::size_t max_array_name_depth = 16U;
                    std::vector<std::string> visited_identifiers;
                    visited_identifiers.reserve(8U);
                    for (std::size_t depth = 0U; depth < max_array_name_depth; ++depth)
                    {
                        const std::string normalized = normalize_memory_variable_identifier(candidate);
                        if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized) != visited_identifiers.end())
                        {
                            break;
                        }
                        visited_identifiers.push_back(normalized);

                        const PrgValue indirect_value = lookup_variable(frame, candidate);
                        if (indirect_value.kind != PrgValueKind::string)
                        {
                            break;
                        }

                        const std::string next = trim_copy(value_as_string(indirect_value));
                        if (next.empty() || next == candidate || !is_bare_identifier_text(next))
                        {
                            break;
                        }

                        candidate = next;
                    }
                }
                return has_array(candidate, frame) ? candidate : canonical_array_name(candidate, frame);
            };
            const std::string array_name = resolve_array_argument_name(0U);
            const std::string normalized_function = normalize_identifier(function);
            RuntimeArray *array = find_array(array_name);
            const NumericBehavior array_numeric_behavior = numeric_behavior(
                [&](const std::string &option_name) -> std::string
                {
                    const auto &set_state = current_set_state();
                    const auto found = set_state.find(normalize_identifier(option_name));
                    return found == set_state.end() ? std::string{} : found->second;
                });
            const auto throw_subscript_out_of_range = [&]() -> void
            {
                throw PrgCompatibilityError(
                    runtime_text("Runtime.Prg.Array.Error.SubscriptOutOfRange"),
                    1234);
            };
            const auto throw_invalid_array_dimensions = [&]() -> void
            {
                throw PrgCompatibilityError(
                    runtime_text("Runtime.Prg.Array.Error.InvalidDimensions"),
                    230);
            };
            const auto checked_array_position = [&](const double raw, const std::size_t upper) -> std::size_t
            {
                const auto converted = checked_truncated_numeric_to_int64(raw);
                if (!converted.has_value() || *converted < 1 ||
                    static_cast<std::uint64_t>(*converted) > upper)
                {
                    throw_subscript_out_of_range();
                }
                return static_cast<std::size_t>(*converted);
            };
            const auto checked_array_integer = [&](const double raw, const int error_code) -> std::int64_t
            {
                const auto converted = checked_truncated_numeric_to_int64(raw);
                if (converted.has_value())
                {
                    return *converted;
                }
                if (error_code == 1234)
                {
                    throw_subscript_out_of_range();
                }
                throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.InvalidArgument"), error_code);
            };

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
                const int display_flag = arguments.size() >= 4U
                    ? static_cast<int>(std::llround(value_as_number(arguments[3])))
                    : 0;
                return populate_directory_array(array_name, skeleton, attributes, display_flag);
            }

            if (normalized_function == "afields")
            {
                const std::string designator = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                return populate_fields_array(array_name, designator);
            }

            if (normalized_function == "aused")
            {
                return populate_used_aliases_array(array_name);
            }

            if (normalized_function == "asessions")
            {
                return populate_sessions_array(array_name);
            }

            if (normalized_function == "afont")
            {
                const std::string font_filter = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                const int size_filter = arguments.size() >= 3U
                                            ? static_cast<int>(std::llround(value_as_number(arguments[2])))
                                            : 0;
                return populate_font_array(array_name, font_filter, size_filter);
            }

            if (normalized_function == "aprinters")
            {
                return populate_printers_array(array_name);
            }

            if (normalized_function == "agetfileversion" && arguments.size() >= 2U)
            {
                const std::string filepath = value_as_string(arguments[1]);
                return populate_file_version_array(array_name, filepath);
            }

            if (normalized_function == "asize")
            {
                const std::size_t rows = arguments.size() >= 2U
                                             ? checked_array_dimension(arguments[1], 0U, true)
                                             : 0U;
                const std::size_t columns = arguments.size() >= 3U
                                                ? checked_array_dimension(arguments[2], 1U, true)
                                                : (array == nullptr ? 1U : array->columns);
                return resize_array(array_name, rows, columns);
            }

            if (normalized_function == "acopy" && raw_arguments.size() >= 2U)
            {
                const std::string target_array_name = resolve_array_argument_name(1U);
                if (array == nullptr || array->values.empty())
                {
                    return make_number_value(0.0);
                }
                const std::size_t source_start = arguments.size() >= 3U
                                                     ? checked_array_position(
                                                           value_as_number(arguments[2]), array->values.size())
                                                     : 1U;
                const std::size_t available = array->values.size() - source_start + 1U;
                std::size_t count = 0U;  // zero is copy_array_values()'s omitted/negative-count sentinel.
                if (arguments.size() >= 4U)
                {
                    const double raw_count = value_as_number(arguments[3]);
                    if (!std::isfinite(raw_count) && array_numeric_behavior == NumericBehavior::copperfin)
                    {
                        throw_subscript_out_of_range();
                    }
                    const auto checked_count = checked_truncated_numeric_to_int64(raw_count);
                    const std::int64_t converted_count = numeric_count_argument(raw_count, array_numeric_behavior);
                    if (converted_count == 0)
                    {
                        return make_number_value(0.0);
                    }
                    if (converted_count > 0)
                    {
                        // COPPERFIN defines an unrepresentably large positive count as "all". An ordinary positive
                        // count beyond the source window is VFP error 1234; VFP9's wrapped positive values follow it.
                        if (!checked_count.has_value() && array_numeric_behavior == NumericBehavior::copperfin)
                        {
                            count = available;
                        }
                        else if (static_cast<std::uint64_t>(converted_count) > available)
                        {
                            throw_subscript_out_of_range();
                        }
                        else
                        {
                            count = static_cast<std::size_t>(converted_count);
                        }
                    }
                }
                const RuntimeArray *target_array = find_array(target_array_name);
                std::size_t target_start = 1U;
                if (arguments.size() >= 5U)
                {
                    if (target_array != nullptr)
                    {
                        target_start = checked_array_position(
                            value_as_number(arguments[4]), target_array->values.size());
                    }
                    else
                    {
                        const auto converted_target_start = checked_truncated_numeric_to_int64(
                            value_as_number(arguments[4]));
                        if (!converted_target_start.has_value() || *converted_target_start < 1 ||
                            static_cast<std::uint64_t>(*converted_target_start) >
                                std::numeric_limits<std::size_t>::max())
                        {
                            throw_subscript_out_of_range();
                        }
                        target_start = static_cast<std::size_t>(*converted_target_start);
                        const std::size_t copy_count = count == 0U ? available : count;
                        if (copy_count > 0U && target_start >
                                                   std::numeric_limits<std::size_t>::max() - (copy_count - 1U))
                        {
                            throw_subscript_out_of_range();
                        }
                    }
                }
                return copy_array_values(array_name, target_array_name, source_start, count, target_start);
            }

            if (array == nullptr)
            {
                return make_number_value(0.0);
            }
            if (normalized_function == "aelement" && arguments.size() >= 2U)
            {
                if (arguments.size() < 3U)
                {
                    return make_number_value(static_cast<double>(
                        checked_array_position(value_as_number(arguments[1]), array->values.size())));
                }
                const std::size_t row = checked_array_position(value_as_number(arguments[1]), array->rows);
                const auto raw_column = checked_truncated_numeric_to_int64(value_as_number(arguments[2]));
                if (!raw_column.has_value() || *raw_column < 1 ||
                    static_cast<std::uint64_t>(*raw_column) > array->columns)
                {
                    throw_invalid_array_dimensions();
                }
                return make_number_value(static_cast<double>(
                    array_linear_index(*array, row, static_cast<std::size_t>(*raw_column))));
            }
            if (normalized_function == "asubscript" && arguments.size() >= 3U)
            {
                const std::size_t element = checked_array_position(value_as_number(arguments[1]), array->values.size());
                const std::int64_t raw_dimension = checked_array_integer(value_as_number(arguments[2]), 1234);
                const std::int64_t maximum_dimension = array->is_two_dimensional ? 2 : 1;
                if (raw_dimension < 1 || raw_dimension > maximum_dimension)
                {
                    throw_subscript_out_of_range();
                }
                const int dimension = static_cast<int>(raw_dimension);
                return make_number_value(static_cast<double>(array_subscript(*array, element, dimension)));
            }
            if (normalized_function == "ascan" && arguments.size() >= 2U)
            {
                const double raw_start = arguments.size() >= 3U ? value_as_number(arguments[2]) : 1.0;
                const double raw_count = arguments.size() >= 4U ? value_as_number(arguments[3]) : -1.0;
                std::int64_t search_column = -1;
                if (arguments.size() >= 5U)
                {
                    search_column = array_numeric_behavior == NumericBehavior::vfp9
                                        ? numeric_count_argument(value_as_number(arguments[4]), array_numeric_behavior)
                                        : checked_array_integer(value_as_number(arguments[4]), 11);
                }
                const std::int64_t raw_flags = arguments.size() >= 6U
                                                   ? checked_array_integer(value_as_number(arguments[5]), 11)
                                                   : 0;
                if (raw_flags < 0 || raw_flags > 31)
                {
                    throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.InvalidArgument"), 11);
                }
                const int flags = static_cast<int>(raw_flags);
                const bool case_insensitive = (flags & 1) != 0;
                const bool predicate_search = (flags & 16) != 0;
                const bool exact_match = (flags & 4) != 0
                                             ? (flags & 2) != 0
                                             : is_set_enabled("exact");
                const std::string predicate_text = predicate_search ? trim_copy(value_as_string(arguments[1])) : std::string{};
                const auto array_value_matches = [&](const PrgValue &left, const PrgValue &right)
                {
                    if (left.is_null || right.is_null)
                    {
                        return left.is_null && right.is_null;
                    }
                    const auto is_live_runtime_object_reference = [&](const PrgValue &value)
                    {
                        int handle = 0;
                        std::string prog_id;
                        if (!parse_object_handle_reference(value, handle, prog_id))
                        {
                            return false;
                        }
                        const auto object = ole_objects.find(handle);
                        return object != ole_objects.end() && object->second.prog_id == prog_id;
                    };
                    const bool left_is_object_reference = is_live_runtime_object_reference(left);
                    const bool right_is_object_reference = is_live_runtime_object_reference(right);
                    if (left_is_object_reference || right_is_object_reference)
                    {
                        return left_is_object_reference && right_is_object_reference &&
                               value_as_string(left) == value_as_string(right);
                    }
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
                    return numeric_prg_values_equal(left, right);
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
                const std::optional<std::size_t> predicate_frame_index =
                    predicate_search && !stack.empty()
                        ? std::optional<std::size_t>{stack.size() - 1U}
                        : std::nullopt;
                auto snapshot_predicate_binding = [&](Frame &predicate_frame, const std::string &name)
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
                    if (const auto local = predicate_frame.locals.find(name); local != predicate_frame.locals.end())
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
                    Frame &predicate_frame = stack.back();
                    for (const std::string &name : predicate_metadata_names)
                    {
                        snapshot_predicate_binding(predicate_frame, name);
                    }
                    snapshot_predicate_binding(predicate_frame, predicate_parameter_name);
                }
                auto restore_predicate_bindings = [&]()
                {
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
                    if (!predicate_frame_index.has_value() || *predicate_frame_index >= stack.size())
                    {
                        return;
                    }
                    Frame &predicate_frame = stack[*predicate_frame_index];
                    for (const auto &[name, value] : saved_locals)
                    {
                        if (value)
                        {
                            predicate_frame.locals[name] = *value;
                        }
                        else
                        {
                            predicate_frame.locals.erase(name);
                        }
                    }
                };
                const auto finish_predicate_scan = [&]()
                {
                    restore_predicate_bindings();
                };
                const std::size_t array_columns = array->columns;
                const std::uint64_t array_binding_identity = array->binding_identity;
                const std::uint64_t array_mutation_generation = array->mutation_generation;
                const auto predicate_value_matches = [&](PrgValue value, std::size_t linear_index)
                {
                    if (!predicate_search)
                    {
                        return false;
                    }
                    if (predicate_block.expression.empty() || stack.empty())
                    {
                        return false;
                    }
                    Frame &predicate_frame = stack.back();
                    const std::size_t row = array_columns > 0U ? (linear_index / array_columns) + 1U : linear_index + 1U;
                    const std::size_t column = array_columns > 0U ? (linear_index % array_columns) + 1U : 1U;
                    assign_variable(predicate_frame, "_ASCANVALUE", value);
                    assign_variable(predicate_frame, "_ASCANINDEX", make_number_value(static_cast<double>(linear_index + 1U)));
                    assign_variable(predicate_frame, "_ASCANROW", make_number_value(static_cast<double>(row)));
                    assign_variable(predicate_frame, "_ASCANCOLUMN", make_number_value(static_cast<double>(column)));
                    if (!predicate_block.parameter.empty())
                    {
                        assign_variable(predicate_frame, predicate_block.parameter, value);
                    }
                    try
                    {
                        return value_as_bool(evaluate_expression(predicate_block.expression, predicate_frame));
                    }
                    catch (...)
                    {
                        finish_predicate_scan();
                        throw;
                    }
                };
                const auto evaluate_predicate_at = [&](std::size_t linear_index)
                {
                    // RQ-CF-PRG-033: reentrant predicate code may invalidate both the
                    // element value and the source array binding before it returns.
                    const PrgValue value = array->values[linear_index];
                    const bool matches = predicate_value_matches(value, linear_index);
                    RuntimeArray *current_array = find_array(array_name);
                    if (current_array == nullptr ||
                        current_array->binding_identity != array_binding_identity ||
                        current_array->mutation_generation != array_mutation_generation)
                    {
                        finish_predicate_scan();
                        throw PrgCompatibilityError(
                            runtime_text("Runtime.Prg.Array.Error.PredicateMutatedSource"),
                            11);
                    }
                    array = current_array;
                    return matches;
                };
                const std::int64_t converted_start = array_numeric_behavior == NumericBehavior::vfp9
                                                         ? numeric_count_argument(raw_start, array_numeric_behavior)
                                                         : checked_array_integer(raw_start, 1234);
                if (converted_start == 0)
                {
                    finish_predicate_scan();
                    throw_subscript_out_of_range();
                }
                const std::size_t start = converted_start < 0
                                              ? 1U
                                              : static_cast<std::size_t>(converted_start);
                if (start > array->values.size())
                {
                    finish_predicate_scan();
                    return make_number_value(0.0);
                }
                const std::int64_t converted_count = checked_array_integer(raw_count, 1234);
                const bool column_scan = search_column > 0 && array->columns > 1U;
                const std::size_t maximum_count = column_scan
                                                      ? (array->rows > start - 1U ? array->rows - (start - 1U) : 0U)
                                                      : array->values.size() - start + 1U;
                if (converted_count > 0 && static_cast<std::uint64_t>(converted_count) > maximum_count)
                {
                    finish_predicate_scan();
                    throw_subscript_out_of_range();
                }
                const std::size_t count = converted_count <= 0
                                              ? 0U
                                              : static_cast<std::size_t>(converted_count);
                if (column_scan)
                {
                    const std::size_t column = static_cast<std::size_t>(search_column);
                    if (column > array->columns)
                    {
                        finish_predicate_scan();
                        throw_subscript_out_of_range();
                    }
                    const std::size_t start_row = start - 1U;
                    const std::size_t available_rows = array->rows > start_row ? array->rows - start_row : 0U;
                    const std::size_t rows_to_scan = count == 0U ? available_rows : std::min(count, available_rows);
                    for (std::size_t row = start_row; row < start_row + rows_to_scan; ++row)
                    {
                        const std::size_t index = (row * array->columns) + (column - 1U);
                        if (index < array->values.size() &&
                            (predicate_search
                                 ? evaluate_predicate_at(index)
                                 : array_value_matches(array->values[index], arguments[1])))
                        {
                            const PrgValue result = make_number_value((flags & 8) != 0
                                                                          ? static_cast<double>(row + 1U)
                                                                          : static_cast<double>(index + 1U));
                            finish_predicate_scan();
                            return result;
                        }
                    }
                    finish_predicate_scan();
                    return make_number_value(0.0);
                }
                const std::size_t begin_index = start - 1U;
                const std::size_t available = array->values.size() - begin_index;
                const std::size_t scan_count = count == 0U ? available : std::min(count, available);
                const std::size_t end_index = begin_index + scan_count;
                for (std::size_t index = begin_index; index < end_index; ++index)
                {
                    if (predicate_search
                            ? evaluate_predicate_at(index)
                            : array_value_matches(array->values[index], arguments[1]))
                    {
                        const PrgValue result = make_number_value((flags & 8) != 0 && array->columns > 1U
                                                                      ? static_cast<double>((index / array->columns) + 1U)
                                                                      : static_cast<double>(index + 1U));
                        finish_predicate_scan();
                        return result;
                    }
                }
                finish_predicate_scan();
                return make_number_value(0.0);
            }
            if (normalized_function == "adel" && arguments.size() >= 2U)
            {
                const std::int64_t raw_row_or_column = arguments.size() >= 3U
                                                           ? checked_array_integer(value_as_number(arguments[2]), 11)
                                                           : 1;
                if (raw_row_or_column < 1 || raw_row_or_column > 2)
                {
                    throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.InvalidArgument"), 11);
                }
                const int row_or_column = static_cast<int>(raw_row_or_column);
                const std::size_t position = checked_array_position(
                    value_as_number(arguments[1]),
                    array->columns > 1U && row_or_column == 2 ? array->columns : array->rows);
                if (array->columns > 1U)
                {
                    if (row_or_column == 2)
                    {
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
                    for (std::size_t index = position - 1U; index + 1U < array->values.size(); ++index)
                    {
                        array->values[index] = array->values[index + 1U];
                    }
                    if (!array->values.empty())
                    {
                        array->values.back() = make_boolean_value(false);
                    }
                }
                mark_array_mutated(*array);
                return make_number_value(1.0);
            }
            if (normalized_function == "ains" && arguments.size() >= 2U)
            {
                const std::int64_t raw_row_or_column = arguments.size() >= 3U
                                                           ? checked_array_integer(value_as_number(arguments[2]), 11)
                                                           : 1;
                if (raw_row_or_column < 1 || raw_row_or_column > 2)
                {
                    throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.InvalidArgument"), 11);
                }
                const int row_or_column = static_cast<int>(raw_row_or_column);
                const std::size_t position = checked_array_position(
                    value_as_number(arguments[1]),
                    array->columns > 1U && row_or_column == 2 ? array->columns : array->rows);
                if (array->columns > 1U)
                {
                    if (row_or_column == 2)
                    {
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
                    for (std::size_t index = array->values.size() - 1U; index > position - 1U; --index)
                    {
                        array->values[index] = array->values[index - 1U];
                    }
                    array->values[position - 1U] = make_boolean_value(false);
                }
                mark_array_mutated(*array);
                return make_number_value(1.0);
            }
            if (normalized_function == "asort")
            {
                const double raw_start = arguments.size() >= 2U ? value_as_number(arguments[1]) : 1.0;
                const double raw_count = arguments.size() >= 3U ? value_as_number(arguments[2]) : -1.0;
                if (!std::isfinite(raw_count) && array_numeric_behavior == NumericBehavior::copperfin)
                {
                    throw_subscript_out_of_range();
                }
                const std::int64_t sort_order = arguments.size() >= 4U
                                                    ? (array_numeric_behavior == NumericBehavior::vfp9
                                                           ? numeric_count_argument(
                                                                 value_as_number(arguments[3]), array_numeric_behavior)
                                                           : checked_array_integer(value_as_number(arguments[3]), 11))
                                                    : 0;
                if (array_numeric_behavior == NumericBehavior::copperfin &&
                    (sort_order < 0 || sort_order > 4))
                {
                    throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.InvalidArgument"), 11);
                }
                const bool descending = sort_order == 2 || sort_order == 4;
                const bool case_insensitive = sort_order == 3 || sort_order == 4;
                const auto is_numeric_array_value = [](const PrgValue &value)
                {
                    return value.kind == PrgValueKind::number ||
                           value.kind == PrgValueKind::int64 ||
                           value.kind == PrgValueKind::uint64 ||
                           value.kind == PrgValueKind::currency;
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
                const std::int64_t converted_start = array_numeric_behavior == NumericBehavior::vfp9
                                                         ? numeric_count_argument(raw_start, array_numeric_behavior)
                                                         : checked_array_integer(raw_start, 1234);
                if (converted_start == 0)
                {
                    throw_subscript_out_of_range();
                }
                const std::size_t start = converted_start < 0
                                              ? 1U
                                              : static_cast<std::size_t>(converted_start);
                if (start > array->values.size())
                {
                    throw_subscript_out_of_range();
                }
                const std::int64_t converted_count = numeric_count_argument(raw_count, array_numeric_behavior);
                if (array->columns <= 1U)
                {
                    const std::size_t begin_index = start - 1U;
                    const std::size_t available = array->values.size() - begin_index;
                    const std::size_t count = converted_count <= 0
                                                  ? available
                                                  : static_cast<std::size_t>(std::min<std::uint64_t>(
                                                        static_cast<std::uint64_t>(converted_count), available));
                    std::sort(array->values.begin() + static_cast<std::ptrdiff_t>(begin_index),
                              array->values.begin() + static_cast<std::ptrdiff_t>(begin_index + count),
                              value_less);
                    mark_array_mutated(*array);
                    return make_number_value(1.0);
                }
                const std::size_t start_index = start - 1U;
                const std::size_t start_row = start_index / array->columns;
                const std::size_t sort_column = start_index % array->columns;
                const std::size_t available_rows = array->rows > start_row ? array->rows - start_row : 0U;
                const std::size_t rows_to_sort = converted_count <= 0
                                                     ? available_rows
                                                     : static_cast<std::size_t>(std::min<std::uint64_t>(
                                                           static_cast<std::uint64_t>(converted_count), available_rows));
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
                mark_array_mutated(*array);
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
            const std::string &effective_error_message = current_error_message();
            const AErrorCompatibilitySnapshot &compatibility = current_error_compatibility();
            const int current_code = current_error_code();
            const int effective_error_code = current_code == 0
                                                 ? classify_runtime_error_code(effective_error_message)
                                                 : current_code;
            const std::string error_parameter = runtime_error_parameter(effective_error_message);
            if (effective_error_code == 1526)
            {
                const std::string sql_detail = compatibility.sql_detail.empty()
                                                   ? error_parameter
                                                   : compatibility.sql_detail;
                const std::string sql_state = compatibility.sql_state.empty()
                                                  ? std::string("HY000")
                                                  : compatibility.sql_state;
                std::vector<PrgValue> values{
                    make_number_value(1526.0),
                    make_string_value(effective_error_message),
                    make_string_value(sql_detail),
                    make_string_value(sql_state),
                    make_number_value(static_cast<double>(compatibility.has_sql_native_code ? compatibility.sql_native_code : -1)),
                    compatibility.sql_context.empty() ? make_empty_value() : make_string_value(compatibility.sql_context),
                    compatibility.sql_payload.empty() ? make_empty_value() : make_string_value(compatibility.sql_payload)};
                assign_array(name, std::move(values), 7U);
                return 1;
            }
            if (effective_error_code == 1429)
            {
                const std::string ole_detail = compatibility.ole_detail.empty()
                                                   ? error_parameter
                                                   : compatibility.ole_detail;
                const std::string ole_app = compatibility.ole_app.empty()
                                                ? std::string("Copperfin OLE")
                                                : compatibility.ole_app;
                std::vector<PrgValue> values{
                    make_number_value(1429.0),
                    make_string_value(effective_error_message),
                    make_string_value(ole_detail),
                    make_string_value(ole_app),
                    compatibility.ole_source.empty() ? make_empty_value() : make_string_value(compatibility.ole_source),
                    compatibility.ole_action.empty() ? make_empty_value() : make_string_value(compatibility.ole_action),
                    make_number_value(static_cast<double>(compatibility.has_ole_native_code ? compatibility.ole_native_code : 1429))};
                assign_array(name, std::move(values), 7U);
                return 1;
            }
            std::vector<PrgValue> values{
                make_number_value(static_cast<double>(effective_error_code)),
                make_string_value(effective_error_message),
                make_string_value(
                    compatibility.thrown_user_value.has_value()
                        ? format_value(*compatibility.thrown_user_value)
                        : (error_parameter == effective_error_message ? std::string{} : error_parameter)),
                make_number_value(static_cast<double>(current_error_work_area() == 0 ? current_selected_work_area() : current_error_work_area())),
                make_number_value(static_cast<double>(current_fault_location().line)),
                make_string_value(current_error_procedure()),
                make_string_value(current_fault_statement())};
            assign_array(name, std::move(values), 7U);
            return 1;
        }
