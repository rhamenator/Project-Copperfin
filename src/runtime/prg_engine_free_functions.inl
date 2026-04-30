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

        std::string dif_escape_string(std::string value)
        {
            std::string escaped;
            escaped.reserve(value.size() + 2U);
            for (const char ch : value)
            {
                if (ch == '"')
                {
                    escaped.push_back('"');
                }
                escaped.push_back(ch);
            }
            return escaped;
        }

        std::string dif_unescape_string(std::string value)
        {
            std::string unescaped;
            unescaped.reserve(value.size());
            for (std::size_t index = 0U; index < value.size(); ++index)
            {
                const char ch = value[index];
                if (ch == '"' && index + 1U < value.size() && value[index + 1U] == '"')
                {
                    unescaped.push_back('"');
                    ++index;
                    continue;
                }
                unescaped.push_back(ch);
            }
            return unescaped;
        }

        bool dif_field_prefers_numeric(const vfp::DbfFieldDescriptor &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            return field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == 'B' || field_type == 'Y';
        }

        std::string serialize_dif_table(
            const std::vector<vfp::DbfFieldDescriptor> &fields,
            const std::vector<std::vector<std::string>> &rows)
        {
            std::ostringstream dif;
            dif << "TABLE\n";
            dif << "0,1\n";
            dif << "\"Copperfin\"\n";
            dif << "VECTORS\n";
            dif << "0," << fields.size() << "\n";
            dif << "\"\"\n";
            dif << "TUPLES\n";
            dif << "0," << (rows.size() + 1U) << "\n";
            dif << "\"\"\n";
            dif << "DATA\n";
            dif << "0,0\n";
            dif << "\"\"\n";

            const auto write_row = [&](const std::vector<std::string> &row_values, bool header_row)
            {
                dif << "-1,0\n";
                dif << "BOT\n";
                for (std::size_t index = 0U; index < fields.size(); ++index)
                {
                    const std::string value = index < row_values.size() ? trim_copy(row_values[index]) : std::string{};
                    if (!header_row && dif_field_prefers_numeric(fields[index]) && !value.empty())
                    {
                        dif << "0," << value << "\n";
                        dif << "V\n";
                        continue;
                    }
                    dif << "1,0\n";
                    dif << "\"" << dif_escape_string(value) << "\"\n";
                }
            };

            std::vector<std::string> header_row;
            header_row.reserve(fields.size());
            for (const auto &field : fields)
            {
                header_row.push_back(field.name);
            }
            write_row(header_row, true);
            for (const auto &row : rows)
            {
                write_row(row, false);
            }

            dif << "-1,0\n";
            dif << "EOD\n";
            return dif.str();
        }

        std::vector<std::vector<std::string>> parse_dif_table(const std::string &contents, std::size_t expected_columns)
        {
            std::vector<std::vector<std::string>> rows;
            std::vector<std::string> current_row;
            current_row.reserve(expected_columns);
            bool in_data_section = false;
            bool in_row = false;

            std::istringstream input(contents);
            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
                if (line == "DATA")
                {
                    in_data_section = true;
                    in_row = false;
                    current_row.clear();
                    continue;
                }
                if (!in_data_section)
                {
                    continue;
                }
                if (line == "EOD")
                {
                    if (!current_row.empty())
                    {
                        rows.push_back(current_row);
                        current_row.clear();
                    }
                    break;
                }
                const std::size_t comma = line.find(',');
                if (comma == std::string::npos)
                {
                    continue;
                }

                const std::string type_token = trim_copy(line.substr(0U, comma));
                std::string payload_line;
                if (!std::getline(input, payload_line))
                {
                    break;
                }
                if (!payload_line.empty() && payload_line.back() == '\r')
                {
                    payload_line.pop_back();
                }

                if (type_token == "-1")
                {
                    if (payload_line == "BOT")
                    {
                        in_row = true;
                        current_row.clear();
                    }
                    else if (payload_line == "EOD")
                    {
                        if (!current_row.empty())
                        {
                            rows.push_back(current_row);
                            current_row.clear();
                        }
                        break;
                    }
                    continue;
                }
                if (!in_row)
                {
                    continue;
                }

                std::string value;
                if (type_token == "1")
                {
                    value = trim_copy(payload_line);
                    if (value.size() >= 2U && value.front() == '"' && value.back() == '"')
                    {
                        value = value.substr(1U, value.size() - 2U);
                    }
                    value = dif_unescape_string(value);
                }
                else
                {
                    value = trim_copy(line.substr(comma + 1U));
                }
                current_row.push_back(std::move(value));
                if (expected_columns != 0U && current_row.size() == expected_columns)
                {
                    rows.push_back(current_row);
                    current_row.clear();
                }
            }

            return rows;
        }

        std::string sylk_escape_string(std::string value)
        {
            std::string escaped;
            escaped.reserve(value.size() + 2U);
            for (const char ch : value)
            {
                if (ch == '"')
                {
                    escaped.push_back('"');
                }
                escaped.push_back(ch);
            }
            return escaped;
        }

        std::string sylk_unescape_string(std::string value)
        {
            std::string unescaped;
            unescaped.reserve(value.size());
            for (std::size_t index = 0U; index < value.size(); ++index)
            {
                const char ch = value[index];
                if (ch == '"' && index + 1U < value.size() && value[index + 1U] == '"')
                {
                    unescaped.push_back('"');
                    ++index;
                    continue;
                }
                unescaped.push_back(ch);
            }
            return unescaped;
        }

        bool sylk_field_prefers_numeric(const vfp::DbfFieldDescriptor &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            return field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == 'B' || field_type == 'Y';
        }

        std::string serialize_sylk_table(
            const std::vector<vfp::DbfFieldDescriptor> &fields,
            const std::vector<std::vector<std::string>> &rows)
        {
            std::ostringstream sylk;
            sylk << "ID;PCopperfin\n";
            sylk << "B;Y" << (rows.size() + 1U) << ";X" << fields.size() << "\n";

            const auto write_row = [&](std::size_t row_index, const std::vector<std::string> &row_values, bool header_row)
            {
                for (std::size_t column_index = 0U; column_index < fields.size(); ++column_index)
                {
                    const std::string value = column_index < row_values.size() ? trim_copy(row_values[column_index]) : std::string{};
                    sylk << "C;Y" << row_index << ";X" << (column_index + 1U) << ";K";
                    if (!header_row && sylk_field_prefers_numeric(fields[column_index]) && !value.empty())
                    {
                        sylk << value;
                    }
                    else
                    {
                        sylk << "\"" << sylk_escape_string(value) << "\"";
                    }
                    sylk << "\n";
                }
            };

            std::vector<std::string> header_row;
            header_row.reserve(fields.size());
            for (const auto &field : fields)
            {
                header_row.push_back(field.name);
            }
            write_row(1U, header_row, true);
            for (std::size_t row_index = 0U; row_index < rows.size(); ++row_index)
            {
                write_row(row_index + 2U, rows[row_index], false);
            }
            sylk << "E\n";
            return sylk.str();
        }

        std::vector<std::vector<std::string>> parse_sylk_table(const std::string &contents, std::size_t expected_columns)
        {
            std::map<std::size_t, std::vector<std::string>> rows_by_index;
            std::istringstream input(contents);
            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
                if (!starts_with_insensitive(line, "C;"))
                {
                    continue;
                }

                std::size_t row_index = 0U;
                std::size_t column_index = 0U;
                std::string value_token;
                std::size_t scan = 2U;
                while (scan < line.size())
                {
                    const std::size_t next = line.find(';', scan);
                    const std::string token = line.substr(scan, next == std::string::npos ? std::string::npos : next - scan);
                    if (starts_with_insensitive(token, "Y"))
                    {
                        row_index = static_cast<std::size_t>(std::max<long long>(0LL, std::stoll(trim_copy(token.substr(1U)))));
                    }
                    else if (starts_with_insensitive(token, "X"))
                    {
                        column_index = static_cast<std::size_t>(std::max<long long>(0LL, std::stoll(trim_copy(token.substr(1U)))));
                    }
                    else if (starts_with_insensitive(token, "K"))
                    {
                        value_token = token.substr(1U);
                    }
                    if (next == std::string::npos)
                    {
                        break;
                    }
                    scan = next + 1U;
                }

                if (row_index == 0U || column_index == 0U)
                {
                    continue;
                }

                std::string value = trim_copy(value_token);
                if (value.size() >= 2U && value.front() == '"' && value.back() == '"')
                {
                    value = sylk_unescape_string(value.substr(1U, value.size() - 2U));
                }

                auto &row = rows_by_index[row_index];
                if (row.size() < expected_columns)
                {
                    row.resize(expected_columns);
                }
                if (column_index <= row.size())
                {
                    row[column_index - 1U] = std::move(value);
                }
            }

            std::vector<std::vector<std::string>> rows;
            rows.reserve(rows_by_index.size());
            for (auto &[row_index, row] : rows_by_index)
            {
                (void)row_index;
                rows.push_back(std::move(row));
            }
            return rows;
        }

        std::string json_escape_string(std::string value)
        {
            std::string escaped;
            escaped.reserve(value.size() + 4U);
            for (const char ch : value)
            {
                switch (ch)
                {
                    case '\\': escaped += "\\\\"; break;
                    case '"': escaped += "\\\""; break;
                    case '\n': escaped += "\\n"; break;
                    case '\r': escaped += "\\r"; break;
                    case '\t': escaped += "\\t"; break;
                    default: escaped.push_back(ch); break;
                }
            }
            return escaped;
        }

        std::string json_unescape_string(const std::string &value)
        {
            std::string unescaped;
            unescaped.reserve(value.size());
            for (std::size_t index = 0U; index < value.size(); ++index)
            {
                const char ch = value[index];
                if (ch == '\\' && index + 1U < value.size())
                {
                    const char escaped = value[++index];
                    switch (escaped)
                    {
                        case '\\': unescaped.push_back('\\'); break;
                        case '"': unescaped.push_back('"'); break;
                        case 'n': unescaped.push_back('\n'); break;
                        case 'r': unescaped.push_back('\r'); break;
                        case 't': unescaped.push_back('\t'); break;
                        default: unescaped.push_back(escaped); break;
                    }
                    continue;
                }
                unescaped.push_back(ch);
            }
            return unescaped;
        }

        bool json_field_prefers_numeric(const vfp::DbfFieldDescriptor &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            return field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == 'B' || field_type == 'Y';
        }

        bool json_field_is_logical(const vfp::DbfFieldDescriptor &field)
        {
            return static_cast<char>(std::toupper(static_cast<unsigned char>(field.type))) == 'L';
        }

        std::string serialize_json_records(
            const std::vector<vfp::DbfFieldDescriptor> &fields,
            const std::vector<std::vector<std::string>> &rows)
        {
            std::ostringstream json;
            json << "[\n";
            for (std::size_t row_index = 0U; row_index < rows.size(); ++row_index)
            {
                json << "  {";
                for (std::size_t field_index = 0U; field_index < fields.size(); ++field_index)
                {
                    if (field_index != 0U)
                    {
                        json << ", ";
                    }
                    const std::string value = field_index < rows[row_index].size() ? trim_copy(rows[row_index][field_index]) : std::string{};
                    json << "\"" << json_escape_string(fields[field_index].name) << "\": ";
                    if (json_field_is_logical(fields[field_index]))
                    {
                        const std::string normalized = normalize_identifier(value);
                        json << ((normalized == "true" || normalized == "t" || normalized == "y") ? "true" : "false");
                    }
                    else if (json_field_prefers_numeric(fields[field_index]) && !value.empty())
                    {
                        json << value;
                    }
                    else
                    {
                        json << "\"" << json_escape_string(value) << "\"";
                    }
                }
                json << "}";
                if (row_index + 1U < rows.size())
                {
                    json << ",";
                }
                json << "\n";
            }
            json << "]\n";
            return json.str();
        }

        std::vector<std::map<std::string, std::string>> parse_json_record_objects(const std::string &contents)
        {
            std::vector<std::map<std::string, std::string>> rows;
            std::size_t position = 0U;
            const auto skip_ws = [&]()
            {
                while (position < contents.size() && std::isspace(static_cast<unsigned char>(contents[position])) != 0)
                {
                    ++position;
                }
            };
            const auto parse_json_string = [&]() -> std::string
            {
                std::string raw;
                if (position >= contents.size() || contents[position] != '"')
                {
                    return raw;
                }
                ++position;
                while (position < contents.size())
                {
                    const char ch = contents[position++];
                    if (ch == '"')
                    {
                        break;
                    }
                    if (ch == '\\' && position < contents.size())
                    {
                        raw.push_back('\\');
                        raw.push_back(contents[position++]);
                        continue;
                    }
                    raw.push_back(ch);
                }
                return json_unescape_string(raw);
            };
            const auto parse_json_literal = [&]() -> std::string
            {
                const std::size_t start = position;
                while (position < contents.size())
                {
                    const char ch = contents[position];
                    if (ch == ',' || ch == '}' || ch == ']' || std::isspace(static_cast<unsigned char>(ch)) != 0)
                    {
                        break;
                    }
                    ++position;
                }
                return trim_copy(contents.substr(start, position - start));
            };

            skip_ws();
            if (position >= contents.size() || contents[position] != '[')
            {
                return rows;
            }
            ++position;
            while (position < contents.size())
            {
                skip_ws();
                if (position < contents.size() && contents[position] == ']')
                {
                    break;
                }
                if (position >= contents.size() || contents[position] != '{')
                {
                    break;
                }
                ++position;
                std::map<std::string, std::string> row;
                while (position < contents.size())
                {
                    skip_ws();
                    if (position < contents.size() && contents[position] == '}')
                    {
                        ++position;
                        break;
                    }
                    const std::string key = parse_json_string();
                    skip_ws();
                    if (position >= contents.size() || contents[position] != ':')
                    {
                        break;
                    }
                    ++position;
                    skip_ws();
                    std::string value;
                    if (position < contents.size() && contents[position] == '"')
                    {
                        value = parse_json_string();
                    }
                    else
                    {
                        value = parse_json_literal();
                    }
                    row[collapse_identifier(key)] = value;
                    skip_ws();
                    if (position < contents.size() && contents[position] == ',')
                    {
                        ++position;
                        continue;
                    }
                    if (position < contents.size() && contents[position] == '}')
                    {
                        ++position;
                        break;
                    }
                }
                if (!row.empty())
                {
                    rows.push_back(std::move(row));
                }
                skip_ws();
                if (position < contents.size() && contents[position] == ',')
                {
                    ++position;
                }
            }
            return rows;
        }

        std::string spreadsheet_xml_escape(std::string value)
        {
            const auto replace_all = [](std::string &text, const std::string &needle, const std::string &replacement)
            {
                std::size_t position = 0U;
                while ((position = text.find(needle, position)) != std::string::npos)
                {
                    text.replace(position, needle.size(), replacement);
                    position += replacement.size();
                }
            };

            replace_all(value, "&", "&amp;");
            replace_all(value, "<", "&lt;");
            replace_all(value, ">", "&gt;");
            replace_all(value, "\"", "&quot;");
            replace_all(value, "\'", "&apos;");
            return value;
        }

        std::string spreadsheet_xml_unescape(std::string value)
        {
            const auto replace_all = [](std::string &text, const std::string &needle, const std::string &replacement)
            {
                std::size_t position = 0U;
                while ((position = text.find(needle, position)) != std::string::npos)
                {
                    text.replace(position, needle.size(), replacement);
                    position += replacement.size();
                }
            };

            replace_all(value, "&lt;", "<");
            replace_all(value, "&gt;", ">");
            replace_all(value, "&quot;", "\"");
            replace_all(value, "&apos;", "\'");
            replace_all(value, "&amp;", "&");
            return value;
        }

        std::string spreadsheetml_cell_type(const vfp::DbfFieldDescriptor &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == 'B' || field_type == 'Y')
            {
                return "Number";
            }
            if (field_type == 'L')
            {
                return "Boolean";
            }
            return "String";
        }

        std::string spreadsheetml_cell_value(const vfp::DbfFieldDescriptor &field, const std::string &raw_value)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            const std::string value = trim_copy(raw_value);
            if (field_type == 'L')
            {
                const std::string normalized = normalize_identifier(value);
                if (normalized == "true" || normalized == "t" || normalized == "y")
                {
                    return "1";
                }
                if (normalized == "false" || normalized == "f" || normalized == "n")
                {
                    return "0";
                }
                return "0";
            }
            return value;
        }

        std::string serialize_spreadsheetml_workbook(
            const std::vector<vfp::DbfFieldDescriptor> &fields,
            const std::vector<std::vector<std::string>> &rows)
        {
            std::ostringstream xml;
            xml << "<?xml version=\"1.0\"?>\n";
            xml << "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"";
            xml << " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\">\n";
            xml << "  <Worksheet ss:Name=\"Copperfin\">\n";
            xml << "    <Table>\n";
            xml << "      <Row>\n";
            for (const auto &field : fields)
            {
                xml << "        <Cell><Data ss:Type=\"String\">" << spreadsheet_xml_escape(field.name) << "</Data></Cell>\n";
            }
            xml << "      </Row>\n";
            for (const auto &row : rows)
            {
                xml << "      <Row>\n";
                for (std::size_t index = 0U; index < fields.size(); ++index)
                {
                    const std::string value = index < row.size() ? spreadsheetml_cell_value(fields[index], row[index]) : std::string{};
                    xml << "        <Cell><Data ss:Type=\"" << spreadsheetml_cell_type(fields[index]) << "\">"
                        << spreadsheet_xml_escape(value) << "</Data></Cell>\n";
                }
                xml << "      </Row>\n";
            }
            xml << "    </Table>\n";
            xml << "  </Worksheet>\n";
            xml << "</Workbook>\n";
            return xml.str();
        }

        std::vector<std::vector<std::string>> parse_spreadsheetml_workbook(const std::string &xml_text)
        {
            std::vector<std::vector<std::string>> rows;
            std::size_t scan = 0U;
            while (true)
            {
                std::size_t row_start = xml_text.find("<Row", scan);
                if (row_start == std::string::npos)
                {
                    break;
                }
                row_start = xml_text.find('>', row_start);
                if (row_start == std::string::npos)
                {
                    break;
                }
                const std::size_t row_end = xml_text.find("</Row>", row_start);
                if (row_end == std::string::npos)
                {
                    break;
                }

                const std::string row_text = xml_text.substr(row_start + 1U, row_end - row_start - 1U);
                std::vector<std::string> row_values;
                std::size_t cell_scan = 0U;
                while (true)
                {
                    std::size_t data_start = row_text.find("<Data", cell_scan);
                    if (data_start == std::string::npos)
                    {
                        break;
                    }
                    const std::size_t data_tag_end = row_text.find('>', data_start);
                    if (data_tag_end == std::string::npos)
                    {
                        break;
                    }
                    const std::size_t data_end = row_text.find("</Data>", data_tag_end);
                    if (data_end == std::string::npos)
                    {
                        break;
                    }
                    const std::string data_tag = row_text.substr(data_start, data_tag_end - data_start + 1U);
                    std::string value = spreadsheet_xml_unescape(row_text.substr(data_tag_end + 1U, data_end - data_tag_end - 1U));
                    if (data_tag.find("Boolean") != std::string::npos)
                    {
                        value = trim_copy(value) == "1" ? "true" : "false";
                    }
                    row_values.push_back(std::move(value));
                    cell_scan = data_end + 7U;
                }

                if (!row_values.empty())
                {
                    rows.push_back(std::move(row_values));
                }
                scan = row_end + 6U;
            }
            return rows;
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
