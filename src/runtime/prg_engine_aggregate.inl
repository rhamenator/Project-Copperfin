// prg_engine_aggregate.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        PrgValue aggregate_function_value(
            const std::string &function,
            const std::vector<std::string> &raw_arguments,
            const Frame &frame)
        {
            const auto is_numeric_aggregate_field = [](char field_type)
            {
                switch (field_type)
                {
                case 'N':
                case 'n':
                case 'F':
                case 'f':
                case 'B':
                case 'b':
                case 'I':
                case 'i':
                case 'Y':
                case 'y':
                    return true;
                default:
                    return false;
                }
            };
            const auto first_numeric_field_expression = [&](const CursorState &cursor) -> std::string
            {
                const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(cursor);
                const auto found = std::find_if(
                    fields.begin(),
                    fields.end(),
                    [&](const vfp::DbfFieldDescriptor &field)
                    {
                        return is_numeric_aggregate_field(field.type);
                    });
                return found == fields.end() ? std::string{} : found->name;
            };

            std::string value_expression;
            std::string condition_expression;
            std::string designator;

            if (function == "count")
            {
                if (raw_arguments.size() == 1U)
                {
                    designator = try_parse_designator_argument(raw_arguments[0], frame);
                    if (designator.empty())
                    {
                        condition_expression = raw_arguments[0];
                    }
                }
                else if (raw_arguments.size() >= 2U)
                {
                    condition_expression = raw_arguments[0];
                    designator = try_parse_designator_argument(raw_arguments[1], frame);
                }
            }
            else
            {
                if (!raw_arguments.empty())
                {
                    value_expression = raw_arguments[0];
                }
                if (raw_arguments.size() == 2U)
                {
                    designator = try_parse_designator_argument(raw_arguments[1], frame);
                    if (designator.empty())
                    {
                        condition_expression = raw_arguments[1];
                    }
                }
                else if (raw_arguments.size() >= 3U)
                {
                    condition_expression = raw_arguments[1];
                    designator = try_parse_designator_argument(raw_arguments[2], frame);
                }
            }

            CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr)
            {
                return make_number_value(0.0);
            }

            if (cursor->record_count == 0U || (!cursor->remote && cursor->source_path.empty()))
            {
                return make_number_value(0.0);
            }

            if (function != "count" && trim_copy(value_expression).empty())
            {
                value_expression = first_numeric_field_expression(*cursor);
                if (value_expression.empty())
                {
                    if (function == "min" || function == "max")
                    {
                        return make_empty_value();
                    }
                    return make_number_value(0.0);
                }
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(*cursor);
            double sum = 0.0;
            double min_value = 0.0;
            double max_value = 0.0;
            std::size_t matched_count = 0U;

            for (std::size_t recno = 1U; recno <= cursor->record_count; ++recno)
            {
                move_cursor_to(*cursor, static_cast<long long>(recno));
                if (!current_record_matches_visibility(*cursor, frame, condition_expression))
                {
                    continue;
                }

                if (function == "count")
                {
                    ++matched_count;
                    continue;
                }

                const PrgValue value = evaluate_expression(value_expression, frame, cursor);
                if (value.kind == PrgValueKind::empty)
                {
                    continue;
                }
                if (value.kind == PrgValueKind::string && trim_copy(value.string_value).empty())
                {
                    continue;
                }

                const double numeric_value = value_as_number(value);
                if (matched_count == 0U)
                {
                    min_value = numeric_value;
                    max_value = numeric_value;
                }
                else
                {
                    min_value = std::min(min_value, numeric_value);
                    max_value = std::max(max_value, numeric_value);
                }
                sum += numeric_value;
                ++matched_count;
            }

            restore_cursor_snapshot(*cursor, original);

            if (function == "count")
            {
                return make_number_value(static_cast<double>(matched_count));
            }
            if (matched_count == 0U)
            {
                return make_number_value(0.0);
            }
            if (function == "sum")
            {
                return make_number_value(sum);
            }
            if (function == "avg" || function == "average")
            {
                return make_number_value(sum / static_cast<double>(matched_count));
            }
            if (function == "min")
            {
                return make_number_value(min_value);
            }
            if (function == "max")
            {
                return make_number_value(max_value);
            }
            return make_number_value(0.0);
        }

        std::vector<std::size_t> collect_aggregate_scope_records(
            CursorState &cursor,
            const Frame &frame,
            const AggregateScopeClause &scope,
            const std::string &for_expression,
            const std::string &while_expression)
        {
            std::vector<std::size_t> records;
            if (cursor.record_count == 0U)
            {
                return records;
            }

            std::size_t start_recno = 1U;
            std::size_t end_recno = cursor.record_count;
            switch (scope.kind)
            {
            case AggregateScopeKind::all_records:
                break;
            case AggregateScopeKind::rest_records:
                if (cursor.eof || cursor.recno > cursor.record_count)
                {
                    return records;
                }
                start_recno = cursor.recno == 0U ? 1U : cursor.recno;
                break;
            case AggregateScopeKind::next_records:
            {
                const long long requested = static_cast<long long>(std::llround(value_as_number(evaluate_expression(scope.raw_value, frame, &cursor))));
                if (requested <= 0)
                {
                    return records;
                }
                if (cursor.eof || cursor.recno > cursor.record_count)
                {
                    return records;
                }
                start_recno = cursor.recno == 0U ? 1U : cursor.recno;
                end_recno = std::min(cursor.record_count, start_recno + static_cast<std::size_t>(requested - 1LL));
                break;
            }
            case AggregateScopeKind::record:
            {
                const long long requested = static_cast<long long>(std::llround(value_as_number(evaluate_expression(scope.raw_value, frame, &cursor))));
                if (requested < 1LL || requested > static_cast<long long>(cursor.record_count))
                {
                    return records;
                }
                start_recno = static_cast<std::size_t>(requested);
                end_recno = start_recno;
                break;
            }
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            for (std::size_t recno = start_recno; recno <= end_recno; ++recno)
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                if (!while_expression.empty() && !value_as_bool(evaluate_expression(while_expression, frame, &cursor)))
                {
                    break;
                }
                if (current_record_matches_visibility(cursor, frame, for_expression))
                {
                    records.push_back(recno);
                }
            }
            restore_cursor_snapshot(cursor, original);

            return records;
        }

        PrgValue aggregate_record_values(
            CursorState &cursor,
            const std::string &function,
            const std::string &value_expression,
            const std::vector<std::size_t> &records,
            const Frame &frame)
        {
            if (function == "count")
            {
                return make_number_value(static_cast<double>(records.size()));
            }
            if (records.empty())
            {
                return make_number_value(0.0);
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            double sum = 0.0;
            double min_value = 0.0;
            double max_value = 0.0;
            std::size_t matched_count = 0U;

            for (const std::size_t recno : records)
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                const PrgValue value = evaluate_expression(value_expression, frame, &cursor);
                if (value.kind == PrgValueKind::empty)
                {
                    continue;
                }
                if (value.kind == PrgValueKind::string && trim_copy(value.string_value).empty())
                {
                    continue;
                }

                const double numeric_value = value_as_number(value);
                if (matched_count == 0U)
                {
                    min_value = numeric_value;
                    max_value = numeric_value;
                }
                else
                {
                    min_value = std::min(min_value, numeric_value);
                    max_value = std::max(max_value, numeric_value);
                }
                sum += numeric_value;
                ++matched_count;
            }

            restore_cursor_snapshot(cursor, original);

            if (matched_count == 0U)
            {
                return make_number_value(0.0);
            }
            if (function == "sum")
            {
                return make_number_value(sum);
            }
            if (function == "avg" || function == "average")
            {
                return make_number_value(sum / static_cast<double>(matched_count));
            }
            if (function == "min")
            {
                return make_number_value(min_value);
            }
            if (function == "max")
            {
                return make_number_value(max_value);
            }
            return make_number_value(0.0);
        }

        bool execute_total_command(
            const Statement &statement,
            Frame &frame,
            std::string &error_message)
        {
            const auto parsed = parse_total_command_plan(statement.expression, error_message);
            if (!parsed.has_value())
            {
                return false;
            }

            const TotalCommandPlan &plan = *parsed;
            CursorState *cursor = resolve_cursor_target_expression(plan.in_expression, frame);
            if (cursor == nullptr)
            {
                error_message = plan.in_expression.empty()
                                    ? "TOTAL requires a selected work area"
                                    : "TOTAL target work area not found";
                return false;
            }
            std::vector<vfp::DbfFieldDescriptor> source_fields;
            std::vector<vfp::DbfRecord> source_records;
            if (cursor->remote)
            {
                source_records = cursor->remote_records;
                if (!source_records.empty())
                {
                    source_fields.reserve(source_records.front().values.size());
                    for (const auto &value : source_records.front().values)
                    {
                        vfp::DbfFieldDescriptor field;
                        field.name = value.field_name;
                        field.type = value.field_type == '\0' ? 'C' : value.field_type;
                        if (field.type == 'N' || field.type == 'F')
                        {
                            field.length = 18U;
                            field.decimal_count = 0U;
                        }
                        else
                        {
                            field.length = 32U;
                            field.decimal_count = 0U;
                        }
                        source_fields.push_back(std::move(field));
                    }

                    for (const auto &record : source_records)
                    {
                        for (auto &field : source_fields)
                        {
                            const std::string value_text = record_field_value(record, field.name).value_or(std::string{});
                            if (field.type == 'N' || field.type == 'F')
                            {
                                field.length = static_cast<std::uint8_t>(std::max<int>(field.length, 18));
                                continue;
                            }
                            field.length = static_cast<std::uint8_t>(
                                std::max<int>(field.length, static_cast<int>(std::max<std::size_t>(1U, value_text.size()))));
                        }
                    }
                }
            }
            else
            {
                if (cursor->source_path.empty())
                {
                    error_message = "TOTAL requires a local table-backed cursor";
                    return false;
                }

                const auto table_result = vfp::parse_dbf_table_from_file(cursor->source_path, cursor->record_count);
                if (!table_result.ok)
                {
                    error_message = table_result.error;
                    return false;
                }

                source_fields = table_result.table.fields;
                source_records = table_result.table.records;
            }

            const auto field_by_name = [&](const std::string &field_name) -> const vfp::DbfFieldDescriptor *
            {
                const auto found = std::find_if(
                    source_fields.begin(),
                    source_fields.end(),
                    [&](const vfp::DbfFieldDescriptor &field)
                    {
                        return collapse_identifier(field.name) == collapse_identifier(field_name);
                    });
                return found == source_fields.end() ? nullptr : &*found;
            };
            const auto is_total_numeric_field = [](const vfp::DbfFieldDescriptor &field)
            {
                return field.type == 'N' || field.type == 'F' || field.type == 'I' || field.type == 'Y';
            };
            const auto make_total_output_field = [](const vfp::DbfFieldDescriptor &field)
            {
                vfp::DbfFieldDescriptor output_field = field;
                if (output_field.type == 'I')
                {
                    output_field.length = 4U;
                    output_field.decimal_count = 0U;
                }
                else if (output_field.type == 'Y')
                {
                    output_field.length = 8U;
                    output_field.decimal_count = std::max<std::uint8_t>(output_field.decimal_count, 4U);
                }
                else
                {
                    output_field.length = static_cast<std::uint8_t>(
                        std::max<int>(output_field.length, output_field.decimal_count == 0U ? 18 : 20));
                }
                return output_field;
            };

            const vfp::DbfFieldDescriptor *on_field = field_by_name(plan.on_field_name);
            if (on_field == nullptr)
            {
                error_message = "TOTAL ON field was not found";
                return false;
            }

            std::vector<const vfp::DbfFieldDescriptor *> total_fields;
            if (plan.field_names.empty())
            {
                for (const auto &field : source_fields)
                {
                    if (is_total_numeric_field(field) &&
                        collapse_identifier(field.name) != collapse_identifier(on_field->name))
                    {
                        total_fields.push_back(&field);
                    }
                }
            }
            else
            {
                for (const std::string &field_name : plan.field_names)
                {
                    const vfp::DbfFieldDescriptor *field = field_by_name(field_name);
                    if (field == nullptr)
                    {
                        error_message = "TOTAL field was not found: " + field_name;
                        return false;
                    }
                    if (!is_total_numeric_field(*field))
                    {
                        error_message = "TOTAL only supports numeric FIELDS in the first pass";
                        return false;
                    }
                    total_fields.push_back(field);
                }
            }
            if (total_fields.empty())
            {
                error_message = "TOTAL requires at least one numeric field to total";
                return false;
            }

            std::vector<std::size_t> records = collect_aggregate_scope_records(
                *cursor,
                frame,
                plan.scope,
                plan.for_expression,
                plan.while_expression);
            if (records.empty())
            {
                const std::string target_path = value_as_string(evaluate_expression(plan.target_expression, frame));
                std::vector<vfp::DbfFieldDescriptor> output_fields;
                output_fields.push_back(*on_field);
                for (const auto *field : total_fields)
                {
                    output_fields.push_back(make_total_output_field(*field));
                }
                const auto create_result = vfp::create_dbf_table_file(target_path, output_fields, {});
                if (!create_result.ok)
                {
                    error_message = create_result.error;
                    return false;
                }
                return true;
            }

            struct TotalGroup
            {
                std::string group_value;
                std::vector<double> sums;
            };

            std::vector<TotalGroup> groups;
            const auto append_record_to_group = [&](const vfp::DbfRecord &record)
            {
                const std::string group_value = record_field_value(record, on_field->name).value_or(std::string{});
                if (groups.empty() || groups.back().group_value != group_value)
                {
                    groups.push_back({.group_value = group_value, .sums = std::vector<double>(total_fields.size(), 0.0)});
                }

                for (std::size_t index = 0; index < total_fields.size(); ++index)
                {
                    const std::string value_text = trim_copy(record_field_value(record, total_fields[index]->name).value_or(std::string{}));
                    if (!value_text.empty())
                    {
                        groups.back().sums[index] += std::stod(value_text);
                    }
                }
            };

            for (const std::size_t recno : records)
            {
                if (recno == 0U || recno > source_records.size())
                {
                    continue;
                }
                append_record_to_group(source_records[recno - 1U]);
            }

            std::vector<vfp::DbfFieldDescriptor> output_fields;
            output_fields.push_back(*on_field);
            for (const auto *field : total_fields)
            {
                output_fields.push_back(make_total_output_field(*field));
            }

            std::vector<std::vector<std::string>> output_records;
            output_records.reserve(groups.size());
            for (const auto &group : groups)
            {
                std::vector<std::string> record;
                record.push_back(group.group_value);
                for (std::size_t index = 0; index < total_fields.size(); ++index)
                {
                    record.push_back(format_total_numeric_value(group.sums[index], total_fields[index]->decimal_count));
                }
                output_records.push_back(std::move(record));
            }

            const std::string target_path = value_as_string(evaluate_expression(plan.target_expression, frame));
            const auto create_result = vfp::create_dbf_table_file(target_path, output_fields, output_records);
            if (!create_result.ok)
            {
                error_message = create_result.error;
                return false;
            }

            return true;
        }

        bool execute_calculate_command(
            const Statement &statement,
            Frame &frame,
            std::string &error_message)
        {
            const std::vector<CalculateAssignment> assignments = parse_calculate_assignments(statement.expression);
            if (assignments.empty())
            {
                error_message = "CALCULATE requires one or more aggregate TO/INTO assignments";
                return false;
            }

            for (const auto &assignment : assignments)
            {
                const std::size_t open_paren = assignment.aggregate_expression.find('(');
                const std::size_t close_paren = assignment.aggregate_expression.rfind(')');
                if (open_paren == std::string::npos || close_paren == std::string::npos || close_paren <= open_paren)
                {
                    error_message = "CALCULATE requires aggregate expressions like COUNT() or SUM(field)";
                    return false;
                }

                const std::string function = normalize_identifier(assignment.aggregate_expression.substr(0U, open_paren));
                const std::string inner = trim_copy(assignment.aggregate_expression.substr(open_paren + 1U, close_paren - open_paren - 1U));
                std::vector<std::string> raw_arguments;
                if (!inner.empty())
                {
                    raw_arguments = split_csv_like(inner);
                }
                if (!statement.secondary_expression.empty())
                {
                    if (function == "count")
                    {
                        if (raw_arguments.empty())
                        {
                            raw_arguments.push_back(statement.secondary_expression);
                        }
                        else
                        {
                            raw_arguments[0] = "(" + raw_arguments[0] + ") AND (" + statement.secondary_expression + ")";
                        }
                    }
                    else if (raw_arguments.size() < 2U)
                    {
                        raw_arguments.push_back(statement.secondary_expression);
                    }
                    else
                    {
                        raw_arguments[1] = "(" + raw_arguments[1] + ") AND (" + statement.secondary_expression + ")";
                    }
                }
                if (!statement.tertiary_expression.empty())
                {
                    raw_arguments.push_back(statement.tertiary_expression);
                }

                assign_variable(frame, assignment.variable_name, aggregate_function_value(function, raw_arguments, frame));
            }

            return true;
        }

        bool execute_command_aggregate(
            const Statement &statement,
            Frame &frame,
            const std::string &function,
            std::string &error_message)
        {
            CursorState *cursor = resolve_cursor_target_expression(statement.quaternary_expression, frame);
            if (cursor == nullptr)
            {
                error_message = statement.quaternary_expression.empty()
                                    ? uppercase_copy(function) + " requires a selected work area"
                                    : uppercase_copy(function) + " target work area not found";
                return false;
            }

            const std::string target_text = trim_copy(statement.identifier);
            bool to_array = false;
            std::string array_name;
            if (!target_text.empty() && starts_with_insensitive(target_text, "ARRAY"))
            {
                std::string array_target_tail = trim_copy(target_text.substr(5U));
                if (!array_target_tail.empty() && array_target_tail.front() == ',')
                {
                    array_target_tail = trim_copy(array_target_tail.substr(1U));
                }
                if (array_target_tail.empty())
                {
                    error_message = uppercase_copy(function) + " TO ARRAY requires a target array name";
                    return false;
                }

                std::vector<std::string> array_targets = split_csv_like(array_target_tail);
                for (std::string &candidate : array_targets)
                {
                    candidate = trim_copy(std::move(candidate));
                }
                array_targets.erase(
                    std::remove_if(array_targets.begin(), array_targets.end(), [](const std::string &candidate)
                                   { return candidate.empty(); }),
                    array_targets.end());
                if (array_targets.size() != 1U)
                {
                    error_message = uppercase_copy(function) + " TO ARRAY accepts exactly one array target";
                    return false;
                }

                array_name = array_targets.front();
                to_array = true;
            }

            std::string expression_text;
            const AggregateScopeClause scope = parse_aggregate_scope_clause(statement.expression, expression_text);
            std::vector<std::string> targets;
            if (!to_array && !statement.identifier.empty())
            {
                targets = split_csv_like(statement.identifier);
            }
            for (std::string &target : targets)
            {
                target = trim_copy(std::move(target));
            }

            if (function == "count")
            {
                const std::string normalized_expression = normalize_identifier(expression_text);
                if (!expression_text.empty() && normalized_expression != "all")
                {
                    error_message = "COUNT only supports the first-pass ALL/FOR/TO forms right now";
                    return false;
                }
                if (targets.size() > 1U)
                {
                    error_message = "COUNT TO only accepts a single variable target";
                    return false;
                }

                const std::vector<std::size_t> records = collect_aggregate_scope_records(
                    *cursor,
                    frame,
                    scope,
                    statement.secondary_expression,
                    statement.tertiary_expression);
                const PrgValue result = aggregate_record_values(*cursor, function, {}, records, frame);
                if (to_array)
                {
                    assign_array(array_name, {result}, 1U);
                }
                else if (!targets.empty())
                {
                    assign_variable(frame, targets.front(), result);
                }
                return true;
            }

            const auto is_numeric_aggregate_field = [](char field_type)
            {
                switch (field_type)
                {
                case 'N':
                case 'n':
                case 'F':
                case 'f':
                case 'B':
                case 'b':
                case 'I':
                case 'i':
                case 'Y':
                case 'y':
                    return true;
                default:
                    return false;
                }
            };
            const auto first_numeric_field_expression = [&]() -> std::string
            {
                const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(*cursor);
                const auto found = std::find_if(
                    fields.begin(),
                    fields.end(),
                    [&](const vfp::DbfFieldDescriptor &field)
                    {
                        return is_numeric_aggregate_field(field.type);
                    });
                return found == fields.end() ? std::string{} : found->name;
            };

            std::vector<std::string> expressions;
            const std::string normalized_expression = normalize_identifier(expression_text);
            if (expression_text.empty() || normalized_expression == "all")
            {
                const std::string implicit_expression = first_numeric_field_expression();
                if (implicit_expression.empty())
                {
                    const PrgValue fallback = (function == "min" || function == "max")
                                                  ? make_empty_value()
                                                  : make_number_value(0.0);
                    if (to_array)
                    {
                        assign_array(array_name, {fallback}, 1U);
                    }
                    else if (!targets.empty())
                    {
                        if (targets.size() > 1U)
                        {
                            error_message = uppercase_copy(function) + " TO requires one variable per aggregate expression";
                            return false;
                        }
                        assign_variable(frame, targets.front(), fallback);
                    }
                    return true;
                }
                expressions.push_back(implicit_expression);
            }
            else
            {
                expressions = split_csv_like(expression_text);
                for (std::string &expression : expressions)
                {
                    expression = trim_copy(std::move(expression));
                }
                expressions.erase(
                    std::remove_if(expressions.begin(), expressions.end(), [](const std::string &expression)
                                   { return expression.empty(); }),
                    expressions.end());
                if (expressions.empty())
                {
                    error_message = uppercase_copy(function) + " requires one or more expressions";
                    return false;
                }
            }
            if (!targets.empty() && targets.size() != expressions.size())
            {
                error_message = uppercase_copy(function) + " TO requires one variable per aggregate expression";
                return false;
            }

            const std::vector<std::size_t> records = collect_aggregate_scope_records(
                *cursor,
                frame,
                scope,
                statement.secondary_expression,
                statement.tertiary_expression);

            if (to_array)
            {
                std::vector<PrgValue> array_values;
                array_values.reserve(expressions.size());
                for (const std::string &expression : expressions)
                {
                    array_values.push_back(aggregate_record_values(*cursor, function, expression, records, frame));
                }
                assign_array(array_name, array_values, 1U);
                return true;
            }

            for (std::size_t index = 0; index < expressions.size(); ++index)
            {
                const PrgValue result = aggregate_record_values(*cursor, function, expressions[index], records, frame);
                if (!targets.empty())
                {
                    assign_variable(frame, targets[index], result);
                }
            }

            return true;
        }

        bool execute_seek(
            CursorState &cursor,
            const std::string &search_key,
            bool move_pointer,
            bool preserve_pointer_on_miss,
            const std::string &order_designator,
            std::optional<bool> descending_override = std::nullopt,
            std::string *error_message = nullptr,
            std::string *used_order_name = nullptr,
            std::string *used_order_normalization_hint = nullptr,
            std::string *used_order_collation_hint = nullptr,
            bool *used_order_descending = nullptr)
        {
            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            if (!trim_copy(order_designator).empty() && !activate_order(cursor, order_designator, descending_override))
            {
                if (error_message != nullptr)
                {
                    *error_message = last_error_message;
                }
                restore_cursor_snapshot(cursor, original);
                return false;
            }

            const bool found = seek_in_cursor(cursor, search_key);
            const std::string runtime_error = last_error_message;
            if (used_order_name != nullptr)
            {
                *used_order_name = cursor.active_order_name;
            }
            if (used_order_normalization_hint != nullptr)
            {
                *used_order_normalization_hint = cursor.active_order_normalization_hint;
            }
            if (used_order_collation_hint != nullptr)
            {
                *used_order_collation_hint = cursor.active_order_collation_hint;
            }
            if (used_order_descending != nullptr)
            {
                *used_order_descending = cursor.active_order_descending;
            }
            if (!move_pointer || (!found && preserve_pointer_on_miss))
            {
                cursor.recno = original.recno;
                cursor.bof = original.bof;
                cursor.eof = original.eof;
                if (!move_pointer)
                {
                    cursor.found = original.found;
                }
            }
            cursor.active_order_name = original.active_order_name;
            cursor.active_order_expression = original.active_order_expression;
            cursor.active_order_for_expression = original.active_order_for_expression;
            cursor.active_order_path = original.active_order_path;
            cursor.active_order_normalization_hint = original.active_order_normalization_hint;
            cursor.active_order_collation_hint = original.active_order_collation_hint;
            cursor.active_order_key_domain_hint = original.active_order_key_domain_hint;
            cursor.active_order_descending = original.active_order_descending;

            if (!found && error_message != nullptr && !runtime_error.empty())
            {
                *error_message = runtime_error;
            }

            return found;
        }

        std::string order_function_value(const std::string &designator, bool include_path) const
        {
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr || cursor->active_order_name.empty())
            {
                return {};
            }

            if (!include_path)
            {
                return uppercase_copy(cursor->active_order_name);
            }

            if (!cursor->active_order_path.empty())
            {
                return uppercase_copy(cursor->active_order_path);
            }

            return uppercase_copy(cursor->active_order_name);
        }

        std::string tag_function_value(const std::string &index_file_name, std::size_t tag_number, const std::string &designator) const
        {
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr || cursor->orders.empty())
            {
                return {};
            }

            std::size_t resolved_index = tag_number == 0U ? 0U : tag_number - 1U;
            if (!trim_copy(index_file_name).empty())
            {
                const std::string normalized_target_path = normalize_path(unquote_string(index_file_name));
                const std::string normalized_target_name =
                    collapse_identifier(std::filesystem::path(normalized_target_path.empty() ? index_file_name : normalized_target_path).filename().string());
                std::vector<const CursorState::OrderState *> matching_orders;
                for (const CursorState::OrderState &order : cursor->orders)
                {
                    const std::string normalized_order_path = normalize_path(order.index_path);
                    if ((!normalized_target_path.empty() && normalized_order_path == normalized_target_path) ||
                        collapse_identifier(std::filesystem::path(normalized_order_path).filename().string()) == normalized_target_name)
                    {
                        matching_orders.push_back(&order);
                    }
                }
                if (resolved_index < matching_orders.size())
                {
                    return uppercase_copy(matching_orders[resolved_index]->name);
                }
                return {};
            }

            if (resolved_index >= cursor->orders.size())
            {
                return {};
            }

            return uppercase_copy(cursor->orders[resolved_index].name);
        }
