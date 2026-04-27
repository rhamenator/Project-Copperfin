// prg_engine_free_functions.inl
// Free helper functions. Included inside anonymous namespace in prg_engine.cpp.
// This file must not be compiled separately.

        int current_process_id()
        {
#if defined(_WIN32)
            return _getpid();
#else
            return getpid();
#endif
        }

        std::optional<std::string> get_environment_variable_value(const std::string &name)
        {
#if defined(_WIN32)
            char *raw = nullptr;
            std::size_t length = 0U;
            if (_dupenv_s(&raw, &length, name.c_str()) != 0 || raw == nullptr)
            {
                return std::nullopt;
            }
            std::string value(raw);
            free(raw);
            return value;
#else
            const char *raw = std::getenv(name.c_str());
            if (raw == nullptr)
            {
                return std::nullopt;
            }
            return std::string(raw);
#endif
        }

        bool set_environment_variable_value(const std::string &name, const std::string &value)
        {
            if (name.empty())
            {
                return false;
            }

#if defined(_WIN32)
            return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
            if (value.empty())
            {
                return unsetenv(name.c_str()) == 0;
            }
            return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
        }

        // Wildcard match: '*' matches any sequence, '?' matches a single char (case-insensitive).
        static bool field_wildcard_match(const std::string &name, const std::string &pattern)
        {
            const std::string n = uppercase_copy(name);
            const std::string p = uppercase_copy(pattern);
            // dp[i][j] = true if n[0..i-1] matches p[0..j-1]
            const std::size_t ni = n.size(), pi = p.size();
            std::vector<std::vector<bool>> dp(ni + 1U, std::vector<bool>(pi + 1U, false));
            dp[0][0] = true;
            for (std::size_t j = 1U; j <= pi; ++j)
            {
                if (p[j - 1U] == '*')
                    dp[0][j] = dp[0][j - 1U];
            }
            for (std::size_t i = 1U; i <= ni; ++i)
            {
                for (std::size_t j = 1U; j <= pi; ++j)
                {
                    if (p[j - 1U] == '*')
                    {
                        dp[i][j] = dp[i - 1U][j] || dp[i][j - 1U];
                    }
                    else if (p[j - 1U] == '?' || p[j - 1U] == n[i - 1U])
                    {
                        dp[i][j] = dp[i - 1U][j - 1U];
                    }
                }
            }
            return dp[ni][pi];
        }

        // Sentinels encoded as first element of the returned vector.
        // "__LIKE__"  = include only fields matching the wildcard pattern in element [1]
        // "__EXCEPT__" = include all fields NOT matching patterns in elements [1..]
        std::vector<std::string> parse_field_filter_clause(const std::string &fields_clause)
        {
            const std::string trimmed = trim_copy(fields_clause);

            if (starts_with_insensitive(trimmed, "LIKE "))
            {
                // FIELDS LIKE <pattern>  (single wildcard pattern)
                const std::string pattern = collapse_identifier(trim_copy(trimmed.substr(5U)));
                std::vector<std::string> result;
                result.push_back("__LIKE__");
                result.push_back(pattern);
                return result;
            }

            if (starts_with_insensitive(trimmed, "EXCEPT "))
            {
                // FIELDS EXCEPT <name1, name2, ...>  (exact names or patterns to exclude)
                std::vector<std::string> result;
                result.push_back("__EXCEPT__");
                std::string remaining = trim_copy(trimmed.substr(7U));
                while (!remaining.empty())
                {
                    const auto comma = remaining.find(',');
                    const std::string token = collapse_identifier(trim_copy(
                        comma == std::string::npos ? remaining : remaining.substr(0U, comma)));
                    if (!token.empty())
                        result.push_back(token);
                    if (comma == std::string::npos)
                        break;
                    remaining = remaining.substr(comma + 1U);
                }
                return result;
            }

            std::vector<std::string> field_filter;
            std::string remaining = trimmed;
            while (!remaining.empty())
            {
                const auto comma = remaining.find(',');
                const std::string token = collapse_identifier(trim_copy(
                    comma == std::string::npos ? remaining : remaining.substr(0U, comma)));
                if (!token.empty())
                {
                    field_filter.push_back(token);
                }
                if (comma == std::string::npos)
                {
                    break;
                }
                remaining = remaining.substr(comma + 1U);
            }
            return field_filter;
        }

        bool field_matches_filter(const std::string &field_name, const std::vector<std::string> &field_filter)
        {
            if (field_filter.empty())
            {
                return true;
            }
            if (!field_filter.empty() && field_filter[0] == "__LIKE__")
            {
                if (field_filter.size() < 2U)
                    return true;
                return field_wildcard_match(field_name, field_filter[1]);
            }
            if (!field_filter.empty() && field_filter[0] == "__EXCEPT__")
            {
                // Exclude if field matches any listed name/pattern.
                for (std::size_t i = 1U; i < field_filter.size(); ++i)
                {
                    if (field_wildcard_match(field_name, field_filter[i]) ||
                        collapse_identifier(field_filter[i]) == collapse_identifier(field_name))
                    {
                        return false;
                    }
                }
                return true;
            }
            return std::find_if(
                       field_filter.begin(),
                       field_filter.end(),
                       [&](const std::string &candidate)
                       {
                           return collapse_identifier(candidate) == collapse_identifier(field_name);
                       }) != field_filter.end();
        }

        std::string format_sdf_field_value(const vfp::DbfFieldDescriptor &field, std::string value)
        {
            value = trim_copy(std::move(value));
            if (value.size() > field.length)
            {
                value = value.substr(0U, field.length);
            }
            if (value.size() >= field.length)
            {
                return value;
            }

            const std::string padding(field.length - value.size(), ' ');
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (field_type == 'N' || field_type == 'F' || field_type == 'I' ||
                field_type == 'B' || field_type == 'Y')
            {
                return padding + value;
            }
            return value + padding;
        }

        std::vector<std::string> split_sdf_lines(const std::string &contents)
        {
            std::vector<std::string> lines;
            std::size_t start = 0U;
            while (start < contents.size())
            {
                std::size_t end = contents.find('\n', start);
                if (end == std::string::npos)
                {
                    end = contents.size();
                }
                std::string line = contents.substr(start, end - start);
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
                if (!line.empty())
                {
                    lines.push_back(std::move(line));
                }
                start = end + 1U;
            }
            return lines;
        }

        bool wildcard_match_insensitive(const std::string &pattern, const std::string &text)
        {
            const std::string p = lowercase_copy(pattern);
            const std::string t = lowercase_copy(text);
            std::size_t pattern_index = 0U;
            std::size_t text_index = 0U;
            std::size_t star_index = std::string::npos;
            std::size_t star_text_index = 0U;
            while (text_index < t.size())
            {
                if (pattern_index < p.size() && (p[pattern_index] == '?' || p[pattern_index] == t[text_index]))
                {
                    ++pattern_index;
                    ++text_index;
                }
                else if (pattern_index < p.size() && p[pattern_index] == '*')
                {
                    star_index = pattern_index++;
                    star_text_index = text_index;
                }
                else if (star_index != std::string::npos)
                {
                    pattern_index = star_index + 1U;
                    text_index = ++star_text_index;
                }
                else
                {
                    return false;
                }
            }
            while (pattern_index < p.size() && p[pattern_index] == '*')
            {
                ++pattern_index;
            }
            return pattern_index == p.size();
        }

        std::string format_file_time_part(const std::filesystem::file_time_type &file_time, bool date_part)
        {
            const auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                file_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            const std::time_t raw_time = std::chrono::system_clock::to_time_t(system_time);
            const std::tm local = local_time_from_time_t(raw_time);
            std::ostringstream stream;
            if (date_part)
            {
                stream << std::setfill('0') << std::setw(2) << (local.tm_mon + 1) << "/"
                       << std::setw(2) << local.tm_mday << "/"
                       << std::setw(4) << (local.tm_year + 1900);
            }
            else
            {
                stream << std::setfill('0') << std::setw(2) << local.tm_hour << ":"
                       << std::setw(2) << local.tm_min << ":"
                       << std::setw(2) << local.tm_sec;
            }
            return stream.str();
        }

        std::string file_attributes_for_adir(const std::filesystem::directory_entry &entry)
        {
            std::string attributes;
            std::error_code ignored;
            if (entry.is_directory(ignored))
            {
                attributes += "D";
            }
            const auto name = entry.path().filename().string();
            if (!name.empty() && name.front() == '.')
            {
                attributes += "H";
            }
            if ((entry.status(ignored).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none)
            {
                attributes += "R";
            }
            return attributes;
        }

        struct DelimitedTextOptions
        {
            char delimiter = ',';
            char quote = '"';
            bool quote_character_fields = true;
        };

        DelimitedTextOptions parse_delimited_text_options(const std::string &type, const std::string &with_clause)
        {
            DelimitedTextOptions options;
            if (normalize_identifier(type) == "tab")
            {
                options.delimiter = '\t';
            }

            std::string clause = trim_copy(with_clause);
            if (clause.empty())
            {
                return options;
            }
            const std::string normalized = normalize_identifier(clause);
            if (normalized == "tab")
            {
                options.delimiter = '\t';
                return options;
            }
            if (normalized == "blank" || normalized == "space")
            {
                options.delimiter = ' ';
                return options;
            }

            const std::size_t character_clause = find_keyword_top_level(clause, "CHARACTER");
            if (character_clause != std::string::npos)
            {
                std::string quote_clause = trim_copy(clause.substr(0U, character_clause));
                if (const std::size_t trailing_with = find_keyword_top_level(quote_clause, "WITH");
                    trailing_with != std::string::npos)
                {
                    quote_clause = trim_copy(quote_clause.substr(0U, trailing_with));
                }
                quote_clause = unquote_string(quote_clause);
                if (!quote_clause.empty())
                {
                    options.quote = quote_clause.front();
                }
                std::string delimiter_clause = trim_copy(clause.substr(character_clause + 9U));
                delimiter_clause = unquote_string(delimiter_clause);
                if (!delimiter_clause.empty())
                {
                    options.delimiter = delimiter_clause.front();
                }
                return options;
            }

            clause = unquote_string(clause);
            if (!clause.empty())
            {
                options.quote = clause.front();
            }
            return options;
        }

        std::string format_delimited_field_value(
            const vfp::DbfFieldDescriptor &field,
            const std::string &raw_value,
            const DelimitedTextOptions &options)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            const std::string value = trim_copy(raw_value);
            const bool quote_value = options.quote_character_fields &&
                                     !(field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == 'B' ||
                                       field_type == 'Y' || field_type == 'L');
            if (!quote_value)
            {
                return value;
            }

            std::string escaped;
            escaped.reserve(value.size() + 2U);
            escaped.push_back(options.quote);
            for (const char ch : value)
            {
                if (ch == options.quote)
                {
                    escaped.push_back(options.quote);
                }
                escaped.push_back(ch);
            }
            escaped.push_back(options.quote);
            return escaped;
        }

        std::vector<std::string> parse_delimited_text_line(const std::string &line, const DelimitedTextOptions &options)
        {
            std::vector<std::string> values;
            std::string current;
            bool in_quotes = false;
            for (std::size_t index = 0U; index < line.size(); ++index)
            {
                const char ch = line[index];
                if (ch == options.quote)
                {
                    if (in_quotes && index + 1U < line.size() && line[index + 1U] == options.quote)
                    {
                        current.push_back(options.quote);
                        ++index;
                    }
                    else
                    {
                        in_quotes = !in_quotes;
                    }
                    continue;
                }
                if (!in_quotes && ch == options.delimiter)
                {
                    values.push_back(trim_copy(current));
                    current.clear();
                    continue;
                }
                current.push_back(ch);
            }
            values.push_back(trim_copy(current));
            return values;
        }

        std::vector<ReplaceAssignment> parse_update_set_assignments(const std::string &text)
        {
            std::vector<ReplaceAssignment> assignments;
            for (const std::string &part : split_csv_like(text))
            {
                const std::size_t equals = part.find('=');
                if (equals == std::string::npos)
                {
                    continue;
                }
                assignments.push_back({.field_name = trim_copy(part.substr(0U, equals)),
                                       .expression = trim_copy(part.substr(equals + 1U))});
            }
            return assignments;
        }

        std::map<std::string, CursorState::FieldRule> field_rules_from_declarations(
            const std::vector<TableFieldDeclaration> &declarations)
        {
            std::map<std::string, CursorState::FieldRule> rules;
            for (const auto &declaration : declarations)
            {
                if (!declaration.nullable || declaration.has_default)
                {
                    rules[collapse_identifier(declaration.descriptor.name)] = CursorState::FieldRule{
                        .nullable = declaration.nullable,
                        .has_default = declaration.has_default,
                        .default_expression = declaration.default_expression};
                }
            }
            return rules;
        }

        bool parse_datetime_storage_contract(const std::string &raw, int &julian_day, int &millis)
        {
            const std::string text = trim_copy(raw);
            if (text.empty())
            {
                julian_day = 0;
                millis = 0;
                return true;
            }

            const std::string lowered = lowercase_copy(text);
            constexpr const char *julian_prefix = "julian:";
            constexpr const char *millis_prefix = "millis:";
            if (lowered.rfind(julian_prefix, 0U) != 0U)
            {
                return false;
            }

            const std::size_t millis_pos = lowered.find(millis_prefix);
            if (millis_pos == std::string::npos || millis_pos <= 7U)
            {
                return false;
            }

            const std::string julian_text = trim_copy(text.substr(7U, millis_pos - 7U));
            const std::string millis_text = trim_copy(text.substr(millis_pos + 7U));
            if (julian_text.empty() || millis_text.empty())
            {
                return false;
            }

            try
            {
                std::size_t consumed = 0U;
                julian_day = std::stoi(julian_text, &consumed, 10);
                if (consumed != julian_text.size())
                {
                    return false;
                }

                millis = std::stoi(millis_text, &consumed, 10);
                if (consumed != millis_text.size())
                {
                    return false;
                }
            }
            catch (const std::exception &)
            {
                return false;
            }

            return julian_day >= 0 && millis >= 0;
        }

        bool parse_runtime_or_storage_date_string(const std::string &raw, int &year, int &month, int &day)
        {
            if (parse_runtime_date_string(raw, year, month, day))
            {
                return true;
            }

            const std::string text = trim_copy(raw);
            if (text.size() != 10U || text[4U] != '-' || text[7U] != '-')
            {
                return false;
            }
            for (std::size_t index = 0U; index < text.size(); ++index)
            {
                if (index == 4U || index == 7U)
                {
                    continue;
                }
                if (std::isdigit(static_cast<unsigned char>(text[index])) == 0)
                {
                    return false;
                }
            }

            try
            {
                year = std::stoi(text.substr(0U, 4U));
                month = std::stoi(text.substr(5U, 2U));
                day = std::stoi(text.substr(8U, 2U));
            }
            catch (const std::exception &)
            {
                return false;
            }

            return year > 0 && month >= 1 && month <= 12 && day >= 1 && day <= days_in_month(year, month);
        }

        bool parse_runtime_or_storage_datetime_string(
            const std::string &raw,
            int &year,
            int &month,
            int &day,
            int &hour,
            int &minute,
            int &second)
        {
            if (parse_runtime_datetime_string(raw, year, month, day, hour, minute, second))
            {
                return true;
            }

            const std::string text = trim_copy(raw);
            if (text.empty())
            {
                return false;
            }

            const auto separator = text.find_first_of(" T");
            const std::string date_part = separator == std::string::npos ? text : text.substr(0U, separator);
            const std::string time_part = separator == std::string::npos ? std::string{} : trim_copy(text.substr(separator + 1U));
            if (!parse_runtime_or_storage_date_string(date_part, year, month, day))
            {
                return false;
            }

            hour = 0;
            minute = 0;
            second = 0;
            return time_part.empty() || parse_runtime_time_string(time_part, hour, minute, second);
        }

        std::string format_runtime_date_storage_string(int year, int month, int day)
        {
            std::ostringstream stream;
            stream << std::setfill('0')
                   << std::setw(4) << year
                   << std::setw(2) << month
                   << std::setw(2) << day;
            return stream.str();
        }

        std::string format_runtime_datetime_storage_string(int year, int month, int day, int hour, int minute, int second)
        {
            const int julian_day = date_to_julian(year, month, day);
            const int millis = (((hour * 60) + minute) * 60 + second) * 1000;
            return "julian:" + std::to_string(julian_day) + " millis:" + std::to_string(millis);
        }

        std::string serialize_prg_value_for_record_field(const vfp::DbfRecordValue &field, const PrgValue &value)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.field_type)));
            const std::string text = value_as_string(value);
            const std::string trimmed = trim_copy(text);
            if (field_type == 'D')
            {
                if (trimmed.empty())
                {
                    return {};
                }

                int year = 0;
                int month = 0;
                int day = 0;
                if (parse_runtime_or_storage_date_string(trimmed, year, month, day))
                {
                    return format_runtime_date_storage_string(year, month, day);
                }
            }
            else if (field_type == 'T')
            {
                if (trimmed.empty())
                {
                    return {};
                }

                int julian_day = 0;
                int millis = 0;
                if (parse_datetime_storage_contract(trimmed, julian_day, millis))
                {
                    return "julian:" + std::to_string(julian_day) + " millis:" + std::to_string(millis);
                }

                int year = 0;
                int month = 0;
                int day = 0;
                int hour = 0;
                int minute = 0;
                int second = 0;
                if (parse_runtime_or_storage_datetime_string(trimmed, year, month, day, hour, minute, second))
                {
                    return format_runtime_datetime_storage_string(year, month, day, hour, minute, second);
                }
                if (parse_runtime_or_storage_date_string(trimmed, year, month, day))
                {
                    return format_runtime_datetime_storage_string(year, month, day, 0, 0, 0);
                }
            }

            return text;
        }

        PrgValue record_value_to_prg_value(const vfp::DbfRecordValue &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.field_type)));
            const std::string text = trim_copy(field.display_value);
            if (field.is_null)
            {
                return make_empty_value();
            }
            if (field_type == 'L')
            {
                return make_boolean_value(normalize_identifier(text) == "true" || normalize_identifier(text) == "t" ||
                                          normalize_identifier(text) == "y" || text == ".T.");
            }
            if (field_type == 'N' || field_type == 'F' || field_type == 'I' ||
                field_type == 'B' || field_type == 'Y')
            {
                if (text.empty())
                {
                    return make_number_value(0.0);
                }
                try
                {
                    return make_number_value(std::stod(text));
                }
                catch (const std::exception &)
                {
                    return make_string_value(field.display_value);
                }
            }
            if (field_type == 'D')
            {
                if (text.empty())
                {
                    return make_string_value("");
                }

                int year = 0;
                int month = 0;
                int day = 0;
                if (parse_runtime_or_storage_date_string(field.display_value, year, month, day))
                {
                    return make_string_value(format_runtime_date_string(year, month, day));
                }
            }
            if (field_type == 'T')
            {
                if (text.empty())
                {
                    return make_string_value("");
                }

                int julian_day = 0;
                int millis = 0;
                if (parse_datetime_storage_contract(field.display_value, julian_day, millis))
                {
                    if (julian_day == 0 && millis == 0)
                    {
                        return make_string_value("");
                    }

                    int year = 0;
                    int month = 0;
                    int day = 0;
                    if (julian_to_runtime_date(julian_day, year, month, day))
                    {
                        const int total_seconds = millis / 1000;
                        const int hour = total_seconds / 3600;
                        const int minute = (total_seconds / 60) % 60;
                        const int second = total_seconds % 60;
                        if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59)
                        {
                            return make_string_value(format_runtime_datetime_string(year, month, day, hour, minute, second));
                        }
                    }
                }

                int year = 0;
                int month = 0;
                int day = 0;
                int hour = 0;
                int minute = 0;
                int second = 0;
                if (parse_runtime_or_storage_datetime_string(field.display_value, year, month, day, hour, minute, second))
                {
                    return make_string_value(format_runtime_datetime_string(year, month, day, hour, minute, second));
                }
            }
            return make_string_value(field.display_value);
        }

        PrgValue blank_value_for_field(const vfp::DbfRecordValue &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.field_type)));
            if (field_type == 'L')
            {
                return make_boolean_value(false);
            }
            if (field_type == 'N' || field_type == 'F' || field_type == 'I' ||
                field_type == 'B' || field_type == 'Y')
            {
                return make_number_value(0.0);
            }
            if (field_type == 'D' || field_type == 'T')
            {
                return make_string_value("");
            }
            return make_empty_value();
        }

        int classify_runtime_error_code(const std::string &message)
        {
            const std::string normalized = normalize_identifier(message);
            if (normalized.find("unable to resolve do target") != std::string::npos)
            {
                return 1001;
            }
            if (normalized.find("work area not found") != std::string::npos ||
                normalized.find("no current work area") != std::string::npos)
            {
                return 1002;
            }
            if (normalized.find("sql handle not found") != std::string::npos ||
                normalized.find("sqlexec") != std::string::npos ||
                normalized.find("sqlprepare") != std::string::npos ||
                normalized.find("odbc") != std::string::npos)
            {
                return 1526;
            }
            if (normalized.find("ole object") != std::string::npos ||
                normalized.find("automation") != std::string::npos)
            {
                return 1429;
            }
            if (normalized.find("unable to open") != std::string::npos ||
                normalized.find("unable to write") != std::string::npos ||
                normalized.find("file") != std::string::npos)
            {
                return 1003;
            }
            if (normalized.find("resource fault") != std::string::npos ||
                normalized.find("budget") != std::string::npos ||
                normalized.find("loop") != std::string::npos)
            {
                return 1099;
            }
            return 1;
        }

        vfp::DbfRecord make_synthetic_sql_record(std::size_t recno)
        {
            const auto synthetic_name = [&]()
            {
                switch (recno)
                {
                case 1U:
                    return std::string{"ALPHA"};
                case 2U:
                    return std::string{"BRAVO"};
                case 3U:
                    return std::string{"CHARLIE"};
                default:
                    return "ROW" + std::to_string(recno);
                }
            };

            return vfp::DbfRecord{
                .record_index = recno - 1U,
                .deleted = false,
                .values = {
                    vfp::DbfRecordValue{.field_name = "ID", .field_type = 'N', .display_value = std::to_string(recno)},
                    vfp::DbfRecordValue{.field_name = "NAME", .field_type = 'C', .display_value = synthetic_name()},
                    vfp::DbfRecordValue{.field_name = "AMOUNT", .field_type = 'N', .display_value = std::to_string(recno * 10U)},
                }};
        }

