// prg_engine_cursor.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        int next_available_work_area() const
        {
            return std::max(1, current_session_state().next_work_area);
        }

        int allocate_work_area()
        {
            DataSessionState &session = current_session_state();
            const int allocated = next_available_work_area();
            session.next_work_area = allocated + 1;
            return allocated;
        }

        int reserve_work_area(int requested_area)
        {
            DataSessionState &session = current_session_state();
            if (requested_area <= 0)
            {
                return allocate_work_area();
            }
            if (requested_area >= session.next_work_area)
            {
                session.next_work_area = requested_area + 1;
            }
            return requested_area;
        }

        int select_work_area(int requested_area)
        {
            DataSessionState &session = current_session_state();
            requested_area = reserve_work_area(requested_area);
            session.selected_work_area = requested_area;
            return session.selected_work_area;
        }

        CursorState *find_cursor_by_area(int area)
        {
            auto &session = current_session_state();
            const auto found = session.cursors.find(area);
            return found == session.cursors.end() ? nullptr : &found->second;
        }

        const CursorState *find_cursor_by_area(int area) const
        {
            const auto &session = current_session_state();
            const auto found = session.cursors.find(area);
            return found == session.cursors.end() ? nullptr : &found->second;
        }

        CursorState *find_cursor_by_alias(const std::string &alias)
        {
            const std::string normalized = normalize_identifier(alias);
            if (normalized.empty())
            {
                return find_cursor_by_area(current_selected_work_area());
            }

            auto &session = current_session_state();
            const auto found = std::find_if(session.aliases.begin(), session.aliases.end(), [&](const auto &pair)
                                            { return normalize_identifier(pair.second) == normalized; });
            if (found == session.aliases.end())
            {
                return nullptr;
            }
            return find_cursor_by_area(found->first);
        }

        const CursorState *find_cursor_by_alias(const std::string &alias) const
        {
            const std::string normalized = normalize_identifier(alias);
            if (normalized.empty())
            {
                return find_cursor_by_area(current_selected_work_area());
            }

            const auto &session = current_session_state();
            const auto found = std::find_if(session.aliases.begin(), session.aliases.end(), [&](const auto &pair)
                                            { return normalize_identifier(pair.second) == normalized; });
            if (found == session.aliases.end())
            {
                return nullptr;
            }
            return find_cursor_by_area(found->first);
        }

        CursorState *resolve_cursor_target(const std::string &designator)
        {
            const std::string trimmed = trim_copy(designator);
            if (trimmed.empty())
            {
                return find_cursor_by_area(current_selected_work_area());
            }

            const std::string normalized_designator =
                trimmed.size() >= 2U && trimmed.front() == '\'' && trimmed.back() == '\''
                    ? unquote_string(trimmed)
                    : trimmed;

            const bool numeric_selection = std::all_of(normalized_designator.begin(), normalized_designator.end(), [](unsigned char ch)
                                                       { return std::isdigit(ch) != 0; });
            if (numeric_selection)
            {
                return find_cursor_by_area(std::stoi(normalized_designator));
            }
            return find_cursor_by_alias(normalized_designator);
        }

        const CursorState *resolve_cursor_target(const std::string &designator) const
        {
            const std::string trimmed = trim_copy(designator);
            if (trimmed.empty())
            {
                return find_cursor_by_area(current_selected_work_area());
            }

            const std::string normalized_designator =
                trimmed.size() >= 2U && trimmed.front() == '\'' && trimmed.back() == '\''
                    ? unquote_string(trimmed)
                    : trimmed;

            const bool numeric_selection = std::all_of(normalized_designator.begin(), normalized_designator.end(), [](unsigned char ch)
                                                       { return std::isdigit(ch) != 0; });
            if (numeric_selection)
            {
                return find_cursor_by_area(std::stoi(normalized_designator));
            }
            return find_cursor_by_alias(normalized_designator);
        }

        void close_cursor(const std::string &designator)
        {
            DataSessionState &session = current_session_state();
            CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr)
            {
                return;
            }
            const int closed_work_area = cursor->work_area;
            if (session.selected_work_area == cursor->work_area)
            {
                session.selected_work_area = cursor->work_area;
            }
            session.aliases.erase(cursor->work_area);
            session.table_locks.erase(cursor->work_area);
            session.record_locks.erase(cursor->work_area);
            session.cursors.erase(cursor->work_area);
            session.next_work_area = std::min(session.next_work_area, closed_work_area);
        }

        std::vector<CursorState::OrderState> load_cursor_orders(const std::string &table_path) const
        {
            std::vector<CursorState::OrderState> orders;
            const auto inspection = vfp::inspect_asset(table_path);
            if (!inspection.ok)
            {
                return orders;
            }

            for (const auto &index_asset : inspection.indexes)
            {
                if (!index_asset.probe.tags.empty())
                {
                    for (const auto &tag : index_asset.probe.tags)
                    {
                        if (tag.key_expression_hint.empty())
                        {
                            continue;
                        }
                        orders.push_back({.name = tag.name_hint.empty() ? collapse_identifier(tag.key_expression_hint) : tag.name_hint,
                                          .expression = tag.key_expression_hint,
                                          .for_expression = tag.for_expression_hint,
                                          .index_path = normalize_path(index_asset.path),
                                          .normalization_hint = tag.normalization_hint,
                                          .collation_hint = tag.collation_hint,
                                          .key_domain_hint = index_asset.probe.key_domain_hint,
                                          .descending = false});
                    }
                    continue;
                }

                if (!index_asset.probe.key_expression_hint.empty())
                {
                    const std::string fallback_name = std::filesystem::path(index_asset.path).stem().string();
                    orders.push_back({.name = fallback_name.empty() ? collapse_identifier(index_asset.probe.key_expression_hint) : fallback_name,
                                      .expression = index_asset.probe.key_expression_hint,
                                      .for_expression = index_asset.probe.for_expression_hint,
                                      .index_path = normalize_path(index_asset.path),
                                      .normalization_hint = index_asset.probe.normalization_hint,
                                      .collation_hint = index_asset.probe.collation_hint,
                                      .key_domain_hint = index_asset.probe.key_domain_hint,
                                      .descending = false});
                }
            }

            return orders;
        }

        std::string format_order_metadata_detail(
            const std::string &order_name,
            const std::string &normalization_hint,
            const std::string &collation_hint,
            bool descending = false) const
        {
            std::string detail = order_name.empty() ? "0" : order_name;
            if (!normalization_hint.empty() || !collation_hint.empty() || descending)
            {
                detail += " [";
                bool needs_separator = false;
                if (!normalization_hint.empty())
                {
                    detail += "norm=" + normalization_hint;
                    needs_separator = true;
                }
                if (!collation_hint.empty())
                {
                    if (needs_separator)
                    {
                        detail += ", ";
                    }
                    detail += "coll=" + collation_hint;
                    needs_separator = true;
                }
                if (descending)
                {
                    if (needs_separator)
                    {
                        detail += ", ";
                    }
                    detail += "dir=descending";
                }
                detail += "]";
            }
            return detail;
        }

        std::optional<bool> parse_order_direction_override(const std::string &text) const
        {
            const std::string upper = uppercase_copy(trim_copy(text));
            if (upper == "DESCENDING")
            {
                return true;
            }
            if (upper == "ASCENDING")
            {
                return false;
            }
            return std::nullopt;
        }

        struct SeekFunctionOrderDesignator
        {
            std::string order_designator;
            std::optional<bool> descending_override;
        };

        SeekFunctionOrderDesignator parse_seek_function_order_designator(const std::string &raw_designator) const
        {
            SeekFunctionOrderDesignator parsed{
                .order_designator = trim_copy(raw_designator),
                .descending_override = std::nullopt};
            if (parsed.order_designator.empty())
            {
                return parsed;
            }

            const std::string upper = uppercase_copy(parsed.order_designator);
            constexpr const char *descending_suffix = " DESCENDING";
            constexpr const char *ascending_suffix = " ASCENDING";

            if (upper.size() > std::char_traits<char>::length(descending_suffix) &&
                upper.rfind(descending_suffix) == upper.size() - std::char_traits<char>::length(descending_suffix))
            {
                parsed.order_designator = trim_copy(parsed.order_designator.substr(
                    0U,
                    parsed.order_designator.size() - std::char_traits<char>::length(descending_suffix)));
                parsed.descending_override = true;
                return parsed;
            }

            if (upper.size() > std::char_traits<char>::length(ascending_suffix) &&
                upper.rfind(ascending_suffix) == upper.size() - std::char_traits<char>::length(ascending_suffix))
            {
                parsed.order_designator = trim_copy(parsed.order_designator.substr(
                    0U,
                    parsed.order_designator.size() - std::char_traits<char>::length(ascending_suffix)));
                parsed.descending_override = false;
            }

            return parsed;
        }

        int compare_order_keys(
            const std::string &left,
            const std::string &right,
            const std::string &key_domain_hint,
            bool descending) const
        {
            const int comparison = compare_index_keys(left, right, key_domain_hint);
            return descending ? -comparison : comparison;
        }

        bool order_normalization_hint_contains(const std::string &hints, const std::string &token) const
        {
            std::stringstream stream(hints);
            std::string part;
            while (std::getline(stream, part, ','))
            {
                if (lowercase_copy(trim_copy(part)) == token)
                {
                    return true;
                }
            }
            return false;
        }

        bool order_for_expression_matches_record(const std::string &for_expression, const vfp::DbfRecord &record) const
        {
            const std::string trimmed = trim_copy(for_expression);
            std::string canonical = uppercase_copy(trim_copy(trimmed));
            canonical.erase(
                std::remove_if(canonical.begin(), canonical.end(), [](unsigned char ch)
                               { return std::isspace(ch) != 0; }),
                canonical.end());
            if (canonical.empty())
            {
                return true;
            }
            if (canonical == "DELETED()=.F.")
            {
                return !record.deleted;
            }
            if (canonical == "DELETED()=.T.")
            {
                return record.deleted;
            }

            const auto split_top_level_comparison = [](const std::string &expression)
                -> std::optional<std::tuple<std::string, std::string, std::string>>
            {
                static constexpr std::array<const char *, 8U> operators = {
                    "==",
                    "<>",
                    "!=",
                    ">=",
                    "<=",
                    "=",
                    ">",
                    "<"};
                int depth = 0;
                char quote = '\0';
                for (std::size_t index = 0; index < expression.size(); ++index)
                {
                    const char ch = expression[index];
                    if (quote != '\0')
                    {
                        if (ch == quote)
                        {
                            quote = '\0';
                        }
                        continue;
                    }
                    if (ch == '\'' || ch == '"')
                    {
                        quote = ch;
                        continue;
                    }
                    if (ch == '(')
                    {
                        ++depth;
                        continue;
                    }
                    if (ch == ')' && depth > 0)
                    {
                        --depth;
                        continue;
                    }
                    if (depth != 0)
                    {
                        continue;
                    }

                    for (const char *operator_text : operators)
                    {
                        const std::size_t length = std::char_traits<char>::length(operator_text);
                        if (index + length > expression.size())
                        {
                            continue;
                        }
                        if (expression.compare(index, length, operator_text) == 0)
                        {
                            return std::make_tuple(
                                trim_copy(expression.substr(0U, index)),
                                std::string(operator_text),
                                trim_copy(expression.substr(index + length)));
                        }
                    }
                }
                return std::nullopt;
            };

            const auto parse_boolean_literal = [](const std::string &expression) -> std::optional<bool>
            {
                const std::string upper = uppercase_copy(trim_copy(expression));
                if (upper == ".T." || upper == ".TRUE.")
                {
                    return true;
                }
                if (upper == ".F." || upper == ".FALSE.")
                {
                    return false;
                }
                return std::nullopt;
            };

            const auto evaluate_filter_operand = [&](const std::string &expression)
            {
                struct EvaluatedOperand
                {
                    std::string text;
                    std::optional<double> numeric;
                    std::optional<bool> boolean;
                };

                EvaluatedOperand operand{};
                const std::string upper = uppercase_copy(trim_copy(expression));
                if (upper == "DELETED()")
                {
                    operand.boolean = record.deleted;
                    operand.text = record.deleted ? ".T." : ".F.";
                    return operand;
                }

                operand.text = evaluate_index_expression(expression, record);
                operand.numeric = try_parse_numeric_index_value(operand.text);
                operand.boolean = parse_boolean_literal(expression);
                if (!operand.boolean.has_value())
                {
                    operand.boolean = parse_boolean_literal(operand.text);
                }
                return operand;
            };

            const auto comparison = split_top_level_comparison(trimmed);
            if (!comparison.has_value())
            {
                return true;
            }

            const auto &[left_expression, operator_text, right_expression] = *comparison;
            if (left_expression.empty() || right_expression.empty())
            {
                return true;
            }

            const auto left = evaluate_filter_operand(left_expression);
            const auto right = evaluate_filter_operand(right_expression);

            int relation = 0;
            if (left.boolean.has_value() && right.boolean.has_value())
            {
                relation = left.boolean == right.boolean ? 0 : (*left.boolean ? 1 : -1);
            }
            else if (left.numeric.has_value() && right.numeric.has_value())
            {
                if (*left.numeric < *right.numeric)
                {
                    relation = -1;
                }
                else if (*left.numeric > *right.numeric)
                {
                    relation = 1;
                }
            }
            else
            {
                relation = compare_index_keys(left.text, right.text, "");
            }

            if (operator_text == "=" || operator_text == "==")
            {
                return relation == 0;
            }
            if (operator_text == "<>" || operator_text == "!=")
            {
                return relation != 0;
            }
            if (operator_text == ">")
            {
                return relation > 0;
            }
            if (operator_text == ">=")
            {
                return relation >= 0;
            }
            if (operator_text == "<")
            {
                return relation < 0;
            }
            if (operator_text == "<=")
            {
                return relation <= 0;
            }
            return true;
        }

        std::string normalize_seek_key_for_order(std::string value, const std::string &normalization_hint) const
        {
            if (order_normalization_hint_contains(normalization_hint, "alltrim"))
            {
                value = trim_copy(std::move(value));
            }
            else
            {
                if (order_normalization_hint_contains(normalization_hint, "ltrim"))
                {
                    value.erase(
                        value.begin(),
                        std::find_if(value.begin(), value.end(), [](unsigned char ch)
                                     { return std::isspace(ch) == 0; }));
                }
                if (order_normalization_hint_contains(normalization_hint, "rtrim"))
                {
                    value.erase(
                        std::find_if(value.rbegin(), value.rend(), [](unsigned char ch)
                                     { return std::isspace(ch) == 0; })
                            .base(),
                        value.end());
                }
            }

            if (order_normalization_hint_contains(normalization_hint, "upper"))
            {
                value = uppercase_copy(std::move(value));
            }
            else if (order_normalization_hint_contains(normalization_hint, "lower"))
            {
                value = lowercase_copy(std::move(value));
            }

            return value;
        }

        std::string normalize_seek_key_for_collation(
            std::string value,
            const std::string &collation_hint,
            const std::string &key_domain_hint) const
        {
            if (key_domain_hint == "numeric_or_date")
            {
                return value;
            }

            const std::string normalized_collation_hint = lowercase_copy(trim_copy(collation_hint));
            if (normalized_collation_hint == "case-folded")
            {
                return uppercase_copy(std::move(value));
            }

            const auto found_collate = current_set_state().find("collate");
            const std::string session_collate = uppercase_copy(trim_copy(
                found_collate == current_set_state().end() ? std::string{"MACHINE"} : found_collate->second));
            if (!session_collate.empty() && session_collate != "MACHINE")
            {
                return uppercase_copy(std::move(value));
            }

            return value;
        }

        std::string derive_order_normalization_hint(const std::string &expression) const
        {
            const std::string upper = uppercase_copy(trim_copy(expression));
            std::vector<std::string> hints;
            const auto append_hint = [&](const std::string &hint)
            {
                if (std::find(hints.begin(), hints.end(), hint) == hints.end())
                {
                    hints.push_back(hint);
                }
            };

            if (upper.find("UPPER(") != std::string::npos)
            {
                append_hint("upper");
            }
            if (upper.find("LOWER(") != std::string::npos)
            {
                append_hint("lower");
            }
            if (upper.find("ALLTRIM(") != std::string::npos)
            {
                append_hint("alltrim");
            }
            if (upper.find("LTRIM(") != std::string::npos)
            {
                append_hint("ltrim");
            }
            if (upper.find("RTRIM(") != std::string::npos)
            {
                append_hint("rtrim");
            }

            std::string joined;
            for (std::size_t index = 0; index < hints.size(); ++index)
            {
                if (index != 0U)
                {
                    joined += ",";
                }
                joined += hints[index];
            }
            return joined;
        }

        std::string derive_order_collation_hint(
            const std::string &expression,
            const std::string &normalization_hint) const
        {
            const std::string upper = uppercase_copy(trim_copy(expression));
            if (upper.find("UPPER(") != std::string::npos || upper.find("LOWER(") != std::string::npos)
            {
                return "case-folded";
            }
            if (upper.find("CHRTRAN(") != std::string::npos ||
                upper.find("STRTRAN(") != std::string::npos ||
                !normalization_hint.empty())
            {
                return "expression-normalized";
            }
            return {};
        }

        struct TemporaryOrderExpressionState
        {
            std::string expression;
            std::string for_expression;
        };

        TemporaryOrderExpressionState parse_temporary_order_expression_state(const std::string &raw_expression) const
        {
            TemporaryOrderExpressionState parsed{
                .expression = trim_copy(raw_expression),
                .for_expression = {}};
            if (parsed.expression.empty())
            {
                return parsed;
            }

            const std::size_t for_position = find_keyword_top_level(parsed.expression, "FOR");
            if (for_position == std::string::npos)
            {
                return parsed;
            }

            const std::string order_expression = trim_copy(parsed.expression.substr(0U, for_position));
            const std::string for_expression = trim_copy(parsed.expression.substr(for_position + 3U));
            if (order_expression.empty() || for_expression.empty())
            {
                return parsed;
            }

            parsed.expression = order_expression;
            parsed.for_expression = for_expression;
            return parsed;
        }

        std::string derive_order_key_domain_hint(const CursorState &cursor, const std::string &expression) const
        {
            const std::string normalized_expression = collapse_identifier(unquote_identifier(trim_copy(expression)));
            if (normalized_expression.empty())
            {
                return {};
            }

            std::vector<vfp::DbfFieldDescriptor> descriptors;
            if (cursor.remote)
            {
                descriptors = cursor.remote_fields;
                if (descriptors.empty() && !cursor.remote_records.empty())
                {
                    descriptors.reserve(cursor.remote_records.front().values.size());
                    for (const auto &value : cursor.remote_records.front().values)
                    {
                        descriptors.push_back(vfp::DbfFieldDescriptor{
                            .name = value.field_name,
                            .type = value.field_type,
                            .length = 0U,
                            .decimal_count = 0U});
                    }
                }
            }
            else if (!cursor.source_path.empty())
            {
                const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, cursor.record_count);
                if (table_result.ok)
                {
                    descriptors = table_result.table.fields;
                }
            }

            for (const auto &descriptor : descriptors)
            {
                if (collapse_identifier(descriptor.name) != normalized_expression)
                {
                    continue;
                }

                switch (std::toupper(static_cast<unsigned char>(descriptor.type)))
                {
                case 'N':
                case 'F':
                case 'B':
                case 'I':
                case 'Y':
                case 'D':
                case 'T':
                    return "numeric_or_date";
                default:
                    return {};
                }
            }

            return {};
        }

        bool can_open_table_cursor(
            const std::string &resolved_path,
            const std::string &alias,
            bool remote,
            bool allow_again,
            int target_area)
        {
            DataSessionState &session = current_session_state();
            const std::string normalized_alias = normalize_identifier(alias);
            const std::string normalized_path = normalize_path(resolved_path);

            for (const auto &[work_area, cursor] : session.cursors)
            {
                if (work_area == target_area)
                {
                    continue;
                }

                if (!normalized_alias.empty() && normalize_identifier(cursor.alias) == normalized_alias)
                {
                    last_error_message = "Alias already open in this data session: " + alias;
                    return false;
                }

                if (!remote && !allow_again && !normalized_path.empty() &&
                    normalize_path(cursor.source_path) == normalized_path)
                {
                    last_error_message = "Table already open in this data session; USE AGAIN is required: " + resolved_path;
                    return false;
                }
            }

            return true;
        }

        std::optional<int> resolve_use_target_work_area(const std::string &in_expression, const Frame &frame)
        {
            const std::string trimmed_expression = trim_copy(in_expression);
            if (trimmed_expression.empty())
            {
                return current_selected_work_area();
            }

            const std::string area_text = evaluate_cursor_designator_expression(trimmed_expression, frame);
            if (area_text.empty())
            {
                return 0;
            }

            const bool numeric_selection = std::all_of(area_text.begin(), area_text.end(), [](unsigned char ch)
                                                       { return std::isdigit(ch) != 0; });
            if (numeric_selection)
            {
                return std::stoi(area_text);
            }

            CursorState *existing = find_cursor_by_alias(area_text);
            if (existing == nullptr)
            {
                last_error_message = "USE target work area not found: " + area_text;
                return std::nullopt;
            }

            return existing->work_area;
        }

        std::string resolve_sql_cursor_auto_target()
        {
            return find_cursor_by_area(current_selected_work_area()) == nullptr ? std::string{} : "0";
        }

        bool is_set_enabled(const std::string &option_name) const
        {
            const auto session_found = set_state_by_session.find(current_data_session);
            if (session_found == set_state_by_session.end())
            {
                return false;
            }

            const auto found = session_found->second.find(normalize_identifier(option_name));
            if (found == session_found->second.end())
            {
                return false;
            }

            const std::string normalized_value = normalize_identifier(found->second);
                     return normalized_value != "off" && normalized_value != "false" && normalized_value != "0" &&
                         normalized_value != ".f." && normalized_value != "no" && normalized_value != "n";
        }

        bool is_set_enabled_or_default(const std::string &option_name, bool default_value) const
        {
            const auto session_found = set_state_by_session.find(current_data_session);
            if (session_found == set_state_by_session.end())
            {
                return default_value;
            }

            const auto found = session_found->second.find(normalize_identifier(option_name));
            if (found == session_found->second.end())
            {
                return default_value;
            }

            const std::string normalized_value = normalize_identifier(found->second);
            return normalized_value != "off" && normalized_value != "false" && normalized_value != "0" &&
                   normalized_value != ".f." && normalized_value != "no" && normalized_value != "n";
        }

        std::map<std::string, std::string> &current_set_state()
        {
            auto [iterator, _] = set_state_by_session.try_emplace(current_data_session);
            return iterator->second;
        }

        const std::map<std::string, std::string> &current_set_state() const
        {
            const auto found = set_state_by_session.find(current_data_session);
            if (found != set_state_by_session.end())
            {
                return found->second;
            }

            static const std::map<std::string, std::string> empty_state;
            return empty_state;
        }

        void move_cursor_to(CursorState &cursor, long long target_recno)
        {
            if (cursor.record_count == 0U)
            {
                cursor.recno = 0U;
                cursor.bof = true;
                cursor.eof = true;
                return;
            }

            if (target_recno <= 0)
            {
                cursor.recno = 0U;
                cursor.bof = true;
                cursor.eof = false;
                return;
            }

            const auto record_count = static_cast<long long>(cursor.record_count);
            if (target_recno > record_count)
            {
                cursor.recno = static_cast<std::size_t>(record_count + 1);
                cursor.bof = false;
                cursor.eof = true;
                return;
            }

            cursor.recno = static_cast<std::size_t>(target_recno);
            cursor.bof = false;
            cursor.eof = false;
        }

        bool activate_order(
            CursorState &cursor,
            const std::string &order_designator,
            std::optional<bool> descending_override = std::nullopt)
        {
            const std::string trimmed = trim_copy(order_designator);
            if (trimmed.empty() || trimmed == "0")
            {
                cursor.active_order_name.clear();
                cursor.active_order_expression.clear();
                cursor.active_order_for_expression.clear();
                cursor.active_order_path.clear();
                cursor.active_order_normalization_hint.clear();
                cursor.active_order_collation_hint.clear();
                cursor.active_order_key_domain_hint.clear();
                cursor.active_order_descending = false;
                return true;
            }

            std::string target_name = trimmed;
            if (starts_with_insensitive(target_name, "TAG "))
            {
                target_name = trim_copy(target_name.substr(4U));
            }
            target_name = unquote_identifier(target_name);

            const bool numeric_selection = !target_name.empty() &&
                                           std::all_of(target_name.begin(), target_name.end(), [](unsigned char ch)
                                                       { return std::isdigit(ch) != 0; });
            if (numeric_selection)
            {
                const std::size_t index = static_cast<std::size_t>(std::max(1, std::stoi(target_name))) - 1U;
                if (index >= cursor.orders.size())
                {
                    last_error_message = "Requested order does not exist";
                    return false;
                }

                cursor.active_order_name = cursor.orders[index].name;
                cursor.active_order_expression = cursor.orders[index].expression;
                cursor.active_order_for_expression = cursor.orders[index].for_expression;
                cursor.active_order_path = cursor.orders[index].index_path;
                cursor.active_order_normalization_hint = cursor.orders[index].normalization_hint;
                cursor.active_order_collation_hint = cursor.orders[index].collation_hint;
                cursor.active_order_key_domain_hint = cursor.orders[index].key_domain_hint;
                cursor.active_order_descending = descending_override.value_or(cursor.orders[index].descending);
                return true;
            }

            const std::string normalized_target = collapse_identifier(target_name);
            const auto found = std::find_if(cursor.orders.begin(), cursor.orders.end(), [&](const CursorState::OrderState &order)
                                            { return collapse_identifier(order.name) == normalized_target; });
            if (found == cursor.orders.end())
            {
                const TemporaryOrderExpressionState parsed = parse_temporary_order_expression_state(target_name);
                const std::string normalization_hint = derive_order_normalization_hint(parsed.expression);
                cursor.active_order_name = uppercase_copy(parsed.expression);
                cursor.active_order_expression = parsed.expression;
                cursor.active_order_for_expression = parsed.for_expression;
                cursor.active_order_path.clear();
                cursor.active_order_normalization_hint = normalization_hint;
                cursor.active_order_collation_hint = derive_order_collation_hint(parsed.expression, normalization_hint);
                cursor.active_order_key_domain_hint = derive_order_key_domain_hint(cursor, parsed.expression);
                cursor.active_order_descending = descending_override.value_or(false);
                return true;
            }

            cursor.active_order_name = found->name;
            cursor.active_order_expression = found->expression;
            cursor.active_order_for_expression = found->for_expression;
            cursor.active_order_path = found->index_path;
            cursor.active_order_normalization_hint = found->normalization_hint;
            cursor.active_order_collation_hint = found->collation_hint;
            cursor.active_order_key_domain_hint = found->key_domain_hint;
            cursor.active_order_descending = descending_override.value_or(found->descending);
            return true;
        }

        bool seek_in_cursor(CursorState &cursor, const std::string &search_key)
        {
            cursor.found = false;
            if (!cursor.remote && cursor.source_path.empty())
            {
                last_error_message = "SEEK requires a local table-backed cursor";
                return false;
            }

            if (cursor.active_order_expression.empty())
            {
                if (!cursor.orders.empty())
                {
                    cursor.active_order_name = cursor.orders.front().name;
                    cursor.active_order_expression = cursor.orders.front().expression;
                    cursor.active_order_for_expression = cursor.orders.front().for_expression;
                    cursor.active_order_path = cursor.orders.front().index_path;
                    cursor.active_order_normalization_hint = cursor.orders.front().normalization_hint;
                    cursor.active_order_collation_hint = cursor.orders.front().collation_hint;
                    cursor.active_order_key_domain_hint = cursor.orders.front().key_domain_hint;
                    cursor.active_order_descending = cursor.orders.front().descending;
                }
                else
                {
                    last_error_message = "SEEK requires an active order";
                    return false;
                }
            }

            const std::string normalized_target = normalize_seek_key_for_collation(
                normalize_seek_key_for_order(search_key, cursor.active_order_normalization_hint),
                cursor.active_order_collation_hint,
                cursor.active_order_key_domain_hint);
            std::vector<IndexedCandidate> candidates;
            if (cursor.remote)
            {
                candidates.reserve(cursor.remote_records.size());
                for (const auto &record : cursor.remote_records)
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
            }
            else
            {
                const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, cursor.record_count);
                if (!table_result.ok)
                {
                    last_error_message = table_result.error;
                    return false;
                }

                candidates.reserve(table_result.table.records.size());
                for (const auto &record : table_result.table.records)
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
            }

            std::sort(candidates.begin(), candidates.end(), [&](const IndexedCandidate &left, const IndexedCandidate &right)
                      {
            const int comparison = compare_order_keys(
                left.key,
                right.key,
                cursor.active_order_key_domain_hint,
                cursor.active_order_descending);
            if (comparison != 0) {
                return comparison < 0;
            }
            return left.recno < right.recno; });

            const auto lower = std::lower_bound(
                candidates.begin(),
                candidates.end(),
                normalized_target,
                [&](const IndexedCandidate &candidate, const std::string &value)
                {
                    return compare_order_keys(
                               candidate.key,
                               value,
                               cursor.active_order_key_domain_hint,
                               cursor.active_order_descending) < 0;
                });

            const bool exact_match_required = is_set_enabled("exact");
            const auto is_match = [&](const std::string &candidate)
            {
                if (exact_match_required)
                {
                    return candidate == normalized_target;
                }
                return candidate.rfind(normalized_target, 0U) == 0U;
            };

            if (lower != candidates.end() && is_match(lower->key))
            {
                move_cursor_to(cursor, static_cast<long long>(lower->recno));
                cursor.found = true;
                return true;
            }

            if (is_set_enabled("near") && lower != candidates.end())
            {
                move_cursor_to(cursor, static_cast<long long>(lower->recno));
                cursor.found = false;
                return false;
            }

            move_cursor_to(cursor, static_cast<long long>(cursor.record_count + 1U));
            return false;
        }

        CursorPositionSnapshot capture_cursor_snapshot(const CursorState &cursor) const
        {
            return {
                .recno = cursor.recno,
                .found = cursor.found,
                .bof = cursor.bof,
                .eof = cursor.eof,
                .active_order_name = cursor.active_order_name,
                .active_order_expression = cursor.active_order_expression,
                .active_order_for_expression = cursor.active_order_for_expression,
                .active_order_path = cursor.active_order_path,
                .active_order_normalization_hint = cursor.active_order_normalization_hint,
                .active_order_collation_hint = cursor.active_order_collation_hint,
                .active_order_key_domain_hint = cursor.active_order_key_domain_hint,
                .active_order_descending = cursor.active_order_descending};
        }
