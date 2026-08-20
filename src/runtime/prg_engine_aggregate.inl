// prg_engine_aggregate.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        std::optional<double> try_parse_aggregate_numeric_value(const PrgValue &value)
        {
            if (value.kind == PrgValueKind::empty)
            {
                return std::nullopt;
            }
            if (value.kind == PrgValueKind::string && trim_copy(value.string_value).empty())
            {
                return std::nullopt;
            }

            try
            {
                return value_as_number(value);
            }
            catch (...)
            {
                return std::nullopt;
            }
        }

        PrgValue aggregate_function_value(
            const std::string &function,
            const std::vector<std::string> &raw_arguments,
            const Frame &frame,
            CursorState *preferred_cursor = nullptr)
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
            std::string while_expression;

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
                    if (designator.empty())
                    {
                        while_expression = raw_arguments[1];
                    }
                    else if (raw_arguments.size() >= 3U)
                    {
                        while_expression = raw_arguments[2];
                    }
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
                    const std::string candidate_designator = try_parse_designator_argument(raw_arguments[1], frame);
                    if (!candidate_designator.empty())
                    {
                        designator = candidate_designator;
                        while_expression = raw_arguments[2];
                    }
                    else
                    {
                        condition_expression = raw_arguments[1];
                        designator = try_parse_designator_argument(raw_arguments[2], frame);
                        if (raw_arguments.size() >= 4U)
                        {
                            while_expression = raw_arguments[3];
                        }
                        else if (designator.empty())
                        {
                            while_expression = raw_arguments[2];
                        }
                    }
                }
            }

            CursorState *cursor = designator.empty() && preferred_cursor != nullptr
                                      ? preferred_cursor
                                      : resolve_cursor_target(designator);
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
                if (!while_expression.empty() && !value_as_bool(evaluate_expression(while_expression, frame, cursor)))
                {
                    break;
                }
                if (!current_record_matches_visibility(*cursor, frame, condition_expression))
                {
                    continue;
                }

                if (function == "count")
                {
                    ++matched_count;
                    continue;
                }

                const auto numeric_value = try_parse_aggregate_numeric_value(
                    evaluate_expression(value_expression, frame, cursor));
                if (!numeric_value.has_value())
                {
                    continue;
                }
                if (matched_count == 0U)
                {
                    min_value = *numeric_value;
                    max_value = *numeric_value;
                }
                else
                {
                    min_value = std::min(min_value, *numeric_value);
                    max_value = std::max(max_value, *numeric_value);
                }
                sum += *numeric_value;
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
            const std::string &while_expression,
            bool honor_set_deleted = true)
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
                if (current_record_matches_visibility(
                        cursor,
                        frame,
                        for_expression,
                        honor_set_deleted))
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
                const auto numeric_value = try_parse_aggregate_numeric_value(
                    evaluate_expression(value_expression, frame, &cursor));
                if (!numeric_value.has_value())
                {
                    continue;
                }
                if (matched_count == 0U)
                {
                    min_value = *numeric_value;
                    max_value = *numeric_value;
                }
                else
                {
                    min_value = std::min(min_value, *numeric_value);
                    max_value = std::max(max_value, *numeric_value);
                }
                sum += *numeric_value;
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
                                    ? runtime_text("Runtime.Prg.Total.Error.RequiresSelectedWorkArea")
                                    : runtime_text("Runtime.Prg.Total.Error.TargetWorkAreaNotFound");
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
                    error_message = runtime_text("Runtime.Prg.Total.Error.RequiresLocalTableBackedCursor");
                    return false;
                }

                const auto table_result = parse_cursor_table(*cursor, cursor->record_count);
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
                error_message = runtime_text("Runtime.Prg.Total.Error.OnFieldNotFound");
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
                        error_message = runtime_text("Runtime.Prg.Total.Error.FieldNotFound", {{"fieldName", field_name}});
                        return false;
                    }
                    if (!is_total_numeric_field(*field))
                    {
                        error_message = runtime_text("Runtime.Prg.Total.Error.OnlyNumericFields");
                        return false;
                    }
                    total_fields.push_back(field);
                }
            }
            if (total_fields.empty())
            {
                error_message = runtime_text("Runtime.Prg.Total.Error.RequiresNumericField");
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
                        if (const auto parsed = try_parse_invariant_double(value_text, true); parsed.has_value())
                        {
                            groups.back().sums[index] += *parsed;
                        }
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
            const auto resolve_target_identifier = [&](const std::string &raw_identifier) -> std::string
            {
                std::string resolved_identifier = apply_with_context(raw_identifier, frame);
                if (!resolved_identifier.empty() && resolved_identifier.front() == '&')
                {
                    const PrgValue expanded_identifier = evaluate_expression(resolved_identifier, frame);
                    const std::string expanded_text = trim_copy(value_as_string(expanded_identifier));
                    if (!expanded_text.empty())
                    {
                        resolved_identifier = expanded_text;
                    }
                }
                return resolved_identifier;
            };

            const std::vector<CalculateAssignment> assignments = parse_calculate_assignments(statement.expression);
            if (assignments.empty())
            {
                error_message = runtime_text("Runtime.Prg.Aggregate.Error.CalculateRequiresAssignments");
                return false;
            }

            for (const auto &assignment : assignments)
            {
                const std::size_t open_paren = assignment.aggregate_expression.find('(');
                const std::size_t close_paren = assignment.aggregate_expression.rfind(')');
                if (open_paren == std::string::npos || close_paren == std::string::npos || close_paren <= open_paren)
                {
                    error_message = runtime_text("Runtime.Prg.Aggregate.Error.CalculateRequiresAggregateExpression");
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
                if (!statement.quaternary_expression.empty())
                {
                    raw_arguments.push_back(statement.quaternary_expression);
                }

                assign_variable(frame,
                                resolve_target_identifier(assignment.variable_name),
                                aggregate_function_value(function, raw_arguments, frame));
            }

            return true;
        }

        bool execute_command_aggregate(
            const Statement &statement,
            Frame &frame,
            const std::string &function,
            std::string &error_message)
        {
            const auto resolve_target_identifier = [&](const std::string &raw_identifier) -> std::string
            {
                std::string resolved_identifier = apply_with_context(raw_identifier, frame);
                if (!resolved_identifier.empty() && resolved_identifier.front() == '&')
                {
                    const PrgValue expanded_identifier = evaluate_expression(resolved_identifier, frame);
                    const std::string expanded_text = trim_copy(value_as_string(expanded_identifier));
                    if (!expanded_text.empty())
                    {
                        resolved_identifier = expanded_text;
                    }
                }
                return resolved_identifier;
            };

            CursorState *cursor = resolve_cursor_target_expression(statement.quaternary_expression, frame);
            if (cursor == nullptr)
            {
                const localization::PlaceholderMap function_placeholders{
                    {"function", uppercase_copy(function)}
                };
                error_message = statement.quaternary_expression.empty()
                                    ? runtime_text("Runtime.Prg.Aggregate.Error.RequiresSelectedWorkArea", function_placeholders)
                                    : runtime_text("Runtime.Prg.Aggregate.Error.TargetWorkAreaNotFound", function_placeholders);
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
                    error_message = runtime_text(
                        "Runtime.Prg.Aggregate.Error.ToArrayRequiresTargetArrayName",
                        {
                            {"function", uppercase_copy(function)},
                            {"toKeyword", "TO"},
                            {"arrayKeyword", "ARRAY"},
                        });
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
                    error_message = runtime_text(
                        "Runtime.Prg.Aggregate.Error.ToArraySingleTargetOnly",
                        {
                            {"function", uppercase_copy(function)},
                            {"toKeyword", "TO"},
                            {"arrayKeyword", "ARRAY"},
                        });
                    return false;
                }

                array_name = resolve_target_identifier(array_targets.front());
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
                target = resolve_target_identifier(trim_copy(std::move(target)));
            }

            if (function == "count")
            {
                if (targets.size() > 1U)
                {
                    error_message = runtime_text("Runtime.Prg.Aggregate.Error.CountToSingleTarget");
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
                            error_message = runtime_text(
                                "Runtime.Prg.Aggregate.Error.ToRequiresVariablePerAggregateExpression",
                                {
                                    {"function", uppercase_copy(function)},
                                    {"toKeyword", "TO"},
                                });
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
                    error_message = runtime_text(
                        "Runtime.Prg.Aggregate.Error.RequiresExpressions",
                        {{"function", uppercase_copy(function)}});
                    return false;
                }
            }
            if (!targets.empty() && targets.size() != expressions.size())
            {
                error_message = runtime_text(
                    "Runtime.Prg.Aggregate.Error.ToRequiresVariablePerAggregateExpression",
                    {
                        {"function", uppercase_copy(function)},
                        {"toKeyword", "TO"},
                    });
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
            const Frame &frame,
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

            std::string rushmore_seek_detail;
            if (options.rushmore_planning.enabled &&
                trim_copy(order_designator).empty() &&
                !cursor.remote &&
                !cursor.active_order_path.empty() &&
                cursor.active_order_for_expression.empty())
            {
                const std::string normalized_order_expression =
                    collapse_identifier(unquote_identifier(trim_copy(cursor.active_order_expression)));
                const auto fields = cursor_field_descriptors(cursor);
                std::optional<std::string> matched_field;
                for (const auto &field : fields)
                {
                    const std::string normalized_field = collapse_identifier(field.name);
                    if (normalized_field.empty())
                    {
                        continue;
                    }
                    const bool direct_match = normalized_order_expression == normalized_field;
                    const bool normalized_match =
                        normalized_order_expression == "UPPER" + normalized_field ||
                        normalized_order_expression == "LOWER" + normalized_field ||
                        normalized_order_expression == "ALLTRIM" + normalized_field ||
                        normalized_order_expression == "LTRIM" + normalized_field ||
                        normalized_order_expression == "RTRIM" + normalized_field;
                    if (direct_match || normalized_match)
                    {
                        if (matched_field.has_value())
                        {
                            matched_field.reset();
                            break;
                        }
                        matched_field = field.name;
                    }
                }

                if (matched_field.has_value())
                {
                    const RushmoreCursorMetadata metadata{
                        cursor.alias,
                        cursor.active_order_expression,
                        static_cast<std::uint64_t>(cursor.record_count),
                        0,
                        std::nullopt};
                    const RushmorePredicateDescriptor predicate{
                        .normalized_expression = *matched_field + " =",
                        .field_name = *matched_field,
                        .operation = "=",
                        .complexity_units = 1U,
                        .exact_match = is_set_enabled("exact")};
                    const RushmoreCostModelInput table_scan_input{
                        RushmorePlanKind::table_scan,
                        metadata,
                        predicate,
                        0};
                    const RushmoreCostModelInput index_seek_input{
                        RushmorePlanKind::index_seek,
                        metadata,
                        predicate,
                        0};
                    const auto table_scan_cost = rushmore_estimate_plan_cost(
                        table_scan_input,
                        options.rushmore_planning.cost_model);
                    const auto index_seek_cost = rushmore_estimate_plan_cost(
                        index_seek_input,
                        options.rushmore_planning.cost_model);
                    if (index_seek_cost < table_scan_cost)
                    {
                        rushmore_seek_detail = "SEEK -> index_seek via " + cursor.active_order_name + " (" +
                            runtime_text("Runtime.IndexSeek.PlanDecision.SeekCostModelSelected") + ")";
                    }
                    else
                    {
                        rushmore_seek_detail = "SEEK -> fallback (" +
                            runtime_text("Runtime.IndexSeek.PlanDecision.SeekCostModelRejected") + ")";
                    }
                }
                else
                {
                    rushmore_seek_detail = "SEEK -> fallback (" +
                        runtime_text("Runtime.IndexSeek.PlanDecision.SeekMetadataUnavailable") + ")";
                }
            }

            bool found = false;
            try
            {
                found = seek_in_cursor(cursor, search_key, frame, &original);
            }
            catch (...)
            {
                restore_cursor_snapshot(cursor, original);
                throw;
            }
            const std::string runtime_error = last_error_message;
            if (!rushmore_seek_detail.empty())
            {
                events.push_back({.category = "runtime.rushmore", .detail = rushmore_seek_detail});
            }
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
            restore_cursor_order_snapshot(cursor, original);

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
                const std::vector<const CursorState::OrderState *> matching_orders =
                    matching_orders_for_index_file(*cursor, index_file_name);
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

        std::vector<const CursorState::OrderState *> matching_orders_for_index_file(
            const CursorState &cursor,
            const std::string &index_file_name) const
        {
            std::vector<const CursorState::OrderState *> matching_orders;
            const std::string normalized_target_path = normalize_path(unquote_string(index_file_name));
            const std::string normalized_target_name =
                collapse_identifier(copperfin::platform::path_to_utf8_string(
                    copperfin::platform::path_from_utf8_string(
                        normalized_target_path.empty() ? index_file_name : normalized_target_path).filename()));
            for (const CursorState::OrderState &order : cursor.orders)
            {
                const std::string normalized_order_path = normalize_path(order.index_path);
                if ((!normalized_target_path.empty() && normalized_order_path == normalized_target_path) ||
                    collapse_identifier(copperfin::platform::path_to_utf8_string(
                        copperfin::platform::path_from_utf8_string(normalized_order_path).filename())) ==
                        normalized_target_name)
                {
                    matching_orders.push_back(&order);
                }
            }
            return matching_orders;
        }

        std::size_t tagno_function_value(
            const std::string &index_name,
            const std::string &index_file_name,
            const std::string &designator) const
        {
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr)
            {
                return 0U;
            }

            if (trim_copy(index_name).empty())
            {
                if (cursor->active_order_name.empty() || cursor->active_order_path.empty())
                {
                    return 0U;
                }
                for (std::size_t index = 0; index < cursor->orders.size(); ++index)
                {
                    const CursorState::OrderState &order = cursor->orders[index];
                    if (collapse_identifier(order.name) == collapse_identifier(cursor->active_order_name) &&
                        normalize_path(order.index_path) == normalize_path(cursor->active_order_path))
                    {
                        return index + 1U;
                    }
                }
                return 0U;
            }

            const std::string normalized_name = collapse_identifier(unquote_string(index_name));
            const std::string normalized_index_path = normalize_path(unquote_string(index_file_name));
            const std::string normalized_index_file_name = collapse_identifier(copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(normalized_index_path.empty() ? index_file_name : normalized_index_path).filename()));
            for (std::size_t index = 0; index < cursor->orders.size(); ++index)
            {
                const CursorState::OrderState &candidate = cursor->orders[index];
                const std::string candidate_path = normalize_path(candidate.index_path);
                if (!trim_copy(index_file_name).empty() &&
                    candidate_path != normalized_index_path &&
                    collapse_identifier(copperfin::platform::path_to_utf8_string(
                        copperfin::platform::path_from_utf8_string(candidate_path).filename())) != normalized_index_file_name)
                {
                    continue;
                }
                const std::string order_file_name = collapse_identifier(copperfin::platform::path_to_utf8_string(
                    copperfin::platform::path_from_utf8_string(candidate.index_path).filename()));
                if (collapse_identifier(candidate.name) == normalized_name || order_file_name == normalized_name)
                {
                    return index + 1U;
                }
            }
            return 0U;
        }

        std::string key_function_value(const std::string &index_file_name, std::size_t index_number, const std::string &designator) const
        {
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr || cursor->orders.empty() || index_number == 0U)
            {
                return {};
            }

            const std::size_t resolved_index = index_number - 1U;
            if (!trim_copy(index_file_name).empty())
            {
                const std::vector<const CursorState::OrderState *> matching_orders =
                    matching_orders_for_index_file(*cursor, index_file_name);
                if (resolved_index < matching_orders.size())
                {
                    return matching_orders[resolved_index]->expression;
                }
                return {};
            }

            if (resolved_index >= cursor->orders.size())
            {
                return {};
            }

            return cursor->orders[resolved_index].expression;
        }

        std::size_t tag_count_function_value(const std::string &index_file_name, const std::string &designator) const
        {
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr)
            {
                return 0U;
            }

            if (trim_copy(index_file_name).empty())
            {
                return cursor->orders.size();
            }

            return matching_orders_for_index_file(*cursor, index_file_name).size();
        }
