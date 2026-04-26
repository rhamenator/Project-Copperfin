// prg_engine_expression.inl
// ExpressionParser: inline expression evaluation helper.
// This file is #included directly into prg_engine.cpp inside namespace copperfin::runtime.
// It must not be compiled separately.

    std::optional<PrgValue> evaluate_date_time_function(
        const std::string& function,
        const std::vector<PrgValue>& arguments);
    std::optional<PrgValue> evaluate_string_function(
        const std::string& function,
        const std::vector<PrgValue>& arguments,
        bool exact_string_compare,
        std::size_t memo_width = 50U);
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
        const std::string& error_handler,
        const std::function<int(const std::string&)>& aerror_callback,
        const std::function<PrgValue(const std::string&)>& eval_expression_callback,
        const std::function<std::string(const std::string&)>& set_callback);

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
                const std::string &error_handler,
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
                std::function<PrgValue(int, const std::string &)> sql_get_prop_callback,
                std::function<int(int, const std::string &, const PrgValue &)> sql_set_prop_callback,
                std::function<int(const std::string &, const std::string &)> register_ole_callback,
                std::function<PrgValue(const std::string &, const std::string &, const std::vector<PrgValue> &)> ole_invoke_callback,
                std::function<PrgValue(const std::string &)> ole_property_callback,
                std::function<PrgValue(const std::string &)> eval_expression_callback,
                std::function<std::string(const std::string &)> set_callback,
                std::function<void(const std::string &, const std::string &)> record_event_callback,
                std::function<std::size_t()> memowidth_callback,
                std::function<PrgValue(const std::string &, const std::vector<PrgValue> &)> declared_dll_invoke_callback)
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
                  sql_get_prop_callback_(std::move(sql_get_prop_callback)),
                  sql_set_prop_callback_(std::move(sql_set_prop_callback)),
                  register_ole_callback_(std::move(register_ole_callback)),
                  ole_invoke_callback_(std::move(ole_invoke_callback)),
                  ole_property_callback_(std::move(ole_property_callback)),
                  eval_expression_callback_(std::move(eval_expression_callback)),
                  set_callback_(std::move(set_callback)),
                  record_event_callback_(std::move(record_event_callback)),
                  memowidth_callback_(std::move(memowidth_callback)),
                  declared_dll_invoke_callback_(std::move(declared_dll_invoke_callback)),
                  text_(text),
                  frame_(frame),
                  globals_(globals),
                  default_directory_(default_directory),
                  last_error_message_(last_error_message),
                  last_error_code_(last_error_code),
                  last_error_procedure_(last_error_procedure),
                  last_error_line_(last_error_line),
                  error_handler_(error_handler),
                  exact_string_compare_(exact_string_compare)
            {
            }

            PrgValue parse()
            {
                position_ = 0;
                PrgValue value = parse_comparison();
                skip_whitespace();
                return value;
            }

        private:
            PrgValue parse_comparison()
            {
                PrgValue left = parse_additive();
                while (true)
                {
                    skip_whitespace();
                    if (match("<>"))
                    {
                        left = make_boolean_value(!values_equal(left, parse_additive()));
                    }
                    else if (match("<="))
                    {
                        PrgValue right = parse_additive();
                        if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                            (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            left = make_boolean_value(static_cast<std::int64_t>(value_as_number(left)) <=
                                                      static_cast<std::int64_t>(value_as_number(right)));
                        }
                        else
                        {
                            left = make_boolean_value(value_as_number(left) <= value_as_number(right));
                        }
                    }
                    else if (match(">="))
                    {
                        PrgValue right = parse_additive();
                        if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                            (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            left = make_boolean_value(static_cast<std::int64_t>(value_as_number(left)) >=
                                                      static_cast<std::int64_t>(value_as_number(right)));
                        }
                        else
                        {
                            left = make_boolean_value(value_as_number(left) >= value_as_number(right));
                        }
                    }
                    else if (match("==") || match("="))
                    {
                        left = make_boolean_value(values_equal(left, parse_additive()));
                    }
                    else if (match("<"))
                    {
                        PrgValue right = parse_additive();
                        if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                            (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            left = make_boolean_value(static_cast<std::int64_t>(value_as_number(left)) <
                                                      static_cast<std::int64_t>(value_as_number(right)));
                        }
                        else
                        {
                            left = make_boolean_value(value_as_number(left) < value_as_number(right));
                        }
                    }
                    else if (match(">"))
                    {
                        PrgValue right = parse_additive();
                        if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                            (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            left = make_boolean_value(static_cast<std::int64_t>(value_as_number(left)) >
                                                      static_cast<std::int64_t>(value_as_number(right)));
                        }
                        else
                        {
                            left = make_boolean_value(value_as_number(left) > value_as_number(right));
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
                        if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                        {
                            left = make_string_value(value_as_string(left) + value_as_string(right));
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
                        if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
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
                        if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
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
                        if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
                            (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64))
                        {
                            const std::int64_t divisor = static_cast<std::int64_t>(value_as_number(right));
                            if (divisor == 0)
                                throw std::runtime_error("Division by zero in integer expression");
                            left = make_int64_value(static_cast<std::int64_t>(value_as_number(left)) / divisor);
                        }
                        else
                        {
                            left = make_number_value(value_as_number(left) / value_as_number(right));
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
                if (match("!"))
                {
                    return make_boolean_value(!value_as_bool(parse_unary()));
                }
                if (match("-"))
                {
                    PrgValue operand = parse_unary();
                    if (operand.kind == PrgValueKind::int64)
                    {
                        return make_int64_value(-operand.int64_value);
                    }
                    return make_number_value(-value_as_number(operand));
                }
                return parse_primary();
            }

            PrgValue parse_primary()
            {
                skip_whitespace();
                if (match("("))
                {
                    PrgValue value = parse_comparison();
                    match(")");
                    return value;
                }
                if (match("&"))
                {
                    return parse_macro_reference();
                }
                if (peek() == '\'')
                {
                    return make_string_value(parse_string());
                }
                if (peek() == '{')
                {
                    return make_string_value(parse_braced_literal());
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
                }
                if (std::isdigit(static_cast<unsigned char>(peek())) != 0)
                {
                    return make_number_value(parse_number());
                }

                const std::string identifier = parse_identifier();
                if (identifier.empty())
                {
                    return {};
                }

                skip_whitespace();
                if (match("["))
                {
                    const std::size_t row = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(parse_comparison())));
                    std::size_t column = 1U;
                    skip_whitespace();
                    if (match(","))
                    {
                        column = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(parse_comparison())));
                    }
                    match("]");
                    return array_value_callback_(identifier, row, column);
                }

                if (match("("))
                {
                    std::vector<PrgValue> arguments;
                    std::vector<std::string> raw_arguments;
                    skip_whitespace();
                    if (!match(")"))
                    {
                        while (true)
                        {
                            const std::size_t argument_start = position_;
                            arguments.push_back(parse_comparison());

                            std::size_t argument_end = position_;
                            skip_whitespace();
                            if (normalize_identifier(identifier) == "cast" && arguments.size() == 1U)
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
                            raw_arguments.push_back(
                                trim_copy(text_.substr(argument_start, argument_end - argument_start)));
                            skip_whitespace();
                            if (match(")"))
                            {
                                break;
                            }
                            match(",");
                        }
                    }
                    if (array_exists_callback_(identifier))
                    {
                        const std::size_t row = arguments.empty()
                                                    ? 0U
                                                    : static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[0])));
                        const std::size_t column = arguments.size() < 2U
                                                       ? 1U
                                                       : static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[1])));
                        return array_value_callback_(identifier, row, column);
                    }
                    return evaluate_function(identifier, arguments, raw_arguments);
                }

                return resolve_identifier(identifier);
            }

            PrgValue evaluate_function(
                const std::string &identifier,
                const std::vector<PrgValue> &arguments,
                const std::vector<std::string> &raw_arguments)
            {
                const std::string function = normalize_identifier(identifier);
                const auto member_separator = function.find('.');
                if (member_separator != std::string::npos)
                {
                    const std::string base_name = function.substr(0U, member_separator);
                    const std::string member_path = function.substr(member_separator + 1U);
                    return ole_invoke_callback_(base_name, member_path, arguments);
                }
                if ((function == "min" || function == "max") && arguments.size() >= 2U)
                {
                    PrgValue result = arguments.front();
                    for (std::size_t index = 1U; index < arguments.size(); ++index)
                    {
                        const PrgValue &candidate = arguments[index];
                        bool candidate_wins = false;
                        if (result.kind == PrgValueKind::string || candidate.kind == PrgValueKind::string)
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
                if (function == "createobject" && !arguments.empty())
                {
                    const std::string prog_id = value_as_string(arguments[0]);
                    const int handle = register_ole_callback_(prog_id, "createobject");
                    record_event_callback_("ole.createobject", prog_id);
                    return make_string_value("object:" + prog_id + "#" + std::to_string(handle));
                }
                if (function == "getobject" && !arguments.empty())
                {
                    const std::string source = value_as_string(arguments[0]);
                    const int handle = register_ole_callback_(source, "getobject");
                    record_event_callback_("ole.getobject", source);
                    return make_string_value("object:" + source + "#" + std::to_string(handle));
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
                if (const auto string_result = evaluate_string_function(function, arguments, exact_string_compare_, memowidth_callback_()))
                {
                    return *string_result;
                }
                if (const auto type_result = evaluate_type_function(function, arguments, array_exists_callback_, eval_expression_callback_))
                {
                    return *type_result;
                }
                if (const auto numeric_result = evaluate_numeric_function(function, arguments))
                {
                    return *numeric_result;
                }
                if (const auto date_time_result = evaluate_date_time_function(function, arguments))
                {
                    return *date_time_result;
                }
                // --- Array / variable helpers ---
                if (function == "alen" && !arguments.empty())
                {
                    const std::string array_name = raw_arguments.empty() ? std::string{} : raw_arguments[0];
                    const int dimension = arguments.size() >= 2U ? static_cast<int>(value_as_number(arguments[1])) : 0;
                    return make_number_value(static_cast<double>(array_length_callback_(array_name, dimension)));
                }
                if ((function == "acopy" || function == "adel" || function == "adir" || function == "aelement" ||
                     function == "afields" || function == "ains" || function == "alines" || function == "asort" ||
                     function == "ascan" || function == "asize" || function == "asubscript" || function == "aused") &&
                    !arguments.empty())
                {
                    return array_function_callback_(function, raw_arguments, arguments);
                }
                // --- Misc ---
                if (function == "getenv" && !arguments.empty())
                {
                    const std::string env_key = value_as_string(arguments[0]);
                    return make_string_value(get_environment_variable_value(env_key).value_or(std::string{}));
                }
                if (const auto path_result = evaluate_path_function(function, arguments, default_directory_))
                {
                    return *path_result;
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
                                                          error_handler_,
                                                          aerror_callback_,
                                                          eval_expression_callback_,
                                                          set_callback_))
                {
                    return *runtime_surface_result;
                }
                // --- Declared DLL function invocation ---
                if (declared_dll_invoke_callback_)
                {
                    PrgValue dll_result = declared_dll_invoke_callback_(function, arguments);
                    if (dll_result.kind != PrgValueKind::empty)
                    {
                        return dll_result;
                    }
                    // If result is empty the callback may mean "not found", fall through.
                }
                return make_string_value(function);
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
                    if (ch == '\'')
                    {
                        while (position_ < text_.size())
                        {
                            const char string_ch = text_[position_++];
                            if (string_ch == '\'')
                            {
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

            PrgValue parse_macro_reference()
            {
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

                if (peek() == '.')
                {
                    ++position_;
                }

                const std::string expanded = trim_copy(value_as_string(resolve_identifier(macro_name)));
                if (expanded.empty())
                {
                    return make_empty_value();
                }
                // Always treat macro expansion as an expression, not just a string
                const PrgValue expanded_value = eval_expression_callback_(expanded);
                if (expanded_value.kind != PrgValueKind::empty)
                {
                    return expanded_value;
                }
                // If not an expression, return as string
                return make_string_value(expanded);
            }

            std::string parse_string()
            {
                std::string result;
                if (!match("'"))
                {
                    return result;
                }
                while (position_ < text_.size())
                {
                    const char ch = text_[position_++];
                    if (ch == '\'')
                    {
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

            bool values_equal(const PrgValue &left, const PrgValue &right) const
            {
                if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                {
                    const std::string left_value = value_as_string(left);
                    const std::string right_value = value_as_string(right);
                    if (exact_string_compare_)
                    {
                        return trim_copy(left_value) == trim_copy(right_value);
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
            std::function<PrgValue(int, const std::string &)> sql_get_prop_callback_;
            std::function<int(int, const std::string &, const PrgValue &)> sql_set_prop_callback_;
            std::function<int(const std::string &, const std::string &)> register_ole_callback_;
            std::function<PrgValue(const std::string &, const std::string &, const std::vector<PrgValue> &)> ole_invoke_callback_;
            std::function<PrgValue(const std::string &)> ole_property_callback_;
            std::function<PrgValue(const std::string &)> eval_expression_callback_;
            std::function<std::string(const std::string &)> set_callback_;
            std::function<void(const std::string &, const std::string &)> record_event_callback_;
            std::function<std::size_t()> memowidth_callback_;
            std::function<PrgValue(const std::string &, const std::vector<PrgValue> &)> declared_dll_invoke_callback_;
            const std::string &text_;
            const Frame &frame_;
            const std::map<std::string, PrgValue> &globals_;
            const std::string &default_directory_;
            const std::string &last_error_message_;
            int last_error_code_ = 0;
            const std::string &last_error_procedure_;
            std::size_t last_error_line_ = 0;
            const std::string &error_handler_;
            bool exact_string_compare_ = false;
            std::size_t position_ = 0;
        };

    } // namespace
