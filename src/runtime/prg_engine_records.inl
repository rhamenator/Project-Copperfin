// prg_engine_records.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        void restore_cursor_snapshot(CursorState &cursor, const CursorPositionSnapshot &snapshot) const
        {
            cursor.recno = snapshot.recno;
            cursor.found = snapshot.found;
            cursor.bof = snapshot.bof;
            cursor.eof = snapshot.eof;
            cursor.active_order_name = snapshot.active_order_name;
            cursor.active_order_expression = snapshot.active_order_expression;
            cursor.active_order_for_expression = snapshot.active_order_for_expression;
            cursor.active_order_path = snapshot.active_order_path;
            cursor.active_order_normalization_hint = snapshot.active_order_normalization_hint;
            cursor.active_order_collation_hint = snapshot.active_order_collation_hint;
            cursor.active_order_key_domain_hint = snapshot.active_order_key_domain_hint;
            cursor.active_order_descending = snapshot.active_order_descending;
        }

        bool current_record_matches_visibility(const CursorState &cursor, const Frame &frame, const std::string &extra_expression)
        {
            const auto record = current_record(cursor);
            if (!record.has_value())
            {
                return false;
            }
            if (is_set_enabled("deleted") && record->deleted)
            {
                return false;
            }
            if (!cursor.filter_expression.empty() && !value_as_bool(evaluate_expression(cursor.filter_expression, frame, &cursor)))
            {
                return false;
            }
            if (!extra_expression.empty() && !value_as_bool(evaluate_expression(extra_expression, frame, &cursor)))
            {
                return false;
            }
            return true;
        }

        bool seek_visible_record(
            CursorState &cursor,
            const Frame &frame,
            long long start_recno,
            int direction,
            const std::string &extra_expression,
            const std::string &while_expression,
            bool preserve_on_failure)
        {
            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            const long long first = direction >= 0 ? std::max<long long>(1, start_recno) : std::min<long long>(start_recno, static_cast<long long>(cursor.record_count));
            for (long long recno = first;
                 recno >= 1 && recno <= static_cast<long long>(cursor.record_count);
                 recno += direction)
            {
                move_cursor_to(cursor, recno);
                if (!while_expression.empty() && !value_as_bool(evaluate_expression(while_expression, frame, &cursor)))
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
            long long next_start = static_cast<long long>(cursor.recno) + direction;
            while (remaining > 0)
            {
                if (!seek_visible_record(cursor, frame, next_start, direction, {}, {}, false))
                {
                    return false;
                }
                --remaining;
                next_start = static_cast<long long>(cursor.recno) + direction;
            }
            return true;
        }

        std::optional<vfp::DbfRecord> current_record(const CursorState &cursor) const
        {
            if (cursor.recno == 0U || cursor.eof)
            {
                return std::nullopt;
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

            const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, cursor.recno);
            if (!table_result.ok || cursor.recno > table_result.table.records.size())
            {
                return std::nullopt;
            }

            return table_result.table.records[cursor.recno - 1U];
        }

        std::optional<PrgValue> resolve_field_value(const std::string &identifier, const CursorState *preferred_cursor) const
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

                const std::string requested = collapse_identifier(field_name);
                std::string current;
                for (char ch : field_list->second)
                {
                    if (ch == ',')
                    {
                        const std::string normalized = collapse_identifier(current);
                        if (normalized == "ALL" || normalized == requested)
                        {
                            return true;
                        }
                        current.clear();
                    }
                    else
                    {
                        current.push_back(ch);
                    }
                }
                const std::string normalized = collapse_identifier(current);
                return normalized == "ALL" || normalized == requested;
            };

            const auto value_from_record = [&](const CursorState *cursor, const std::string &field_name) -> std::optional<PrgValue>
            {
                if (cursor == nullptr)
                {
                    return std::nullopt;
                }
                if (!field_is_visible(field_name))
                {
                    return std::nullopt;
                }
                const auto record = current_record(*cursor);
                if (!record.has_value())
                {
                    return std::nullopt;
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

                switch (raw_field->field_type)
                {
                case 'L':
                    return make_boolean_value(normalize_index_value(*field_value) == "true");
                case 'N':
                case 'F':
                case 'I':
                case 'Y':
                    if (trim_copy(*field_value).empty())
                    {
                        return make_number_value(0.0);
                    }
                    return make_number_value(std::stod(trim_copy(*field_value)));
                default:
                    return make_string_value(*field_value);
                }
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
                last_error_message = "This command requires a local table-backed cursor";
                return false;
            }

            const bool found = seek_visible_record(
                cursor,
                frame,
                static_cast<long long>(start_recno),
                1,
                for_expression,
                while_expression,
                false);
            cursor.found = found;
            return true;
        }

        bool replace_current_record_fields(
            CursorState &cursor,
            const std::vector<ReplaceAssignment> &assignments,
            const Frame &frame)
        {
            if (cursor.remote)
            {
                if (cursor.recno == 0U || cursor.eof || cursor.recno > cursor.remote_records.size())
                {
                    last_error_message = "REPLACE requires a current remote record";
                    return false;
                }

                vfp::DbfRecord &record = cursor.remote_records[cursor.recno - 1U];
                for (const auto &assignment : assignments)
                {
                    const PrgValue value = evaluate_expression(assignment.expression, frame);
                    const std::string normalized_field = collapse_identifier(assignment.field_name);
                    auto field = std::find_if(record.values.begin(), record.values.end(), [&](vfp::DbfRecordValue &candidate)
                                              { return collapse_identifier(candidate.field_name) == normalized_field; });
                    if (field == record.values.end())
                    {
                        last_error_message = "Field not found on remote SQL cursor: " + assignment.field_name;
                        return false;
                    }
                    field->display_value = value_as_string(value);
                }
                return true;
            }
            if (cursor.source_path.empty() || cursor.recno == 0U || cursor.eof)
            {
                last_error_message = "REPLACE requires a current local record";
                return false;
            }
            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            for (const auto &assignment : assignments)
            {
                const PrgValue value = evaluate_expression(assignment.expression, frame);
                const auto result = vfp::replace_record_field_value(
                    cursor.source_path,
                    cursor.recno - 1U,
                    assignment.field_name,
                    value_as_string(value));
                if (!result.ok)
                {
                    last_error_message = result.error;
                    return false;
                }
                cursor.record_count = result.record_count;
            }
            return true;
        }

        bool replace_records(
            CursorState &cursor,
            const std::vector<ReplaceAssignment> &assignments,
            const Frame &frame,
            const std::string &for_expression,
            const std::string &while_expression)
        {
            if (trim_copy(for_expression).empty() && trim_copy(while_expression).empty())
            {
                return replace_current_record_fields(cursor, assignments, frame);
            }

            const std::size_t original_recno = cursor.recno;
            const bool original_found = cursor.found;
            const bool original_bof = cursor.bof;
            const bool original_eof = cursor.eof;
            std::size_t replaced_count = 0U;

            for (std::size_t recno = 1U; recno <= cursor.record_count; ++recno)
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                if (cursor.recno == 0U || cursor.eof)
                {
                    continue;
                }
                if (!while_expression.empty() && !value_as_bool(evaluate_expression(while_expression, frame, &cursor)))
                {
                    break;
                }
                if (!cursor.filter_expression.empty() && !value_as_bool(evaluate_expression(cursor.filter_expression, frame, &cursor)))
                {
                    continue;
                }
                if (!value_as_bool(evaluate_expression(for_expression, frame, &cursor)))
                {
                    continue;
                }

                if (!replace_current_record_fields(cursor, assignments, frame))
                {
                    return false;
                }
                ++replaced_count;
            }

            if (replaced_count == 0U)
            {
                move_cursor_to(cursor, static_cast<long long>(original_recno));
                cursor.found = original_found;
                cursor.bof = original_bof;
                cursor.eof = original_eof;
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

        std::vector<vfp::DbfFieldDescriptor> cursor_field_descriptors(const CursorState &cursor)
        {
            std::vector<vfp::DbfFieldDescriptor> fields;
            if (cursor.remote)
            {
                const auto add_remote_field = [&](const std::string &name, char type)
                {
                    fields.push_back(vfp::DbfFieldDescriptor{
                        .name = name,
                        .type = type,
                        .length = static_cast<std::uint8_t>(type == 'N' || type == 'F' ? 18U : 32U),
                        .decimal_count = 0U});
                };

                if (!cursor.remote_records.empty())
                {
                    fields.reserve(cursor.remote_records.front().values.size());
                    for (const auto &value : cursor.remote_records.front().values)
                    {
                        const char type = value.field_type == '\0' ? 'C' : value.field_type;
                        add_remote_field(value.field_name, type);
                    }
                    return fields;
                }
                add_remote_field("ID", 'N');
                add_remote_field("NAME", 'C');
                add_remote_field("AMOUNT", 'N');
                return fields;
            }

            if (cursor.source_path.empty())
            {
                return fields;
            }
            const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, std::max<std::size_t>(cursor.record_count, 1U));
            if (!table_result.ok)
            {
                last_error_message = table_result.error;
                return {};
            }

            return table_result.table.fields;
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
            const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, cursor.recno);
            if (!table_result.ok)
            {
                last_error_message = table_result.error;
                return std::nullopt;
            }
            if (cursor.recno > table_result.table.records.size())
            {
                return std::nullopt;
            }
            const auto &record = table_result.table.records[cursor.recno - 1U];
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
                    last_error_message = "NOT NULL field not found: " + field_name;
                    return false;
                }
                const std::string normalized_value = lowercase_copy(trim_copy(*value));
                if (normalized_value.empty() || normalized_value == "null")
                {
                    last_error_message = "NOT NULL constraint failed for field: " + field_name;
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
                    last_error_message = "INSERT INTO could not resolve target field names";
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
                last_error_message = "INSERT INTO requires at least one target field";
                return false;
            }
            if (values.size() != fields.size())
            {
                last_error_message = "INSERT INTO field/value counts do not match";
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

            const bool defaults_ok = default_assignments.empty() || replace_current_record_fields(cursor, default_assignments, frame);
            const bool explicit_ok = defaults_ok && replace_current_record_fields(cursor, assignments, frame);
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
                last_error_message = "APPEND BLANK requires a local table-backed cursor";
                return false;
            }
            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            const auto result = vfp::append_blank_record_to_file(cursor.source_path);
            if (!result.ok)
            {
                last_error_message = result.error;
                return false;
            }

            cursor.record_count = result.record_count;
            move_cursor_to(cursor, static_cast<long long>(result.record_count));
            cursor.found = false;
            return true;
        }

        bool set_deleted_flag(
            CursorState &cursor,
            const Frame &frame,
            const std::string &for_expression,
            const std::string &while_expression,
            bool deleted)
        {
            if (cursor.remote)
            {
                std::vector<std::size_t> target_records;
                if (for_expression.empty() && while_expression.empty())
                {
                    if (cursor.recno == 0U || cursor.eof || cursor.recno > cursor.remote_records.size())
                    {
                        last_error_message = "This command requires a current remote record";
                        return false;
                    }
                    target_records.push_back(cursor.recno);
                }
                else
                {
                    const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
                    for (std::size_t index = 0; index < cursor.remote_records.size(); ++index)
                    {
                        move_cursor_to(cursor, static_cast<long long>(index + 1U));
                        if (!while_expression.empty() && !value_as_bool(evaluate_expression(while_expression, frame, &cursor)))
                        {
                            break;
                        }
                        if (current_record_matches_visibility(cursor, frame, for_expression))
                        {
                            target_records.push_back(index + 1U);
                        }
                    }
                    restore_cursor_snapshot(cursor, original);
                }

                for (const std::size_t recno : target_records)
                {
                    cursor.remote_records[recno - 1U].deleted = deleted;
                }
                return true;
            }
            if (cursor.source_path.empty())
            {
                last_error_message = "This command requires a local table-backed cursor";
                return false;
            }
            if (!ensure_transaction_backup_for_table(cursor.source_path))
            {
                return false;
            }

            std::vector<std::size_t> target_records;
            if (for_expression.empty() && while_expression.empty())
            {
                if (cursor.recno == 0U || cursor.eof)
                {
                    last_error_message = "This command requires a current local record";
                    return false;
                }
                target_records.push_back(cursor.recno);
            }
            else
            {
                const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, cursor.record_count);
                if (!table_result.ok)
                {
                    last_error_message = table_result.error;
                    return false;
                }

                const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
                for (const auto &record : table_result.table.records)
                {
                    move_cursor_to(cursor, static_cast<long long>(record.record_index + 1U));
                    if (!while_expression.empty() && !value_as_bool(evaluate_expression(while_expression, frame, &cursor)))
                    {
                        break;
                    }
                    if (current_record_matches_visibility(cursor, frame, for_expression))
                    {
                        target_records.push_back(record.record_index + 1U);
                    }
                }
                restore_cursor_snapshot(cursor, original);
            }

            for (const std::size_t recno : target_records)
            {
                const auto result = vfp::set_record_deleted_flag(cursor.source_path, recno - 1U, deleted);
                if (!result.ok)
                {
                    last_error_message = result.error;
                    return false;
                }
                cursor.record_count = result.record_count;
            }

            return true;
        }

        bool ensure_exclusive_table_maintenance(const CursorState &cursor, const std::string &command_name)
        {
            if (cursor.remote || cursor.exclusive)
            {
                return true;
            }

            last_error_message = command_name + " requires exclusive use of the target cursor";
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
                last_error_message = "PACK requires a local table-backed cursor";
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
                last_error_message = "ZAP requires a local table-backed cursor";
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

            DataSessionState &session = current_session_state();
            const int area = cursor->work_area;
            const std::string normalized_function = normalize_identifier(function);
            if (normalized_function == "flock" || normalized_function == "lock")
            {
                session.table_locks.insert(area);
                events.push_back({.category = "runtime.lock",
                                  .detail = cursor->alias.empty() ? std::to_string(area) : cursor->alias + " FLOCK",
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                return make_boolean_value(true);
            }
            if (normalized_function == "isflocked")
            {
                return make_boolean_value(session.table_locks.contains(area));
            }

            std::size_t recno = cursor->recno;
            if (!arguments.empty() && arguments.front().kind != PrgValueKind::string)
            {
                recno = static_cast<std::size_t>(std::max<double>(0.0, std::llround(value_as_number(arguments.front()))));
            }
            if (recno == 0U || recno > cursor->record_count)
            {
                return make_boolean_value(false);
            }

            if (normalized_function == "rlock")
            {
                session.record_locks[area].insert(recno);
                events.push_back({.category = "runtime.lock",
                                  .detail = (cursor->alias.empty() ? std::to_string(area) : cursor->alias) + " RLOCK " + std::to_string(recno),
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                return make_boolean_value(true);
            }
            if (normalized_function == "isrlocked")
            {
                const auto found = session.record_locks.find(area);
                return make_boolean_value(found != session.record_locks.end() && found->second.contains(recno));
            }

            return make_empty_value();
        }

        void unlock_cursor_locks(CursorState *cursor, bool all_locks)
        {
            DataSessionState &session = current_session_state();
            if (all_locks || cursor == nullptr)
            {
                session.table_locks.clear();
                session.record_locks.clear();
                return;
            }

            session.table_locks.erase(cursor->work_area);
            session.record_locks.erase(cursor->work_area);
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
