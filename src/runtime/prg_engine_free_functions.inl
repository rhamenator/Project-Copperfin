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

