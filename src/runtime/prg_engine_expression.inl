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
        bool exact_string_compare);
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
                std::function<std::size_t(const std::string &)> record_count_callback,
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
                std::function<PrgValue(const std::string &, const std::vector<PrgValue> &)> declared_dll_invoke_callback)
                : current_work_area_(current_work_area),
                  next_free_work_area_callback_(std::move(next_free_work_area_callback)),
                  resolve_work_area_callback_(std::move(resolve_work_area_callback)),
                  alias_lookup_callback_(std::move(alias_lookup_callback)),
                  used_callback_(std::move(used_callback)),
                  dbf_lookup_callback_(std::move(dbf_lookup_callback)),
                  field_count_callback_(std::move(field_count_callback)),
                  record_count_callback_(std::move(record_count_callback)),
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
                            raw_arguments.push_back(trim_copy(text_.substr(argument_start, position_ - argument_start)));
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
                if (function == "reccount")
                {
                    const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
                    return make_number_value(static_cast<double>(record_count_callback_(designator)));
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
                if (function == "transform" && !arguments.empty())
                {
                    const std::string picture = arguments.size() >= 2U ? uppercase_copy(value_as_string(arguments[1])) : std::string{};
                    if (!picture.empty())
                    {
                        if (picture.find("@!") != std::string::npos)
                        {
                            return make_string_value(uppercase_copy(value_as_string(arguments[0])));
                        }
                        if (picture.find("@L") != std::string::npos)
                        {
                            std::string value = value_as_string(arguments[0]);
                            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
                                           { return static_cast<char>(std::tolower(ch)); });
                            return make_string_value(std::move(value));
                        }

                        const std::size_t decimal_pos = picture.find('.');
                        if (decimal_pos != std::string::npos)
                        {
                            std::size_t decimals = 0U;
                            for (std::size_t index = decimal_pos + 1U; index < picture.size(); ++index)
                            {
                                if (picture[index] == '9' || picture[index] == '#' || picture[index] == '0')
                                {
                                    ++decimals;
                                }
                            }
                            std::ostringstream stream;
                            stream << std::fixed << std::setprecision(static_cast<int>(decimals)) << value_as_number(arguments[0]);
                            return make_string_value(stream.str());
                        }
                    }
                    return make_string_value(value_as_string(arguments[0]));
                }
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
                if (function == "file" && !arguments.empty())
                {
                    std::filesystem::path path(value_as_string(arguments[0]));
                    if (path.is_relative())
                    {
                        path = std::filesystem::path(default_directory_) / path;
                    }
                    return make_boolean_value(std::filesystem::exists(path));
                }
                if (function == "sys")
                {
                    if (!arguments.empty())
                    {
                        const long long sys_code = std::llround(value_as_number(arguments[0]));
                        if (sys_code == 16)
                        {
                            return make_string_value(frame_.file_path);
                        }
                        if (sys_code == 2018)
                        {
                            return make_string_value(uppercase_copy(runtime_error_parameter(last_error_message_)));
                        }
                    }
                    return make_string_value("0");
                }
                if ((function == "at" || function == "atc") && arguments.size() >= 2U)
                {
                    const bool case_insensitive = function == "atc";
                    std::string needle = value_as_string(arguments[0]);
                    std::string haystack = value_as_string(arguments[1]);
                    const std::size_t occurrence = arguments.size() >= 3U
                                                       ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                                       : 1U;
                    if (case_insensitive)
                    {
                        needle = uppercase_copy(std::move(needle));
                        haystack = uppercase_copy(std::move(haystack));
                    }
                    if (needle.empty())
                    {
                        return make_number_value(0.0);
                    }
                    std::size_t found_count = 0U;
                    std::size_t search_pos = 0U;
                    while (search_pos <= haystack.size())
                    {
                        const auto found = haystack.find(needle, search_pos);
                        if (found == std::string::npos)
                        {
                            break;
                        }
                        ++found_count;
                        if (found_count == occurrence)
                        {
                            return make_number_value(static_cast<double>(found + 1U));
                        }
                        search_pos = found + needle.size();
                    }
                    return make_number_value(0.0);
                }
                if ((function == "rat" || function == "ratc") && arguments.size() >= 2U)
                {
                    const bool case_insensitive = function == "ratc";
                    std::string needle = value_as_string(arguments[0]);
                    std::string haystack = value_as_string(arguments[1]);
                    const std::size_t occurrence = arguments.size() >= 3U
                                                       ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                                       : 1U;
                    if (case_insensitive)
                    {
                        needle = uppercase_copy(std::move(needle));
                        haystack = uppercase_copy(std::move(haystack));
                    }
                    if (needle.empty())
                    {
                        return make_number_value(0.0);
                    }
                    std::size_t found_count = 0U;
                    std::size_t search_pos = std::string::npos;
                    while (true)
                    {
                        const auto found = haystack.rfind(needle, search_pos);
                        if (found == std::string::npos)
                        {
                            break;
                        }
                        ++found_count;
                        if (found_count == occurrence)
                        {
                            return make_number_value(static_cast<double>(found + 1U));
                        }
                        if (found == 0U)
                        {
                            break;
                        }
                        search_pos = found - 1U;
                    }
                    return make_number_value(0.0);
                }
                if ((function == "atline" || function == "atcline" || function == "ratline") && arguments.size() >= 2U)
                {
                    const bool reverse = function == "ratline";
                    const bool case_insensitive = function == "atcline";
                    std::string needle = value_as_string(arguments[0]);
                    std::vector<std::string> lines = split_text_lines(value_as_string(arguments[1]));
                    const std::size_t occurrence = arguments.size() >= 3U
                                                       ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                                       : 1U;
                    if (case_insensitive)
                    {
                        needle = uppercase_copy(std::move(needle));
                        for (std::string &line : lines)
                        {
                            line = uppercase_copy(std::move(line));
                        }
                    }
                    if (needle.empty())
                    {
                        return make_number_value(0.0);
                    }
                    std::size_t found_count = 0U;
                    if (reverse)
                    {
                        for (std::size_t index = lines.size(); index > 0U; --index)
                        {
                            if (lines[index - 1U].find(needle) == std::string::npos)
                            {
                                continue;
                            }
                            ++found_count;
                            if (found_count == occurrence)
                            {
                                return make_number_value(static_cast<double>(index));
                            }
                        }
                    }
                    else
                    {
                        for (std::size_t index = 0U; index < lines.size(); ++index)
                        {
                            if (lines[index].find(needle) == std::string::npos)
                            {
                                continue;
                            }
                            ++found_count;
                            if (found_count == occurrence)
                            {
                                return make_number_value(static_cast<double>(index + 1U));
                            }
                        }
                    }
                    return make_number_value(0.0);
                }
                if (function == "substr" && arguments.size() >= 2U)
                {
                    const std::string source = value_as_string(arguments[0]);
                    const std::size_t start = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1]) - 1.0));
                    const std::size_t length = arguments.size() >= 3U
                                                   ? static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])))
                                                   : std::string::npos;
                    return make_string_value(start >= source.size() ? std::string{} : source.substr(start, length));
                }
                if (function == "justpath" && !arguments.empty())
                {
                    return make_string_value(portable_path_parent(value_as_string(arguments[0])));
                }
                if (function == "str" && !arguments.empty())
                {
                    const int decimals = arguments.size() >= 3U
                                             ? static_cast<int>(std::max(0.0, value_as_number(arguments[2])))
                                             : 0;
                    std::ostringstream stream;
                    stream << std::fixed << std::setprecision(decimals) << value_as_number(arguments[0]);
                    std::string result = stream.str();
                    if (arguments.size() >= 2U)
                    {
                        const int width = static_cast<int>(std::llround(value_as_number(arguments[1])));
                        if (width > 0)
                        {
                            if (result.size() > static_cast<std::size_t>(width))
                            {
                                return make_string_value(std::string(static_cast<std::size_t>(width), '*'));
                            }
                            if (result.size() < static_cast<std::size_t>(width))
                            {
                                result.insert(result.begin(), static_cast<std::size_t>(width) - result.size(), ' ');
                            }
                        }
                    }
                    return make_string_value(std::move(result));
                }
                if (function == "transform" && !arguments.empty())
                {
                    const std::string picture = arguments.size() >= 2U ? uppercase_copy(value_as_string(arguments[1])) : std::string{};
                    if (!picture.empty())
                    {
                        if (picture.find("@!") != std::string::npos)
                        {
                            return make_string_value(uppercase_copy(value_as_string(arguments[0])));
                        }
                        if (picture.find("@L") != std::string::npos)
                        {
                            std::string value = value_as_string(arguments[0]);
                            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
                                           { return static_cast<char>(std::tolower(ch)); });
                            return make_string_value(std::move(value));
                        }

                        const std::size_t decimal_pos = picture.find('.');
                        if (decimal_pos != std::string::npos)
                        {
                            std::size_t decimals = 0U;
                            for (std::size_t index = decimal_pos + 1U; index < picture.size(); ++index)
                            {
                                if (picture[index] == '9' || picture[index] == '#' || picture[index] == '0')
                                {
                                    ++decimals;
                                }
                            }
                            std::ostringstream stream;
                            stream << std::fixed << std::setprecision(static_cast<int>(decimals)) << value_as_number(arguments[0]);
                            return make_string_value(stream.str());
                        }
                    }
                    return make_string_value(value_as_string(arguments[0]));
                }
                if (function == "alltrim" && !arguments.empty())
                {
                    return make_string_value(trim_copy(value_as_string(arguments[0])));
                }
                if (function == "chr" && !arguments.empty())
                {
                    return make_string_value(std::string(1U, static_cast<char>(std::llround(value_as_number(arguments[0])))));
                }
                if (function == "message")
                {
                    return make_string_value(last_error_message_);
                }
                if (function == "aerror" && !raw_arguments.empty())
                {
                    return make_number_value(static_cast<double>(aerror_callback_(raw_arguments[0])));
                }
                if (function == "eval" && !arguments.empty())
                {
                    return eval_expression_callback_(value_as_string(arguments[0]));
                }
                if (function == "evaluate" && !arguments.empty())
                {
                    return eval_expression_callback_(value_as_string(arguments[0]));
                }
                if (function == "set" && !arguments.empty())
                {
                    return make_string_value(set_callback_(value_as_string(arguments[0])));
                }
                if (function == "error")
                {
                    return make_number_value(static_cast<double>(last_error_code_));
                }
                if (function == "program")
                {
                    return make_string_value(last_error_procedure_);
                }
                if (function == "lineno")
                {
                    return make_number_value(static_cast<double>(last_error_line_));
                }
                if (function == "version")
                {
                    return make_number_value(arguments.empty() ? 9.0 : 0.0);
                }
                if (function == "on" && !arguments.empty())
                {
                    return make_string_value(uppercase_copy(value_as_string(arguments[0])) == "ERROR" ? error_handler_ : std::string{});
                }
                if (function == "messagebox" && !arguments.empty())
                {
                    return make_number_value(1.0);
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
                if (const auto string_result = evaluate_string_function(function, arguments, exact_string_compare_))
                {
                    return *string_result;
                }
                if (function == "strextract" && arguments.size() >= 3U)
                {
                    const std::string src = value_as_string(arguments[0]);
                    const std::string begin_delim = value_as_string(arguments[1]);
                    const std::string end_delim = value_as_string(arguments[2]);
                    const std::size_t occurrence = arguments.size() >= 4U
                                                       ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[3])))
                                                       : 1U;
                    const int flags = arguments.size() >= 5U ? static_cast<int>(value_as_number(arguments[4])) : 0;
                    const bool case_insensitive = (flags & 1) != 0;
                    const bool end_delimiter_optional = (flags & 2) != 0;
                    const bool include_delimiters = (flags & 4) != 0;
                    std::size_t search_pos = 0U;
                    std::size_t found_count = 0U;
                    while (search_pos <= src.size())
                    {
                        std::size_t begin_pos;
                        if (begin_delim.empty())
                        {
                            begin_pos = 0U;
                        }
                        else if (case_insensitive)
                        {
                            const std::string src_up = uppercase_copy(src.substr(search_pos));
                            const std::string bd_up = uppercase_copy(begin_delim);
                            const std::size_t rel = src_up.find(bd_up);
                            begin_pos = rel == std::string::npos ? std::string::npos : search_pos + rel;
                        }
                        else
                        {
                            begin_pos = src.find(begin_delim, search_pos);
                        }
                        if (begin_pos == std::string::npos)
                            break;
                        const std::size_t content_start = begin_pos + begin_delim.size();
                        ++found_count;
                        if (found_count == occurrence)
                        {
                            if (end_delim.empty())
                                return make_string_value(src.substr(content_start));
                            std::size_t end_pos;
                            if (case_insensitive)
                            {
                                const std::string remaining_up = uppercase_copy(src.substr(content_start));
                                const std::string ed_up = uppercase_copy(end_delim);
                                const std::size_t rel = remaining_up.find(ed_up);
                                end_pos = rel == std::string::npos ? std::string::npos : content_start + rel;
                            }
                            else
                            {
                                end_pos = src.find(end_delim, content_start);
                            }
                            if (end_pos == std::string::npos)
                            {
                                return end_delimiter_optional
                                           ? make_string_value(include_delimiters ? src.substr(begin_pos) : src.substr(content_start))
                                           : make_string_value(std::string{});
                            }
                            return make_string_value(include_delimiters
                                                         ? src.substr(begin_pos, end_pos + end_delim.size() - begin_pos)
                                                         : src.substr(content_start, end_pos - content_start));
                        }
                        search_pos = content_start;
                    }
                    return make_string_value(std::string{});
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
                     function == "ascan" || function == "asize" || function == "asubscript") &&
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
                if (function == "numlock" || function == "capslock" || function == "scrolllock")
                {
                    return make_boolean_value(false);
                }
                if (function == "cursorsetprop" || function == "cursorgetprop")
                {
                    return make_number_value(0.0);
                }
                // --- CAST(<expr> AS <type>) ---
                if (function == "cast" && !arguments.empty())
                {
                    // raw_arguments[0] contains "<expr> AS <type>" - evaluate_function receives the
                    // pre-evaluated argument list, so we need the raw text of the first argument.
                    // We look for the type name in the raw first argument after the AS keyword.
                    std::string type_name;
                    if (!raw_arguments.empty())
                    {
                        const std::string raw = uppercase_copy(raw_arguments[0]);
                        const auto as_pos = raw.rfind(" AS ");
                        if (as_pos != std::string::npos)
                        {
                            type_name = trim_copy(raw.substr(as_pos + 4U));
                        }
                    }
                    const PrgValue src = arguments[0];
                    if (type_name == "INT64" || type_name == "LONGLONG" || type_name == "BIGINT")
                    {
                        return make_int64_value(static_cast<std::int64_t>(value_as_number(src)));
                    }
                    if (type_name == "UINT64" || type_name == "ULONGLONG" || type_name == "UBIGINT")
                    {
                        return make_uint64_value(static_cast<std::uint64_t>(value_as_number(src)));
                    }
                    if (type_name == "INT" || type_name == "INT32" || type_name == "INTEGER" || type_name == "LONG" || type_name == "INT16" || type_name == "SHORT")
                    {
                        return make_int64_value(static_cast<std::int64_t>(std::trunc(value_as_number(src))));
                    }
                    if (type_name == "BYTE" || type_name == "UINT8")
                    {
                        return make_uint64_value(static_cast<std::uint64_t>(value_as_number(src)) & 0xFFULL);
                    }
                    if (type_name == "FLOAT" || type_name == "SINGLE")
                    {
                        return make_number_value(static_cast<double>(static_cast<float>(value_as_number(src))));
                    }
                    if (type_name == "DOUBLE" || type_name == "NUMERIC")
                    {
                        return make_number_value(value_as_number(src));
                    }
                    if (type_name == "STRING" || type_name == "CHAR" || type_name == "VARCHAR" || type_name == "CHARACTER")
                    {
                        return make_string_value(value_as_string(src));
                    }
                    if (type_name == "LOGICAL" || type_name == "BOOL" || type_name == "BOOLEAN")
                    {
                        return make_boolean_value(value_as_bool(src));
                    }
                    // Unknown cast type - return source unchanged
                    return src;
                }
                // --- Bitwise integer operations ---
                if (function == "bitand" && arguments.size() >= 2U)
                {
                    const auto a = static_cast<std::int64_t>(value_as_number(arguments[0]));
                    const auto b = static_cast<std::int64_t>(value_as_number(arguments[1]));
                    return make_int64_value(a & b);
                }
                if (function == "bitor" && arguments.size() >= 2U)
                {
                    const auto a = static_cast<std::int64_t>(value_as_number(arguments[0]));
                    const auto b = static_cast<std::int64_t>(value_as_number(arguments[1]));
                    return make_int64_value(a | b);
                }
                if (function == "bitxor" && arguments.size() >= 2U)
                {
                    const auto a = static_cast<std::int64_t>(value_as_number(arguments[0]));
                    const auto b = static_cast<std::int64_t>(value_as_number(arguments[1]));
                    return make_int64_value(a ^ b);
                }
                if (function == "bitnot" && !arguments.empty())
                {
                    const auto a = static_cast<std::int64_t>(value_as_number(arguments[0]));
                    return make_int64_value(~a);
                }
                if (function == "bitlshift" && arguments.size() >= 2U)
                {
                    const auto a = static_cast<std::int64_t>(value_as_number(arguments[0]));
                    const int n = static_cast<int>(value_as_number(arguments[1]));
                    return make_int64_value(a << n);
                }
                if (function == "bitrshift" && arguments.size() >= 2U)
                {
                    const auto a = static_cast<std::int64_t>(value_as_number(arguments[0]));
                    const int n = static_cast<int>(value_as_number(arguments[1]));
                    return make_int64_value(a >> n);
                }
                // --- BINTOC / CTOBIN: binary byte-string packing (VFP-compatible) ---
                if (function == "bintoc" && !arguments.empty())
                {
                    // BINTOC(<number> [, <width>])  - packs as little-endian bytes
                    const auto val = static_cast<std::int64_t>(value_as_number(arguments[0]));
                    const int width = arguments.size() >= 2U
                                          ? static_cast<int>(value_as_number(arguments[1]))
                                          : 4; // default 4 bytes like VFP
                    std::string result(static_cast<std::size_t>(width), '\0');
                    std::uint64_t uval = static_cast<std::uint64_t>(val);
                    for (int i = 0; i < width; ++i)
                    {
                        result[static_cast<std::size_t>(i)] = static_cast<char>(uval & 0xFFU);
                        uval >>= 8;
                    }
                    return make_string_value(std::move(result));
                }
                if (function == "ctobin" && !arguments.empty())
                {
                    // CTOBIN(<string> [, <type>]) - unpacks little-endian bytes
                    const std::string s = value_as_string(arguments[0]);
                    const std::string type = arguments.size() >= 2U
                                                 ? uppercase_copy(value_as_string(arguments[1]))
                                                 : std::string("N"); // default: unsigned (VFP N = INTEGER)
                    std::uint64_t uval = 0U;
                    for (std::size_t i = s.size(); i-- > 0U;)
                    {
                        uval = (uval << 8) | static_cast<std::uint8_t>(s[i]);
                    }
                    if (type == "N" || type == "INTEGER" || type == "INT")
                    {
                        return make_int64_value(static_cast<std::int64_t>(uval));
                    }
                    return make_uint64_value(uval);
                }
                if (function == "isdigit" && !arguments.empty())
                {
                    const std::string s = value_as_string(arguments[0]);
                    return make_boolean_value(!s.empty() && std::isdigit(static_cast<unsigned char>(s[0])) != 0);
                }
                if (function == "isalpha" && !arguments.empty())
                {
                    const std::string s = value_as_string(arguments[0]);
                    return make_boolean_value(!s.empty() && std::isalpha(static_cast<unsigned char>(s[0])) != 0);
                }
                if (function == "islower" && !arguments.empty())
                {
                    const std::string s = value_as_string(arguments[0]);
                    return make_boolean_value(!s.empty() && std::islower(static_cast<unsigned char>(s[0])) != 0);
                }
                if (function == "isupper" && !arguments.empty())
                {
                    const std::string s = value_as_string(arguments[0]);
                    return make_boolean_value(!s.empty() && std::isupper(static_cast<unsigned char>(s[0])) != 0);
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
            std::function<std::size_t(const std::string &)> record_count_callback_;
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
