// prg_engine_records.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        void restore_cursor_order_snapshot(CursorState &cursor, const CursorPositionSnapshot &snapshot) const
        {
            cursor.active_order_name = snapshot.active_order_name;
            cursor.active_order_expression = snapshot.active_order_expression;
            cursor.active_order_for_expression = snapshot.active_order_for_expression;
            cursor.active_order_path = snapshot.active_order_path;
            cursor.active_order_normalization_hint = snapshot.active_order_normalization_hint;
            cursor.active_order_collation_hint = snapshot.active_order_collation_hint;
            cursor.active_order_key_domain_hint = snapshot.active_order_key_domain_hint;
            cursor.active_order_descending = snapshot.active_order_descending;
        }

        void restore_cursor_snapshot(CursorState &cursor, const CursorPositionSnapshot &snapshot) const
        {
            cursor.recno = snapshot.recno;
            cursor.found = snapshot.found;
            cursor.bof = snapshot.bof;
            cursor.eof = snapshot.eof;
            restore_cursor_order_snapshot(cursor, snapshot);
        }

        bool evaluate_visibility_expression(const std::string &expression, const Frame &frame, const CursorState *cursor)
        {
            const std::string trimmed_expression = trim_copy(expression);
            if (trimmed_expression.empty())
            {
                return true;
            }

            auto &set_state = current_set_state();
            const auto saved_fields_enabled = set_state.find("fields_enabled");
            const auto saved_fields = set_state.find("fields");
            const bool had_fields_enabled = saved_fields_enabled != set_state.end();
            const bool had_fields = saved_fields != set_state.end();
            const std::string fields_enabled_value = had_fields_enabled ? saved_fields_enabled->second : std::string{};
            const std::string fields_value = had_fields ? saved_fields->second : std::string{};
            set_state["fields_enabled"] = "off";

            const auto restore_fields = [&]()
            {
                if (had_fields_enabled)
                {
                    set_state["fields_enabled"] = fields_enabled_value;
                }
                else
                {
                    set_state.erase("fields_enabled");
                }
                if (had_fields)
                {
                    set_state["fields"] = fields_value;
                }
                else
                {
                    set_state.erase("fields");
                }
            };

            try
            {
                struct InSubquery
                {
                    std::string left_expression;
                    std::string subquery;
                    bool negated = false;
                };

                const auto parse_in_subquery = [&]() -> std::optional<InSubquery>
                {
                    const std::string upper_expression = uppercase_copy(trimmed_expression);
                    bool in_single_quote = false;
                    bool in_double_quote = false;
                    int parentheses_depth = 0;
                    const auto is_word_char = [](char current)
                    {
                        return std::isalnum(static_cast<unsigned char>(current)) != 0 || current == '_';
                    };

                    for (std::size_t index = 0U; index + 2U <= upper_expression.size(); ++index)
                    {
                        const char current = upper_expression[index];
                        if (in_single_quote)
                        {
                            if (current == '\'' && index + 1U < upper_expression.size() &&
                                upper_expression[index + 1U] == '\'')
                            {
                                ++index;
                            }
                            else if (current == '\'')
                            {
                                in_single_quote = false;
                            }
                            continue;
                        }
                        if (in_double_quote)
                        {
                            if (current == '"')
                            {
                                in_double_quote = false;
                            }
                            continue;
                        }
                        if (current == '\'')
                        {
                            in_single_quote = true;
                            continue;
                        }
                        if (current == '"')
                        {
                            in_double_quote = true;
                            continue;
                        }
                        if (current == '(')
                        {
                            ++parentheses_depth;
                            continue;
                        }
                        if (current == ')')
                        {
                            if (parentheses_depth > 0)
                            {
                                --parentheses_depth;
                            }
                            continue;
                        }
                        if (parentheses_depth != 0 || upper_expression.compare(index, 2U, "IN") != 0 ||
                            (index > 0U && is_word_char(upper_expression[index - 1U])) ||
                            (index + 2U < upper_expression.size() && is_word_char(upper_expression[index + 2U])))
                        {
                            continue;
                        }

                        std::size_t subquery_start = index + 2U;
                        while (subquery_start < trimmed_expression.size() &&
                               std::isspace(static_cast<unsigned char>(trimmed_expression[subquery_start])) != 0)
                        {
                            ++subquery_start;
                        }
                        if (subquery_start >= trimmed_expression.size() ||
                            trimmed_expression[subquery_start] != '(')
                        {
                            continue;
                        }

                        const std::size_t open_parenthesis = subquery_start++;
                        int subquery_depth = 1;
                        in_single_quote = false;
                        in_double_quote = false;
                        for (; subquery_start < trimmed_expression.size(); ++subquery_start)
                        {
                            const char subquery_character = trimmed_expression[subquery_start];
                            if (in_single_quote)
                            {
                                if (subquery_character == '\'' && subquery_start + 1U < trimmed_expression.size() &&
                                    trimmed_expression[subquery_start + 1U] == '\'')
                                {
                                    ++subquery_start;
                                }
                                else if (subquery_character == '\'')
                                {
                                    in_single_quote = false;
                                }
                                continue;
                            }
                            if (in_double_quote)
                            {
                                if (subquery_character == '"')
                                {
                                    in_double_quote = false;
                                }
                                continue;
                            }
                            if (subquery_character == '\'')
                            {
                                in_single_quote = true;
                            }
                            else if (subquery_character == '"')
                            {
                                in_double_quote = true;
                            }
                            else if (subquery_character == '(')
                            {
                                ++subquery_depth;
                            }
                            else if (subquery_character == ')' && --subquery_depth == 0)
                            {
                                const std::string trailing = trim_copy(
                                    trimmed_expression.substr(subquery_start + 1U));
                                const std::string subquery = trim_copy(
                                    trimmed_expression.substr(open_parenthesis + 1U,
                                                              subquery_start - open_parenthesis - 1U));
                                const std::string upper_subquery = uppercase_copy(subquery);
                                if (trailing.empty() && upper_subquery.rfind("SELECT", 0U) == 0U &&
                                    (upper_subquery.size() == 6U ||
                                     !is_word_char(upper_subquery[6U])))
                                {
                                    std::string left_expression =
                                        trim_copy(trimmed_expression.substr(0U, index));
                                    bool negated = false;
                                    const std::string upper_left_expression = uppercase_copy(left_expression);
                                    if (upper_left_expression.size() >= 3U &&
                                        upper_left_expression.compare(upper_left_expression.size() - 3U, 3U, "NOT") == 0 &&
                                        (upper_left_expression.size() == 3U ||
                                         !is_word_char(upper_left_expression[upper_left_expression.size() - 4U])))
                                    {
                                        left_expression = trim_copy(left_expression.substr(0U, left_expression.size() - 3U));
                                        negated = true;
                                    }
                                    return InSubquery{std::move(left_expression), subquery, negated};
                                }
                                break;
                            }
                        }
                    }
                    return std::nullopt;
                };

                if (const auto in_subquery = parse_in_subquery(); in_subquery.has_value())
                {
                    const PrgValue left_value = evaluate_expression(in_subquery->left_expression, frame, cursor);
                    if (left_value.is_null)
                    {
                        restore_fields();
                        return false;
                    }

                    std::vector<std::vector<PrgValue>> rows;
                    if (!materialize_select_query_rows(in_subquery->subquery, frame, rows))
                    {
                        throw std::runtime_error(last_error_message);
                    }
                    const auto values_match = [&](const PrgValue &left, const PrgValue &right)
                    {
                        const auto is_numeric = [](PrgValueKind kind)
                        {
                            return kind == PrgValueKind::number || kind == PrgValueKind::int64 ||
                                   kind == PrgValueKind::uint64 || kind == PrgValueKind::currency;
                        };
                        if (left.is_null || right.is_null)
                        {
                            return left.is_null && right.is_null;
                        }
                        if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                        {
                            const std::string left_text = value_as_string(left);
                            const std::string right_text = value_as_string(right);
                            return is_set_enabled("exact")
                                       ? rtrim_space_copy(left_text) == rtrim_space_copy(right_text)
                                       : left_text.rfind(right_text, 0U) == 0U;
                        }
                        if (left.kind == PrgValueKind::boolean || right.kind == PrgValueKind::boolean)
                        {
                            return value_as_bool(left) == value_as_bool(right);
                        }
                        if (is_numeric(left.kind) && is_numeric(right.kind))
                        {
                            return std::abs(value_as_number(left) - value_as_number(right)) < 0.000001;
                        }
                        return value_as_string(left) == value_as_string(right);
                    };
                    bool contains_null = false;
                    for (const auto &row : rows)
                    {
                        if (row.empty())
                        {
                            continue;
                        }
                        if (row.front().is_null)
                        {
                            contains_null = true;
                            continue;
                        }
                        if (values_match(left_value, row.front()))
                        {
                            restore_fields();
                            return !in_subquery->negated;
                        }
                    }
                    restore_fields();
                    return in_subquery->negated && !contains_null;
                }

                const PrgValue evaluated = evaluate_expression(trimmed_expression, frame, cursor);
                if (evaluated.kind == PrgValueKind::string && trimmed_expression.front() == '&')
                {
                    const std::string expanded_expression = trim_copy(value_as_string(evaluated));
                    if (!expanded_expression.empty() && expanded_expression != trimmed_expression)
                    {
                        const bool result = value_as_bool(evaluate_expression(expanded_expression, frame, cursor));
                        restore_fields();
                        return result;
                    }
                }

                const bool result = value_as_bool(evaluated);
                restore_fields();
                return result;
            }
            catch (...)
            {
                restore_fields();
                throw;
            }
        }

        bool current_record_matches_visibility(
            const CursorState &cursor,
            const Frame &frame,
            const std::string &extra_expression,
            bool honor_set_deleted = true,
            bool honor_filter = true)
        {
            const auto record = current_record(cursor);
            if (!record.has_value())
            {
                return false;
            }
            if (honor_set_deleted && is_set_enabled("deleted") && record->deleted)
            {
                return false;
            }
            if (honor_filter &&
                !cursor.filter_expression.empty() &&
                !evaluate_visibility_expression(cursor.filter_expression, frame, &cursor))
            {
                return false;
            }
            if (!extra_expression.empty() && !evaluate_visibility_expression(extra_expression, frame, &cursor))
            {
                return false;
            }
            return true;
        }

        bool filter_expression_matches_record(
            CursorState &cursor,
            const Frame &frame,
            const vfp::DbfRecord &record,
            std::size_t recno,
            const CursorPositionSnapshot *evaluation_context = nullptr)
        {
            if (cursor.filter_expression.empty())
            {
                return true;
            }
            if (std::any_of(
                    record_evaluation_overrides.rbegin(),
                    record_evaluation_overrides.rend(),
                    [&](const auto &entry)
                    {
                        return entry.first == &cursor;
                    }))
            {
                return true;
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            record_evaluation_overrides.emplace_back(&cursor, &record);
            try
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                if (evaluation_context != nullptr)
                {
                    restore_cursor_order_snapshot(cursor, *evaluation_context);
                }
                const bool matches = current_record_matches_visibility(cursor, frame, {}, false);
                restore_cursor_snapshot(cursor, original);
                record_evaluation_overrides.pop_back();
                return matches;
            }
            catch (...)
            {
                restore_cursor_snapshot(cursor, original);
                record_evaluation_overrides.pop_back();
                throw;
            }
        }

        std::optional<std::vector<IndexedCandidate>> load_ordered_record_candidates(const CursorState &cursor) const
        {
            if (cursor.active_order_expression.empty())
            {
                return std::nullopt;
            }

            std::vector<vfp::DbfRecord> local_records;
            const std::vector<vfp::DbfRecord> *records = &cursor.remote_records;
            if (!cursor.remote)
            {
                const auto table_result = parse_cursor_table(cursor, cursor.record_count);
                if (!table_result.ok)
                {
                    return std::nullopt;
                }
                local_records = table_result.table.records;
                records = &local_records;
            }

            std::vector<IndexedCandidate> candidates;
            candidates.reserve(records->size());
            for (const auto &record : *records)
            {
                if (!order_for_expression_matches_record(cursor.active_order_for_expression, record))
                {
                    continue;
                }
                candidates.push_back({.key = normalize_seek_key_for_collation(
                                          normalize_seek_key_for_order(
                                              evaluate_index_expression(cursor.active_order_expression, record),
                                              cursor.active_order_normalization_hint),
                                          cursor.active_order_collation_hint,
                                          cursor.active_order_key_domain_hint),
                                      .recno = record.record_index + 1U});
            }

            std::sort(candidates.begin(), candidates.end(), [&](const IndexedCandidate &left, const IndexedCandidate &right)
                      {
                          const int comparison = compare_order_keys(
                              left.key,
                              right.key,
                              cursor.active_order_key_domain_hint,
                              cursor.active_order_descending);
                          if (comparison != 0)
                          {
                              return comparison < 0;
                          }
                          return left.recno < right.recno;
                      });
            return candidates;
        }

        bool seek_ordered_visible_record(
            CursorState &cursor,
            const Frame &frame,
            int direction,
            long long start_recno,
            bool start_after_current,
            const std::string &extra_expression,
            const std::string &while_expression)
        {
            const auto candidates = load_ordered_record_candidates(cursor);
            if (!candidates.has_value())
            {
                return false;
            }

            std::ptrdiff_t index = direction >= 0 ? 0 : static_cast<std::ptrdiff_t>(candidates->size()) - 1;
            if (start_after_current)
            {
                const auto current = std::find_if(candidates->begin(), candidates->end(), [&](const auto &candidate)
                                                  { return candidate.recno == cursor.recno; });
                if (current != candidates->end())
                {
                    index = static_cast<std::ptrdiff_t>(std::distance(candidates->begin(), current)) + direction;
                }
                else if (cursor.eof && direction < 0)
                {
                    index = static_cast<std::ptrdiff_t>(candidates->size()) - 1;
                }
                else if (cursor.bof && direction > 0)
                {
                    index = 0;
                }
                else
                {
                    index = direction >= 0 ? static_cast<std::ptrdiff_t>(candidates->size())
                                           : static_cast<std::ptrdiff_t>(-1);
                }
            }
            else if (start_recno <= 1)
            {
                index = direction >= 0 ? 0 : static_cast<std::ptrdiff_t>(candidates->size()) - 1;
            }
            else
            {
                if (direction >= 0)
                {
                    const auto requested = std::find_if(candidates->begin(), candidates->end(), [&](const auto &candidate)
                                                         { return candidate.recno >= static_cast<std::size_t>(start_recno); });
                    index = requested == candidates->end()
                        ? static_cast<std::ptrdiff_t>(candidates->size())
                        : static_cast<std::ptrdiff_t>(std::distance(candidates->begin(), requested));
                }
                else
                {
                    const auto requested = std::find_if(candidates->rbegin(), candidates->rend(), [&](const auto &candidate)
                                                         { return candidate.recno <= static_cast<std::size_t>(start_recno); });
                    index = requested == candidates->rend()
                        ? static_cast<std::ptrdiff_t>(-1)
                        : static_cast<std::ptrdiff_t>(candidates->size() - 1U -
                                                       static_cast<std::size_t>(std::distance(candidates->rbegin(), requested)));
                }
            }

            for (; index >= 0 && index < static_cast<std::ptrdiff_t>(candidates->size()); index += direction)
            {
                move_cursor_to(cursor, static_cast<long long>((*candidates)[static_cast<std::size_t>(index)].recno));
                if (!while_expression.empty() && !evaluate_visibility_expression(while_expression, frame, &cursor))
                {
                    break;
                }
                if (current_record_matches_visibility(cursor, frame, extra_expression))
                {
                    return true;
                }
            }

            if (direction >= 0)
            {
                move_cursor_to(cursor, static_cast<long long>(cursor.record_count + 1U));
            }
            else
            {
                move_cursor_to(cursor, 0);
            }
            return false;
        }

        bool seek_visible_record(
            CursorState &cursor,
            const Frame &frame,
            long long start_recno,
            int direction,
            const std::string &extra_expression,
            const std::string &while_expression,
            bool preserve_on_failure,
            bool honor_active_order = false,
            bool start_after_current = false)
        {
            if (honor_active_order && !cursor.active_order_expression.empty())
            {
                return seek_ordered_visible_record(
                    cursor,
                    frame,
                    direction,
                    start_recno,
                    start_after_current,
                    extra_expression,
                    while_expression);
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            const long long first = direction >= 0 ? std::max<long long>(1, start_recno) : std::min<long long>(start_recno, static_cast<long long>(cursor.record_count));
            for (long long recno = first;
                 recno >= 1 && recno <= static_cast<long long>(cursor.record_count);
                 recno += direction)
            {
                move_cursor_to(cursor, recno);
                if (!while_expression.empty() && !evaluate_visibility_expression(while_expression, frame, &cursor))
                {
                    break;
                }
                if (current_record_matches_visibility(cursor, frame, extra_expression))
                {
                    return true;
                }
            }

            if (preserve_on_failure)
            {
                restore_cursor_snapshot(cursor, original);
            }
            else if (direction >= 0)
            {
                move_cursor_to(cursor, static_cast<long long>(cursor.record_count + 1U));
            }
            else
            {
                move_cursor_to(cursor, 0);
            }
            return false;
        }

        bool move_by_visible_records(CursorState &cursor, const Frame &frame, long long delta)
        {
            if (delta == 0)
            {
                return current_record_matches_visibility(cursor, frame, {});
            }

            const int direction = delta > 0 ? 1 : -1;
            long long remaining = std::llabs(delta);
            while (remaining > 0)
            {
                if (!seek_visible_record(cursor, frame, static_cast<long long>(cursor.recno) + direction, direction, {}, {}, false, true, true))
                {
                    return false;
                }
                --remaining;
            }
            return true;
        }

        std::vector<std::size_t> record_iteration_order(const CursorState &cursor) const
        {
            if (const auto candidates = load_ordered_record_candidates(cursor); candidates.has_value())
            {
                std::vector<std::size_t> recnos;
                recnos.reserve(candidates->size());
                for (const auto &candidate : *candidates)
                {
                    recnos.push_back(candidate.recno);
                }
                return recnos;
            }

            std::vector<std::size_t> recnos;
            recnos.reserve(cursor.record_count);
            for (std::size_t recno = 1U; recno <= cursor.record_count; ++recno)
            {
                recnos.push_back(recno);
            }
            return recnos;
        }

        std::optional<vfp::DbfRecord> current_record(const CursorState &cursor) const
        {
            const auto record_override = std::find_if(
                record_evaluation_overrides.rbegin(),
                record_evaluation_overrides.rend(),
                [&](const auto &entry)
                {
                    return entry.first == &cursor && entry.second != nullptr;
                });
            if (record_override != record_evaluation_overrides.rend())
            {
                return *record_override->second;
            }

            if (cursor.recno == 0U || cursor.eof)
            {
                return std::nullopt;
            }

            if (const auto buffered = cursor.buffered_records.find(cursor.recno);
                buffered != cursor.buffered_records.end())
            {
                return buffered->second;
            }

            if (cursor.remote)
            {
                if (cursor.recno > cursor.record_count || cursor.recno > cursor.remote_records.size())
                {
                    return std::nullopt;
                }
                return cursor.remote_records[cursor.recno - 1U];
            }
            if (cursor.source_path.empty())
            {
                return std::nullopt;
            }

            const auto table_result = parse_cursor_table(cursor, cursor.recno);
            if (!table_result.ok || cursor.recno > table_result.table.records.size())
            {
                return std::nullopt;
            }

            return table_result.table.records[cursor.recno - 1U];
        }

        bool dbf_records_match(const vfp::DbfRecord &left, const vfp::DbfRecord &right) const
        {
            if (left.deleted != right.deleted || left.values.size() != right.values.size())
            {
                return false;
            }
            for (std::size_t index = 0U; index < left.values.size(); ++index)
            {
                const auto &left_value = left.values[index];
                const auto &right_value = right.values[index];
                if (left_value.field_name != right_value.field_name ||
                    left_value.field_type != right_value.field_type ||
                    left_value.is_null != right_value.is_null ||
                    left_value.display_value != right_value.display_value ||
                    left_value.memo_block_number != right_value.memo_block_number)
                {
                    return false;
                }
            }
            return true;
        }

        bool optimistic_record_matches_disk(
            CursorState &cursor,
            std::size_t recno,
            const std::string &command)
        {
            const auto original = cursor.buffered_original_records.find(recno);
            if (original == cursor.buffered_original_records.end())
            {
                return true;
            }
            const auto table_result = parse_cursor_table(cursor, recno);
            if (!table_result.ok || recno > table_result.table.records.size() ||
                !dbf_records_match(original->second, table_result.table.records[recno - 1U]))
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.OptimisticBufferConflict",
                    {
                        {"alias", cursor.alias.empty() ? std::to_string(cursor.work_area) : cursor.alias},
                        {"command", command},
                        {"recno", std::to_string(recno)}
                    });
                return false;
            }
            return true;
        }

        std::optional<PrgValue> resolve_field_value(const std::string &identifier, const CursorState *preferred_cursor)
        {
            const auto field_is_visible = [this](const std::string &field_name) -> bool
            {
                if (collapse_identifier(field_name) == "DELETED")
                {
                    return true;
                }

                const auto &set_state = current_set_state();
                const auto enabled = set_state.find("fields_enabled");
                if (enabled == set_state.end() || !is_set_enabled("fields_enabled"))
                {
                    return true;
                }

                const auto field_list = set_state.find("fields");
                if (field_list == set_state.end() || trim_copy(field_list->second).empty())
                {
                    return true;
                }

                const std::string field_filter_text = trim_copy(field_list->second);
                if (collapse_identifier(field_filter_text) == "ALL")
                {
                    return true;
                }

                const std::vector<std::string> field_filter = parse_field_filter_clause(field_filter_text);
                return field_filter.empty() || field_matches_filter(field_name, field_filter);
            };

            const auto value_from_record = [&](const CursorState *cursor, const std::string &field_name) -> std::optional<PrgValue>
            {
                if (cursor == nullptr)
                {
                    return std::nullopt;
                }
                if (!field_is_visible(field_name))
                {
                    // VFP-style SET FIELDS hides field access by returning an empty value
                    // rather than raising an unresolved identifier fault.
                    return make_string_value("");
                }
                const auto record = current_record(*cursor);
                if (!record.has_value())
                {
                    const auto fields = cursor_field_descriptors(*cursor);
                    const auto field = std::find_if(
                        fields.begin(),
                        fields.end(),
                        [&](const vfp::DbfFieldDescriptor &candidate)
                        {
                            return collapse_identifier(candidate.name) == collapse_identifier(field_name);
                        });
                    if (field == fields.end())
                    {
                        return std::nullopt;
                    }

                    vfp::DbfRecordValue blank_field;
                    blank_field.field_name = field->name;
                    blank_field.field_type = field->type;
                    return blank_value_for_field(blank_field);
                }
                if (collapse_identifier(field_name) == "DELETED")
                {
                    return make_boolean_value(record->deleted);
                }
                const auto field_value = record_field_value(*record, field_name);
                if (!field_value.has_value())
                {
                    return std::nullopt;
                }

                const auto raw_field = std::find_if(record->values.begin(), record->values.end(), [&](const vfp::DbfRecordValue &value)
                                                    { return collapse_identifier(value.field_name) == collapse_identifier(field_name); });
                if (raw_field == record->values.end())
                {
                    return make_string_value(*field_value);
                }
                return record_value_to_prg_value(*raw_field);
            };

            const auto separator = identifier.find('.');
            if (separator != std::string::npos)
            {
                const std::string designator = identifier.substr(0U, separator);
                const std::string field_name = identifier.substr(separator + 1U);
                if (auto value = value_from_record(resolve_cursor_target(designator), field_name))
                {
                    return value;
                }
            }

            if (auto value = value_from_record(preferred_cursor, identifier))
            {
                return value;
            }

            return value_from_record(resolve_cursor_target({}), identifier);
        }

        std::optional<std::size_t> find_matching_endscan(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::scan_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endscan_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        bool locate_next_matching_record(
            CursorState &cursor,
            const std::string &for_expression,
            const std::string &while_expression,
            const Frame &frame,
            std::size_t start_recno)
        {
            if (!cursor.remote && cursor.source_path.empty())
            {
                last_error_message = runtime_text("Runtime.Prg.Records.Error.RequiresLocalTableBackedCursor");
                return false;
            }

            const std::string locate_detail = for_expression.empty() ? std::string{"ALL"} : for_expression;
            std::string rushmore_detail = locate_detail + " -> linear_scan";

            if (!for_expression.empty() && !cursor.orders.empty())
            {
                auto cached_pattern = index_pattern_cache.find(for_expression);
                IndexExpressionPattern pattern;
                if (cached_pattern != index_pattern_cache.end())
                {
                    pattern = cached_pattern->second;
                }
                else
                {
                    std::vector<std::string> available_fields;
                    for (const auto &field : cursor.remote_fields)
                    {
                        available_fields.push_back(field.name);
                    }

                    pattern = analyze_filter_expression(for_expression, available_fields);
                    index_pattern_cache[for_expression] = pattern;
                }

                if (pattern.confidence != OptimizationConfidence::not_applicable)
                {
                    std::vector<IndexOrderCandidate> available_orders;
                    for (const auto &order : cursor.orders)
                    {
                        available_orders.push_back(IndexOrderCandidate{
                            .order_name = order.name,
                            .order_expression = order.expression,
                            .order_for_expression = order.for_expression,
                            .order_path = order.index_path,
                            .normalization_hint = order.normalization_hint,
                            .collation_hint = order.collation_hint,
                            .key_domain_hint = order.key_domain_hint,
                            .is_descending = order.descending
                        });
                    }

                    auto plan = create_index_seek_plan(pattern, available_orders, cursor.active_order_name);

                    if (options.rushmore_planning.enabled)
                    {
                        const bool is_single_comparison =
                            pattern.recognized_predicates.size() == 1U &&
                            pattern.residual_predicates.empty() &&
                            (pattern.operator_kind == IndexOperatorKind::equal ||
                             pattern.operator_kind == IndexOperatorKind::not_equal ||
                             pattern.operator_kind == IndexOperatorKind::less_than ||
                             pattern.operator_kind == IndexOperatorKind::less_than_or_equal ||
                             pattern.operator_kind == IndexOperatorKind::greater_than ||
                             pattern.operator_kind == IndexOperatorKind::greater_than_or_equal);
                        bool cost_model_considered = false;
                        bool cost_model_selected = false;
                        if (plan.can_optimize && plan.selected_order && is_single_comparison)
                        {
                            cost_model_considered = true;
                            std::optional<RushmorePredicateDescriptor> predicate =
                                pattern.recognized_predicates.front();
                            const RushmoreCursorMetadata metadata{
                                cursor.alias,
                                plan.selected_order->order_expression,
                                static_cast<std::uint64_t>(cursor.record_count),
                                0,
                                std::nullopt};
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
                            cost_model_selected = index_seek_cost < table_scan_cost;
                        }

                        if (cost_model_considered && !cost_model_selected)
                        {
                            plan.can_optimize = false;
                            plan.selected_order.reset();
                            plan.strategy = IndexSeekPlan::ExecutionStrategy::linear_scan;
                            plan.decision_rationale = runtime_text(
                                "Runtime.IndexSeek.PlanDecision.CostModelRejected");
                        }
                    }

                    if (!plan.decision_rationale.empty())
                    {
                        rushmore_detail = locate_detail + " -> linear_scan (" + plan.decision_rationale + ")";
                    }

                    if (plan.can_optimize && plan.selected_order && start_recno <= 1U)
                    {
                        if (pattern.operands.size() > 1U && !pattern.operands[1].raw_text.empty())
                        {
                            const std::string search_key_text = pattern.operands[1].raw_text;
                            const auto search_value = evaluate_expression(search_key_text, frame);
                            const std::string search_key = value_as_string(search_value);

                            const CursorPositionSnapshot saved_cursor = capture_cursor_snapshot(cursor);
                            const auto restore_order_metadata = [&]() {
                                restore_cursor_order_snapshot(cursor, saved_cursor);
                            };
                            const auto restore_full_state = [&]() {
                                restore_order_metadata();
                                cursor.recno = saved_cursor.recno;
                                cursor.found = saved_cursor.found;
                                cursor.bof = saved_cursor.bof;
                                cursor.eof = saved_cursor.eof;
                            };

                            try
                            {
                                cursor.active_order_name = plan.selected_order->order_name;
                                cursor.active_order_expression = plan.selected_order->order_expression;
                                cursor.active_order_for_expression = plan.selected_order->order_for_expression;
                                cursor.active_order_path = plan.selected_order->order_path;
                                cursor.active_order_normalization_hint = plan.selected_order->normalization_hint;
                                cursor.active_order_collation_hint = plan.selected_order->collation_hint;
                                cursor.active_order_key_domain_hint = plan.selected_order->key_domain_hint;
                                cursor.active_order_descending = plan.selected_order->is_descending;
                                cursor.recno = start_recno;
                                cursor.bof = (start_recno <= 1U);
                                cursor.eof = (start_recno > cursor.record_count);
                                cursor.found = false;

                                if (seek_in_cursor(cursor, search_key, frame, &saved_cursor))
                                {
                                    // The index seek only guarantees the cursor landed on a record whose
                                    // key relates to search_key per the index's own ordering/match rules; for
                                    // non-equality operators (and to guard against any other seek/for-expression
                                    // mismatch) the landed record must still be re-checked against the actual
                                    // for_expression before trusting it, otherwise e.g. a record with AGE == 30
                                    // could be reported as matching LOCATE FOR AGE > 30.
                                    restore_order_metadata();
                                    if (current_record_matches_visibility(cursor, frame, for_expression, true, false) &&
                                        (while_expression.empty() || evaluate_visibility_expression(while_expression, frame, &cursor)))
                                    {
                                        cursor.found = true;
                                        rushmore_detail = locate_detail + " -> index_seek via " + plan.selected_order->order_name +
                                                          " (" + plan.decision_rationale + ")";
                                        events.push_back({.category = "runtime.rushmore", .detail = rushmore_detail});
                                        return true;  // Index seek optimization succeeded
                                    }
                                }
                            }
                            catch (...)
                            {
                            }

                            restore_full_state();
                            rushmore_detail = locate_detail + " -> linear_scan after index_seek via " + plan.selected_order->order_name +
                                              " (" + plan.decision_rationale + ")";
                        }
                    }
                }
            }

            const bool start_after_current = cursor.recno > 0U &&
                start_recno == cursor.recno + 1U;
            const bool found = seek_visible_record(
                cursor,
                frame,
                static_cast<long long>(start_recno),
                1,
                for_expression,
                while_expression,
                false,
                true,
                start_after_current);
            cursor.found = found;
            events.push_back({.category = "runtime.rushmore", .detail = rushmore_detail});
            return true;
        }

        bool pause_for_lock_retry(const std::string &detail,
                                  const SourceLocation &location,
                                  std::size_t attempt_number)
        {
            if (!ensure_non_blocking_critical_section_policy("LOCK RETRY", location, detail))
            {
                return false;
            }

            std::size_t sleep_duration_ms = 0U;
            if (scheduler_yield_sleep_ms != 0U)
            {
                sleep_duration_ms = scheduler_yield_sleep_ms * attempt_number;
            }

            events.push_back({.category = "runtime.lock_retry",
                              .detail = detail + " attempt=" + std::to_string(attempt_number) +
                                        (sleep_duration_ms == 0U ? " sleep=yield"
                                                                 : " sleep=" + std::to_string(sleep_duration_ms) + "ms"),
                              .location = location});

            if (sleep_duration_ms == 0U)
            {
                std::this_thread::yield();
                return true;
            }

            for (std::size_t elapsed = 0U; elapsed < sleep_duration_ms; ++elapsed)
            {
                if (task_cancel_requested != nullptr && task_cancel_requested->load(std::memory_order_relaxed))
                {
                    (void)handle_async_runtime_cancellation(
                        location,
                        current_statement() == nullptr ? std::string{} : current_statement()->text,
                        runtime_text("Runtime.Prg.Records.Error.LockRetryCancelled"));
                    return false;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1U));
            }

            return true;
        }

        bool acquire_table_lock(CursorState &cursor,
                                const std::string &context,
                                bool explicit_lock_command,
                                bool &new_lock)
        {
            const ReprocessPolicy policy = current_reprocess_policy();
            const std::string owner_key = current_lock_owner_key();
            const std::string resource_key = cursor_lock_resource_key(cursor);
            DataSessionState &session = current_session_state();
            const SourceLocation location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location;

            for (std::size_t attempt = 0U;; ++attempt)
            {
                {
                    std::lock_guard<std::mutex> lock(concurrency_state->mutex);
                    const auto table_owner_found = concurrency_state->table_lock_owner_by_resource.find(resource_key);
                    bool other_record_lock_present = false;
                    const auto shared_record_found = concurrency_state->record_lock_owner_by_resource.find(resource_key);
                    if (shared_record_found != concurrency_state->record_lock_owner_by_resource.end())
                    {
                        for (const auto &[_, record_owner] : shared_record_found->second)
                        {
                            if (record_owner != owner_key)
                            {
                                other_record_lock_present = true;
                                break;
                            }
                        }
                    }

                    const bool table_conflict = table_owner_found != concurrency_state->table_lock_owner_by_resource.end() &&
                                                table_owner_found->second != owner_key;
                    if (!table_conflict && !other_record_lock_present)
                    {
                        const bool already_owned = session.table_locks.contains(cursor.work_area);
                        session.table_locks.insert(cursor.work_area);
                        concurrency_state->table_lock_owner_by_resource[resource_key] = owner_key;
                        new_lock = !already_owned;
                        if (explicit_lock_command)
                        {
                            events.push_back({.category = "runtime.lock",
                                              .detail = cursor.alias.empty() ? std::to_string(cursor.work_area) : cursor.alias + " FLOCK",
                                              .location = location});
                        }
                        return true;
                    }
                }

                if (attempt >= policy.retry_budget)
                {
                    events.push_back({.category = "runtime.lock_timeout",
                                      .detail = context + " timeout reprocess=" + policy.display_value,
                                      .location = location});
                    if (!explicit_lock_command)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Records.Error.TableLockTimeout",
                            {{"context", context}, {"reprocess", policy.display_value}});
                    }
                    return false;
                }

                if (!pause_for_lock_retry(context + " reprocess=" + policy.display_value, location, attempt + 1U))
                {
                    return false;
                }
            }
        }

        bool acquire_record_lock(CursorState &cursor,
                                 std::size_t recno,
                                 const std::string &context,
                                 bool explicit_lock_command,
                                 bool &new_lock)
        {
            const ReprocessPolicy policy = current_reprocess_policy();
            const std::string owner_key = current_lock_owner_key();
            const std::string resource_key = cursor_lock_resource_key(cursor);
            DataSessionState &session = current_session_state();
            const SourceLocation location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location;

            for (std::size_t attempt = 0U;; ++attempt)
            {
                {
                    std::lock_guard<std::mutex> lock(concurrency_state->mutex);
                    const auto table_owner_found = concurrency_state->table_lock_owner_by_resource.find(resource_key);
                    const bool table_conflict = table_owner_found != concurrency_state->table_lock_owner_by_resource.end() &&
                                                table_owner_found->second != owner_key;
                    const auto shared_record_found = concurrency_state->record_lock_owner_by_resource.find(resource_key);
                    const auto owner_found = shared_record_found == concurrency_state->record_lock_owner_by_resource.end()
                                                 ? typename std::map<std::size_t, std::string>::const_iterator{}
                                                 : shared_record_found->second.find(recno);
                    const bool record_conflict = shared_record_found != concurrency_state->record_lock_owner_by_resource.end() &&
                                                 owner_found != shared_record_found->second.end() &&
                                                 owner_found->second != owner_key;
                    if (!table_conflict && !record_conflict)
                    {
                        const bool already_owned = session.record_locks[cursor.work_area].contains(recno);
                        session.record_locks[cursor.work_area].insert(recno);
                        concurrency_state->record_lock_owner_by_resource[resource_key][recno] = owner_key;
                        new_lock = !already_owned;
                        if (explicit_lock_command)
                        {
                            events.push_back({.category = "runtime.lock",
                                              .detail = (cursor.alias.empty() ? std::to_string(cursor.work_area) : cursor.alias) +
                                                        " RLOCK " + std::to_string(recno),
                                              .location = location});
                        }
                        return true;
                    }
                }

                if (attempt >= policy.retry_budget)
                {
                    events.push_back({.category = "runtime.lock_timeout",
                                      .detail = context + " timeout recno=" + std::to_string(recno) +
                                                " reprocess=" + policy.display_value,
                                      .location = location});
                    if (!explicit_lock_command)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Records.Error.RecordLockTimeout",
                            {{"context", context}, {"reprocess", policy.display_value}});
                    }
                    return false;
                }

                if (!pause_for_lock_retry(context + " recno=" + std::to_string(recno) +
                                          " reprocess=" + policy.display_value,
                                          location,
                                          attempt + 1U))
                {
                    return false;
                }
            }
        }

        bool replace_current_record_fields(
            CursorState &cursor,
            const std::vector<ReplaceAssignment> &assignments,
            const Frame &frame,
            bool truncate_character_overflow_for_local_fields = false)
        {
            struct EvaluatedReplaceAssignment
            {
                std::string field_name;
                std::string serialized_value;
                bool additive = false;
            };

            const auto serialize_value_for_cursor_field = [&](const std::string &field_name, const PrgValue &value)
            {
                vfp::DbfRecordValue field{
                    .field_name = field_name,
                    .field_type = 'C',
                    .is_null = false,
                    .display_value = {}};
                const std::string normalized_field = collapse_identifier(field_name);
                if (cursor.remote && cursor.recno > 0U && cursor.recno <= cursor.remote_records.size())
                {
                    const auto &record = cursor.remote_records[cursor.recno - 1U];
                    const auto found = std::find_if(
                        record.values.begin(),
                        record.values.end(),
                        [&](const vfp::DbfRecordValue &candidate)
                        {
                            return collapse_identifier(candidate.field_name) == normalized_field;
                        });
                    if (found != record.values.end())
                    {
                        field.field_type = found->field_type;
                    }
                }
                else
                {
                    const auto descriptors = cursor_field_descriptors(cursor);
                    const auto found = std::find_if(
                        descriptors.begin(),
                        descriptors.end(),
                        [&](const vfp::DbfFieldDescriptor &candidate)
                        {
                            return collapse_identifier(candidate.name) == normalized_field;
                        });
                    if (found != descriptors.end())
                    {
                        field.field_type = found->type;
                    }
                }
                return serialize_prg_value_for_record_field(field, value);
            };

            if (cursor.remote)
            {
                if (cursor.recno == 0U || cursor.eof || cursor.recno > cursor.remote_records.size())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Records.Error.CommandRequiresCurrentRemoteRecord",
                        {{"command", "REPLACE"}});
                    return false;
                }

                vfp::DbfRecord &record = cursor.remote_records[cursor.recno - 1U];
                std::vector<EvaluatedReplaceAssignment> evaluated_assignments;
                evaluated_assignments.reserve(assignments.size());
                for (const auto &assignment : assignments)
                {
                    const PrgValue value = evaluate_expression(assignment.expression, frame);
                    const std::string normalized_field = collapse_identifier(assignment.field_name);
                    auto field = std::find_if(record.values.begin(), record.values.end(), [&](vfp::DbfRecordValue &candidate)
                                              { return collapse_identifier(candidate.field_name) == normalized_field; });
                    if (field == record.values.end())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Records.Error.RemoteSqlFieldNotFound",
                            {{"fieldName", assignment.field_name}});
                        return false;
                    }
                    std::string serialized_value = serialize_value_for_cursor_field(assignment.field_name, value);
                    if (assignment.additive && field->field_type == 'M')
                    {
                        serialized_value = field->display_value + serialized_value;
                    }
                    evaluated_assignments.push_back({
                        .field_name = assignment.field_name,
                        .serialized_value = std::move(serialized_value),
                        .additive = assignment.additive});
                }

                for (const auto &assignment : evaluated_assignments)
                {
                    const std::string normalized_field = collapse_identifier(assignment.field_name);
                    auto field = std::find_if(record.values.begin(), record.values.end(), [&](vfp::DbfRecordValue &candidate)
                                              { return collapse_identifier(candidate.field_name) == normalized_field; });
                    if (field == record.values.end())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Records.Error.RemoteSqlFieldNotFound",
                            {{"fieldName", assignment.field_name}});
                        return false;
                    }
                    field->display_value = assignment.serialized_value;
                }
                synchronize_relations_for_parent(cursor, frame);
                return true;
            }

            if (cursor.buffering_mode == 2 || cursor.buffering_mode == 3 ||
                cursor.buffering_mode == 4 || cursor.buffering_mode == 5)
            {
                if (cursor.source_path.empty() || cursor.recno == 0U || cursor.eof)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Records.Error.CommandRequiresCurrentLocalRecord",
                        {{"command", "REPLACE"}});
                    return false;
                }

                auto buffered = cursor.buffered_records.find(cursor.recno);
                bool acquired_buffer_lock = false;
                if ((cursor.buffering_mode == 2 || cursor.buffering_mode == 4) &&
                    !cursor.buffered_record_locks.contains(cursor.recno))
                {
                    bool new_lock = false;
                    if (!acquire_record_lock(cursor, cursor.recno, "REPLACE", false, new_lock))
                    {
                        return false;
                    }
                    if (new_lock)
                    {
                        cursor.buffered_record_locks.insert(cursor.recno);
                        acquired_buffer_lock = true;
                    }
                }
                if (buffered == cursor.buffered_records.end())
                {
                    const auto table_result = parse_cursor_table(cursor, cursor.recno);
                    if (!table_result.ok || cursor.recno > table_result.table.records.size())
                    {
                        if (acquired_buffer_lock)
                        {
                            cursor.buffered_record_locks.erase(cursor.recno);
                            unlock_cursor_record_lock(cursor, cursor.recno);
                        }
                        last_error_message = table_result.error.empty()
                            ? runtime_text("Runtime.Prg.Records.Error.CommandRequiresCurrentLocalRecord", {{"command", "REPLACE"}})
                            : table_result.error;
                        return false;
                    }
                    buffered = cursor.buffered_records.emplace(
                        cursor.recno,
                        table_result.table.records[cursor.recno - 1U]).first;
                    cursor.buffered_original_records.emplace(cursor.recno, buffered->second);
                }

                std::vector<EvaluatedReplaceAssignment> evaluated_assignments;
                evaluated_assignments.reserve(assignments.size());
                for (const auto &assignment : assignments)
                {
                    const PrgValue value = evaluate_expression(assignment.expression, frame);
                    std::string serialized_value = serialize_value_for_cursor_field(assignment.field_name, value);
                    if (truncate_character_overflow_for_local_fields)
                    {
                        const std::string normalized_field = collapse_identifier(assignment.field_name);
                        const auto descriptors = cursor_field_descriptors(cursor);
                        const auto descriptor = std::find_if(
                            descriptors.begin(),
                            descriptors.end(),
                            [&](const vfp::DbfFieldDescriptor &candidate)
                            {
                                return collapse_identifier(candidate.name) == normalized_field;
                            });
                        if (descriptor != descriptors.end() && descriptor->type == 'C')
                        {
                            const std::string trimmed = trim_copy(serialized_value);
                            serialized_value = trimmed.size() > descriptor->length
                                ? trimmed.substr(0U, descriptor->length)
                                : trimmed;
                        }
                    }
                    evaluated_assignments.push_back({
                        .field_name = assignment.field_name,
                        .serialized_value = std::move(serialized_value),
                        .additive = assignment.additive});
                }

                for (const auto &assignment : evaluated_assignments)
                {
                    const std::string normalized_field = collapse_identifier(assignment.field_name);
                    auto field = std::find_if(
                        buffered->second.values.begin(),
                        buffered->second.values.end(),
                        [&](vfp::DbfRecordValue &candidate)
                        {
                            return collapse_identifier(candidate.field_name) == normalized_field;
                        });
                    if (field == buffered->second.values.end())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Records.Error.ConstraintFieldNotFound",
                            {{"constraint", "REPLACE"}, {"fieldName", assignment.field_name}});
                        return false;
                    }
                    if (assignment.additive && field->field_type == 'M')
                    {
                        field->display_value += assignment.serialized_value;
                    }
                    else
                    {
                        field->display_value = assignment.serialized_value;
                    }
                    field->is_null = false;
                    const std::size_t field_index = static_cast<std::size_t>(
                        std::distance(buffered->second.values.begin(), field));
                    cursor.buffered_field_states[cursor.recno][field_index] =
                        cursor.buffered_appended_records.contains(cursor.recno) ? 4 : 2;
                }
                synchronize_relations_for_parent(cursor, frame);
                return true;
            }

            if (cursor.source_path.empty() || cursor.recno == 0U || cursor.eof)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.CommandRequiresCurrentLocalRecord",
                    {{"command", "REPLACE"}});
                return false;
            }
            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            bool temporary_record_lock = false;
            if (!acquire_record_lock(cursor, cursor.recno, "REPLACE", false, temporary_record_lock))
            {
                return false;
            }

            std::vector<EvaluatedReplaceAssignment> evaluated_assignments;
            evaluated_assignments.reserve(assignments.size());
            for (const auto &assignment : assignments)
            {
                const PrgValue value = evaluate_expression(assignment.expression, frame);
                std::string serialized_value = serialize_value_for_cursor_field(assignment.field_name, value);
                if (truncate_character_overflow_for_local_fields)
                {
                    const std::string normalized_field = collapse_identifier(assignment.field_name);
                    const auto descriptors = cursor_field_descriptors(cursor);
                    const auto descriptor = std::find_if(
                        descriptors.begin(),
                        descriptors.end(),
                        [&](const vfp::DbfFieldDescriptor &candidate)
                        {
                            return collapse_identifier(candidate.name) == normalized_field;
                        });
                    if (descriptor != descriptors.end() && descriptor->type == 'C')
                    {
                        const std::string trimmed = trim_copy(serialized_value);
                        if (trimmed.size() > descriptor->length)
                        {
                            serialized_value = trimmed.substr(0U, descriptor->length);
                        }
                        else
                        {
                            serialized_value = trimmed;
                        }
                    }
                }
                evaluated_assignments.push_back({
                    .field_name = assignment.field_name,
                    .serialized_value = std::move(serialized_value),
                    .additive = assignment.additive});
            }

            for (const auto &assignment : evaluated_assignments)
            {
                const auto result = assignment.additive
                    ? vfp::replace_record_field_value_additive(
                          cursor.source_path,
                          cursor.recno - 1U,
                          assignment.field_name,
                          assignment.serialized_value)
                    : vfp::replace_record_field_value(
                          cursor.source_path,
                          cursor.recno - 1U,
                          assignment.field_name,
                          assignment.serialized_value);
                if (!result.ok)
                {
                    last_error_message = result.error;
                    if (temporary_record_lock)
                    {
                        unlock_cursor_record_lock(cursor, cursor.recno);
                    }
                    return false;
                }
                cursor.record_count = result.record_count;
            }
            if (temporary_record_lock)
            {
                unlock_cursor_record_lock(cursor, cursor.recno);
            }
            synchronize_relations_for_parent(cursor, frame);
            return true;
        }

        bool replace_records(
            CursorState &cursor,
            const std::vector<ReplaceAssignment> &assignments,
            const Frame &frame,
            const std::optional<AggregateScopeClause> &scope,
            const std::string &for_expression,
            const std::string &while_expression)
        {
            if (!scope.has_value() &&
                trim_copy(for_expression).empty() &&
                trim_copy(while_expression).empty())
            {
                return replace_current_record_fields(cursor, assignments, frame, true);
            }

            const AggregateScopeClause effective_scope = scope.value_or(AggregateScopeClause{});
            const std::vector<std::size_t> target_records = collect_aggregate_scope_records(
                cursor,
                frame,
                effective_scope,
                for_expression,
                while_expression,
                effective_scope.kind != AggregateScopeKind::record);
            for (const std::size_t recno : target_records)
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                if (!replace_current_record_fields(cursor, assignments, frame, true))
                {
                    return false;
                }
            }
            return true;
        }

        std::vector<std::string> cursor_field_names(const CursorState &cursor)
        {
            std::vector<std::string> names;
            const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(cursor);
            names.reserve(fields.size());
            for (const auto &field : fields)
            {
                names.push_back(field.name);
            }
            return names;
        }

        std::vector<std::string> effective_visible_field_names(
            const CursorState &cursor,
            const std::string &override_field_list_text)
        {
            std::vector<std::string> all_fields = cursor_field_names(cursor);
            if (all_fields.empty())
            {
                return all_fields;
            }

            std::string field_list_text = trim_copy(override_field_list_text);
            if (field_list_text.empty())
            {
                const auto &set_state = current_set_state();
                const auto enabled = set_state.find("fields_enabled");
                if (enabled == set_state.end() || !is_set_enabled("fields_enabled"))
                {
                    return all_fields;
                }

                const auto fields = set_state.find("fields");
                if (fields == set_state.end())
                {
                    return all_fields;
                }

                field_list_text = trim_copy(fields->second);
            }

            if (field_list_text.empty())
            {
                return all_fields;
            }

            if (collapse_identifier(field_list_text) == "ALL")
            {
                return all_fields;
            }

            const std::vector<std::string> field_filter = parse_field_filter_clause(field_list_text);
            if (field_filter.empty())
            {
                return all_fields;
            }

            const bool pattern_filter = !field_filter.empty() &&
                (field_filter.front() == "__LIKE__" || field_filter.front() == "__EXCEPT__");
            if (pattern_filter)
            {
                std::vector<std::string> visible_fields;
                for (const std::string &candidate : all_fields)
                {
                    if (field_matches_filter(candidate, field_filter))
                    {
                        visible_fields.push_back(candidate);
                    }
                }
                return visible_fields.empty() ? all_fields : visible_fields;
            }

            std::vector<std::string> visible_fields;
            for (std::string requested_field : split_csv_like(field_list_text))
            {
                requested_field = trim_copy(std::move(requested_field));
                if (requested_field.empty())
                {
                    continue;
                }

                const std::string normalized_request = collapse_identifier(requested_field);
                if (normalized_request == "ALL")
                {
                    return all_fields;
                }

                const auto match = std::find_if(all_fields.begin(), all_fields.end(), [&](const std::string &candidate)
                {
                    return collapse_identifier(candidate) == normalized_request;
                });
                if (match != all_fields.end())
                {
                    visible_fields.push_back(*match);
                }
            }

            return visible_fields.empty() ? all_fields : visible_fields;
        }

        std::vector<vfp::DbfFieldDescriptor> cursor_field_descriptors(const CursorState &cursor)
        {
            std::vector<vfp::DbfFieldDescriptor> fields;
            if (cursor.remote)
            {
                if (!cursor.remote_fields.empty())
                {
                    return cursor.remote_fields;
                }

                if (!cursor.remote_records.empty())
                {
                    fields.reserve(cursor.remote_records.front().values.size());
                    for (const auto &value : cursor.remote_records.front().values)
                    {
                        const char type = value.field_type == '\0' ? 'C' : value.field_type;
                        fields.push_back(vfp::DbfFieldDescriptor{
                            .name = value.field_name,
                            .type = type,
                            .length = static_cast<std::uint8_t>(type == 'N' || type == 'F' ? 18U : 32U),
                            .decimal_count = 0U});
                    }
                    return fields;
                }
                fields.push_back(vfp::DbfFieldDescriptor{.name = "ID", .type = 'N', .length = 18U, .decimal_count = 0U});
                fields.push_back(vfp::DbfFieldDescriptor{.name = "NAME", .type = 'C', .length = 32U, .decimal_count = 0U});
                fields.push_back(vfp::DbfFieldDescriptor{.name = "AMOUNT", .type = 'N', .length = 18U, .decimal_count = 0U});
                return fields;
            }

            if (cursor.source_path.empty())
            {
                return fields;
            }
            return cursor.local_fields;
        }

        std::string cursor_field_name(const std::string &designator, std::size_t one_based_index)
        {
            if (one_based_index == 0U)
            {
                return {};
            }
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr)
            {
                return {};
            }
            const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(*cursor);
            return one_based_index <= fields.size() ? fields[one_based_index - 1U].name : std::string{};
        }

        std::size_t cursor_field_size(const std::string &designator, const std::string &field_name, std::size_t one_based_index)
        {
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr)
            {
                return 0U;
            }
            const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(*cursor);
            if (one_based_index > 0U)
            {
                return one_based_index <= fields.size() ? fields[one_based_index - 1U].length : 0U;
            }
            const std::string normalized_field = collapse_identifier(field_name);
            const auto found = std::find_if(
                fields.begin(),
                fields.end(),
                [&](const vfp::DbfFieldDescriptor &field)
                {
                    return collapse_identifier(field.name) == normalized_field;
                });
            return found == fields.end() ? 0U : found->length;
        }

        std::optional<std::string> current_record_field_display_value(CursorState &cursor, const std::string &field_name)
        {
            const std::string normalized = collapse_identifier(field_name);
            if (cursor.remote)
            {
                if (cursor.recno == 0U || cursor.eof || cursor.recno > cursor.remote_records.size())
                {
                    return std::nullopt;
                }
                const auto &record = cursor.remote_records[cursor.recno - 1U];
                const auto value = std::find_if(record.values.begin(), record.values.end(), [&](const vfp::DbfRecordValue &candidate)
                                                { return collapse_identifier(candidate.field_name) == normalized; });
                if (value == record.values.end())
                {
                    return std::nullopt;
                }
                if (value->is_null)
                {
                    return "null";
                }
                return value->display_value;
            }

            if (cursor.source_path.empty() || cursor.recno == 0U || cursor.eof)
            {
                return std::nullopt;
            }
            const auto record = current_record(cursor);
            if (!record.has_value())
            {
                return std::nullopt;
            }
            const auto value = std::find_if(record->values.begin(), record->values.end(), [&](const vfp::DbfRecordValue &candidate)
                                            { return collapse_identifier(candidate.field_name) == normalized; });
            if (value == record->values.end())
            {
                return std::nullopt;
            }
            if (value->is_null)
            {
                return "null";
            }
            return value->display_value;
        }

        bool commit_buffered_record(CursorState &cursor, std::size_t recno, bool force_update = false)
        {
            const auto buffered = cursor.buffered_records.find(recno);
            if (buffered == cursor.buffered_records.end())
            {
                return true;
            }
            if (cursor.source_path.empty())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor",
                    {{"command", "TABLEUPDATE"}});
                return false;
            }
            if (cursor.buffering_mode == 3 && !force_update &&
                !optimistic_record_matches_disk(cursor, recno, "TABLEUPDATE"))
            {
                return false;
            }
            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            const bool appended = cursor.buffered_appended_records.contains(recno);
            const auto field_states = cursor.buffered_field_states.find(recno);
            const auto field_requires_update = [&](std::size_t field_index) -> bool
            {
                if (appended)
                {
                    return true;
                }
                if (field_states == cursor.buffered_field_states.end())
                {
                    return false;
                }
                const auto state = field_states->second.find(field_index);
                return state != field_states->second.end() &&
                    (state->second == 2 || state->second == 4);
            };
            for (std::size_t field_index = 0U; field_index < buffered->second.values.size(); ++field_index)
            {
                if (!field_requires_update(field_index))
                {
                    continue;
                }
                const auto &field = buffered->second.values[field_index];
                const auto result = vfp::replace_record_field_value(
                    cursor.source_path,
                    recno - 1U,
                    field.field_name,
                    field.display_value);
                if (!result.ok)
                {
                    last_error_message = result.error;
                    return false;
                }
                cursor.record_count = result.record_count;
            }

            const auto deletion_state = cursor.buffered_deletion_states.find(recno);
            const bool deletion_requires_update = appended ||
                (deletion_state != cursor.buffered_deletion_states.end() &&
                 (deletion_state->second == 2 || deletion_state->second == 4));
            if (deletion_requires_update)
            {
                const auto deleted_result = vfp::set_record_deleted_flag(
                    cursor.source_path,
                    recno - 1U,
                    buffered->second.deleted);
                if (!deleted_result.ok)
                {
                    last_error_message = deleted_result.error;
                    return false;
                }
                cursor.record_count = deleted_result.record_count;
            }
            cursor.buffered_records.erase(buffered);
            cursor.buffered_original_records.erase(recno);
            cursor.buffered_field_states.erase(recno);
            cursor.buffered_deletion_states.erase(recno);
            if ((cursor.buffering_mode == 2 || cursor.buffering_mode == 4) &&
                cursor.buffered_record_locks.erase(recno) != 0U)
            {
                unlock_cursor_record_lock(cursor, recno);
            }
            return true;
        }

        std::optional<PrgValue> cursor_buffering_function(
            const std::string &function,
            const std::vector<PrgValue> &arguments,
            const Frame &frame)
        {
            if (function != "cursorsetprop" && function != "cursorgetprop" &&
                function != "tableupdate" && function != "tablerevert" &&
                function != "getnextmodified" && function != "getfldstate" &&
                function != "setfldstate" && function != "oldval" &&
                function != "curval")
            {
                return std::nullopt;
            }

            const auto cursor_for_argument = [&](std::size_t index) -> CursorState *
            {
                return index < arguments.size()
                    ? resolve_cursor_target(value_as_string(arguments[index]))
                    : resolve_cursor_target({});
            };
            const auto require_local_cursor = [&](CursorState *cursor, const std::string &command) -> bool
            {
                if (cursor != nullptr && !cursor->remote && !cursor->source_path.empty())
                {
                    return true;
                }
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor",
                    {{"command", command}});
                return false;
            };

            if (function == "getnextmodified")
            {
                if (arguments.empty())
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.TooFewArguments"),
                        1229);
                }
                CursorState *cursor = arguments.size() >= 2U
                    ? cursor_for_argument(1U)
                    : resolve_cursor_target({});
                if (cursor == nullptr)
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.AliasNotFound"),
                        13);
                }
                if (cursor->buffering_mode < 4 || cursor->buffering_mode > 5)
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.TableBufferingNotEnabled"),
                        1596);
                }

                const double requested_record = value_as_number(arguments[0]);
                const std::size_t start_record = std::isfinite(requested_record) && requested_record > 0.0
                    ? static_cast<std::size_t>(requested_record)
                    : 0U;
                const auto next = cursor->buffered_records.upper_bound(start_record);
                return next == cursor->buffered_records.end()
                    ? make_number_value(0.0)
                    : make_number_value(static_cast<double>(next->first));
            }

            if (function == "getfldstate")
            {
                if (arguments.empty())
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.TooFewArguments"),
                        1229);
                }
                CursorState *cursor = arguments.size() >= 2U
                    ? cursor_for_argument(1U)
                    : resolve_cursor_target({});
                if (cursor == nullptr)
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.AliasNotFound"),
                        13);
                }
                if (!require_local_cursor(cursor, "GETFLDSTATE"))
                {
                    return make_empty_value();
                }
                if (cursor->buffering_mode < 2 || cursor->buffering_mode > 5)
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.TableBufferingNotEnabled"),
                        1596);
                }
                if (cursor->recno == 0U || cursor->eof)
                {
                    return make_null_value();
                }

                const auto record = current_record(*cursor);
                if (!record.has_value())
                {
                    return make_null_value();
                }
                const bool appended = cursor->buffered_appended_records.contains(cursor->recno);
                const int unchanged_state = appended ? 3 : 1;
                const auto field_state = [&](std::size_t field_index) -> int
                {
                    const auto record_states = cursor->buffered_field_states.find(cursor->recno);
                    if (record_states == cursor->buffered_field_states.end())
                    {
                        return unchanged_state;
                    }
                    const auto state = record_states->second.find(field_index);
                    return state == record_states->second.end() ? unchanged_state : state->second;
                };
                const auto deletion_state = [&]() -> int
                {
                    const auto state = cursor->buffered_deletion_states.find(cursor->recno);
                    return state == cursor->buffered_deletion_states.end() ? unchanged_state : state->second;
                };

                const bool numeric_argument = arguments[0].kind == PrgValueKind::number ||
                    arguments[0].kind == PrgValueKind::int64 ||
                    arguments[0].kind == PrgValueKind::uint64 ||
                    arguments[0].kind == PrgValueKind::currency;
                const double requested = numeric_argument ? value_as_number(arguments[0]) : 0.0;
                const bool is_integral = numeric_argument && std::isfinite(requested) &&
                    requested == std::trunc(requested);
                if (is_integral && requested == -1.0)
                {
                    std::string states;
                    states.reserve(record->values.size() + 1U);
                    states += static_cast<char>('0' + deletion_state());
                    for (std::size_t index = 0U; index < record->values.size(); ++index)
                    {
                        states += static_cast<char>('0' + field_state(index));
                    }
                    return make_string_value(states);
                }
                if (is_integral && requested == 0.0)
                {
                    return make_number_value(static_cast<double>(deletion_state()));
                }
                if (is_integral && requested > 0.0 &&
                    requested <= static_cast<double>(record->values.size()))
                {
                    return make_number_value(static_cast<double>(
                        field_state(static_cast<std::size_t>(requested - 1.0))));
                }
                if (numeric_argument)
                {
                    return make_empty_value();
                }

                const std::string field_name = collapse_identifier(value_as_string(arguments[0]));
                const auto field = std::find_if(
                    record->values.begin(),
                    record->values.end(),
                    [&](const vfp::DbfRecordValue &candidate)
                    {
                        return collapse_identifier(candidate.field_name) == field_name;
                    });
                if (field == record->values.end())
                {
                    return make_empty_value();
                }
                return make_number_value(static_cast<double>(field_state(static_cast<std::size_t>(
                    std::distance(record->values.begin(), field)))));
            }

            if (function == "setfldstate")
            {
                if (arguments.size() < 2U)
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.TooFewArguments"),
                        1229);
                }
                CursorState *cursor = arguments.size() >= 3U
                    ? cursor_for_argument(2U)
                    : resolve_cursor_target({});
                if (cursor == nullptr)
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.AliasNotFound"),
                        13);
                }
                if (!require_local_cursor(cursor, "SETFLDSTATE"))
                {
                    return make_boolean_value(false);
                }
                if (cursor->buffering_mode < 2 || cursor->buffering_mode > 5 ||
                    cursor->recno == 0U || cursor->eof)
                {
                    return make_boolean_value(false);
                }

                const double requested_state = value_as_number(arguments[1U]);
                if (!std::isfinite(requested_state) ||
                    requested_state != std::trunc(requested_state) ||
                    requested_state < 1.0 || requested_state > 4.0)
                {
                    return make_boolean_value(false);
                }
                const int state = static_cast<int>(requested_state);
                const auto record = current_record(*cursor);
                if (!record.has_value())
                {
                    return make_boolean_value(false);
                }

                const auto materialize_modified_record = [&]() -> bool
                {
                    if (state != 2 && state != 4)
                    {
                        return true;
                    }
                    if (cursor->buffered_records.contains(cursor->recno))
                    {
                        return true;
                    }

                    bool acquired_buffer_lock = false;
                    if ((cursor->buffering_mode == 2 || cursor->buffering_mode == 4) &&
                        !cursor->buffered_record_locks.contains(cursor->recno))
                    {
                        bool new_lock = false;
                        if (!acquire_record_lock(
                                *cursor, cursor->recno, "SETFLDSTATE", false, new_lock))
                        {
                            return false;
                        }
                        if (new_lock)
                        {
                            cursor->buffered_record_locks.insert(cursor->recno);
                            acquired_buffer_lock = true;
                        }
                    }

                    const auto table_result = parse_cursor_table(*cursor, cursor->recno);
                    if (!table_result.ok || cursor->recno > table_result.table.records.size())
                    {
                        if (acquired_buffer_lock)
                        {
                            cursor->buffered_record_locks.erase(cursor->recno);
                            unlock_cursor_record_lock(*cursor, cursor->recno);
                        }
                        last_error_message = table_result.error.empty()
                            ? runtime_text(
                                  "Runtime.Prg.Records.Error.CommandRequiresCurrentLocalRecord",
                                  {{"command", "SETFLDSTATE"}})
                            : table_result.error;
                        return false;
                    }
                    const vfp::DbfRecord original = table_result.table.records[cursor->recno - 1U];
                    cursor->buffered_records.emplace(cursor->recno, original);
                    cursor->buffered_original_records.emplace(cursor->recno, original);
                    return true;
                };

                const bool numeric_argument = arguments[0].kind == PrgValueKind::number ||
                    arguments[0].kind == PrgValueKind::int64 ||
                    arguments[0].kind == PrgValueKind::uint64 ||
                    arguments[0].kind == PrgValueKind::currency;
                const double requested_field = numeric_argument ? value_as_number(arguments[0]) : 0.0;
                const bool is_integral = numeric_argument && std::isfinite(requested_field) &&
                    requested_field == std::trunc(requested_field);
                if (is_integral && requested_field == 0.0)
                {
                    if (!materialize_modified_record())
                    {
                        return make_boolean_value(false);
                    }
                    cursor->buffered_deletion_states[cursor->recno] = state;
                    return make_boolean_value(true);
                }
                if (is_integral && requested_field > 0.0 &&
                    requested_field <= static_cast<double>(record->values.size()))
                {
                    if (!materialize_modified_record())
                    {
                        return make_boolean_value(false);
                    }
                    cursor->buffered_field_states[cursor->recno][
                        static_cast<std::size_t>(requested_field - 1.0)] = state;
                    return make_boolean_value(true);
                }
                if (numeric_argument)
                {
                    return make_boolean_value(false);
                }

                const std::string field_name = collapse_identifier(value_as_string(arguments[0]));
                const auto field = std::find_if(
                    record->values.begin(),
                    record->values.end(),
                    [&](const vfp::DbfRecordValue &candidate)
                    {
                        return collapse_identifier(candidate.field_name) == field_name;
                    });
                if (field == record->values.end())
                {
                    return make_boolean_value(false);
                }
                if (!materialize_modified_record())
                {
                    return make_boolean_value(false);
                }
                cursor->buffered_field_states[cursor->recno][static_cast<std::size_t>(
                    std::distance(record->values.begin(), field))] = state;
                return make_boolean_value(true);
            }

            if (function == "oldval")
            {
                if (arguments.empty())
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.TooFewArguments"),
                        1229);
                }
                CursorState *cursor = arguments.size() >= 2U
                    ? cursor_for_argument(1U)
                    : resolve_cursor_target({});
                if (cursor == nullptr)
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.AliasNotFound"),
                        13);
                }
                if (!require_local_cursor(cursor, "OLDVAL") ||
                    cursor->buffering_mode < 2 || cursor->buffering_mode > 5 ||
                    cursor->recno == 0U || cursor->eof)
                {
                    return make_empty_value();
                }

                const auto original = cursor->buffered_original_records.find(cursor->recno);
                if (original == cursor->buffered_original_records.end())
                {
                    return make_empty_value();
                }

                record_evaluation_overrides.emplace_back(cursor, &original->second);
                try
                {
                    const PrgValue result = evaluate_expression(
                        value_as_string(arguments[0U]), frame, cursor);
                    record_evaluation_overrides.pop_back();
                    return result;
                }
                catch (...)
                {
                    record_evaluation_overrides.pop_back();
                    throw;
                }
            }

            if (function == "curval")
            {
                if (arguments.empty())
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.TooFewArguments"),
                        1229);
                }
                CursorState *cursor = arguments.size() >= 2U
                    ? cursor_for_argument(1U)
                    : resolve_cursor_target({});
                if (cursor == nullptr)
                {
                    throw PrgCompatibilityError(
                        runtime_text("Runtime.Prg.Records.Error.AliasNotFound"),
                        13);
                }
                if (!require_local_cursor(cursor, "CURVAL") || cursor->recno == 0U || cursor->eof)
                {
                    return make_empty_value();
                }

                const auto table_result = parse_cursor_table(*cursor, cursor->recno);
                if (!table_result.ok || cursor->recno > table_result.table.records.size())
                {
                    return make_empty_value();
                }
                const vfp::DbfRecord &on_disk_record = table_result.table.records[cursor->recno - 1U];
                record_evaluation_overrides.emplace_back(cursor, &on_disk_record);
                try
                {
                    const PrgValue result = evaluate_expression(
                        value_as_string(arguments[0U]), frame, cursor);
                    record_evaluation_overrides.pop_back();
                    return result;
                }
                catch (...)
                {
                    record_evaluation_overrides.pop_back();
                    throw;
                }
            }

            if (function == "cursorgetprop")
            {
                if (arguments.empty() ||
                    uppercase_copy(trim_copy(value_as_string(arguments[0]))) != "BUFFERING")
                {
                    return make_number_value(0.0);
                }
                CursorState *cursor = cursor_for_argument(1U);
                if (cursor == nullptr)
                {
                    return make_number_value(0.0);
                }
                return require_local_cursor(cursor, "CURSORGETPROP")
                    ? make_number_value(static_cast<double>(cursor->buffering_mode))
                    : make_number_value(0.0);
            }

            if (function == "cursorsetprop")
            {
                if (arguments.size() < 2U ||
                    uppercase_copy(trim_copy(value_as_string(arguments[0]))) != "BUFFERING")
                {
                    return make_boolean_value(false);
                }
                CursorState *cursor = cursor_for_argument(2U);
                if (!require_local_cursor(cursor, "CURSORSETPROP"))
                {
                    return make_boolean_value(false);
                }
                const int requested_mode = static_cast<int>(std::llround(value_as_number(arguments[1])));
                if (requested_mode != 1 && requested_mode != 2 && requested_mode != 3 &&
                    requested_mode != 4 && requested_mode != 5)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Records.Error.UnsupportedCursorBufferingMode",
                        {{"mode", std::to_string(requested_mode)}});
                    return make_boolean_value(false);
                }
                if (requested_mode != cursor->buffering_mode && !cursor->buffered_records.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Records.Error.PendingCursorBufferChanges",
                        {{"command", "CURSORSETPROP"}});
                    return make_boolean_value(false);
                }
                cursor->buffering_mode = requested_mode;
                return make_boolean_value(true);
            }

            const bool revert = function == "tablerevert";
            const std::size_t alias_argument_index = revert ? 1U : 2U;
            CursorState *cursor = cursor_for_argument(alias_argument_index);
            if (!require_local_cursor(cursor, revert ? "TABLEREVERT" : "TABLEUPDATE"))
            {
                return make_boolean_value(false);
            }
            if (cursor->buffering_mode != 2 && cursor->buffering_mode != 3 &&
                cursor->buffering_mode != 4 && cursor->buffering_mode != 5)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.CursorBufferingNotEnabled",
                    {{"command", revert ? "TABLEREVERT" : "TABLEUPDATE"}});
                return make_boolean_value(false);
            }
            if (revert)
            {
                if (cursor->buffering_mode == 2 || cursor->buffering_mode == 3)
                {
                    if (cursor->recno != 0U && !cursor->eof)
                    {
                        cursor->buffered_records.erase(cursor->recno);
                        cursor->buffered_original_records.erase(cursor->recno);
                        cursor->buffered_field_states.erase(cursor->recno);
                        cursor->buffered_deletion_states.erase(cursor->recno);
                        if (cursor->buffered_record_locks.erase(cursor->recno) != 0U)
                        {
                            unlock_cursor_record_lock(*cursor, cursor->recno);
                        }
                    }
                    return make_boolean_value(true);
                }
                if (cursor->buffering_mode == 4)
                {
                    while (!cursor->buffered_record_locks.empty())
                    {
                        unlock_cursor_record_lock(*cursor, *cursor->buffered_record_locks.begin());
                    }
                }
                const std::size_t appended_count = cursor->buffered_appended_records.size();
                const std::size_t persisted_record_count =
                    cursor->record_count >= appended_count ? cursor->record_count - appended_count : 0U;
                cursor->buffered_records.clear();
                cursor->buffered_original_records.clear();
                cursor->buffered_field_states.clear();
                cursor->buffered_deletion_states.clear();
                cursor->buffered_appended_records.clear();
                cursor->record_count = persisted_record_count;
                move_cursor_to(
                    *cursor,
                    std::min<std::size_t>(cursor->recno, persisted_record_count));
                return make_boolean_value(true);
            }

            const bool force_update = arguments.size() >= 2U && value_as_bool(arguments[1]);
            if (cursor->buffering_mode == 2 || cursor->buffering_mode == 3)
            {
                return make_boolean_value(
                    cursor->recno == 0U || cursor->eof ||
                    commit_buffered_record(*cursor, cursor->recno, force_update));
            }

            if (cursor->buffered_records.empty())
            {
                return make_boolean_value(true);
            }
            if (!ensure_transaction_backup_for_table(cursor->source_path))
            {
                return make_boolean_value(false);
            }
            if (cursor->buffering_mode == 5 && !force_update)
            {
                for (const auto &[recno, _] : cursor->buffered_records)
                {
                    if (!cursor->buffered_appended_records.contains(recno) &&
                        !optimistic_record_matches_disk(*cursor, recno, "TABLEUPDATE"))
                    {
                        return make_boolean_value(false);
                    }
                }
            }
            for (const auto &[recno, record] : cursor->buffered_records)
            {
                const bool appended = cursor->buffered_appended_records.contains(recno);
                std::size_t persisted_recno = recno;
                if (appended)
                {
                    const int buffering_mode = cursor->buffering_mode;
                    cursor->buffering_mode = 1;
                    const bool appended = append_blank_record(*cursor);
                    cursor->buffering_mode = buffering_mode;
                    if (!appended)
                    {
                        return make_boolean_value(false);
                    }
                    persisted_recno = cursor->record_count;
                }
                const auto field_states = cursor->buffered_field_states.find(recno);
                const auto field_requires_update = [&](std::size_t field_index) -> bool
                {
                    if (appended)
                    {
                        return true;
                    }
                    if (field_states == cursor->buffered_field_states.end())
                    {
                        return false;
                    }
                    const auto state = field_states->second.find(field_index);
                    return state != field_states->second.end() &&
                        (state->second == 2 || state->second == 4);
                };
                for (std::size_t field_index = 0U; field_index < record.values.size(); ++field_index)
                {
                    if (!field_requires_update(field_index))
                    {
                        continue;
                    }
                    const auto &field = record.values[field_index];
                    const auto result = vfp::replace_record_field_value(
                        cursor->source_path,
                        persisted_recno - 1U,
                        field.field_name,
                        field.display_value);
                    if (!result.ok)
                    {
                        last_error_message = result.error;
                        return make_boolean_value(false);
                    }
                    cursor->record_count = result.record_count;
                }
                const auto deletion_state = cursor->buffered_deletion_states.find(recno);
                const bool deletion_requires_update = appended ||
                    (deletion_state != cursor->buffered_deletion_states.end() &&
                     (deletion_state->second == 2 || deletion_state->second == 4));
                if (deletion_requires_update)
                {
                    const auto deleted_result = vfp::set_record_deleted_flag(
                        cursor->source_path,
                        persisted_recno - 1U,
                        record.deleted);
                    if (!deleted_result.ok)
                    {
                        last_error_message = deleted_result.error;
                        return make_boolean_value(false);
                    }
                    cursor->record_count = deleted_result.record_count;
                }
                if (cursor->buffering_mode == 4 &&
                    !appended &&
                    cursor->buffered_record_locks.erase(recno) != 0U)
                {
                    unlock_cursor_record_lock(*cursor, recno);
                }
            }
            cursor->buffered_records.clear();
            cursor->buffered_original_records.clear();
            cursor->buffered_field_states.clear();
            cursor->buffered_deletion_states.clear();
            cursor->buffered_appended_records.clear();
            return make_boolean_value(true);
        }

        bool validate_not_null_fields(CursorState &cursor)
        {
            for (const auto &[field_name, rule] : cursor.field_rules)
            {
                if (rule.nullable)
                {
                    continue;
                }
                const auto value = current_record_field_display_value(cursor, field_name);
                if (!value.has_value())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Records.Error.ConstraintFieldNotFound",
                        {
                            {"constraint", "NOT NULL"},
                            {"fieldName", field_name}
                        });
                    return false;
                }
                const std::string normalized_value = lowercase_copy(trim_copy(*value));
                if (normalized_value.empty() || normalized_value == "null")
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Records.Error.ConstraintFailedForField",
                        {
                            {"constraint", "NOT NULL"},
                            {"fieldName", field_name}
                        });
                    return false;
                }
            }
            return true;
        }

        bool insert_record_values(
            CursorState &cursor,
            const Frame &frame,
            const std::string &field_list_text,
            const std::string &value_list_text)
        {
            std::vector<std::string> fields;
            if (trim_copy(field_list_text).empty())
            {
                fields = cursor_field_names(cursor);
                if (fields.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Records.Error.InsertTargetFieldsResolveFailed",
                        {{"command", "INSERT INTO"}});
                    return false;
                }
            }
            else
            {
                for (std::string field : split_csv_like(field_list_text))
                {
                    field = trim_copy(std::move(field));
                    if (!field.empty())
                    {
                        fields.push_back(field);
                    }
                }
            }

            std::vector<std::string> values = split_csv_like(value_list_text);
            if (fields.empty())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.InsertRequiresTargetField",
                    {{"command", "INSERT INTO"}});
                return false;
            }
            if (values.size() != fields.size())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.InsertFieldValueCountMismatch",
                    {{"command", "INSERT INTO"}});
                return false;
            }

            const std::size_t original_record_count = cursor.record_count;
            const std::size_t original_recno = cursor.recno;
            const bool original_found = cursor.found;
            const bool original_bof = cursor.bof;
            const bool original_eof = cursor.eof;

            if (!append_blank_record(cursor))
            {
                return false;
            }

            std::vector<ReplaceAssignment> assignments;
            assignments.reserve(fields.size());
            std::vector<std::string> explicit_fields;
            explicit_fields.reserve(fields.size());
            for (std::size_t index = 0U; index < fields.size(); ++index)
            {
                explicit_fields.push_back(collapse_identifier(fields[index]));
                assignments.push_back({.field_name = fields[index],
                                       .expression = trim_copy(values[index])});
            }
            std::vector<ReplaceAssignment> default_assignments;
            for (const auto &[field_name, rule] : cursor.field_rules)
            {
                if (!rule.has_default)
                {
                    continue;
                }
                if (std::find(explicit_fields.begin(), explicit_fields.end(), field_name) != explicit_fields.end())
                {
                    continue;
                }
                default_assignments.push_back({.field_name = field_name,
                                               .expression = rule.default_expression});
            }

            const bool defaults_ok = default_assignments.empty() || replace_current_record_fields(cursor, default_assignments, frame, false);
            const bool explicit_ok = defaults_ok && replace_current_record_fields(cursor, assignments, frame, false);
            if (explicit_ok && validate_not_null_fields(cursor))
            {
                return true;
            }

            const std::string replace_error = last_error_message;
            if (cursor.remote)
            {
                if (cursor.remote_records.size() > original_record_count)
                {
                    cursor.remote_records.resize(original_record_count);
                }
                cursor.record_count = cursor.remote_records.size();
                move_cursor_to(cursor, static_cast<long long>(std::min(original_recno, cursor.record_count)));
            }
            else if (!cursor.source_path.empty())
            {
                const auto rollback_result = vfp::truncate_dbf_table_file(cursor.source_path, original_record_count);
                if (rollback_result.ok)
                {
                    cursor.record_count = rollback_result.record_count;
                    move_cursor_to(cursor, static_cast<long long>(std::min(original_recno, cursor.record_count)));
                }
                else
                {
                    last_error_message = replace_error + " (rollback failed: " + rollback_result.error + ")";
                    return false;
                }
            }
            cursor.found = original_found;
            cursor.bof = original_bof;
            cursor.eof = original_eof;
            last_error_message = replace_error;
            return false;
        }

        bool append_blank_record(CursorState &cursor)
        {
            if (cursor.remote)
            {
                const std::size_t recno = cursor.remote_records.size() + 1U;
                cursor.remote_records.push_back(vfp::DbfRecord{
                    .record_index = recno - 1U,
                    .deleted = false,
                    .values = {
                        vfp::DbfRecordValue{.field_name = "ID", .field_type = 'N', .display_value = std::to_string(recno)},
                        vfp::DbfRecordValue{.field_name = "NAME", .field_type = 'C', .display_value = ""},
                        vfp::DbfRecordValue{.field_name = "AMOUNT", .field_type = 'N', .display_value = "0"},
                    }});
                cursor.record_count = cursor.remote_records.size();
                move_cursor_to(cursor, static_cast<long long>(cursor.record_count));
                cursor.found = false;
                return true;
            }
            if (cursor.source_path.empty())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor",
                    {{"command", "APPEND BLANK"}});
                return false;
            }

            if (cursor.buffering_mode == 2)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.RowBufferingAppendUnsupported",
                    {{"mode", "2"}});
                return false;
            }

            if (cursor.buffering_mode == 3 || cursor.buffering_mode == 4 || cursor.buffering_mode == 5)
            {
                const auto table_result = parse_table_path(
                    cursor.source_path,
                    std::numeric_limits<std::size_t>::max());
                if (!table_result.ok)
                {
                    last_error_message = table_result.error;
                    return false;
                }

                const std::size_t recno =
                    table_result.table.records.size() + cursor.buffered_appended_records.size() + 1U;
                vfp::DbfRecord pending_record;
                pending_record.record_index = recno - 1U;
                pending_record.deleted = false;
                for (const auto &field : cursor_field_descriptors(cursor))
                {
                    pending_record.values.push_back(vfp::DbfRecordValue{
                        .field_name = field.name,
                        .field_type = field.type,
                        .display_value = ""});
                }
                cursor.buffered_records[recno] = std::move(pending_record);
                cursor.buffered_appended_records.insert(recno);
                cursor.record_count = table_result.table.records.size() + cursor.buffered_appended_records.size();
                move_cursor_to(cursor, static_cast<long long>(recno));
                cursor.found = false;
                return true;
            }

            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            DataSessionState &session = current_session_state();
            bool temporary_table_lock = false;
            if (!acquire_table_lock(cursor, "APPEND BLANK", false, temporary_table_lock))
            {
                return false;
            }

            const auto result = vfp::append_blank_record_to_file(cursor.source_path);
            if (!result.ok)
            {
                last_error_message = result.error;
                if (temporary_table_lock)
                {
                    session.table_locks.erase(cursor.work_area);
                    release_shared_table_lock_ownership(cursor, current_data_session);
                }
                return false;
            }

            cursor.record_count = result.record_count;
            move_cursor_to(cursor, static_cast<long long>(result.record_count));
            cursor.found = false;
            if (temporary_table_lock)
            {
                session.table_locks.erase(cursor.work_area);
                release_shared_table_lock_ownership(cursor, current_data_session);
            }
            return true;
        }

        bool set_deleted_flag(
            CursorState &cursor,
            const Frame &frame,
            const std::optional<AggregateScopeClause> &scope,
            const std::string &for_expression,
            const std::string &while_expression,
            bool deleted)
        {
            if (cursor.remote)
            {
                std::vector<std::size_t> target_records;
                if (!scope.has_value() && for_expression.empty() && while_expression.empty())
                {
                    if (cursor.recno == 0U || cursor.eof || cursor.recno > cursor.remote_records.size())
                    {
                        last_error_message = runtime_text("Runtime.Prg.Records.Error.RequiresCurrentRemoteRecord");
                        return false;
                    }
                    target_records.push_back(cursor.recno);
                }
                else
                {
                    target_records = collect_aggregate_scope_records(
                        cursor,
                        frame,
                        scope.value_or(AggregateScopeClause{}),
                        for_expression,
                        while_expression,
                        deleted);
                }

                for (const std::size_t recno : target_records)
                {
                    cursor.remote_records[recno - 1U].deleted = deleted;
                }
                return true;
            }
            if (cursor.source_path.empty())
            {
                last_error_message = runtime_text("Runtime.Prg.Records.Error.RequiresLocalTableBackedCursor");
                return false;
            }

            if (cursor.buffering_mode == 2 || cursor.buffering_mode == 3 ||
                cursor.buffering_mode == 4 || cursor.buffering_mode == 5)
            {
                if ((cursor.buffering_mode == 2 || cursor.buffering_mode == 3) &&
                    (scope.has_value() || !for_expression.empty() || !while_expression.empty()))
                {
                    last_error_message = runtime_text("Runtime.Prg.Records.Error.RequiresCurrentLocalRecord");
                    return false;
                }
                std::vector<std::size_t> target_records;
                if (!scope.has_value() && for_expression.empty() && while_expression.empty())
                {
                    if (cursor.recno == 0U || cursor.eof)
                    {
                        last_error_message = runtime_text("Runtime.Prg.Records.Error.RequiresCurrentLocalRecord");
                        return false;
                    }
                    target_records.push_back(cursor.recno);
                }
                else
                {
                    target_records = collect_aggregate_scope_records(
                        cursor,
                        frame,
                        scope.value_or(AggregateScopeClause{}),
                        for_expression,
                        while_expression,
                        deleted);
                }

                for (const std::size_t recno : target_records)
                {
                    if ((cursor.buffering_mode == 2 || cursor.buffering_mode == 4) &&
                        !cursor.buffered_record_locks.contains(recno))
                    {
                        bool new_lock = false;
                        if (!acquire_record_lock(cursor, recno, deleted ? "DELETE" : "RECALL", false, new_lock))
                        {
                            return false;
                        }
                        if (new_lock)
                        {
                            cursor.buffered_record_locks.insert(recno);
                        }
                    }
                    auto buffered = cursor.buffered_records.find(recno);
                    if (buffered == cursor.buffered_records.end())
                    {
                        const auto table_result = parse_cursor_table(cursor, recno);
                        if (!table_result.ok || recno > table_result.table.records.size())
                        {
                            if ((cursor.buffering_mode == 2 || cursor.buffering_mode == 4) &&
                                cursor.buffered_record_locks.erase(recno) != 0U)
                            {
                                unlock_cursor_record_lock(cursor, recno);
                            }
                            last_error_message = table_result.error.empty()
                                ? runtime_text("Runtime.Prg.Records.Error.RequiresCurrentLocalRecord")
                                : table_result.error;
                            return false;
                        }
                        buffered = cursor.buffered_records.emplace(
                            recno,
                            table_result.table.records[recno - 1U]).first;
                        cursor.buffered_original_records.emplace(recno, buffered->second);
                    }
                    buffered->second.deleted = deleted;
                    cursor.buffered_deletion_states[recno] =
                        cursor.buffered_appended_records.contains(recno) ? 4 : 2;
                }
                return true;
            }

            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            std::vector<std::size_t> target_records;
            if (!scope.has_value() && for_expression.empty() && while_expression.empty())
            {
                if (cursor.recno == 0U || cursor.eof)
                {
                    last_error_message = runtime_text("Runtime.Prg.Records.Error.RequiresCurrentLocalRecord");
                    return false;
                }
                target_records.push_back(cursor.recno);
            }
            else
            {
                target_records = collect_aggregate_scope_records(
                    cursor,
                    frame,
                    scope.value_or(AggregateScopeClause{}),
                    for_expression,
                    while_expression,
                    deleted);
            }

            for (const std::size_t recno : target_records)
            {
                bool temporary_record_lock = false;
                if (!acquire_record_lock(cursor, recno, deleted ? "DELETE" : "RECALL", false, temporary_record_lock))
                {
                    return false;
                }
                const auto result = vfp::set_record_deleted_flag(cursor.source_path, recno - 1U, deleted);
                if (!result.ok)
                {
                    last_error_message = result.error;
                    if (temporary_record_lock)
                    {
                        unlock_cursor_record_lock(cursor, recno);
                    }
                    return false;
                }
                cursor.record_count = result.record_count;
                if (temporary_record_lock)
                {
                    unlock_cursor_record_lock(cursor, recno);
                }
            }

            return true;
        }

        bool ensure_exclusive_table_maintenance(const CursorState &cursor, const std::string &command_name)
        {
            if (cursor.remote || cursor.exclusive)
            {
                return true;
            }

            last_error_message = runtime_text(
                "Runtime.Prg.Records.Error.RequiresExclusiveCursorForMaintenance",
                {{"command", command_name}});
            return false;
        }

        bool pack_cursor(CursorState &cursor)
        {
            const std::size_t original_recno = cursor.recno;
            if (cursor.remote)
            {
                cursor.remote_records.erase(
                    std::remove_if(
                        cursor.remote_records.begin(),
                        cursor.remote_records.end(),
                        [](const vfp::DbfRecord &record)
                        {
                            return record.deleted;
                        }),
                    cursor.remote_records.end());
                for (std::size_t index = 0U; index < cursor.remote_records.size(); ++index)
                {
                    cursor.remote_records[index].record_index = index;
                }
                cursor.record_count = cursor.remote_records.size();
                move_cursor_to(cursor, static_cast<long long>(std::min(original_recno, cursor.record_count)));
                cursor.found = false;
                return true;
            }

            if (cursor.source_path.empty())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor",
                    {{"command", "PACK"}});
                return false;
            }
            if (!ensure_exclusive_table_maintenance(cursor, "PACK"))
            {
                return false;
            }
            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            const auto result = vfp::pack_dbf_table_file(cursor.source_path);
            if (!result.ok)
            {
                last_error_message = result.error;
                return false;
            }

            cursor.record_count = result.record_count;
            move_cursor_to(cursor, static_cast<long long>(std::min(original_recno, cursor.record_count)));
            cursor.found = false;
            return true;
        }

        bool zap_cursor(CursorState &cursor)
        {
            if (cursor.remote)
            {
                cursor.remote_records.clear();
                cursor.record_count = 0U;
                move_cursor_to(cursor, 0);
                cursor.found = false;
                return true;
            }

            if (cursor.source_path.empty())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor",
                    {{"command", "ZAP"}});
                return false;
            }
            if (!ensure_exclusive_table_maintenance(cursor, "ZAP"))
            {
                return false;
            }
            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            const auto result = vfp::zap_dbf_table_file(cursor.source_path);
            if (!result.ok)
            {
                last_error_message = result.error;
                return false;
            }

            cursor.record_count = result.record_count;
            move_cursor_to(cursor, 0);
            cursor.found = false;
            return true;
        }

        PrgValue runtime_lock_function(
            const std::string &function,
            const std::vector<std::string> &raw_arguments,
            const std::vector<PrgValue> &arguments)
        {
            const auto resolve_lock_cursor = [&]() -> CursorState *
            {
                if (arguments.empty())
                {
                    return resolve_cursor_target({});
                }
                if (arguments.front().kind == PrgValueKind::string)
                {
                    return resolve_cursor_target(value_as_string(arguments.front()));
                }
                if (!raw_arguments.empty())
                {
                    const std::string raw = trim_copy(raw_arguments.front());
                    if (!raw.empty() && !std::all_of(raw.begin(), raw.end(), [](unsigned char ch)
                                                     { return std::isdigit(ch) != 0; }))
                    {
                        if (CursorState *cursor = resolve_cursor_target(raw))
                        {
                            return cursor;
                        }
                    }
                }
                return resolve_cursor_target({});
            };

            CursorState *cursor = resolve_lock_cursor();
            if (cursor == nullptr)
            {
                return make_boolean_value(false);
            }

            const int area = cursor->work_area;
            const std::string normalized_function = normalize_identifier(function);
            if (normalized_function == "flock" || normalized_function == "lock")
            {
                bool new_lock = false;
                return make_boolean_value(acquire_table_lock(*cursor,
                                                            (cursor->alias.empty() ? std::to_string(area) : cursor->alias) + " FLOCK",
                                                            true,
                                                            new_lock));
            }
            if (normalized_function == "isflocked")
            {
                const std::string resource_key = cursor_lock_resource_key(*cursor);
                std::lock_guard<std::mutex> lock(concurrency_state->mutex);
                return make_boolean_value(concurrency_state->table_lock_owner_by_resource.contains(resource_key));
            }

            const auto bounded_record_number = [](double requested) -> std::size_t
            {
                // Keep llround inside its defined domain and avoid converting its
                // integral result back through double before checking size_t.
                constexpr double first_out_of_range_record_number = 0x1p+63;
                if (!std::isfinite(requested) || requested < 0.0 ||
                    requested >= first_out_of_range_record_number)
                {
                    return 0U;
                }

                const long long rounded = std::llround(requested);
                if (rounded <= 0LL)
                {
                    return 0U;
                }

                const std::uintmax_t unsigned_rounded = static_cast<std::uintmax_t>(rounded);
                if (unsigned_rounded > static_cast<std::uintmax_t>(std::numeric_limits<std::size_t>::max()))
                {
                    return 0U;
                }
                return static_cast<std::size_t>(unsigned_rounded);
            };

            std::size_t recno = cursor->recno;
            if (!arguments.empty() && arguments.front().kind != PrgValueKind::string)
            {
                recno = bounded_record_number(value_as_number(arguments.front()));
            }
            if (recno == 0U || recno > cursor->record_count)
            {
                return make_boolean_value(false);
            }

            if (normalized_function == "rlock")
            {
                bool new_lock = false;
                return make_boolean_value(acquire_record_lock(*cursor,
                                                             recno,
                                                             (cursor->alias.empty() ? std::to_string(area) : cursor->alias) + " RLOCK",
                                                             true,
                                                             new_lock));
            }
            if (normalized_function == "isrlocked")
            {
                const std::string resource_key = cursor_lock_resource_key(*cursor);
                std::lock_guard<std::mutex> lock(concurrency_state->mutex);
                const auto shared_record_found = concurrency_state->record_lock_owner_by_resource.find(resource_key);
                return make_boolean_value(shared_record_found != concurrency_state->record_lock_owner_by_resource.end() &&
                                          shared_record_found->second.contains(recno));
            }

            return make_empty_value();
        }

        void unlock_cursor_locks(CursorState *cursor, bool all_locks)
        {
            DataSessionState &session = current_session_state();
            if (all_locks || cursor == nullptr)
            {
                for (auto &[_, held_cursor] : session.cursors)
                {
                    release_shared_lock_ownership_for_cursor(held_cursor, session, current_data_session);
                    held_cursor.buffered_original_records.clear();
                    held_cursor.buffered_record_locks.clear();
                }
                session.table_locks.clear();
                session.record_locks.clear();
                return;
            }

            release_shared_lock_ownership_for_cursor(*cursor, session, current_data_session);
            cursor->buffered_original_records.clear();
            cursor->buffered_record_locks.clear();
            session.table_locks.erase(cursor->work_area);
            session.record_locks.erase(cursor->work_area);
        }

        void unlock_cursor_record_lock(CursorState &cursor, std::size_t recno)
        {
            DataSessionState &session = current_session_state();
            cursor.buffered_record_locks.erase(recno);
            auto found = session.record_locks.find(cursor.work_area);
            if (found == session.record_locks.end())
            {
                return;
            }

            found->second.erase(recno);
            release_shared_record_lock_ownership(cursor, recno, current_data_session);
            if (found->second.empty())
            {
                session.record_locks.erase(found);
            }
        }

        std::string evaluate_cursor_designator_expression(const std::string &expression, const Frame &frame)
        {
            const std::string trimmed_expression = trim_copy(expression);
            if (trimmed_expression.empty())
            {
                return {};
            }

            if (trimmed_expression.size() >= 2U &&
                ((trimmed_expression.front() == '\'' && trimmed_expression.back() == '\'') ||
                 (trimmed_expression.front() == '"' && trimmed_expression.back() == '"')))
            {
                return unquote_identifier(trimmed_expression);
            }

            const PrgValue evaluated = evaluate_expression(trimmed_expression, frame);
            const std::string designator = trim_copy(value_as_string(evaluated));
            if (!designator.empty())
            {
                return designator;
            }

            return is_bare_identifier_text(trimmed_expression) ? trimmed_expression : std::string{};
        }

        std::string try_parse_designator_argument(const std::string &raw_argument, const Frame &frame)
        {
            if (raw_argument.empty())
            {
                return {};
            }

            const std::string designator = evaluate_cursor_designator_expression(raw_argument, frame);
            return resolve_cursor_target(designator) == nullptr ? std::string{} : designator;
        }

        CursorState *resolve_cursor_target_expression(const std::string &raw_designator, const Frame &frame)
        {
            return resolve_cursor_target(evaluate_cursor_designator_expression(raw_designator, frame));
        }
