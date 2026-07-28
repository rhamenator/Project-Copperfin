// prg_engine_expression.inl
// ExpressionParser: inline expression evaluation helper.
// This file is #included directly into prg_engine.cpp inside namespace copperfin::runtime.
// It must not be compiled separately.

    std::optional<PrgValue> evaluate_date_time_function(
        const std::string& function,
        const std::vector<PrgValue>& arguments,
        const std::function<std::string(const std::string&)>& set_callback);
    std::optional<PrgValue> evaluate_date_time_additive(
        const PrgValue& left,
        const PrgValue& right,
        bool subtract,
        const std::function<std::string(const std::string&)>& set_callback);
    std::optional<int> compare_date_time_values(
        const PrgValue& left,
        const PrgValue& right,
        const std::function<std::string(const std::string&)>& set_callback);
    std::optional<PrgValue> evaluate_string_function(
        const std::string& function,
        const std::vector<PrgValue>& arguments,
        bool exact_string_compare,
        std::size_t memo_width,
        const std::function<std::string(const std::string&)>& set_callback);
    std::optional<PrgValue> evaluate_type_function(
        const std::string& function,
        const std::vector<PrgValue>& arguments,
        const std::function<bool(const std::string&)>& array_exists_callback,
        const std::function<PrgValue(const std::string&)>& eval_expression_callback);
    std::optional<PrgValue> evaluate_numeric_function(
        const std::string& function,
        const std::vector<PrgValue>& arguments);
    std::optional<PrgValue> evaluate_path_function(
        const std::string& function,
        const std::vector<PrgValue>& arguments,
        const std::string& default_directory);
    std::optional<PrgValue> evaluate_runtime_surface_function(
        const std::string& function,
        const std::vector<PrgValue>& arguments,
        const std::vector<std::string>& raw_arguments,
        const std::string& default_directory,
        const std::string& frame_file_path,
        const std::string& last_error_message,
        int last_error_code,
        const std::string& last_error_procedure,
        std::size_t last_error_line,
        const std::string& current_program_name,
        std::size_t program_stack_depth,
        const std::function<std::optional<RuntimeProgramStackFrame>(long long)>& program_stack_frame_callback,
        const std::string& error_handler,
        const std::string& shutdown_handler,
        const std::function<int(const std::string&)>& aerror_callback,
        const std::function<PrgValue(const std::string&)>& eval_expression_callback,
        const std::function<std::string(const std::string&)>& set_callback,
        const std::function<std::optional<RuntimeSurfaceCursorSnapshot>(const std::string&)>& snapshot_cursor_callback,
        const std::function<std::optional<std::size_t>(const RuntimeSurfaceCursorSnapshot&, const std::string&)>& load_cursor_snapshot_callback,
        bool require_verified_file_byte_overrides,
        const std::function<std::optional<std::string>(const std::filesystem::path&)>& read_verified_file_callback,
        const std::function<RuntimeOleObjectState*(const PrgValue&)>& resolve_object_callback,
        const std::function<std::optional<PrgValue>(const PrgValue&, const std::string&)>& read_native_member_callback,
        const std::function<bool(const PrgValue&, const std::string&, const PrgValue&)>& write_native_member_callback,
        const std::function<std::optional<std::int64_t>(std::int64_t)>& whandle_from_hwnd_callback,
        const std::function<std::optional<std::int64_t>(std::int64_t)>& hwnd_from_whandle_callback,
        const std::function<void(const std::string&, std::vector<PrgValue>)>& assign_array_callback,
        const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_prompt_callback,
        const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_bar_count_callback,
        const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_bar_position_callback,
        const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_bar_skip_callback,
        const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_bar_mark_callback,
        const std::function<void(const std::string&, const std::string&)>& record_event_callback);

    namespace
    {

        class ExpressionParser
        {
        public:
            ExpressionParser(
                const std::string &text,
                const Frame &frame,
                const std::map<std::string, PrgValue> &globals,
                const std::string &default_directory,
                const std::string &last_error_message,
                int last_error_code,
                const std::string &last_error_procedure,
                std::size_t last_error_line,
                const std::string &current_program_name,
                std::size_t program_stack_depth,
                std::function<std::optional<RuntimeProgramStackFrame>(long long)> program_stack_frame_callback,
                const std::string &error_handler,
                const std::string &shutdown_handler,
                bool exact_string_compare,
                int current_work_area,
                std::function<int()> next_free_work_area_callback,
                std::function<int(const std::string &)> resolve_work_area_callback,
                std::function<std::string(const std::string &)> alias_lookup_callback,
                std::function<bool(const std::string &)> used_callback,
                std::function<std::string(const std::string &)> dbf_lookup_callback,
                std::function<std::size_t(const std::string &)> field_count_callback,
                std::function<std::string(std::size_t, const std::string &)> field_name_callback,
                std::function<std::size_t(const std::string &, std::size_t, const std::string &)> field_size_callback,
                std::function<std::size_t(const std::string &)> record_count_callback,
                std::function<std::size_t(const std::string &)> record_length_callback,
                std::function<std::size_t(const std::string &)> recno_callback,
                std::function<bool(const std::string &)> found_callback,
                std::function<bool(const std::string &)> eof_callback,
                std::function<bool(const std::string &)> bof_callback,
                std::function<PrgValue(const std::string &, const std::vector<std::string> &, const std::vector<PrgValue> &)> lock_function_callback,
                std::function<std::optional<PrgValue>(const std::string &)> field_lookup_callback,
                std::function<bool(const std::string &)> array_exists_callback,
                std::function<std::size_t(const std::string &, int)> array_length_callback,
                std::function<PrgValue(const std::string &, std::size_t, std::size_t)> array_value_callback,
                std::function<PrgValue(const std::string &, const std::vector<std::string> &, const std::vector<PrgValue> &)> array_function_callback,
                std::function<int(const std::string &)> aerror_callback,
                std::function<PrgValue(const std::string &, const std::vector<std::string> &)> aggregate_callback,
                std::function<std::string(const std::string &, bool)> order_callback,
                std::function<std::string(const std::string &, std::size_t, const std::string &)> tag_callback,
                std::function<bool(const std::string &, bool, const std::string &, const std::string &)> seek_callback,
                std::function<bool(const std::string &, bool, const std::string &, const std::string &)> indexseek_callback,
                std::function<std::string()> foxtoolver_callback,
                std::function<int()> mainhwnd_callback,
                std::function<int(const std::string &, const std::string &, const std::string &, const std::string &, const std::string &)> regfn_callback,
                std::function<PrgValue(int, const std::vector<PrgValue> &)> callfn_callback,
                std::function<int(const std::string &, const std::string &)> sql_connect_callback,
                std::function<int(int, const std::string &, const std::string &)> sql_exec_callback,
                std::function<bool(int)> sql_disconnect_callback,
                std::function<int(int)> sql_row_count_callback,
                std::function<int(int, const std::string &)> sql_prepare_callback,
                std::function<int(int)> sql_cancel_callback,
                std::function<int(int)> sql_commit_callback,
                std::function<int(int)> sql_rollback_callback,
                std::function<int(int, const std::string &)> sql_databases_callback,
                std::function<int(int, const std::string &, const std::string &)> sql_primary_keys_callback,
                std::function<int(int, const std::string &, const std::string &)> sql_foreign_keys_callback,
                std::function<int(int, const std::string &, const std::string &)> sql_tables_callback,
                std::function<int(int, const std::string &, const std::string &, const std::string &)> sql_columns_callback,
                std::function<PrgValue(int, const std::string &)> sql_get_prop_callback,
                std::function<int(int, const std::string &, const PrgValue &)> sql_set_prop_callback,
                std::function<int(const std::string &, const std::string &, const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> register_ole_callback,
                std::function<PrgValue(const std::string &, const std::string &, const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> ole_invoke_callback,
                std::function<PrgValue(const std::string &)> ole_property_callback,
                std::function<PrgValue(const std::string &)> eval_expression_callback,
                std::function<std::string(const std::string &)> set_callback,
                std::function<std::optional<RuntimeSurfaceCursorSnapshot>(const std::string &)> snapshot_cursor_callback,
                std::function<std::optional<std::size_t>(const RuntimeSurfaceCursorSnapshot &, const std::string &)> load_cursor_snapshot_callback,
                bool require_verified_file_byte_overrides,
                std::function<std::optional<std::string>(const std::filesystem::path &)> read_verified_file_callback,
                std::function<std::optional<PrgValue>(const std::string &, const std::vector<PrgValue> &)> cursor_buffering_callback,
                std::function<void(const std::string &, const std::string &)> record_event_callback,
                std::function<RuntimeOleObjectState*(const PrgValue &)> resolve_object_callback,
                std::function<RuntimeOleObjectState*(const std::string &)> resolve_object_path_callback,
                std::function<std::optional<PrgValue>(const PrgValue &, const std::string &)> read_native_member_callback,
                std::function<std::optional<PrgValue>(const PrgValue &, const std::string &, const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> invoke_native_member_callback,
                std::function<bool(const PrgValue &, const std::string &, const PrgValue &)> write_native_member_callback,
                std::function<std::optional<std::int64_t>(std::int64_t)> whandle_from_hwnd_callback,
                std::function<std::optional<std::int64_t>(std::int64_t)> hwnd_from_whandle_callback,
                std::function<void(const std::string &, std::vector<PrgValue>)> assign_array_callback,
                std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_prompt_callback,
                std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_bar_count_callback,
                std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_bar_position_callback,
                std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_bar_skip_callback,
                std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_bar_mark_callback,
                std::function<PrgValue(const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> bindevent_callback,
                std::function<PrgValue(const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> raiseevent_callback,
                std::function<PrgValue(const std::vector<PrgValue> &)> unbindevents_callback,
                std::function<PrgValue(const std::vector<PrgValue> &, const std::vector<std::string> &)> aevents_callback,
                std::function<std::size_t()> memowidth_callback,
                std::function<std::optional<PrgValue>(const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> base_method_invoke_callback,
                std::function<std::optional<PrgValue>(const std::string &, const std::vector<PrgValue> &, const std::vector<std::string> &, const std::vector<std::optional<std::string>> &, std::size_t, std::size_t)> user_routine_invoke_callback,
                std::function<std::optional<PrgValue>(const std::string &, const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> declared_dll_invoke_callback,
                ExpressionContinuation *expression_continuation)
                : current_work_area_(current_work_area),
                  next_free_work_area_callback_(std::move(next_free_work_area_callback)),
                  resolve_work_area_callback_(std::move(resolve_work_area_callback)),
                  alias_lookup_callback_(std::move(alias_lookup_callback)),
                  used_callback_(std::move(used_callback)),
                  dbf_lookup_callback_(std::move(dbf_lookup_callback)),
                  field_count_callback_(std::move(field_count_callback)),
                  field_name_callback_(std::move(field_name_callback)),
                  field_size_callback_(std::move(field_size_callback)),
                  record_count_callback_(std::move(record_count_callback)),
                  record_length_callback_(std::move(record_length_callback)),
                  recno_callback_(std::move(recno_callback)),
                  found_callback_(std::move(found_callback)),
                  eof_callback_(std::move(eof_callback)),
                  bof_callback_(std::move(bof_callback)),
                  lock_function_callback_(std::move(lock_function_callback)),
                  field_lookup_callback_(std::move(field_lookup_callback)),
                  array_exists_callback_(std::move(array_exists_callback)),
                  array_length_callback_(std::move(array_length_callback)),
                  array_value_callback_(std::move(array_value_callback)),
                  array_function_callback_(std::move(array_function_callback)),
                  aerror_callback_(std::move(aerror_callback)),
                  aggregate_callback_(std::move(aggregate_callback)),
                  order_callback_(std::move(order_callback)),
                  tag_callback_(std::move(tag_callback)),
                  seek_callback_(std::move(seek_callback)),
                  indexseek_callback_(std::move(indexseek_callback)),
                  foxtoolver_callback_(std::move(foxtoolver_callback)),
                  mainhwnd_callback_(std::move(mainhwnd_callback)),
                  regfn_callback_(std::move(regfn_callback)),
                  callfn_callback_(std::move(callfn_callback)),
                  sql_connect_callback_(std::move(sql_connect_callback)),
                  sql_exec_callback_(std::move(sql_exec_callback)),
                  sql_disconnect_callback_(std::move(sql_disconnect_callback)),
                  sql_row_count_callback_(std::move(sql_row_count_callback)),
                  sql_prepare_callback_(std::move(sql_prepare_callback)),
                  sql_cancel_callback_(std::move(sql_cancel_callback)),
                  sql_commit_callback_(std::move(sql_commit_callback)),
                  sql_rollback_callback_(std::move(sql_rollback_callback)),
                  sql_databases_callback_(std::move(sql_databases_callback)),
                  sql_primary_keys_callback_(std::move(sql_primary_keys_callback)),
                  sql_foreign_keys_callback_(std::move(sql_foreign_keys_callback)),
                  sql_tables_callback_(std::move(sql_tables_callback)),
                  sql_columns_callback_(std::move(sql_columns_callback)),
                  sql_get_prop_callback_(std::move(sql_get_prop_callback)),
                  sql_set_prop_callback_(std::move(sql_set_prop_callback)),
                  register_ole_callback_(std::move(register_ole_callback)),
                  ole_invoke_callback_(std::move(ole_invoke_callback)),
                  ole_property_callback_(std::move(ole_property_callback)),
                  eval_expression_callback_(std::move(eval_expression_callback)),
                  set_callback_(std::move(set_callback)),
                  snapshot_cursor_callback_(std::move(snapshot_cursor_callback)),
                  load_cursor_snapshot_callback_(std::move(load_cursor_snapshot_callback)),
                  require_verified_file_byte_overrides_(require_verified_file_byte_overrides),
                  read_verified_file_callback_(std::move(read_verified_file_callback)),
                  cursor_buffering_callback_(std::move(cursor_buffering_callback)),
                  record_event_callback_(std::move(record_event_callback)),
                  resolve_object_callback_(std::move(resolve_object_callback)),
                  resolve_object_path_callback_(std::move(resolve_object_path_callback)),
                  read_native_member_callback_(std::move(read_native_member_callback)),
                  invoke_native_member_callback_(std::move(invoke_native_member_callback)),
                  write_native_member_callback_(std::move(write_native_member_callback)),
                  whandle_from_hwnd_callback_(std::move(whandle_from_hwnd_callback)),
                  hwnd_from_whandle_callback_(std::move(hwnd_from_whandle_callback)),
                  assign_array_callback_(std::move(assign_array_callback)),
                  popup_prompt_callback_(std::move(popup_prompt_callback)),
                  popup_bar_count_callback_(std::move(popup_bar_count_callback)),
                  popup_bar_position_callback_(std::move(popup_bar_position_callback)),
                  popup_bar_skip_callback_(std::move(popup_bar_skip_callback)),
                  popup_bar_mark_callback_(std::move(popup_bar_mark_callback)),
                  bindevent_callback_(std::move(bindevent_callback)),
                  raiseevent_callback_(std::move(raiseevent_callback)),
                  unbindevents_callback_(std::move(unbindevents_callback)),
                  aevents_callback_(std::move(aevents_callback)),
                  memowidth_callback_(std::move(memowidth_callback)),
                  base_method_invoke_callback_(std::move(base_method_invoke_callback)),
                  user_routine_invoke_callback_(std::move(user_routine_invoke_callback)),
                  declared_dll_invoke_callback_(std::move(declared_dll_invoke_callback)),
                  expression_continuation_(expression_continuation),
                  text_(text),
                  frame_(frame),
                  globals_(globals),
                  default_directory_(default_directory),
                  last_error_message_(last_error_message),
                  last_error_code_(last_error_code),
                  last_error_procedure_(last_error_procedure),
                  last_error_line_(last_error_line),
                  current_program_name_(current_program_name),
                  program_stack_depth_(program_stack_depth),
                  program_stack_frame_callback_(std::move(program_stack_frame_callback)),
                  error_handler_(error_handler),
                shutdown_handler_(shutdown_handler),
                  exact_string_compare_(exact_string_compare)
            {
            }

        PrgValue parse()
            {
                position_ = 0;
                PrgValue value = parse_expression();
                skip_whitespace();
                return value;
            }

        private:
            struct ScopedEvaluationSuppression
            {
                explicit ScopedEvaluationSuppression(ExpressionParser& parser)
                    : parser_(parser),
                      previous_(parser.suppress_evaluation_)
                {
                    parser_.suppress_evaluation_ = true;
                }

                ~ScopedEvaluationSuppression()
                {
                    parser_.suppress_evaluation_ = previous_;
                }

            private:
                ExpressionParser& parser_;
                bool previous_;
            };

            struct ScopedMacroTextPreservation
            {
                ScopedMacroTextPreservation(
                    ExpressionParser &parser,
                    bool enabled,
                    bool evaluate_resolved_text = true)
                    : parser_(parser),
                      previous_(parser.preserve_macro_text_),
                      previous_evaluate_resolved_text_(parser.evaluate_preserved_macro_text_)
                {
                    parser_.preserve_macro_text_ = enabled;
                    parser_.evaluate_preserved_macro_text_ = evaluate_resolved_text;
                }

                ~ScopedMacroTextPreservation()
                {
                    parser_.preserve_macro_text_ = previous_;
                    parser_.evaluate_preserved_macro_text_ = previous_evaluate_resolved_text_;
                }

            private:
                ExpressionParser &parser_;
                bool previous_;
                bool previous_evaluate_resolved_text_;
            };

            static PrgValue currency_arithmetic(
                const PrgValue &left,
                const PrgValue &right,
                char operation)
            {
                if (left.kind == PrgValueKind::currency &&
                    right.kind == PrgValueKind::currency &&
                    (operation == '+' || operation == '-'))
                {
                    if (operation == '-' && right.currency_value == std::numeric_limits<std::int64_t>::min())
                    {
                        return make_number_value(value_as_number(left) - value_as_number(right));
                    }
                    const std::int64_t right_value = operation == '+'
                                                         ? right.currency_value
                                                         : -right.currency_value;
                    if ((right_value > 0 && left.currency_value > std::numeric_limits<std::int64_t>::max() - right_value) ||
                        (right_value < 0 && left.currency_value < std::numeric_limits<std::int64_t>::min() - right_value))
                    {
                        return make_number_value(value_as_number(left) +
                                                 (operation == '+' ? value_as_number(right) : -value_as_number(right)));
                    }
                    return make_currency_value(left.currency_value + right_value);
                }

                const bool left_currency = left.kind == PrgValueKind::currency;
                const bool right_currency = right.kind == PrgValueKind::currency;
                if ((operation == '*' && left_currency != right_currency) ||
                    (operation == '/' && left_currency && !right_currency))
                {
                    const long double result = operation == '*'
                                                    ? static_cast<long double>(value_as_number(left)) * value_as_number(right)
                                                    : static_cast<long double>(value_as_number(left)) / value_as_number(right);
                    const long double scaled = std::round(result * 10000.0L);
                    if (std::isfinite(scaled) &&
                        scaled >= static_cast<long double>(std::numeric_limits<std::int64_t>::min()) &&
                        scaled <= static_cast<long double>(std::numeric_limits<std::int64_t>::max()))
                    {
                        return make_currency_value(static_cast<std::int64_t>(scaled));
                    }
                }

                return make_number_value(
                    operation == '*'
                        ? value_as_number(left) * value_as_number(right)
                        : value_as_number(left) / value_as_number(right));
            }

            PrgValue parse_expression()
            {
                return parse_or();
            }

            PrgValue parse_or()
            {
                PrgValue left = parse_and();
                while (true)
                {
                    if (match_logical_or_operator())
                    {
                        if (suppress_evaluation_)
                        {
                            (void)parse_and();
                            left = make_boolean_value(false);
                            continue;
                        }

                        if (value_as_bool(left))
                        {
                            ScopedEvaluationSuppression suppress_rhs(*this);
                            (void)parse_and();
                            left = make_boolean_value(true);
                            continue;
                        }

                        left = make_boolean_value(value_as_bool(parse_and()));
                        continue;
                    }

                    return left;
                }
            }

            PrgValue parse_and()
            {
                PrgValue left = parse_comparison();
                while (true)
                {
                    if (match_logical_and_operator())
                    {
                        if (suppress_evaluation_)
                        {
                            (void)parse_comparison();
                            left = make_boolean_value(false);
                            continue;
                        }

                        if (!value_as_bool(left))
                        {
                            ScopedEvaluationSuppression suppress_rhs(*this);
                            (void)parse_comparison();
                            left = make_boolean_value(false);
                            continue;
                        }

                        left = make_boolean_value(value_as_bool(parse_comparison()));
                        continue;
                    }

                    return left;
                }
            }

            PrgValue parse_comparison()
            {
                PrgValue left = parse_additive();
                while (true)
                {
                    skip_whitespace();
                    if (match("<>") || match("!=") || match("#"))
                    {
                        PrgValue right = parse_additive();
                        if (suppress_evaluation_)
                        {
                            left = make_boolean_value(false);
                        }
                        else
                        {
                            left = make_boolean_value(!values_equal(left, right));
                        }
                    }
                    else if (match("<="))
                    {
                        PrgValue right = parse_additive();
                        if (suppress_evaluation_)
                        {
                            left = make_boolean_value(false);
                        }
                        else
                        {
                            left = make_boolean_value(compare_ordered_values(left, right) <= 0);
                        }
                    }
                    else if (match(">="))
                    {
                        PrgValue right = parse_additive();
                        if (suppress_evaluation_)
                        {
                            left = make_boolean_value(false);
                        }
                        else
                        {
                            left = make_boolean_value(compare_ordered_values(left, right) >= 0);
                        }
                    }
                    else if (match("==") || match("="))
                    {
                        PrgValue right = parse_additive();
                        if (suppress_evaluation_)
                        {
                            left = make_boolean_value(false);
                        }
                        else
                        {
                            left = make_boolean_value(values_equal(left, right));
                        }
                    }
                    else if (match("$"))
                    {
                        PrgValue right = parse_additive();
                        if (suppress_evaluation_)
                        {
                            left = make_boolean_value(false);
                        }
                        else
                        {
                            const std::string needle = value_as_string(left);
                            const std::string haystack = value_as_string(right);
                            left = make_boolean_value(haystack.find(needle) != std::string::npos);
                        }
                    }
                    else if (match("<"))
                    {
                        PrgValue right = parse_additive();
                        if (suppress_evaluation_)
                        {
                            left = make_boolean_value(false);
                        }
                        else
                        {
                            left = make_boolean_value(compare_ordered_values(left, right) < 0);
                        }
                    }
                    else if (match(">"))
                    {
                        PrgValue right = parse_additive();
                        if (suppress_evaluation_)
                        {
                            left = make_boolean_value(false);
                        }
                        else
                        {
                            left = make_boolean_value(compare_ordered_values(left, right) > 0);
                        }
                    }
                    else
                    {
                        return left;
                    }
                }
            }

            PrgValue parse_additive()
            {
                PrgValue left = parse_multiplicative();
                while (true)
                {
                    skip_whitespace();
                    if (match("+"))
                    {
                        PrgValue right = parse_multiplicative();
                        if (suppress_evaluation_)
                        {
                            left = make_empty_value();
                        }
                        else if (left.string_flavor != PrgStringFlavor::none ||
                                 right.string_flavor != PrgStringFlavor::none)
                        {
                            const auto result = evaluate_date_time_additive(left, right, false, set_callback_);
                            if (!result.has_value())
                            {
                                throw std::runtime_error(
                                    runtime_text("Runtime.Prg.Expression.Error.OperatorOperandTypeMismatch"));
                            }
                            left = *result;
                        }
                        else if (left.kind == PrgValueKind::string && right.kind == PrgValueKind::string)
                        {
                            left = make_string_value(value_as_string(left) + value_as_string(right));
                        }
                        else if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                        {
                            throw std::runtime_error(
                                runtime_text("Runtime.Prg.Expression.Error.OperatorOperandTypeMismatch"));
                        }
                        else if (left.kind == PrgValueKind::currency || right.kind == PrgValueKind::currency)
                        {
                            left = currency_arithmetic(left, right, '+');
                        }
                        else if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                                 (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            // Preserve integer arithmetic - use int64 as common type
                            left = make_int64_value(
                                static_cast<std::int64_t>(value_as_number(left)) +
                                static_cast<std::int64_t>(value_as_number(right)));
                        }
                        else
                        {
                            left = make_number_value(value_as_number(left) + value_as_number(right));
                        }
                    }
                    else if (match("-"))
                    {
                        PrgValue right = parse_multiplicative();
                        if (suppress_evaluation_)
                        {
                            left = make_empty_value();
                        }
                        else if (left.string_flavor != PrgStringFlavor::none ||
                                 right.string_flavor != PrgStringFlavor::none)
                        {
                            const auto result = evaluate_date_time_additive(left, right, true, set_callback_);
                            if (!result.has_value())
                            {
                                throw std::runtime_error(
                                    runtime_text("Runtime.Prg.Expression.Error.OperatorOperandTypeMismatch"));
                            }
                            left = *result;
                        }
                        else if (left.kind == PrgValueKind::string && right.kind == PrgValueKind::string)
                        {
                            std::string left_text = value_as_string(left);
                            const std::string right_text = value_as_string(right);
                            std::size_t trailing_space_count = 0U;
                            while (!left_text.empty() && left_text.back() == ' ')
                            {
                                left_text.pop_back();
                                ++trailing_space_count;
                            }
                            left_text.append(right_text);
                            left_text.append(trailing_space_count, ' ');
                            left = make_string_value(std::move(left_text));
                        }
                        else if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                        {
                            throw std::runtime_error(
                                runtime_text("Runtime.Prg.Expression.Error.OperatorOperandTypeMismatch"));
                        }
                        else if (left.kind == PrgValueKind::currency || right.kind == PrgValueKind::currency)
                        {
                            left = currency_arithmetic(left, right, '-');
                        }
                        else if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                            (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            left = make_int64_value(
                                static_cast<std::int64_t>(value_as_number(left)) -
                                static_cast<std::int64_t>(value_as_number(right)));
                        }
                        else
                        {
                            left = make_number_value(value_as_number(left) - value_as_number(right));
                        }
                    }
                    else
                    {
                        return left;
                    }
                }
            }

            PrgValue parse_multiplicative()
            {
                PrgValue left = parse_unary();
                while (true)
                {
                    skip_whitespace();
                    if (match("*"))
                    {
                        PrgValue right = parse_unary();
                        if (suppress_evaluation_)
                        {
                            left = make_empty_value();
                        }
                        else if (left.kind == PrgValueKind::currency || right.kind == PrgValueKind::currency)
                        {
                            left = currency_arithmetic(left, right, '*');
                        }
                        else if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                            (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            left = make_int64_value(
                                static_cast<std::int64_t>(value_as_number(left)) *
                                static_cast<std::int64_t>(value_as_number(right)));
                        }
                        else
                        {
                            left = make_number_value(value_as_number(left) * value_as_number(right));
                        }
                    }
                    else if (match("/"))
                    {
                        PrgValue right = parse_unary();
                        if (suppress_evaluation_)
                        {
                            left = make_empty_value();
                        }
                        else if (left.kind == PrgValueKind::currency || right.kind == PrgValueKind::currency)
                        {
                            const double divisor = value_as_number(right);
                            if (divisor == 0.0)
                            {
                                throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.DivisionByZero"));
                            }
                            left = currency_arithmetic(left, right, '/');
                        }
                        else if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                            (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            const std::int64_t divisor = static_cast<std::int64_t>(value_as_number(right));
                            if (divisor == 0)
                                throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.IntegerDivisionByZero"));
                            left = make_int64_value(static_cast<std::int64_t>(value_as_number(left)) / divisor);
                        }
                        else
                        {
                            const double divisor = value_as_number(right);
                            if (divisor == 0.0)
                                throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.DivisionByZero"));
                            left = make_number_value(value_as_number(left) / divisor);
                        }
                    }
                    else
                    {
                        return left;
                    }
                }
            }

            PrgValue parse_unary()
            {
                skip_whitespace();
                if (match("!") || match_dotted_keyword(".NOT.") || match_identifier_keyword("NOT"))
                {
                    if (suppress_evaluation_)
                    {
                        (void)parse_comparison();
                        return make_boolean_value(false);
                    }
                    return make_boolean_value(!value_as_bool(parse_comparison()));
                }
                if (match("-"))
                {
                    PrgValue operand = parse_unary();
                    if (suppress_evaluation_)
                    {
                        return make_empty_value();
                    }
                    if (operand.kind == PrgValueKind::int64)
                    {
                        return make_int64_value(-operand.int64_value);
                    }
                    if (operand.kind == PrgValueKind::currency &&
                        operand.currency_value != std::numeric_limits<std::int64_t>::min())
                    {
                        return make_currency_value(-operand.currency_value);
                    }
                    return make_number_value(-value_as_number(operand));
                }
                return parse_power();
            }

            PrgValue parse_power()
            {
                PrgValue left = parse_primary();
                skip_whitespace();
                if (match("**") || match("^"))
                {
                    PrgValue right = parse_unary();
                    if (suppress_evaluation_)
                    {
                        return make_empty_value();
                    }
                    return make_number_value(std::pow(value_as_number(left), value_as_number(right)));
                }
                return left;
            }

            PrgValue parse_primary()
            {
                skip_whitespace();
                const std::size_t primary_start = position_;
                if (!suppress_evaluation_ && expression_continuation_ != nullptr)
                {
                    const auto checkpoint =
                        expression_continuation_->primary_checkpoints.find(primary_start);
                    if (checkpoint != expression_continuation_->primary_checkpoints.end())
                    {
                        position_ = checkpoint->second.end;
                        return checkpoint->second.value;
                    }
                }

                PrgValue value = parse_primary_uncached();
                if (!suppress_evaluation_ && expression_continuation_ != nullptr)
                {
                    expression_continuation_->primary_checkpoints[primary_start] =
                        {.end = position_, .value = value};
                }
                return value;
            }

            PrgValue parse_primary_uncached()
            {
                skip_whitespace();
                if (match("("))
                {
                    PrgValue value = parse_expression();
                    match(")");
                    return value;
                }
                const std::size_t macro_start = position_;
                if (match("&"))
                {
                    if (suppress_evaluation_)
                    {
                        skip_macro_reference();
                        return make_empty_value();
                    }
                    PrgValue macro_value = parse_macro_reference(macro_start);
                    if (macro_value.kind == PrgValueKind::string)
                    {
                        const std::string array_name = resolve_array_argument_name({}, &macro_value);
                        if (!array_name.empty() && array_exists_callback_(array_name))
                        {
                            if (skip_whitespace(), match("["))
                            {
                                return parse_array_element_access(array_name, ']');
                            }
                            if (skip_whitespace(), match("("))
                            {
                                return parse_array_element_access(array_name, ')');
                            }
                        }
                    }
                    return macro_value;
                }
                if (peek() == '\'' || peek() == '"')
                {
                    return make_string_value(parse_string());
                }
                if (peek() == '{')
                {
                    return make_string_value(parse_braced_literal());
                }
                if (peek() == '[')
                {
                    return make_string_value(parse_bracket_literal());
                }
                if (peek() == '.')
                {
                    if (match(".T.") || match(".t."))
                    {
                        return make_boolean_value(true);
                    }
                    if (match(".F.") || match(".f."))
                    {
                        return make_boolean_value(false);
                    }
                    if (match(".NULL.") || match(".null."))
                    {
                        return make_null_value();
                    }
                }
                if (std::isdigit(static_cast<unsigned char>(peek())) != 0)
                {
                    return make_number_value(parse_number());
                }

                skip_whitespace();
                const std::size_t primary_start = position_;
                const std::string identifier = parse_identifier();
                if (identifier.empty())
                {
                    return {};
                }

                skip_whitespace();
                if (normalize_identifier(identifier) == "null")
                {
                    return make_null_value();
                }

                    if (match("["))
                    {
                        if (suppress_evaluation_)
                        {
                            (void)parse_expression();
                            skip_whitespace();
                            match("]");
                            return skip_postfix_member_and_collection_access();
                        }
                    return apply_postfix_member_and_collection_access(
                        parse_indexed_identifier_access(identifier, ']'));
                }

                if (match("("))
                {
                    const std::string normalized_identifier = normalize_identifier(identifier);
                    if (normalized_identifier == "iif")
                    {
                        PrgValue value = parse_iif_invocation();
                        if (suppress_evaluation_)
                        {
                            return skip_postfix_member_and_collection_access();
                        }
                        return apply_postfix_member_and_collection_access(value);
                    }
                    if (normalized_identifier == "icase")
                    {
                        PrgValue value = parse_icase_invocation();
                        if (suppress_evaluation_)
                        {
                            return skip_postfix_member_and_collection_access();
                        }
                        return apply_postfix_member_and_collection_access(value);
                    }

                    if (suppress_evaluation_)
                    {
                        (void)parse_invocation_arguments(identifier);
                        return skip_postfix_member_and_collection_access();
                    }

                    // EVALUATE consumes expression source, so indirect macro text must not
                    // execute while its argument is being collected.
                    ScopedMacroTextPreservation macro_text_guard(
                        *this,
                        normalized_identifier == "execscript" ||
                            normalized_identifier == "lookup" ||
                            normalized_identifier == "textmerge" ||
                            normalized_identifier == "eval" ||
                            normalized_identifier == "evaluate",
                        normalized_identifier != "eval" &&
                            normalized_identifier != "evaluate");
                    const auto invocation = parse_invocation_arguments(identifier);
                    const auto &arguments = invocation.arguments;
                    const auto &raw_arguments = invocation.raw_arguments;
                    const auto &argument_references = invocation.argument_references;
                    std::size_t invocation_tail = position_;
                    while (invocation_tail < text_.size() &&
                           std::isspace(static_cast<unsigned char>(text_[invocation_tail])) != 0)
                    {
                        ++invocation_tail;
                    }
                    const bool prefer_function_call =
                        normalized_identifier == "aclass" ||
                        normalized_identifier == "acopy" ||
                        normalized_identifier == "aevents" ||
                        normalized_identifier == "adel" ||
                        normalized_identifier == "adir" ||
                        normalized_identifier == "aelement" ||
                        normalized_identifier == "afields" ||
                        normalized_identifier == "afont" ||
                        normalized_identifier == "agetfileversion" ||
                        normalized_identifier == "ains" ||
                        normalized_identifier == "alen" ||
                        normalized_identifier == "alines" ||
                        normalized_identifier == "aprinters" ||
                        normalized_identifier == "ascan" ||
                        normalized_identifier == "asessions" ||
                        normalized_identifier == "asize" ||
                        normalized_identifier == "asort" ||
                        normalized_identifier == "asubscript" ||
                        normalized_identifier == "aused";
                    if (!prefer_function_call && array_exists_callback_(identifier))
                    {
                        const std::size_t row = arguments.empty()
                                                    ? 0U
                                                    : static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[0])));
                        const std::size_t column = arguments.size() < 2U
                                                       ? 1U
                                                       : static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[1])));
                        return apply_postfix_member_and_collection_access(
                            array_value_callback_(identifier, row, column));
                    }
                    return apply_postfix_member_and_collection_access(
                        evaluate_function(
                            identifier,
                            arguments,
                            raw_arguments,
                            argument_references,
                            primary_start,
                            invocation_tail));
                }

                if (suppress_evaluation_)
                {
                    return skip_postfix_member_and_collection_access();
                }

                return apply_postfix_member_and_collection_access(resolve_identifier(identifier));
            }

            PrgValue evaluate_function(
                const std::string &identifier,
                const std::vector<PrgValue> &arguments,
                const std::vector<std::string> &raw_arguments,
                const std::vector<std::optional<std::string>> &argument_references,
                std::size_t invocation_start,
                std::size_t invocation_end)
            {
                const std::string function = normalize_identifier(identifier);
                const auto is_selector_style_native_member_name =
                    [](const std::string &member_name) -> bool
                {
                    const std::string normalized_member_name =
                        normalize_identifier(member_name);
                    return normalized_member_name == "list" ||
                           normalized_member_name == "listitem" ||
                           normalized_member_name == "itemdata" ||
                           normalized_member_name == "selected" ||
                           normalized_member_name == "selectedid" ||
                           normalized_member_name == "indextoitemid" ||
                           normalized_member_name == "itemidtoindex";
                };
                const auto try_native_collection_default_item =
                    [&]() -> std::optional<PrgValue>
                {
                    if (arguments.empty())
                    {
                        return std::nullopt;
                    }

                    RuntimeOleObjectState *runtime_object = resolve_object_path_callback_(identifier);
                    if (runtime_object == nullptr ||
                        !is_native_collection_object(*runtime_object))
                    {
                        return std::nullopt;
                    }

                    const auto member_separator = identifier.find('.');
                    const std::string base_name =
                        member_separator == std::string::npos
                            ? identifier
                            : identifier.substr(0U, member_separator);
                    const std::string member_path =
                        member_separator == std::string::npos
                            ? std::string("item")
                            : identifier.substr(member_separator + 1U) + ".item";
                    return ole_invoke_callback_(base_name,
                                                member_path,
                                                arguments,
                                                argument_references);
                };

                if (const auto collection_default_item = try_native_collection_default_item();
                    collection_default_item.has_value())
                {
                    return *collection_default_item;
                }

                const auto member_separator = function.find('.');
                if (member_separator != std::string::npos)
                {
                    const std::string raw_base_name = identifier.substr(0U, member_separator);
                    const std::string raw_member_path = identifier.substr(member_separator + 1U);
                    const std::string base_name = function.substr(0U, member_separator);
                    const std::string member_path = function.substr(member_separator + 1U);
                    if (is_selector_style_native_member_name(raw_member_path))
                    {
                        PrgValue current = resolve_identifier(raw_base_name);
                        std::string selector_member_name = raw_member_path;
                        selector_member_name.push_back('(');
                        for (std::size_t index = 0U; index < arguments.size(); ++index)
                        {
                            if (index != 0U)
                            {
                                selector_member_name += ", ";
                            }
                            selector_member_name += format_value(arguments[index]);
                        }
                        selector_member_name.push_back(')');
                        if (const auto selector_value =
                                read_native_member_callback_(current, selector_member_name);
                            selector_value.has_value())
                        {
                            return *selector_value;
                        }
                    }
                    return ole_invoke_callback_(base_name, member_path, arguments, argument_references);
                }
                if ((function == "min" || function == "max") && arguments.size() >= 2U)
                {
                    PrgValue result = arguments.front();
                    for (std::size_t index = 1U; index < arguments.size(); ++index)
                    {
                        const PrgValue &candidate = arguments[index];
                        bool candidate_wins = false;
                        if (result.string_flavor != PrgStringFlavor::none &&
                            candidate.string_flavor != PrgStringFlavor::none)
                        {
                            if (const auto comparison = compare_date_time_values(result, candidate, set_callback_);
                                comparison.has_value())
                            {
                                candidate_wins = function == "min" ? *comparison > 0 : *comparison < 0;
                            }
                            else
                            {
                                candidate_wins = function == "min"
                                                     ? value_as_string(candidate) < value_as_string(result)
                                                     : value_as_string(candidate) > value_as_string(result);
                            }
                        }
                        else if (result.kind == PrgValueKind::string || candidate.kind == PrgValueKind::string)
                        {
                            candidate_wins = function == "min"
                                                 ? value_as_string(candidate) < value_as_string(result)
                                                 : value_as_string(candidate) > value_as_string(result);
                        }
                        else
                        {
                            candidate_wins = function == "min"
                                                 ? value_as_number(candidate) < value_as_number(result)
                                                 : value_as_number(candidate) > value_as_number(result);
                        }
                        if (candidate_wins)
                        {
                            result = candidate;
                        }
                    }
                    return result;
                }
                if (function == "count" || function == "sum" || function == "avg" || function == "average" || function == "min" || function == "max")
                {
                    return aggregate_callback_(function, raw_arguments);
                }
                if (function == "bindevent")
                {
                    return bindevent_callback_
                        ? bindevent_callback_(arguments, argument_references)
                        : make_number_value(0.0);
                }
                if (function == "raiseevent")
                {
                    return raiseevent_callback_
                        ? raiseevent_callback_(arguments, argument_references)
                        : make_boolean_value(false);
                }
                if (function == "unbindevents")
                {
                    return unbindevents_callback_
                        ? unbindevents_callback_(arguments)
                        : make_number_value(0.0);
                }
                if (function == "aevents")
                {
                    return aevents_callback_
                        ? aevents_callback_(arguments, raw_arguments)
                        : make_number_value(0.0);
                }
                if (function == "select")
                {
                    if (arguments.empty())
                    {
                        return make_number_value(static_cast<double>(current_work_area_));
                    }
                    if (arguments[0].kind == PrgValueKind::string)
                    {
                        return make_number_value(static_cast<double>(resolve_work_area_callback_(value_as_string(arguments[0]))));
                    }
                    const int requested = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    return make_number_value(static_cast<double>(requested == 0 ? next_free_work_area_callback_() : resolve_work_area_callback_(std::to_string(requested))));
                }
                if (function == "alias")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_string_value(alias_lookup_callback_(designator));
                }
                if (function == "used")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_boolean_value(used_callback_(designator));
                }
                if (function == "dbf")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_string_value(dbf_lookup_callback_(designator));
                }
                if (function == "fcount")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_number_value(static_cast<double>(field_count_callback_(designator)));
                }
                if (function == "field")
                {
                    if (arguments.empty())
                    {
                        return make_string_value({});
                    }
                    const auto index = static_cast<std::size_t>(std::max<long long>(0LL, std::llround(value_as_number(arguments[0]))));
                    const std::string designator = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    return make_string_value(field_name_callback_(index, designator));
                }
                if (function == "fsize")
                {
                    if (arguments.empty())
                    {
                        return make_number_value(0.0);
                    }
                    std::size_t index = 0U;
                    std::string field_name;
                    if (arguments[0].kind == PrgValueKind::string)
                    {
                        field_name = value_as_string(arguments[0]);
                    }
                    else
                    {
                        index = static_cast<std::size_t>(std::max<long long>(0LL, std::llround(value_as_number(arguments[0]))));
                    }
                    const std::string designator = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    return make_number_value(static_cast<double>(field_size_callback_(field_name, index, designator)));
                }
                if (function == "reccount")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_number_value(static_cast<double>(record_count_callback_(designator)));
                }
                if (function == "recsize" || function == "reclength")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_number_value(static_cast<double>(record_length_callback_(designator)));
                }
                if (function == "recno")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_number_value(static_cast<double>(recno_callback_(designator)));
                }
                if (function == "found")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_boolean_value(found_callback_(designator));
                }
                if (function == "eof")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_boolean_value(eof_callback_(designator));
                }
                if (function == "bof")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_boolean_value(bof_callback_(designator));
                }
                if (function == "rlock" || function == "flock" || function == "lock" ||
                    function == "isrlocked" || function == "isflocked")
                {
                    return lock_function_callback_(function, raw_arguments, arguments);
                }
                if (function == "deleted")
                {
                    const auto deleted_value = field_lookup_callback_(arguments.empty() ? std::string{"deleted"} : value_as_string(arguments[0]) + ".deleted");
                    return deleted_value.has_value() ? *deleted_value : make_boolean_value(false);
                }
                if (function == "order")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    const bool include_path = arguments.size() >= 2U && std::abs(value_as_number(arguments[1])) > 0.000001;
                    return make_string_value(order_callback_(designator, include_path));
                }
                if (function == "tag")
                {
                    const std::string first = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    std::size_t tag_number = 1U;
                    std::string designator;
                    std::string index_file_name;

                    if (!first.empty() && is_index_file_path(first))
                    {
                        index_file_name = first;
                        if (arguments.size() >= 2U)
                        {
                            tag_number = static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[1])));
                        }
                        if (arguments.size() >= 3U)
                        {
                            designator = value_as_string(arguments[2]);
                        }
                    }
                    else
                    {
                        if (!first.empty())
                        {
                            tag_number = static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[0])));
                        }
                        if (arguments.size() >= 2U)
                        {
                            designator = value_as_string(arguments[1]);
                        }
                    }

                    return make_string_value(tag_callback_(index_file_name, tag_number, designator));
                }
                if (function == "seek" && !arguments.empty())
                {
                    const std::string search_key = value_as_string(arguments[0]);
                    const std::string designator = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    const std::string order_designator = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                    return make_boolean_value(seek_callback_(search_key, true, designator, order_designator));
                }
                if (function == "indexseek" && !arguments.empty())
                {
                    const std::string search_key = value_as_string(arguments[0]);
                    const bool move_pointer = arguments.size() >= 2U && value_as_bool(arguments[1]);
                    const std::string designator = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                    const std::string order_designator = arguments.size() >= 4U ? value_as_string(arguments[3]) : std::string{};
                    return make_boolean_value(indexseek_callback_(search_key, move_pointer, designator, order_designator));
                }
                if (function == "foxtoolver")
                {
                    return make_string_value(foxtoolver_callback_());
                }
                if (function == "mainhwnd")
                {
                    return make_number_value(static_cast<double>(mainhwnd_callback_()));
                }
                if ((function == "regfn" || function == "regfn32") && arguments.size() >= 3U)
                {
                    const std::string function_name = value_as_string(arguments[0]);
                    const std::string argument_types = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    const std::string return_type = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                    const std::string dll_name = arguments.size() >= 4U ? value_as_string(arguments[3]) : std::string{};
                    return make_number_value(static_cast<double>(
                        regfn_callback_(function, function_name, argument_types, return_type, dll_name)));
                }
                if (function == "callfn" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    std::vector<PrgValue> call_arguments;
                    call_arguments.reserve(arguments.size() > 0U ? arguments.size() - 1U : 0U);
                    for (std::size_t index = 1U; index < arguments.size(); ++index)
                    {
                        call_arguments.push_back(arguments[index]);
                    }
                    return callfn_callback_(handle, call_arguments);
                }
                if ((function == "createobject" || function == "createobj") && !arguments.empty())
                {
                    const std::string prog_id = value_as_string(arguments[0]);
                    std::vector<PrgValue> create_arguments;
                    create_arguments.reserve(arguments.size() > 0U ? arguments.size() - 1U : 0U);
                    for (std::size_t index = 1U; index < arguments.size(); ++index)
                    {
                        create_arguments.push_back(arguments[index]);
                    }
                    std::vector<std::optional<std::string>> create_argument_references;
                    create_argument_references.reserve(argument_references.size() > 0U ? argument_references.size() - 1U : 0U);
                    for (std::size_t index = 1U; index < argument_references.size(); ++index)
                    {
                        create_argument_references.push_back(argument_references[index]);
                    }
                    const int handle = register_ole_callback_(prog_id, "createobject", create_arguments, create_argument_references);
                    if (handle == 0)
                    {
                        return make_null_value();
                    }
                    record_event_callback_("ole.createobject", prog_id);
                    return make_string_value("object:" + prog_id + "#" + std::to_string(handle));
                }
                if (function == "newobject" && !arguments.empty())
                {
                    const std::string class_name = value_as_string(arguments[0]);
                    const std::string library = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    const std::string server = arguments.size() >= 6U ? value_as_string(arguments[5]) : std::string{};
                    const auto looks_like_library_target = [](const std::string &candidate)
                    {
                        const auto has_suffix = [](const std::string &text, const char *suffix)
                        {
                            const std::size_t suffix_length = std::char_traits<char>::length(suffix);
                            return text.size() >= suffix_length &&
                                   text.compare(text.size() - suffix_length, suffix_length, suffix) == 0;
                        };
                        const std::string trimmed = trim_copy(candidate);
                        if (trimmed.empty())
                        {
                            return false;
                        }
                        if (trimmed.find('/') != std::string::npos ||
                            trimmed.find('\\') != std::string::npos ||
                            trimmed.find(':') != std::string::npos)
                        {
                            return true;
                        }

                        const std::string normalized = lowercase_copy(trimmed);
                        return has_suffix(normalized, ".prg") ||
                               has_suffix(normalized, ".vcx") ||
                               has_suffix(normalized, ".dll") ||
                               has_suffix(normalized, ".ocx") ||
                               has_suffix(normalized, ".exe") ||
                               has_suffix(normalized, ".so") ||
                               has_suffix(normalized, ".dylib") ||
                               has_suffix(normalized, ".fll");
                    };

                    const bool explicit_server = !trim_copy(server).empty();
                    const bool explicit_native_prg_library =
                        !explicit_server &&
                        lowercase_copy(copperfin::platform::path_to_utf8_string(
                            copperfin::platform::path_from_utf8_string(trim_copy(library)).extension())) == ".prg";
                    const bool explicit_local_vcx_library =
                        !explicit_server &&
                        lowercase_copy(copperfin::platform::path_to_utf8_string(
                            copperfin::platform::path_from_utf8_string(trim_copy(library)).extension())) == ".vcx";
                    const bool library_looks_explicit = looks_like_library_target(library);
                    const bool bare_native_candidate = !explicit_server && (!trim_copy(library).empty() ? !library_looks_explicit : true);

                    std::vector<PrgValue> constructor_arguments;
                    std::vector<std::optional<std::string>> constructor_argument_references;
                    if (bare_native_candidate || explicit_native_prg_library || explicit_local_vcx_library)
                    {
                        const std::size_t constructor_start_index =
                            (explicit_native_prg_library || explicit_local_vcx_library) ? 2U : 1U;
                        constructor_arguments.reserve(arguments.size() > constructor_start_index ? arguments.size() - constructor_start_index : 0U);
                        constructor_argument_references.reserve(argument_references.size() > constructor_start_index ? argument_references.size() - constructor_start_index : 0U);
                        for (std::size_t index = constructor_start_index; index < arguments.size(); ++index)
                        {
                            constructor_arguments.push_back(arguments[index]);
                            constructor_argument_references.push_back(
                                index < argument_references.size()
                                    ? argument_references[index]
                                    : std::optional<std::string>{});
                        }
                    }

                    std::string source = bare_native_candidate ? std::string{"newobject"} : (library.empty() ? "newobject" : library);
                    if (explicit_server)
                    {
                        source += "@" + server;
                    }
                    const int handle = register_ole_callback_(class_name, source, constructor_arguments, constructor_argument_references);
                    std::string detail = class_name;
                    if (!library.empty())
                    {
                        detail += ":" + library;
                    }
                    if (!trim_copy(server).empty())
                    {
                        detail += "@" + server;
                    }
                    if (handle == 0)
                    {
                        return make_null_value();
                    }
                    record_event_callback_(
                        explicit_local_vcx_library ? "prg.object.newobject" : "ole.newobject",
                        detail);
                    return make_string_value("object:" + class_name + "#" + std::to_string(handle));
                }
                if (function == "getobject" && !arguments.empty())
                {
                    const std::string source = value_as_string(arguments[0]);
                    const std::string class_name = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    const std::string resolved_prog_id = trim_copy(class_name).empty() ? source : class_name;
                    const std::string source_tag = trim_copy(source).empty() ? "getobject" : "getobject:" + source;
                    const int handle = register_ole_callback_(resolved_prog_id, source_tag, {}, {});
                    record_event_callback_(
                        "ole.getobject",
                        trim_copy(class_name).empty() ? source : source + " -> " + class_name);
                    return make_string_value("object:" + resolved_prog_id + "#" + std::to_string(handle));
                }
                if ((function == "sqlconnect" || function == "sqlstringconnect") && !arguments.empty())
                {
                    const std::string target = value_as_string(arguments[0]);
                    const int handle = sql_connect_callback_(target, function);
                    record_event_callback_("sql.connect", function + ":" + target + " -> " + std::to_string(handle));
                    return make_number_value(static_cast<double>(handle));
                }
                if (function == "sqlexec" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    const std::string command = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    const std::string cursor_alias = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                    return make_number_value(static_cast<double>(sql_exec_callback_(handle, command, cursor_alias)));
                }
                if (function == "sqldisconnect" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    const bool ok = sql_disconnect_callback_(handle);
                    if (ok)
                    {
                        record_event_callback_("sql.disconnect", std::to_string(handle));
                    }
                    return make_number_value(ok ? 1.0 : -1.0);
                }
                if (function == "sqlrowcount" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    return make_number_value(static_cast<double>(sql_row_count_callback_(handle)));
                }
                if (function == "sqlprepare" && arguments.size() >= 2U)
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    return make_number_value(static_cast<double>(sql_prepare_callback_(handle, value_as_string(arguments[1]))));
                }
                if (function == "sqlcancel" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    return make_number_value(static_cast<double>(sql_cancel_callback_(handle)));
                }
                if (function == "sqlcommit" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    return make_number_value(static_cast<double>(sql_commit_callback_(handle)));
                }
                if (function == "sqlrollback" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    return make_number_value(static_cast<double>(sql_rollback_callback_(handle)));
                }
                if (function == "sqldatabases" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    const std::string cursor_alias = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    return make_number_value(static_cast<double>(sql_databases_callback_(handle, cursor_alias)));
                }
                if (function == "sqlprimarykeys" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    const std::string table_name = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    const std::string cursor_alias = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                    return make_number_value(static_cast<double>(sql_primary_keys_callback_(handle, table_name, cursor_alias)));
                }
                if (function == "sqlforeignkeys" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    const std::string table_name = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    const std::string cursor_alias = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                    return make_number_value(static_cast<double>(sql_foreign_keys_callback_(handle, table_name, cursor_alias)));
                }
                if (function == "sqltables" && !arguments.empty())
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    const std::string table_types = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                    const std::string cursor_alias = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                    return make_number_value(static_cast<double>(sql_tables_callback_(handle, table_types, cursor_alias)));
                }
                if (function == "sqlcolumns" && arguments.size() >= 2U)
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    const std::string table_name = value_as_string(arguments[1]);
                    const std::string format = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                    const std::string cursor_alias = arguments.size() >= 4U ? value_as_string(arguments[3]) : std::string{};
                    return make_number_value(static_cast<double>(sql_columns_callback_(handle, table_name, format, cursor_alias)));
                }
                if (function == "sqlgetprop" && arguments.size() >= 2U)
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    return sql_get_prop_callback_(handle, value_as_string(arguments[1]));
                }
                if (function == "sqlsetprop" && arguments.size() >= 3U)
                {
                    const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
                    return make_number_value(static_cast<double>(sql_set_prop_callback_(handle, value_as_string(arguments[1]), arguments[2])));
                }
                if (const auto string_result = evaluate_string_function(function, arguments, exact_string_compare_, memowidth_callback_(), set_callback_))
                {
                    return *string_result;
                }
                // TEXTMERGE(cExpression [, lRecursive [, cLeftDelimiter, cRightDelimiter]])
                // Evaluate <<expr>> (or custom delimiters) interpolation within a string.
                if (function == "textmerge" && !arguments.empty())
                {
                    const std::string text = value_as_string(arguments[0]);
                    const bool recursive = arguments.size() >= 2U && value_as_bool(arguments[1]);
                    std::string left_delim = "<<";
                    std::string right_delim = ">>";
                    if (arguments.size() >= 3U)
                    {
                        left_delim = value_as_string(arguments[2]);
                        right_delim = arguments.size() >= 4U ? value_as_string(arguments[3]) : left_delim;
                    }
                    else
                    {
                        const std::string encoded_delimiters = set_callback_("__textmerge_delimiters__");
                        const std::size_t colon_position = encoded_delimiters.find(':');
                        if (colon_position != std::string::npos)
                        {
                            try
                            {
                                const std::size_t left_length = static_cast<std::size_t>(
                                    std::stoull(encoded_delimiters.substr(0U, colon_position)));
                                const std::string delimiters = encoded_delimiters.substr(colon_position + 1U);
                                if (left_length <= delimiters.size())
                                {
                                    left_delim = delimiters.substr(0U, left_length);
                                    right_delim = delimiters.substr(left_length);
                                }
                            }
                            catch (...)
                            {
                            }
                        }
                    }
                    if (left_delim.empty() || right_delim.empty())
                    {
                        return make_string_value(text);
                    }
                    auto apply_merge = [&](const std::string &src) -> std::string
                    {
                        std::string result;
                        std::size_t pos = 0U;
                        while (pos < src.size())
                        {
                            const auto left_pos = src.find(left_delim, pos);
                            if (left_pos == std::string::npos)
                            {
                                result += src.substr(pos);
                                break;
                            }
                            result += src.substr(pos, left_pos - pos);
                            const auto right_pos = src.find(right_delim, left_pos + left_delim.size());
                            if (right_pos == std::string::npos)
                            {
                                result += src.substr(left_pos);
                                break;
                            }
                            const std::string expr = src.substr(
                                left_pos + left_delim.size(),
                                right_pos - left_pos - left_delim.size());
                            try
                            {
                                result += value_as_string(eval_expression_callback_(expr));
                            }
                            catch (...)
                            {
                                result += left_delim + expr + right_delim;
                            }
                            pos = right_pos + right_delim.size();
                        }
                        return result;
                    };
                    std::string merged = apply_merge(text);
                    if (recursive)
                    {
                        constexpr std::size_t max_recursive_merges = 16U;
                        for (std::size_t depth = 0U; depth < max_recursive_merges; ++depth)
                        {
                            const std::string next = apply_merge(merged);
                            if (next == merged)
                            {
                                break;
                            }
                            merged = next;
                        }
                    }
                    return make_string_value(merged);
                }
                // EXECSCRIPT(cScript [, eParam1, ...])
                // Execute a string as a PRG script and return its return value.
                // First-pass: extracts RETURN <expr> pattern and evaluates the expression.
                // Multi-statement scripts without a detectable RETURN emit a runtime event.
                if (function == "execscript" && !arguments.empty())
                {
                    std::string script = value_as_string(arguments[0]);
                    // Trim leading/trailing whitespace and normalise line endings
                    auto lstrip = [](std::string s) -> std::string
                    {
                        const auto it = std::find_if(s.begin(), s.end(),
                            [](unsigned char c) { return std::isspace(c) == 0; });
                        return std::string(it, s.end());
                    };
                    auto rstrip = [](std::string s) -> std::string
                    {
                        const auto it = std::find_if(s.rbegin(), s.rend(),
                            [](unsigned char c) { return std::isspace(c) == 0; });
                        return std::string(s.begin(), it.base());
                    };
                    const std::string trimmed = rstrip(lstrip(script));
                    // Detect single RETURN <expr> (case-insensitive)
                    const std::string lower7 = trimmed.size() >= 7U
                        ? std::string{
                              static_cast<char>(std::tolower(static_cast<unsigned char>(trimmed[0]))),
                              static_cast<char>(std::tolower(static_cast<unsigned char>(trimmed[1]))),
                              static_cast<char>(std::tolower(static_cast<unsigned char>(trimmed[2]))),
                              static_cast<char>(std::tolower(static_cast<unsigned char>(trimmed[3]))),
                              static_cast<char>(std::tolower(static_cast<unsigned char>(trimmed[4]))),
                              static_cast<char>(std::tolower(static_cast<unsigned char>(trimmed[5]))),
                              static_cast<char>(std::tolower(static_cast<unsigned char>(trimmed[6])))
                          }
                        : std::string{};
                    if (lower7 == "return " && trimmed.find('\n') == std::string::npos)
                    {
                        const std::string return_expr = lstrip(trimmed.substr(7U));
                        try
                        {
                            return eval_expression_callback_(return_expr);
                        }
                        catch (...) {}
                    }
                    record_event_callback_(
                        "runtime.execscript",
                        trimmed.size() > 256U ? trimmed.substr(0U, 256U) + "..." : trimmed);
                    return make_empty_value();
                }
                // LOOKUP(eReturnExpr, eSearchExpr, cTableAlias [, cTagName])
                // Seek eSearchExpr in cTableAlias (optionally on cTagName index) then
                // evaluate eReturnExpr in the context of the found record.
                // The record pointer in the target cursor is permanently moved.
                if (function == "lookup" && arguments.size() >= 3U && !raw_arguments.empty())
                {
                    const auto resolve_lookup_text_argument = [&](std::size_t index) -> std::string
                    {
                        std::string resolved = value_as_string(arguments[index]);
                        if (index < raw_arguments.size())
                        {
                            const std::string raw_text = trim_copy(raw_arguments[index]);
                            if (!raw_text.empty() && raw_text.front() == '&')
                            {
                                std::string expanded_text;
                                const std::string macro_variable_text = trim_copy(raw_text.substr(1U));
                                const bool simple_macro_variable =
                                    !macro_variable_text.empty() &&
                                    std::all_of(
                                        macro_variable_text.begin(),
                                        macro_variable_text.end(),
                                        [](unsigned char ch)
                                        {
                                            return std::isalnum(ch) != 0 || ch == '_';
                                        });
                                if (simple_macro_variable)
                                {
                                    expanded_text = trim_copy(
                                        value_as_string(eval_expression_callback_("m." + macro_variable_text)));
                                    if (!expanded_text.empty())
                                    {
                                        constexpr std::size_t max_macro_text_depth = 16U;
                                        std::vector<std::string> visited_identifiers;
                                        visited_identifiers.reserve(8U);
                                        for (std::size_t depth = 0U; depth < max_macro_text_depth; ++depth)
                                        {
                                            if (!is_bare_identifier_text(expanded_text))
                                            {
                                                break;
                                            }
                                            const std::string normalized_identifier =
                                                normalize_memory_variable_identifier(expanded_text);
                                            if (std::find(
                                                    visited_identifiers.begin(),
                                                    visited_identifiers.end(),
                                                    normalized_identifier) != visited_identifiers.end())
                                            {
                                                break;
                                            }
                                            visited_identifiers.push_back(normalized_identifier);
                                            const std::string next_text = trim_copy(
                                                value_as_string(eval_expression_callback_("m." + expanded_text)));
                                            if (next_text.empty() || next_text == expanded_text)
                                            {
                                                break;
                                            }
                                            expanded_text = next_text;
                                        }
                                    }
                                }
                                if (expanded_text.empty())
                                {
                                    const PrgValue expanded = eval_expression_callback_(raw_text);
                                    expanded_text = trim_copy(value_as_string(expanded));
                                }
                                if (!expanded_text.empty())
                                {
                                    resolved = expanded_text;
                                }
                            }
                        }
                        return resolved;
                    };
                    const std::string return_expr_raw = raw_arguments[0];
                    const std::string return_expr_text =
                        !trim_copy(return_expr_raw).empty() && trim_copy(return_expr_raw).front() == '&'
                            ? resolve_lookup_text_argument(0U)
                            : return_expr_raw;
                    const std::string search_key =
                        raw_arguments.size() >= 2U &&
                        !trim_copy(raw_arguments[1]).empty() &&
                        trim_copy(raw_arguments[1]).front() == '&'
                            ? resolve_lookup_text_argument(1U)
                            : value_as_string(arguments[1]);
                    const std::string table_alias = resolve_lookup_text_argument(2U);
                    const std::string tag_name    = arguments.size() >= 4U ? resolve_lookup_text_argument(3U) : std::string{};
                    PrgValue pre = arguments[0];
                    try
                    {
                        if (!trim_copy(return_expr_text).empty())
                        {
                            pre = eval_expression_callback_(return_expr_text);
                        }
                    }
                    catch (...)
                    {
                    }
                    if (seek_callback_(search_key, /*move_pointer=*/true, table_alias, tag_name))
                    {
                        try
                        {
                            return eval_expression_callback_(return_expr_text);
                        }
                        catch (...)
                        {
                            return arguments[0]; // fallback to pre-seek value
                        }
                    }
                    // Not found — return typed default based on the fully-resolved return expr kind
                    if (pre.kind == PrgValueKind::number) return make_number_value(0.0);
                    if (pre.kind == PrgValueKind::currency) return make_currency_value(0);
                    if (pre.kind == PrgValueKind::boolean) return make_boolean_value(false);
                    return make_boolean_value(false);
                }
                if (const auto type_result = evaluate_type_function(function, arguments, array_exists_callback_, eval_expression_callback_))
                {
                    return *type_result;
                }
                if (const auto numeric_result = evaluate_numeric_function(function, arguments))
                {
                    return *numeric_result;
                }
                if (const auto date_time_result = evaluate_date_time_function(function, arguments, set_callback_))
                {
                    return *date_time_result;
                }
                // --- Array / variable helpers ---
                if (function == "alen" && !arguments.empty())
                {
                    const std::string array_name = resolve_array_argument_name(
                        raw_arguments.empty() ? std::string{} : raw_arguments[0],
                        &arguments[0]);
                    const int dimension = arguments.size() >= 2U ? static_cast<int>(value_as_number(arguments[1])) : 0;
                    return make_number_value(static_cast<double>(array_length_callback_(array_name, dimension)));
                }
                if ((function == "acopy" || function == "adel" || function == "adir" || function == "aelement" ||
                     function == "afields" || function == "afont" || function == "agetfileversion" ||
                     function == "ains" || function == "alines" || function == "aprinters" ||
                     function == "ascan" || function == "asessions" || function == "asize" ||
                     function == "asort" || function == "asubscript" || function == "aused") &&
                    !arguments.empty())
                {
                    return array_function_callback_(function, raw_arguments, arguments);
                }
                // --- Misc ---
                if (function == "pcount" || function == "parameters")
                {
                    return make_number_value(static_cast<double>(frame_.call_arguments.size()));
                }
                if (function == "getenv" && !arguments.empty())
                {
                    const std::string env_key = value_as_string(arguments[0]);
                    return make_string_value(get_environment_variable_value(env_key).value_or(std::string{}));
                }
                if (function == "putenv" && arguments.size() >= 2U)
                {
                    const std::string env_key = value_as_string(arguments[0]);
                    const std::string env_value = value_as_string(arguments[1]);
                    return make_boolean_value(set_environment_variable_value(env_key, env_value));
                }
                if (function == "txnlevel")
                {
                    const std::string level_text = trim_copy(set_callback_("TXNLEVEL"));
                    try
                    {
                        return make_number_value(std::stod(level_text.empty() ? "0" : level_text));
                    }
                    catch (...)
                    {
                        return make_number_value(0.0);
                    }
                }
                if (function == "dodefault")
                {
                    if (base_method_invoke_callback_)
                    {
                        const auto base_result =
                            base_method_invoke_callback_(arguments, argument_references);
                        return base_result.value_or(make_empty_value());
                    }
                    return make_empty_value();
                }
                if (const auto path_result = evaluate_path_function(function, arguments, default_directory_))
                {
                    return *path_result;
                }
                if (cursor_buffering_callback_)
                {
                    if (const auto cursor_buffering_result = cursor_buffering_callback_(function, arguments))
                    {
                        return *cursor_buffering_result;
                    }
                }
                if (const auto runtime_surface_result =
                        evaluate_runtime_surface_function(function,
                                                          arguments,
                                                          raw_arguments,
                                                          default_directory_,
                                                          frame_.file_path,
                                                          last_error_message_,
                                                          last_error_code_,
                                                          last_error_procedure_,
                                                          last_error_line_,
                                                          current_program_name_,
                                                          program_stack_depth_,
                                                          program_stack_frame_callback_,
                                                          error_handler_,
                                                          shutdown_handler_,
                                                          aerror_callback_,
                                                          eval_expression_callback_,
                                                          set_callback_,
                                                          snapshot_cursor_callback_,
                                                          load_cursor_snapshot_callback_,
                                                          require_verified_file_byte_overrides_,
                                                          read_verified_file_callback_,
                                                          resolve_object_callback_,
                                                          read_native_member_callback_,
                                                          write_native_member_callback_,
                                                          whandle_from_hwnd_callback_,
                                                          hwnd_from_whandle_callback_,
                                                          assign_array_callback_,
                                                          popup_prompt_callback_,
                                                          popup_bar_count_callback_,
                                                          popup_bar_position_callback_,
                                                          popup_bar_skip_callback_,
                                                          popup_bar_mark_callback_,
                                                          record_event_callback_))
                {
                    return *runtime_surface_result;
                }
                // --- Declared DLL function invocation ---
                if (declared_dll_invoke_callback_)
                {
                    const std::optional<PrgValue> dll_result =
                        declared_dll_invoke_callback_(function, arguments, argument_references);
                    if (dll_result.has_value())
                    {
                        return *dll_result;
                    }
                }
                if (user_routine_invoke_callback_)
                {
                    const auto user_routine_result =
                        user_routine_invoke_callback_(
                            function,
                            arguments,
                            raw_arguments,
                            argument_references,
                            invocation_start,
                            invocation_end);
                    if (user_routine_result.has_value())
                    {
                        return *user_routine_result;
                    }
                }
                return make_string_value(function);
            }

            PrgValue parse_array_element_access(const std::string &array_name, char close_delimiter)
            {
                const std::size_t row = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(parse_expression())));
                std::size_t column = 1U;
                skip_whitespace();
                if (match(","))
                {
                    column = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(parse_expression())));
                }
                match(std::string(1U, close_delimiter));
                return array_value_callback_(array_name, row, column);
            }

            PrgValue parse_indexed_identifier_access(const std::string &identifier, char close_delimiter)
            {
                const auto is_selector_style_native_member_name =
                    [](const std::string &member_name) -> bool
                {
                    const std::string normalized_member_name =
                        normalize_identifier(member_name);
                        return normalized_member_name == "list" ||
                               normalized_member_name == "listitem" ||
                               normalized_member_name == "itemdata" ||
                               normalized_member_name == "selected" ||
                           normalized_member_name == "selectedid" ||
                           normalized_member_name == "indextoitemid" ||
                           normalized_member_name == "itemidtoindex";
                };
                const PrgValue selector = parse_expression();
                skip_whitespace();
                std::optional<PrgValue> secondary_selector;
                if (match(","))
                {
                    secondary_selector = parse_expression();
                    skip_whitespace();
                }
                match(std::string(1U, close_delimiter));

                if (array_exists_callback_(identifier))
                {
                    const std::size_t row =
                        static_cast<std::size_t>(std::max<double>(0.0, value_as_number(selector)));
                    const std::size_t column =
                        secondary_selector.has_value()
                            ? static_cast<std::size_t>(std::max<double>(
                                  0.0,
                                  value_as_number(*secondary_selector)))
                            : 1U;
                    return array_value_callback_(identifier, row, column);
                }

                const PrgValue identifier_value = resolve_identifier(identifier);
                RuntimeOleObjectState *runtime_object =
                    resolve_object_callback_(identifier_value);
                if (runtime_object != nullptr &&
                    is_native_collection_object(*runtime_object))
                {
                    const auto item_value =
                        invoke_native_collection_method(*runtime_object, "item", {selector});
                    return item_value.value_or(make_empty_value());
                }

                const std::size_t member_separator = identifier.rfind('.');
                if (member_separator != std::string::npos && ole_invoke_callback_)
                {
                    const std::string raw_base_name = identifier.substr(0U, member_separator);
                    const std::string raw_member_path = identifier.substr(member_separator + 1U);
                    const std::string normalized_identifier = normalize_identifier(identifier);
                    const std::size_t normalized_separator = normalized_identifier.rfind('.');
                    const std::string base_name = normalized_identifier.substr(0U, normalized_separator);
                    const std::string member_path = normalized_identifier.substr(normalized_separator + 1U);
                    if (is_selector_style_native_member_name(raw_member_path))
                    {
                        PrgValue current = resolve_identifier(raw_base_name);
                        std::string selector_member_name =
                            raw_member_path + "[" + format_value(selector);
                        if (secondary_selector.has_value())
                        {
                            selector_member_name += ", " + format_value(*secondary_selector);
                        }
                        selector_member_name.push_back(']');
                        if (const auto selector_value =
                                read_native_member_callback_(current, selector_member_name);
                            selector_value.has_value())
                        {
                            return *selector_value;
                        }
                    }
                    std::vector<PrgValue> arguments{selector};
                    if (secondary_selector.has_value())
                    {
                        arguments.push_back(*secondary_selector);
                    }
                    return ole_invoke_callback_(base_name, member_path, arguments, {});
                }

                return make_empty_value();
            }

            PrgValue apply_postfix_member_and_collection_access(PrgValue current)
            {
                while (true)
                {
                    skip_whitespace();
                    if (match("("))
                    {
                        current = parse_native_collection_item_access(current, ')');
                        continue;
                    }

                    if (match("["))
                    {
                        current = parse_native_collection_item_access(current, ']');
                        continue;
                    }

                    if (starts_with_dotted_logical_operator())
                    {
                        return current;
                    }

                    if (!match("."))
                    {
                        return current;
                    }

                    const std::string member_name = parse_identifier();
                    if (member_name.empty())
                    {
                        return {};
                    }

                    skip_whitespace();
                    if (peek() == '[')
                    {
                        match("[");

                        const PrgValue selector = parse_expression();
                        skip_whitespace();
                        std::optional<PrgValue> secondary_selector;
                        if (match(","))
                        {
                            secondary_selector = parse_expression();
                            skip_whitespace();
                        }
                        match("]");

                        if (suppress_evaluation_)
                        {
                            current = make_empty_value();
                            continue;
                        }

                        std::string selector_member_name =
                            member_name + "[" + format_value(selector);
                        if (secondary_selector.has_value())
                        {
                            selector_member_name += ", " + format_value(*secondary_selector);
                        }
                        selector_member_name.push_back(']');

                        const auto selector_value =
                            read_native_member_callback_(current, selector_member_name);
                        if (selector_value.has_value())
                        {
                            current = *selector_value;
                            continue;
                        }

                        const auto member_value =
                            read_native_member_callback_(current, member_name);
                        if (member_value.has_value())
                        {
                            RuntimeOleObjectState *member_object =
                                resolve_object_callback_(*member_value);
                            if (member_object != nullptr &&
                                is_native_collection_object(*member_object))
                            {
                                const auto item_value =
                                    invoke_native_collection_method(*member_object, "item", {selector});
                                if (item_value.has_value())
                                {
                                    current = *item_value;
                                    continue;
                                }
                            }
                        }

                        if (invoke_native_member_callback_)
                        {
                            std::vector<PrgValue> selector_arguments{selector};
                            if (secondary_selector.has_value())
                            {
                                selector_arguments.push_back(*secondary_selector);
                            }
                            const auto invoked_value = invoke_native_member_callback_(
                                current,
                                member_name,
                                selector_arguments,
                                {});
                            if (invoked_value.has_value())
                            {
                                current = *invoked_value;
                                continue;
                            }
                        }

                        return {};
                    }

                    if (peek() == '(')
                    {
                        const auto member_value = read_native_member_callback_(current, member_name);
                        if (member_value.has_value())
                        {
                            RuntimeOleObjectState *member_object = resolve_object_callback_(*member_value);
                            if (member_object != nullptr && is_native_collection_object(*member_object))
                            {
                                match("(");
                                current = parse_native_collection_item_access(*member_value, ')');
                                continue;
                            }
                        }

                        if (!match("(") || !invoke_native_member_callback_)
                        {
                            return {};
                        }
                        const auto invocation = parse_invocation_arguments(member_name);
                        const auto invoked_value = invoke_native_member_callback_(
                            current,
                            member_name,
                            invocation.arguments,
                            invocation.argument_references);
                        if (!invoked_value.has_value())
                        {
                            return {};
                        }
                        current = *invoked_value;
                        continue;
                    }

                    const auto member_value = read_native_member_callback_(current, member_name);
                    if (!member_value.has_value())
                    {
                        return {};
                    }
                    current = *member_value;
                }
            }

            PrgValue parse_native_collection_item_access(const PrgValue &collection_value, char close_delimiter)
            {
                const PrgValue selector = parse_expression();
                return finish_native_collection_item_access(collection_value, selector, close_delimiter);
            }

            PrgValue finish_array_element_access(
                const std::string &array_name,
                const PrgValue &row_selector,
                char close_delimiter)
            {
                const std::size_t row = static_cast<std::size_t>(
                    std::max<double>(0.0, value_as_number(row_selector)));
                std::size_t column = 1U;
                skip_whitespace();
                if (match(","))
                {
                    column = static_cast<std::size_t>(
                        std::max<double>(0.0, value_as_number(parse_expression())));
                }
                match(std::string(1U, close_delimiter));
                return array_value_callback_(array_name, row, column);
            }

            PrgValue finish_native_collection_item_access(
                const PrgValue &collection_value,
                const PrgValue &selector,
                char close_delimiter)
            {
                skip_whitespace();
                if (match(","))
                {
                    (void)parse_expression();
                }
                match(std::string(1U, close_delimiter));

                RuntimeOleObjectState *runtime_object = resolve_object_callback_(collection_value);
                if (runtime_object == nullptr ||
                    !is_native_collection_object(*runtime_object))
                {
                    return {};
                }

                const auto item_value =
                    invoke_native_collection_method(*runtime_object, "item", {selector});
                return item_value.value_or(make_empty_value());
            }

            struct InvocationArguments
            {
                std::vector<PrgValue> arguments;
                std::vector<std::string> raw_arguments;
                std::vector<std::optional<std::string>> argument_references;
            };

            InvocationArguments parse_invocation_arguments(const std::string &identifier)
            {
                InvocationArguments invocation;
                skip_whitespace();
                if (match(")"))
                {
                    return invocation;
                }

                while (true)
                {
                    skip_whitespace();
                    const std::size_t argument_start = position_;
                    std::size_t argument_end = argument_start;
                    std::optional<std::string> argument_reference;

                    if (peek() == '@')
                    {
                        const std::size_t at_start = position_;
                        ++position_;
                        skip_whitespace();
                        const std::string reference_name = parse_identifier();
                        if (!reference_name.empty())
                        {
                            const std::size_t candidate_end = position_;
                            std::size_t lookahead = candidate_end;
                            while (lookahead < text_.size() &&
                                   std::isspace(static_cast<unsigned char>(text_[lookahead])) != 0)
                            {
                                ++lookahead;
                            }
                            const char delimiter = lookahead < text_.size() ? text_[lookahead] : '\0';
                            if (is_memory_variable_reference_text(reference_name) &&
                                (delimiter == ',' || delimiter == ')' || delimiter == '\0'))
                            {
                                argument_reference = reference_name;
                                argument_end = candidate_end;
                                position_ = candidate_end;
                                invocation.arguments.push_back(resolve_identifier(reference_name));
                                invocation.raw_arguments.push_back(
                                    trim_copy(text_.substr(argument_start, argument_end - argument_start)));
                                invocation.argument_references.push_back(std::move(argument_reference));
                                skip_whitespace();
                                if (match(")"))
                                {
                                    break;
                                }
                                if (!match(","))
                                {
                                    throw std::runtime_error(
                                        runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                                }
                                continue;
                            }
                        }
                        position_ = at_start;
                    }

                    invocation.arguments.push_back(parse_expression());
                    argument_end = position_;
                    if (argument_end == argument_start)
                    {
                        throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                    }

                    skip_whitespace();
                    if (normalize_identifier(identifier) == "cast" && invocation.arguments.size() == 1U)
                    {
                        const std::size_t as_start = position_;
                        const std::string remaining = lowercase_copy(text_.substr(position_));
                        if (remaining.rfind("as", 0U) == 0U &&
                            (remaining.size() == 2U || std::isspace(static_cast<unsigned char>(remaining[2])) != 0))
                        {
                            position_ += 2U;
                            skip_whitespace();
                            while (position_ < text_.size())
                            {
                                const char ch = text_[position_];
                                if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_')
                                {
                                    ++position_;
                                    continue;
                                }
                                break;
                            }
                            argument_end = position_;
                        }
                        else
                        {
                            position_ = as_start;
                        }
                    }
                    const std::string raw_argument =
                        trim_copy(text_.substr(argument_start, argument_end - argument_start));
                    if (is_memory_variable_reference_text(raw_argument) && array_exists_callback_(raw_argument))
                    {
                        invocation.arguments.back() = array_value_callback_(raw_argument, 1U, 1U);
                    }
                    invocation.argument_references.push_back(std::nullopt);
                    invocation.raw_arguments.push_back(raw_argument);
                    skip_whitespace();
                    if (match(")"))
                    {
                        break;
                    }
                    if (!match(","))
                    {
                        throw std::runtime_error(
                            runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                    }
                }

                return invocation;
            }

            std::string scan_invocation_argument_text()
            {
                skip_whitespace();
                const std::size_t argument_start = position_;
                std::size_t scan = position_;
                int parenthesis_depth = 0;
                int bracket_depth = 0;
                int brace_depth = 0;
                char string_delimiter = '\0';

                while (scan < text_.size())
                {
                    const char ch = text_[scan];
                    if (string_delimiter != '\0')
                    {
                        ++scan;
                        if (ch == string_delimiter)
                        {
                            if (scan < text_.size() && text_[scan] == string_delimiter)
                            {
                                ++scan;
                            }
                            else
                            {
                                string_delimiter = '\0';
                            }
                        }
                        continue;
                    }

                    if (ch == '\'' || ch == '"')
                    {
                        string_delimiter = ch;
                        ++scan;
                        continue;
                    }

                    if (ch == '(')
                    {
                        ++parenthesis_depth;
                        ++scan;
                        continue;
                    }
                    if (ch == ')')
                    {
                        if (parenthesis_depth == 0 && bracket_depth == 0 && brace_depth == 0)
                        {
                            break;
                        }
                        if (parenthesis_depth > 0)
                        {
                            --parenthesis_depth;
                        }
                        ++scan;
                        continue;
                    }
                    if (ch == '[')
                    {
                        ++bracket_depth;
                        ++scan;
                        continue;
                    }
                    if (ch == ']')
                    {
                        if (bracket_depth > 0)
                        {
                            --bracket_depth;
                        }
                        ++scan;
                        continue;
                    }
                    if (ch == '{')
                    {
                        ++brace_depth;
                        ++scan;
                        continue;
                    }
                    if (ch == '}')
                    {
                        if (brace_depth > 0)
                        {
                            --brace_depth;
                        }
                        ++scan;
                        continue;
                    }
                    if (ch == ',' && parenthesis_depth == 0 && bracket_depth == 0 && brace_depth == 0)
                    {
                        break;
                    }

                    ++scan;
                }

                position_ = scan;
                return trim_copy(text_.substr(argument_start, position_ - argument_start));
            }

            PrgValue parse_iif_invocation()
            {
                skip_whitespace();
                if (match(")"))
                {
                    throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                }

                const PrgValue condition = parse_expression();
                skip_whitespace();
                if (!match(","))
                {
                    throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                }

                const std::string true_branch_text = scan_invocation_argument_text();
                if (true_branch_text.empty())
                {
                    throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                }

                skip_whitespace();
                if (!match(","))
                {
                    throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                }

                const std::string false_branch_text = scan_invocation_argument_text();
                if (false_branch_text.empty())
                {
                    throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                }

                skip_whitespace();
                if (!match(")"))
                {
                    throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedClosingParenthesis"));
                }

                if (suppress_evaluation_)
                {
                    return make_empty_value();
                }

                return value_as_bool(condition)
                           ? eval_expression_callback_(true_branch_text)
                           : eval_expression_callback_(false_branch_text);
            }

            PrgValue parse_icase_invocation()
            {
                skip_whitespace();
                if (match(")"))
                {
                    throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                }

                std::vector<std::string> argument_texts;
                while (true)
                {
                    const std::string argument_text = scan_invocation_argument_text();
                    if (argument_text.empty())
                    {
                        throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                    }
                    argument_texts.push_back(argument_text);

                    skip_whitespace();
                    if (match(")"))
                    {
                        break;
                    }
                    if (!match(","))
                    {
                        throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedClosingParenthesis"));
                    }
                }

                if (argument_texts.size() < 2U)
                {
                    throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"));
                }

                if (suppress_evaluation_)
                {
                    return make_empty_value();
                }

                const bool has_otherwise_result = (argument_texts.size() % 2U) != 0U;
                const std::size_t paired_argument_count =
                    has_otherwise_result ? argument_texts.size() - 1U : argument_texts.size();
                for (std::size_t index = 0U; index < paired_argument_count; index += 2U)
                {
                    const PrgValue condition = eval_expression_callback_(argument_texts[index]);
                    if (value_as_bool(condition))
                    {
                        return eval_expression_callback_(argument_texts[index + 1U]);
                    }
                }

                if (has_otherwise_result)
                {
                    return eval_expression_callback_(argument_texts.back());
                }
                return make_null_value();
            }

            std::string resolve_array_argument_name(
                const std::string &raw_argument,
                const PrgValue *evaluated_argument) const
            {
                std::string candidate = trim_copy(raw_argument);
                if (!is_bare_identifier_text(candidate) &&
                    evaluated_argument != nullptr &&
                    evaluated_argument->kind == PrgValueKind::string)
                {
                    const std::string evaluated_name = trim_copy(value_as_string(*evaluated_argument));
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
                        if (array_exists_callback_(candidate))
                        {
                            break;
                        }
                        const std::string normalized = normalize_memory_variable_identifier(candidate);
                        if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized) != visited_identifiers.end())
                        {
                            break;
                        }
                        visited_identifiers.push_back(normalized);

                        const PrgValue indirect_value = resolve_identifier(candidate);
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
                return candidate;
            }

            PrgValue resolve_identifier(const std::string &identifier) const
            {
                const std::string normalized = normalize_memory_variable_identifier(identifier);
                const auto local = frame_.locals.find(normalized);
                if (local != frame_.locals.end())
                {
                    return local->second;
                }
                const auto global = globals_.find(normalized);
                if (global != globals_.end())
                {
                    return global->second;
                }
                if (starts_with_insensitive(normalize_identifier(identifier), "m."))
                {
                    if (normalized.find('.') != std::string::npos)
                    {
                        return ole_property_callback_(normalized);
                    }
                    return {};
                }
                if (const auto field = field_lookup_callback_(identifier))
                {
                    return *field;
                }
                if (normalize_identifier(identifier) == "_mline")
                {
                    return make_number_value(static_cast<double>(memowidth_callback_()));
                }
                if (normalized.find('.') != std::string::npos)
                {
                    return ole_property_callback_(normalized);
                }
                return {};
            }

            std::string parse_identifier()
            {
                skip_whitespace();
                const auto expand_embedded_macro_identifier = [&](const std::string &macro_identifier)
                {
                    std::string resolved = trim_copy(macro_identifier);
                    if (resolved.empty())
                    {
                        return resolved;
                    }

                    std::vector<std::string> visited_identifiers;
                    constexpr std::size_t max_macro_identifier_depth = 16U;
                    for (std::size_t depth = 0U; depth < max_macro_identifier_depth; ++depth)
                    {
                        const std::string normalized = normalize_memory_variable_identifier(resolved);
                        if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized) != visited_identifiers.end())
                        {
                            break;
                        }
                        visited_identifiers.push_back(normalized);

                        const auto local = frame_.locals.find(normalized);
                        if (local != frame_.locals.end())
                        {
                            const std::string next = trim_copy(value_as_string(local->second));
                            if (next.empty() || next == resolved)
                            {
                                break;
                            }
                            resolved = next;
                            continue;
                        }

                        const auto global = globals_.find(normalized);
                        if (global != globals_.end())
                        {
                            const std::string next = trim_copy(value_as_string(global->second));
                            if (next.empty() || next == resolved)
                            {
                                break;
                            }
                            resolved = next;
                            continue;
                        }

                        break;
                    }

                    return resolved;
                };

                // Check whether an embedded `&macro.` substitution is present
                // anywhere within the upcoming identifier token (e.g. `m&cType.ID`).
                // If so we must build the result dynamically; otherwise use the
                // fast direct-substring path.
                bool has_embedded_macro = false;
                {
                    std::size_t scan = position_;
                    while (scan < text_.size())
                    {
                        const char ch = text_[scan];
                        if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_' || ch == '.')
                        {
                            ++scan;
                            continue;
                        }
                        if (ch == '&')
                        {
                            has_embedded_macro = true;
                        }
                        break;
                    }
                }

                if (!has_embedded_macro)
                {
                    // Fast path: no embedded macro.
                    const std::size_t start = position_;
                    while (position_ < text_.size())
                    {
                        const char ch = text_[position_];
                        if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_' || ch == '.')
                        {
                            ++position_;
                            continue;
                        }
                        break;
                    }
                    return text_.substr(start, position_ - start);
                }

                // Slow path: build identifier with embedded macro expansion.
                // Example: `m&cType.ID` with cType="Customer" → "mCustomerID".
                std::string result;
                while (position_ < text_.size())
                {
                    const char ch = text_[position_];
                    if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_' || ch == '.')
                    {
                        result += ch;
                        ++position_;
                        continue;
                    }
                    if (ch == '&')
                    {
                        ++position_; // consume '&'
                        const std::size_t macro_start = position_;
                        while (position_ < text_.size())
                        {
                            const char mc = text_[position_];
                            if (std::isalnum(static_cast<unsigned char>(mc)) != 0 || mc == '_')
                            {
                                ++position_;
                                continue;
                            }
                            break;
                        }
                        const std::string emb_macro_name = text_.substr(macro_start, position_ - macro_start);
                        if (!emb_macro_name.empty())
                        {
                            // Consume the dot terminator after the embedded macro name.
                            if (position_ < text_.size() && text_[position_] == '.')
                            {
                                ++position_;
                            }
                            result += expand_embedded_macro_identifier(
                                trim_copy(value_as_string(resolve_identifier(emb_macro_name))));
                        }
                        continue;
                    }
                    break;
                }
                return result;
            }

            std::string parse_braced_literal()
            {
                skip_whitespace();
                if (peek() != '{')
                {
                    return {};
                }
                const std::size_t start = position_;
                std::size_t depth = 0U;
                while (position_ < text_.size())
                {
                    const char ch = text_[position_++];
                    if (ch == '\'' || ch == '"')
                    {
                        const char delimiter = ch;
                        while (position_ < text_.size())
                        {
                            const char string_ch = text_[position_++];
                            if (string_ch == delimiter)
                            {
                                if (position_ < text_.size() && text_[position_] == delimiter)
                                {
                                    ++position_;
                                    continue;
                                }
                                break;
                            }
                        }
                        continue;
                    }
                    if (ch == '{')
                    {
                        ++depth;
                        continue;
                    }
                    if (ch == '}')
                    {
                        if (depth == 0U)
                        {
                            break;
                        }
                        --depth;
                        if (depth == 0U)
                        {
                            break;
                        }
                    }
                }
                return text_.substr(start, position_ - start);
            }

            void skip_macro_reference()
            {
                skip_whitespace();
                while (position_ < text_.size())
                {
                    const char ch = text_[position_];
                    if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_')
                    {
                        ++position_;
                        continue;
                    }
                    break;
                }

                if (peek() != '.')
                {
                    return;
                }

                ++position_;
                if (peek() == '.')
                {
                    ++position_;
                    while (position_ < text_.size())
                    {
                        const char ch = text_[position_];
                        if (std::isalnum(static_cast<unsigned char>(ch)) != 0 ||
                            ch == '_' ||
                            ch == '.' ||
                            ch == '&')
                        {
                            ++position_;
                            continue;
                        }
                        break;
                    }
                    return;
                }

                while (position_ < text_.size())
                {
                    const char ch = text_[position_];
                    if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_')
                    {
                        ++position_;
                        continue;
                    }
                    break;
                }
            }

            PrgValue parse_macro_reference(std::size_t macro_start)
            {
                const auto expand_memory_macro_identifier = [&](const std::string &macro_identifier)
                {
                    std::string resolved = trim_copy(macro_identifier);
                    if (resolved.empty())
                    {
                        return resolved;
                    }

                    std::vector<std::string> visited_identifiers;
                    constexpr std::size_t max_macro_identifier_depth = 16U;
                    for (std::size_t depth = 0U; depth < max_macro_identifier_depth; ++depth)
                    {
                        const std::string normalized = normalize_memory_variable_identifier(resolved);
                        if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized) != visited_identifiers.end())
                        {
                            break;
                        }
                        visited_identifiers.push_back(normalized);

                        const auto local = frame_.locals.find(normalized);
                        if (local != frame_.locals.end())
                        {
                            const std::string next = trim_copy(value_as_string(local->second));
                            if (next.empty() || next == resolved)
                            {
                                break;
                            }
                            resolved = next;
                            continue;
                        }

                        const auto global = globals_.find(normalized);
                        if (global != globals_.end())
                        {
                            const std::string next = trim_copy(value_as_string(global->second));
                            if (next.empty() || next == resolved)
                            {
                                break;
                            }
                            resolved = next;
                            continue;
                        }

                        break;
                    }

                    return resolved;
                };
                const auto contains_unquoted_double_angle = [](const std::string &expression)
                {
                    char quote_delimiter = '\0';
                    for (std::size_t index = 0U; index + 1U < expression.size(); ++index)
                    {
                        const char ch = expression[index];
                        if (quote_delimiter != '\0')
                        {
                            if (ch == quote_delimiter)
                            {
                                if ((index + 1U) < expression.size() && expression[index + 1U] == quote_delimiter)
                                {
                                    ++index;
                                    continue;
                                }
                                quote_delimiter = '\0';
                            }
                            continue;
                        }

                        if (ch == '\'' || ch == '"')
                        {
                            quote_delimiter = ch;
                            continue;
                        }
                        if (ch == '<' && expression[index + 1U] == '<')
                        {
                            return true;
                        }
                    }
                    return false;
                };

                skip_whitespace();
                const std::size_t start = position_;
                while (position_ < text_.size())
                {
                    const char ch = text_[position_];
                    if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_')
                    {
                        ++position_;
                        continue;
                    }
                    break;
                }

                const std::string macro_name = text_.substr(start, position_ - start);
                if (macro_name.empty())
                {
                    return make_empty_value();
                }

                // Consume the dot terminator in the `&stem.suffix` form.  The dot
                // separates the macro variable name from the literal continuation of
                // the identifier.  Any alphanumeric / underscore characters that
                // immediately follow the dot are the suffix; they are appended
                // verbatim to the expanded stem value before further evaluation.
                std::string dot_suffix;
                if (peek() == '.')
                {
                    ++position_; // consume the dot terminator
                    if (peek() == '.')
                    {
                        dot_suffix.push_back('.');
                        ++position_;
                        while (position_ < text_.size())
                        {
                            const char sch = text_[position_];
                            if (sch == '&')
                            {
                                ++position_;
                                const std::size_t embedded_macro_start = position_;
                                while (position_ < text_.size())
                                {
                                    const char mch = text_[position_];
                                    if (std::isalnum(static_cast<unsigned char>(mch)) != 0 || mch == '_')
                                    {
                                        ++position_;
                                        continue;
                                    }
                                    break;
                                }

                                const std::string embedded_macro_name = text_.substr(embedded_macro_start, position_ - embedded_macro_start);
                                if (embedded_macro_name.empty())
                                {
                                    break;
                                }

                                dot_suffix += expand_memory_macro_identifier(embedded_macro_name);
                                continue;
                            }
                            if (std::isalnum(static_cast<unsigned char>(sch)) != 0 || sch == '_' || sch == '.')
                            {
                                dot_suffix.push_back(sch);
                                ++position_;
                                continue;
                            }
                            break;
                        }
                    }
                    else
                    {
                        const std::size_t suffix_start = position_;
                        while (position_ < text_.size())
                        {
                            const char sch = text_[position_];
                            if (std::isalnum(static_cast<unsigned char>(sch)) != 0 || sch == '_')
                            {
                                ++position_;
                                continue;
                            }
                            break;
                        }
                        dot_suffix = text_.substr(suffix_start, position_ - suffix_start);
                    }
                }

                const std::string expanded = expand_memory_macro_identifier(
                    trim_copy(value_as_string(resolve_identifier(macro_name))));
                if (expanded.empty())
                {
                    return make_empty_value();
                }

                // Concatenate the literal suffix onto the expanded stem.
                std::string resolved_expression = expanded + dot_suffix;
                std::vector<std::string> visited_macros;
                visited_macros.reserve(8U);
                constexpr std::size_t max_macro_expansion_depth = 16U;
                for (std::size_t depth = 0U; depth < max_macro_expansion_depth; ++depth)
                {
                    const std::string trimmed_expression = trim_copy(resolved_expression);
                    if (!is_bare_identifier_text(trimmed_expression))
                    {
                        resolved_expression = trimmed_expression;
                        break;
                    }

                    const std::string normalized_expression = normalize_memory_variable_identifier(trimmed_expression);
                    bool already_visited = false;
                    for (const std::string &visited_macro : visited_macros)
                    {
                        if (visited_macro == normalized_expression)
                        {
                            already_visited = true;
                            break;
                        }
                    }
                    if (already_visited)
                    {
                        resolved_expression = trimmed_expression;
                        break;
                    }

                    visited_macros.push_back(normalized_expression);
                    const PrgValue indirect_value = resolve_identifier(trimmed_expression);
                    if (indirect_value.kind == PrgValueKind::empty)
                    {
                        resolved_expression = trimmed_expression;
                        break;
                    }

                    const std::string indirect_expression = trim_copy(value_as_string(indirect_value));
                    if (indirect_expression.empty())
                    {
                        return make_empty_value();
                    }

                    resolved_expression = indirect_expression;
                }

                if (preserve_macro_text_)
                {
                    if (evaluate_preserved_macro_text_ &&
                        !contains_unquoted_double_angle(resolved_expression))
                    {
                        try
                        {
                            const PrgValue expanded_value = eval_expression_callback_(resolved_expression);
                            if (expanded_value.kind != PrgValueKind::empty)
                            {
                                return expanded_value;
                            }
                        }
                        catch (...)
                        {
                        }
                    }
                    return make_string_value(resolved_expression);
                }

                // Macro substitution is textual in VFP: replace the reference before the
                // surrounding precedence parser consumes its next operator. Keep the legacy
                // literal fallback for unresolved TEXTMERGE markers, which are not expression
                // syntax and must remain ordinary string values.
                if (!contains_unquoted_double_angle(resolved_expression))
                {
                    text_.replace(macro_start, position_ - macro_start, resolved_expression);
                    position_ = macro_start;
                    if (expression_continuation_ != nullptr)
                    {
                        expression_continuation_->primary_checkpoints.clear();
                    }
                    const PrgValue expanded_value = parse_unary();
                    if (expanded_value.kind == PrgValueKind::empty &&
                        is_bare_identifier_text(resolved_expression))
                    {
                        return make_string_value(resolved_expression);
                    }
                    return expanded_value;
                }

                return make_string_value(resolved_expression);
            }

            std::string parse_string()
            {
                std::string result;
                skip_whitespace();
                if (position_ >= text_.size())
                {
                    return result;
                }
                const char delimiter = text_[position_];
                if (delimiter != '\'' && delimiter != '"')
                {
                    return result;
                }
                ++position_;
                while (position_ < text_.size())
                {
                    const char ch = text_[position_++];
                    if (ch == delimiter)
                    {
                        if (position_ < text_.size() && text_[position_] == delimiter)
                        {
                            result.push_back(delimiter);
                            ++position_;
                            continue;
                        }
                        break;
                    }
                    result.push_back(ch);
                }
                return result;
            }

            std::string parse_bracket_literal()
            {
                std::string result;
                skip_whitespace();
                if (position_ >= text_.size() || text_[position_] != '[')
                {
                    return result;
                }
                ++position_;
                while (position_ < text_.size())
                {
                    const char ch = text_[position_++];
                    if (ch == ']')
                    {
                        if (position_ < text_.size() && text_[position_] == ']')
                        {
                            result.push_back(']');
                            ++position_;
                            continue;
                        }
                        break;
                    }
                    result.push_back(ch);
                }
                return result;
            }

            double parse_number()
            {
                const std::size_t start = position_;
                while (position_ < text_.size())
                {
                    const char ch = text_[position_];
                    if (std::isdigit(static_cast<unsigned char>(ch)) != 0 || ch == '.')
                    {
                        ++position_;
                        continue;
                    }
                    break;
                }
                return std::stod(text_.substr(start, position_ - start));
            }

            bool match(const std::string &value)
            {
                skip_whitespace();
                if (text_.compare(position_, value.size(), value) == 0)
                {
                    position_ += value.size();
                    return true;
                }
                return false;
            }

            bool match_logical_and_operator()
            {
                return match_dotted_keyword(".AND.") || match_identifier_keyword("AND");
            }

            bool match_logical_or_operator()
            {
                return match_dotted_keyword(".OR.") || match_identifier_keyword("OR");
            }

            bool match_dotted_keyword(const std::string& dotted_keyword)
            {
                skip_whitespace();
                if (position_ + dotted_keyword.size() > text_.size())
                {
                    return false;
                }

                for (std::size_t index = 0; index < dotted_keyword.size(); ++index)
                {
                    const unsigned char actual = static_cast<unsigned char>(text_[position_ + index]);
                    const unsigned char expected = static_cast<unsigned char>(dotted_keyword[index]);
                    if (std::tolower(actual) != std::tolower(expected))
                    {
                        return false;
                    }
                }

                position_ += dotted_keyword.size();
                return true;
            }

            bool match_identifier_keyword(const std::string& keyword)
            {
                skip_whitespace();
                if (position_ + keyword.size() > text_.size())
                {
                    return false;
                }

                for (std::size_t index = 0; index < keyword.size(); ++index)
                {
                    const unsigned char actual = static_cast<unsigned char>(text_[position_ + index]);
                    const unsigned char expected = static_cast<unsigned char>(keyword[index]);
                    if (std::tolower(actual) != std::tolower(expected))
                    {
                        return false;
                    }
                }

                const auto is_identifier_char = [](char ch)
                {
                    return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_' || ch == '.';
                };
                if (position_ > 0 && is_identifier_char(text_[position_ - 1]))
                {
                    return false;
                }
                const std::size_t end = position_ + keyword.size();
                if (end < text_.size() && is_identifier_char(text_[end]))
                {
                    return false;
                }

                position_ = end;
                return true;
            }

            bool starts_with_dotted_logical_operator() const
            {
                const auto starts_with_insensitive = [&](std::string_view token)
                {
                    if (position_ + token.size() > text_.size())
                    {
                        return false;
                    }
                    for (std::size_t index = 0; index < token.size(); ++index)
                    {
                        const unsigned char actual = static_cast<unsigned char>(text_[position_ + index]);
                        const unsigned char expected = static_cast<unsigned char>(token[index]);
                        if (std::tolower(actual) != std::tolower(expected))
                        {
                            return false;
                        }
                    }
                    return true;
                };

                return starts_with_insensitive(".AND.") || starts_with_insensitive(".OR.");
            }

            PrgValue skip_postfix_member_and_collection_access()
            {
                while (true)
                {
                    skip_whitespace();
                    if (match("("))
                    {
                        (void)parse_expression();
                        skip_whitespace();
                        while (match(","))
                        {
                            (void)parse_expression();
                            skip_whitespace();
                        }
                        match(")");
                        continue;
                    }

                    if (match("["))
                    {
                        (void)parse_expression();
                        skip_whitespace();
                        if (match(","))
                        {
                            (void)parse_expression();
                        }
                        match("]");
                        continue;
                    }

                    if (starts_with_dotted_logical_operator())
                    {
                        return make_empty_value();
                    }

                    if (!match("."))
                    {
                        return make_empty_value();
                    }

                    const std::string member_name = parse_identifier();
                    if (member_name.empty())
                    {
                        return make_empty_value();
                    }

                    skip_whitespace();
                    if (match("("))
                    {
                        if (!match(")"))
                        {
                            while (true)
                            {
                                (void)parse_expression();
                                skip_whitespace();
                                if (match(")"))
                                {
                                    break;
                                }
                                match(",");
                            }
                        }
                        continue;
                    }
                }
            }

            char peek() const
            {
                return position_ < text_.size() ? text_[position_] : '\0';
            }

            void skip_whitespace()
            {
                while (position_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[position_])) != 0)
                {
                    ++position_;
                }
            }

            int compare_ordered_values(const PrgValue &left, const PrgValue &right) const
            {
                if (left.string_flavor != PrgStringFlavor::none ||
                    right.string_flavor != PrgStringFlavor::none)
                {
                    const auto comparison = compare_date_time_values(left, right, set_callback_);
                    if (!comparison.has_value())
                    {
                        throw std::runtime_error(
                            runtime_text("Runtime.Prg.Expression.Error.OperatorOperandTypeMismatch"));
                    }
                    return *comparison;
                }
                if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                {
                    const std::string left_value = value_as_string(left);
                    const std::string right_value = value_as_string(right);
                    return left_value < right_value ? -1 : (left_value > right_value ? 1 : 0);
                }
                if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                    (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                {
                    const std::int64_t left_value = static_cast<std::int64_t>(value_as_number(left));
                    const std::int64_t right_value = static_cast<std::int64_t>(value_as_number(right));
                    return left_value < right_value ? -1 : (left_value > right_value ? 1 : 0);
                }
                if (left.kind == PrgValueKind::currency && right.kind == PrgValueKind::currency)
                {
                    return left.currency_value < right.currency_value
                               ? -1
                               : (left.currency_value > right.currency_value ? 1 : 0);
                }
                const double left_value = value_as_number(left);
                const double right_value = value_as_number(right);
                return left_value < right_value ? -1 : (left_value > right_value ? 1 : 0);
            }

            bool values_equal(const PrgValue &left, const PrgValue &right) const
            {
                if (left.string_flavor != PrgStringFlavor::none &&
                    right.string_flavor != PrgStringFlavor::none)
                {
                    if (const auto comparison = compare_date_time_values(left, right, set_callback_);
                        comparison.has_value())
                    {
                        return *comparison == 0;
                    }
                }
                if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                {
                    const std::string left_value = value_as_string(left);
                    const std::string right_value = value_as_string(right);
                    if (exact_string_compare_)
                    {
                        return rtrim_space_copy(left_value) == rtrim_space_copy(right_value);
                    }
                    return left_value.rfind(right_value, 0U) == 0U;
                }
                if (left.kind == PrgValueKind::boolean || right.kind == PrgValueKind::boolean)
                {
                    return value_as_bool(left) == value_as_bool(right);
                }
                // Exact integer equality when both sides are integer kinds
                if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                    (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                {
                    return left.kind == PrgValueKind::int64
                               ? (right.kind == PrgValueKind::int64
                                      ? left.int64_value == right.int64_value
                                      : left.int64_value >= 0 && static_cast<std::uint64_t>(left.int64_value) == right.uint64_value)
                               : (right.kind == PrgValueKind::uint64
                                      ? left.uint64_value == right.uint64_value
                                      : right.int64_value >= 0 && left.uint64_value == static_cast<std::uint64_t>(right.int64_value));
                }
                if (left.kind == PrgValueKind::currency && right.kind == PrgValueKind::currency)
                {
                    return left.currency_value == right.currency_value;
                }
                return std::abs(value_as_number(left) - value_as_number(right)) < 0.000001;
            }

            int current_work_area_ = 1;
            std::function<int()> next_free_work_area_callback_;
            std::function<int(const std::string &)> resolve_work_area_callback_;
            std::function<std::string(const std::string &)> alias_lookup_callback_;
            std::function<bool(const std::string &)> used_callback_;
            std::function<std::string(const std::string &)> dbf_lookup_callback_;
            std::function<std::size_t(const std::string &)> field_count_callback_;
            std::function<std::string(std::size_t, const std::string &)> field_name_callback_;
            std::function<std::size_t(const std::string &, std::size_t, const std::string &)> field_size_callback_;
            std::function<std::size_t(const std::string &)> record_count_callback_;
            std::function<std::size_t(const std::string &)> record_length_callback_;
            std::function<std::size_t(const std::string &)> recno_callback_;
            std::function<bool(const std::string &)> found_callback_;
            std::function<bool(const std::string &)> eof_callback_;
            std::function<bool(const std::string &)> bof_callback_;
            std::function<PrgValue(const std::string &, const std::vector<std::string> &, const std::vector<PrgValue> &)> lock_function_callback_;
            std::function<std::optional<PrgValue>(const std::string &)> field_lookup_callback_;
            std::function<bool(const std::string &)> array_exists_callback_;
            std::function<std::size_t(const std::string &, int)> array_length_callback_;
            std::function<PrgValue(const std::string &, std::size_t, std::size_t)> array_value_callback_;
            std::function<PrgValue(const std::string &, const std::vector<std::string> &, const std::vector<PrgValue> &)> array_function_callback_;
            std::function<int(const std::string &)> aerror_callback_;
            std::function<PrgValue(const std::string &, const std::vector<std::string> &)> aggregate_callback_;
            std::function<std::string(const std::string &, bool)> order_callback_;
            std::function<std::string(const std::string &, std::size_t, const std::string &)> tag_callback_;
            std::function<bool(const std::string &, bool, const std::string &, const std::string &)> seek_callback_;
            std::function<bool(const std::string &, bool, const std::string &, const std::string &)> indexseek_callback_;
            std::function<std::string()> foxtoolver_callback_;
            std::function<int()> mainhwnd_callback_;
            std::function<int(const std::string &, const std::string &, const std::string &, const std::string &, const std::string &)> regfn_callback_;
            std::function<PrgValue(int, const std::vector<PrgValue> &)> callfn_callback_;
            std::function<int(const std::string &, const std::string &)> sql_connect_callback_;
            std::function<int(int, const std::string &, const std::string &)> sql_exec_callback_;
            std::function<bool(int)> sql_disconnect_callback_;
            std::function<int(int)> sql_row_count_callback_;
            std::function<int(int, const std::string &)> sql_prepare_callback_;
            std::function<int(int)> sql_cancel_callback_;
            std::function<int(int)> sql_commit_callback_;
            std::function<int(int)> sql_rollback_callback_;
            std::function<int(int, const std::string &)> sql_databases_callback_;
            std::function<int(int, const std::string &, const std::string &)> sql_primary_keys_callback_;
            std::function<int(int, const std::string &, const std::string &)> sql_foreign_keys_callback_;
            std::function<int(int, const std::string &, const std::string &)> sql_tables_callback_;
            std::function<int(int, const std::string &, const std::string &, const std::string &)> sql_columns_callback_;
            std::function<PrgValue(int, const std::string &)> sql_get_prop_callback_;
            std::function<int(int, const std::string &, const PrgValue &)> sql_set_prop_callback_;
            std::function<int(const std::string &, const std::string &, const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> register_ole_callback_;
            std::function<PrgValue(const std::string &, const std::string &, const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> ole_invoke_callback_;
            std::function<PrgValue(const std::string &)> ole_property_callback_;
            std::function<PrgValue(const std::string &)> eval_expression_callback_;
            std::function<std::string(const std::string &)> set_callback_;
            std::function<std::optional<RuntimeSurfaceCursorSnapshot>(const std::string &)> snapshot_cursor_callback_;
            std::function<std::optional<std::size_t>(const RuntimeSurfaceCursorSnapshot &, const std::string &)> load_cursor_snapshot_callback_;
            bool require_verified_file_byte_overrides_ = false;
            std::function<std::optional<std::string>(const std::filesystem::path &)> read_verified_file_callback_;
            std::function<std::optional<PrgValue>(const std::string &, const std::vector<PrgValue> &)> cursor_buffering_callback_;
            std::function<void(const std::string &, const std::string &)> record_event_callback_;
            std::function<RuntimeOleObjectState*(const PrgValue &)> resolve_object_callback_;
            std::function<RuntimeOleObjectState*(const std::string &)> resolve_object_path_callback_;
            std::function<std::optional<PrgValue>(const PrgValue &, const std::string &)> read_native_member_callback_;
            std::function<std::optional<PrgValue>(const PrgValue &, const std::string &, const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> invoke_native_member_callback_;
            std::function<bool(const PrgValue &, const std::string &, const PrgValue &)> write_native_member_callback_;
            std::function<std::optional<std::int64_t>(std::int64_t)> whandle_from_hwnd_callback_;
            std::function<std::optional<std::int64_t>(std::int64_t)> hwnd_from_whandle_callback_;
            std::function<void(const std::string &, std::vector<PrgValue>)> assign_array_callback_;
            std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_prompt_callback_;
            std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_bar_count_callback_;
            std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_bar_position_callback_;
            std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_bar_skip_callback_;
            std::function<std::optional<PrgValue>(const std::vector<PrgValue> &)> popup_bar_mark_callback_;
            std::function<PrgValue(const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> bindevent_callback_;
            std::function<PrgValue(const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> raiseevent_callback_;
            std::function<PrgValue(const std::vector<PrgValue> &)> unbindevents_callback_;
            std::function<PrgValue(const std::vector<PrgValue> &, const std::vector<std::string> &)> aevents_callback_;
            std::function<std::size_t()> memowidth_callback_;
            std::function<std::optional<PrgValue>(const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> base_method_invoke_callback_;
            std::function<std::optional<PrgValue>(const std::string &, const std::vector<PrgValue> &, const std::vector<std::string> &, const std::vector<std::optional<std::string>> &, std::size_t, std::size_t)> user_routine_invoke_callback_;
            std::function<std::optional<PrgValue>(const std::string &, const std::vector<PrgValue> &, const std::vector<std::optional<std::string>> &)> declared_dll_invoke_callback_;
            ExpressionContinuation *expression_continuation_ = nullptr;
            std::string text_;
            bool preserve_macro_text_ = false;
            bool evaluate_preserved_macro_text_ = true;
            const Frame &frame_;
            const std::map<std::string, PrgValue> &globals_;
            const std::string &default_directory_;
            const std::string &last_error_message_;
            int last_error_code_ = 0;
            const std::string &last_error_procedure_;
            std::size_t last_error_line_ = 0;
            const std::string &current_program_name_;
            std::size_t program_stack_depth_ = 0;
            std::function<std::optional<RuntimeProgramStackFrame>(long long)> program_stack_frame_callback_;
            const std::string &error_handler_;
            const std::string &shutdown_handler_;
            bool exact_string_compare_ = false;
            bool suppress_evaluation_ = false;
            std::size_t position_ = 0;
        };

    } // namespace
