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
            return platform::read_environment_variable(name);
        }

        bool set_environment_variable_value(const std::string &name, const std::string &value)
        {
            if (name.empty())
            {
                return false;
            }

            if (value.empty())
            {
                return platform::clear_environment_variable(name);
            }
            return platform::write_environment_variable(name, value);
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
                const std::string pattern = trim_copy(trimmed.substr(5U));
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
                    const std::string token = trim_copy(
                        comma == std::string::npos ? remaining : remaining.substr(0U, comma));
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

        std::vector<vfp::DbfFieldDescriptor> filter_field_descriptors(
            const std::vector<vfp::DbfFieldDescriptor> &fields,
            const std::vector<std::string> &field_filter,
            bool preserve_explicit_field_order = false)
        {
            if (field_filter.empty())
            {
                return fields;
            }

            const bool preserve_order =
                preserve_explicit_field_order &&
                field_filter[0] != "__LIKE__" &&
                field_filter[0] != "__EXCEPT__";

            std::vector<vfp::DbfFieldDescriptor> result;
            if (!preserve_order)
            {
                result.reserve(fields.size());
                for (const auto &field : fields)
                {
                    if (field_matches_filter(field.name, field_filter))
                    {
                        result.push_back(field);
                    }
                }
                return result;
            }

            result.reserve(std::min(fields.size(), field_filter.size()));
            for (const auto &token : field_filter)
            {
                const std::string normalized = collapse_identifier(token);
                if (normalized.empty())
                {
                    continue;
                }

                const bool already_added = std::find_if(
                    result.begin(),
                    result.end(),
                    [&](const vfp::DbfFieldDescriptor &candidate)
                    {
                        return collapse_identifier(candidate.name) == normalized;
                    }) != result.end();
                if (already_added)
                {
                    continue;
                }

                const auto field = std::find_if(
                    fields.begin(),
                    fields.end(),
                    [&](const vfp::DbfFieldDescriptor &candidate)
                    {
                        return collapse_identifier(candidate.name) == normalized;
                    });
                if (field != fields.end())
                {
                    result.push_back(*field);
                }
            }
            return result;
        }

        bool parse_datetime_storage_contract(const std::string &raw, int &julian_day, int &millis);
        bool parse_runtime_or_storage_date_string(const std::string &raw, int &year, int &month, int &day);
        std::string format_runtime_date_storage_string(int year, int month, int day);
        std::string format_runtime_datetime_storage_string(int year, int month, int day, int hour, int minute, int second);

        std::size_t sdf_text_field_width(const vfp::DbfFieldDescriptor &field)
        {
            // SDF stores printable values, rather than the physical DBF
            // payload, for the binary numeric field families.  Keep this
            // mapping beside the formatter so import and a future complete
            // formatter share the recovered VFP layout contract.
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (field_type == 'I' || field_type == '+')
            {
                return 11U;
            }
            if (field_type == 'Y' || field_type == 'B')
            {
                return 21U;
            }
            if (field_type == 'T')
            {
                return 19U;
            }
            if (field_type == 'V')
            {
                return field.length > 0U ? field.length - 1U : 0U;
            }
            if (field_type == 'Q')
            {
                return field.length > 0U ? (field.length - 1U) * 2U : 0U;
            }
            return field.length;
        }

        bool sdf_omits_binary_object_field(const vfp::DbfFieldDescriptor &field)
        {
            // VFP9's SDF interchange has no physical column for General,
            // Blob, or Picture values. Memo import has a separate
            // compatibility policy (#6609), so it remains intentionally
            // outside this shared import/export projection.
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            return field_type == 'G' || field_type == 'W' || field_type == 'P';
        }

        bool sdf_omits_export_field(const vfp::DbfFieldDescriptor &field)
        {
            // VFP9 documents that non-table COPY TO targets omit Memo fields,
            // including an explicitly selected Memo field (#6607). SDF's
            // importer deliberately keeps Memo handling separate (#6609).
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            return sdf_omits_binary_object_field(field) || field_type == 'M';
        }

        bool dif_sylk_omits_export_field(const vfp::DbfFieldDescriptor &field)
        {
            // DIF and SYLK use the same non-table export omission rule as
            // SDF. Import-to-Memo policy remains separate (#6609).
            return sdf_omits_export_field(field);
        }

        bool has_varbinary_field(const std::vector<vfp::DbfFieldDescriptor> &fields)
        {
            return std::any_of(
                fields.begin(),
                fields.end(),
                [](const vfp::DbfFieldDescriptor &field)
                {
                    return static_cast<char>(std::toupper(static_cast<unsigned char>(field.type))) == 'Q';
                });
        }

        bool text_export_omits_object_field(const vfp::DbfFieldDescriptor &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            return field_type == 'G' || field_type == 'P' || field_type == 'W';
        }

        bool text_export_omits_delimited_field(const vfp::DbfFieldDescriptor &field)
        {
            // VFP excludes Memo from non-table text output, while its import
            // contract remains separate (#6609). Object fields retain their
            // existing format-specific handling.
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            return text_export_omits_object_field(field) || field_type == 'M';
        }

        std::string sdf_numeric_zero_text(const vfp::DbfFieldDescriptor &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (field_type == 'Y')
            {
                return "0.0000";
            }
            if (field_type == 'B')
            {
                return "0.00";
            }
            if ((field_type == 'N' || field_type == 'F') && field.decimal_count > 0U)
            {
                return "0." + std::string(field.decimal_count, '0');
            }
            return "0";
        }

        std::string normalize_sdf_field_value_for_storage(
            const vfp::DbfFieldDescriptor &field,
            std::string value)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (field_type == 'L')
            {
                // SDF uses a narrower token contract than the DBF writer:
                // VFP accepts only uppercase T/Y as true and treats every
                // other fixed-width byte, including ?, as false.
                const std::string token = trim_copy(value);
                return token == "T" || token == "Y" ? "true" : "false";
            }
            if ((field_type == 'N' || field_type == 'F') && trim_copy(value).empty())
            {
                // VFP imports a blank printable Numeric/Float SDF cell as a
                // real zero, including for nullable targets (#6623).
                return sdf_numeric_zero_text(field);
            }
            if (field_type == 'D')
            {
                int year = 0;
                int month = 0;
                int day = 0;
                // An SDF Date cell is the eight-byte VFP printable form.
                // Invalid and blank cells append as a blank Date, rather
                // than becoming invalid DBF date bytes.
                const std::string token = trim_copy(value);
                const bool is_printable_sdf_date = token.size() == 8U &&
                    std::all_of(token.begin(), token.end(), [](unsigned char ch) {
                        return std::isdigit(ch) != 0;
                    });
                return is_printable_sdf_date && parse_runtime_date_string(token, year, month, day)
                    ? format_runtime_date_storage_string(year, month, day)
                    : std::string{};
            }
            if (field_type != 'T')
            {
                return value;
            }

            int year = 0;
            int month = 0;
            int day = 0;
            int hour = 0;
            int minute = 0;
            int second = 0;
            // VFP's printable SDF DateTime form is slash-delimited. The
            // broader runtime parser also accepts compact YYYYMMDD, which VFP
            // instead imports from a DateTime SDF column as blank.
            if (value.find('/') != std::string::npos &&
                parse_runtime_datetime_string(value, year, month, day, hour, minute, second))
            {
                return format_runtime_datetime_storage_string(year, month, day, hour, minute, second);
            }
            // VFP SDF import treats a non-printable DateTime column as a
            // blank DateTime, rather than sending its text through the
            // general DBF storage writer.
            return {};
        }

        std::optional<std::string> decode_sdf_varbinary_value(std::string value)
        {
            while (!value.empty() && value.back() == ' ')
            {
                value.pop_back();
            }
            if (value.size() % 2U != 0U)
            {
                return std::nullopt;
            }

            const auto hex_nibble = [](const unsigned char ch) -> std::optional<unsigned char>
            {
                if (ch >= '0' && ch <= '9')
                {
                    return static_cast<unsigned char>(ch - '0');
                }
                if (ch >= 'A' && ch <= 'F')
                {
                    return static_cast<unsigned char>(ch - 'A' + 10U);
                }
                if (ch >= 'a' && ch <= 'f')
                {
                    return static_cast<unsigned char>(ch - 'a' + 10U);
                }
                return std::nullopt;
            };

            std::string decoded;
            decoded.reserve(value.size() / 2U);
            for (std::size_t index = 0U; index < value.size(); index += 2U)
            {
                const auto high = hex_nibble(static_cast<unsigned char>(value[index]));
                const auto low = hex_nibble(static_cast<unsigned char>(value[index + 1U]));
                if (!high.has_value() || !low.has_value())
                {
                    return std::nullopt;
                }
                decoded.push_back(static_cast<char>((*high << 4U) | *low));
            }
            return decoded;
        }

        std::optional<std::string> format_sdf_field_value(
            const vfp::DbfFieldDescriptor &field,
            std::string value,
            const bool is_null = false)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (is_null)
            {
                // VFP9 converts a nullable DBF cell to its fixed-width SDF
                // text representation. This is serialization only: the
                // source DBF NULL flag remains intact (#6635).
                if (field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == '+' ||
                    field_type == 'B' || field_type == 'Y')
                {
                    value = sdf_numeric_zero_text(field);
                }
                else if (field_type == 'L')
                {
                    value = "F";
                }
                else
                {
                    // Character and temporal NULL values are blank in SDF.
                    value.clear();
                }
            }
            else if (field_type == 'D')
            {
                int year = 0;
                int month = 0;
                int day = 0;
                if (parse_runtime_or_storage_date_string(value, year, month, day))
                {
                    value = format_runtime_date_storage_string(year, month, day);
                }
            }
            else if (field_type == 'L')
            {
                const std::string normalized = normalize_identifier(value);
                value = normalized == "true" || normalized == "t" || normalized == "y" ? "T" : "F";
            }
            else if (field_type == 'Q')
            {
                static constexpr char hex_digits[] = "0123456789ABCDEF";
                std::string encoded;
                encoded.reserve(value.size() * 2U);
                for (const unsigned char byte : value)
                {
                    encoded.push_back(hex_digits[byte >> 4U]);
                    encoded.push_back(hex_digits[byte & 0x0fU]);
                }
                value = std::move(encoded);
            }
            else if (field_type == 'T')
            {
                int julian_day = 0;
                int millis = 0;
                if (parse_datetime_storage_contract(value, julian_day, millis))
                {
                    if (julian_day == 0 && millis == 0)
                    {
                        value.clear();
                    }
                    else
                    {
                        int year = 0;
                        int month = 0;
                        int day = 0;
                        if (julian_to_runtime_date(julian_day, year, month, day) &&
                            millis >= 0 && millis < 24 * 60 * 60 * 1000)
                        {
                            const int total_seconds = millis / 1000;
                            value = format_runtime_datetime_string(
                                year,
                                month,
                                day,
                                total_seconds / 3600,
                                (total_seconds / 60) % 60,
                                total_seconds % 60);
                        }
                    }
                }
            }
            // SDF's fixed width supplies only the storage padding.  Leading
            // spaces, tabs, and other bytes from Character-family fields are
            // application data and must not be discarded before padding.
            if (field_type != 'C' && field_type != 'V' && field_type != 'Q')
            {
                value = trim_copy(std::move(value));
            }
            const std::size_t sdf_width = sdf_text_field_width(field);
            if (value.size() > sdf_width)
            {
                // VFP writes asterisks when a Double cannot be represented in
                // its 21-column SDF field.  Do not silently turn that into a
                // different number by truncating its decimal text.  The COPY
                // TO caller reports a conversion remedy before opening the
                // destination, so the command cannot leave a partial file.
                if (field_type == 'B')
                {
                    return std::nullopt;
                }
                value = value.substr(0U, sdf_width);
            }
            if (value.size() >= sdf_width)
            {
                return value;
            }

            const std::string padding(sdf_width - value.size(), ' ');
            if (field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == '+' ||
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
            for (std::size_t index = 0U; index < contents.size(); ++index)
            {
                if (contents[index] != '\r' && contents[index] != '\n')
                {
                    continue;
                }
                if (index != start)
                {
                    lines.push_back(contents.substr(start, index - start));
                }
                if (contents[index] == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n')
                {
                    ++index;
                }
                start = index + 1U;
            }
            if (start < contents.size())
            {
                lines.push_back(contents.substr(start));
            }
            return lines;
        }

        std::vector<std::string> split_sdf_memo_records(const std::string &contents)
        {
            std::vector<std::string> records;
            std::size_t start = 0U;
            for (std::size_t index = 0U; index < contents.size(); ++index)
            {
                if (contents[index] != '\r' && contents[index] != '\n')
                {
                    continue;
                }
                records.push_back(contents.substr(start, index - start));
                if (contents[index] == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n')
                {
                    ++index;
                }
                start = index + 1U;
            }
            if (start < contents.size())
            {
                records.push_back(contents.substr(start));
            }
            return records;
        }

        std::vector<std::string_view> split_dif_sylk_records(const std::string &contents)
        {
            // Copperfin emits LF records, while native VFP9 emits CR records.
            // When LF is present, only LF (and its optional preceding CR) is a
            // delimiter so a lone CR inside a quoted value remains data. For
            // CR-only input, quoted payloads may themselves contain CR, so
            // delimit only while outside a quoted token.
            char delimiter = '\n';
            bool in_quotes = false;
            for (std::size_t index = 0U; index < contents.size(); ++index)
            {
                const char ch = contents[index];
                if (ch == '"')
                {
                    if (in_quotes && index + 1U < contents.size() && contents[index + 1U] == '"')
                    {
                        ++index;
                    }
                    else
                    {
                        in_quotes = !in_quotes;
                    }
                    continue;
                }
                if (!in_quotes && (ch == '\r' || ch == '\n'))
                {
                    delimiter = ch == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n'
                        ? '\n'
                        : ch;
                    break;
                }
            }

            std::vector<std::string_view> records;
            std::size_t record_start = 0U;
            in_quotes = false;
            for (std::size_t index = 0U; index < contents.size(); ++index)
            {
                const char ch = contents[index];
                if (delimiter == '\r' && ch == '"')
                {
                    if (in_quotes && index + 1U < contents.size() && contents[index + 1U] == '"')
                    {
                        ++index;
                    }
                    else
                    {
                        in_quotes = !in_quotes;
                    }
                    continue;
                }
                if (ch != delimiter || (delimiter == '\r' && in_quotes))
                {
                    continue;
                }
                std::size_t record_end = index;
                if (delimiter == '\n' && record_end > record_start && contents[record_end - 1U] == '\r')
                {
                    --record_end;
                }
                records.emplace_back(contents.data() + record_start, record_end - record_start);
                record_start = index + 1U;
            }
            if (record_start < contents.size())
            {
                records.emplace_back(contents.data() + record_start, contents.size() - record_start);
            }
            return records;
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
            if (copperfin::platform::path_is_hidden(entry.path()))
            {
                attributes += "H";
            }
            if (copperfin::platform::path_is_system(entry.path()))
            {
                attributes += "S";
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
            bool preserve_unquoted_quote_bytes = false;
            bool preserve_enclosed_doubled_quotes = false;
            bool truncate_enclosed_doubled_quotes = false;
        };

        DelimitedTextOptions parse_delimited_text_options(const std::string &type, const std::string &with_clause)
        {
            DelimitedTextOptions options;
            const std::string normalized_type = normalize_identifier(type);
            options.preserve_unquoted_quote_bytes =
                normalized_type == "csv" || normalized_type == "delimited";
            options.preserve_enclosed_doubled_quotes = normalized_type == "csv";
            options.truncate_enclosed_doubled_quotes = normalized_type == "delimited";
            if (normalized_type == "tab")
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
            std::string raw_value,
            const DelimitedTextOptions &options,
            const bool is_null = false)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (is_null)
            {
                // VFP9's delimited interchange values are type conversions,
                // not the decoded DBF NULL sentinels.  Date and DateTime use
                // the unquoted blank temporal placeholder (/  /); character
                // remains an empty enclosure (#6637).
                if (field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == 'B' || field_type == 'Y')
                {
                    raw_value = (field_type == 'Y') ? "0.0000" : (field_type == 'B' ? "0.00" : "0");
                }
                else if (field_type == 'L')
                {
                    raw_value = "F";
                }
                else if (field_type == 'D' || field_type == 'T')
                {
                    raw_value = "/  /";
                }
                else
                {
                    raw_value.clear();
                }
            }
            const bool quote_value = options.quote_character_fields &&
                                     !(field_type == 'N' || field_type == 'F' || field_type == 'I' || field_type == 'B' ||
                                       field_type == 'Y' || field_type == 'L' ||
                                       (is_null && (field_type == 'D' || field_type == 'T')));
            // Character fields are enclosed specifically so their text is
            // transported verbatim. Trimming here changes significant
            // leading/trailing spaces and tabs before the enclosure is written.
            const std::string value = quote_value ? raw_value : trim_copy(raw_value);
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

        std::vector<std::string> parse_delimited_text_line(
            const std::string &line,
            const DelimitedTextOptions &options,
            const std::vector<vfp::DbfFieldDescriptor> *target_fields = nullptr)
        {
            std::vector<std::string> values;
            std::string outside_before_quotes;
            std::string quoted_content;
            std::string outside_after_quotes;
            bool in_quotes = false;
            bool current_field_was_quoted = false;
            bool current_field_closed_quote = false;
            bool current_field_started_with_enclosure = false;
            bool discard_after_delimited_doubled_quote = false;
            const auto finish_current_field = [&]() {
                if (!current_field_was_quoted)
                {
                    // Installed VFP9 keeps the leading whitespace of an unquoted Character field
                    // (" Alice" and a tab-led value import verbatim); trailing whitespace is
                    // DBF padding and other target types parse their own text (#6512).
                    const bool character_target =
                        target_fields != nullptr &&
                        values.size() < target_fields->size() &&
                        std::toupper(static_cast<unsigned char>((*target_fields)[values.size()].type)) == 'C';
                    if (character_target)
                    {
                        std::string kept = outside_before_quotes;
                        while (!kept.empty() && std::isspace(static_cast<unsigned char>(kept.back())) != 0)
                        {
                            kept.pop_back();
                        }
                        return kept;
                    }
                    return trim_copy(outside_before_quotes);
                }
                // Whitespace outside an enclosure is field padding, while the
                // enclosed span is character data. Keep the latter verbatim.
                return trim_copy(outside_before_quotes) + quoted_content + trim_copy(outside_after_quotes);
            };
            for (std::size_t index = 0U; index < line.size(); ++index)
            {
                const char ch = line[index];
                if (discard_after_delimited_doubled_quote)
                {
                    if (ch == options.delimiter)
                    {
                        values.push_back(finish_current_field());
                        outside_before_quotes.clear();
                        quoted_content.clear();
                        outside_after_quotes.clear();
                        current_field_was_quoted = false;
                        current_field_closed_quote = false;
                        current_field_started_with_enclosure = false;
                        discard_after_delimited_doubled_quote = false;
                    }
                    continue;
                }
                if (ch == options.quote)
                {
                    const bool character_target =
                        target_fields != nullptr &&
                        values.size() < target_fields->size() &&
                        std::toupper(static_cast<unsigned char>((*target_fields)[values.size()].type)) == 'C';
                    if (options.preserve_unquoted_quote_bytes && character_target &&
                        !in_quotes && !current_field_was_quoted &&
                        !trim_copy(outside_before_quotes).empty())
                    {
                        // VFP treats quotes after unquoted field data as
                        // literal bytes for both CSV and DELIMITED imports.
                        outside_before_quotes.push_back(ch);
                        continue;
                    }
                    if (in_quotes && index + 1U < line.size() && line[index + 1U] == options.quote)
                    {
                        if (options.truncate_enclosed_doubled_quotes && current_field_started_with_enclosure)
                        {
                            // VFP DELIMITED treats the pair as the end of the
                            // enclosed value and ignores the remaining bytes in
                            // that field; CSV follows its separate quote rule.
                            in_quotes = false;
                            current_field_closed_quote = true;
                            discard_after_delimited_doubled_quote = true;
                        }
                        else
                        {
                            quoted_content.push_back(options.quote);
                            const bool character_target_preserves_pair =
                                options.preserve_enclosed_doubled_quotes &&
                                target_fields != nullptr &&
                                values.size() < target_fields->size() &&
                                std::toupper(static_cast<unsigned char>((*target_fields)[values.size()].type)) == 'C';
                            if (character_target_preserves_pair && current_field_started_with_enclosure)
                            {
                                // VFP CSV retains both quote bytes inside an
                                // enclosed Character value. Other target types,
                                // TAB, and unquoted fields retain their
                                // established collapse rule.
                                quoted_content.push_back(options.quote);
                            }
                        }
                        ++index;
                    }
                    else
                    {
                        in_quotes = !in_quotes;
                        current_field_was_quoted = true;
                        if (in_quotes && trim_copy(outside_before_quotes).empty())
                        {
                            current_field_started_with_enclosure = true;
                        }
                        if (!in_quotes)
                        {
                            current_field_closed_quote = true;
                        }
                    }
                    continue;
                }
                if (!in_quotes && ch == options.delimiter)
                {
                    values.push_back(finish_current_field());
                    outside_before_quotes.clear();
                    quoted_content.clear();
                    outside_after_quotes.clear();
                    current_field_was_quoted = false;
                    current_field_closed_quote = false;
                    current_field_started_with_enclosure = false;
                    continue;
                }
                if (in_quotes)
                {
                    quoted_content.push_back(ch);
                }
                else if (current_field_closed_quote)
                {
                    outside_after_quotes.push_back(ch);
                }
                else
                {
                    outside_before_quotes.push_back(ch);
                }
            }
            values.push_back(finish_current_field());
            return values;
        }

        // RQ-CF-PRG-FILE-COMMAND-OPERANDS-001 (#6582, #6589): installed VFP9 treats an
        // unquoted, unparenthesized file-command operand as a literal filename (`ERASE cF`
        // erases the file named "cF", not the file named by variable cF), while a
        // parenthesized name expression, a function call, a macro, a quoted or bracketed
        // string, or any compound expression is still evaluated.
        bool is_bare_file_command_operand(const std::string &operand)
        {
            const std::string text = trim_copy(operand);
            if (text.empty() || text == "?")
            {
                return false;
            }
            for (const unsigned char ch : text)
            {
                if (ch >= 0x80U || std::isalnum(ch) != 0)
                {
                    continue;
                }
                switch (ch)
                {
                case '_': case '.': case '-': case '\\': case '/': case ':':
                case '*': case '?': case '$': case '~': case '#': case '@':
                case '%': case '!':
                    break;
                default:
                    return false;
                }
            }
            return true;
        }

        bool file_pattern_has_wildcard(const std::string &filename)
        {
            return filename.find_first_of("*?") != std::string::npos;
        }

        // File-name characters are compared as decoded UTF-8 code points (an invalid byte counts as
        // one character), so `?` matches one character rather than one byte.
        // When `offsets` is supplied it receives the byte offset of every code point plus the
        // total size, so a run of characters can be copied back out byte for byte (a name that
        // is not valid UTF-8 must not be re-encoded into different bytes).
        std::vector<char32_t> decode_file_name_code_points(const std::string &text,
                                                           std::vector<std::size_t> *offsets = nullptr)
        {
            std::vector<char32_t> code_points;
            code_points.reserve(text.size());
            if (offsets != nullptr)
            {
                offsets->clear();
            }
            for (std::size_t index = 0U; index < text.size();)
            {
                if (offsets != nullptr)
                {
                    offsets->push_back(index);
                }
                const unsigned char lead = static_cast<unsigned char>(text[index]);
                std::size_t length = 1U;
                char32_t value = lead;
                if (lead >= 0xF0U && lead < 0xF8U) { length = 4U; value = lead & 0x07U; }
                else if (lead >= 0xE0U) { length = lead < 0xF0U ? 3U : 1U; value = lead & 0x0FU; }
                else if (lead >= 0xC0U) { length = 2U; value = lead & 0x1FU; }
                if (length > 1U && index + length <= text.size())
                {
                    bool valid = true;
                    for (std::size_t k = 1U; k < length; ++k)
                    {
                        const unsigned char continuation = static_cast<unsigned char>(text[index + k]);
                        if ((continuation & 0xC0U) != 0x80U)
                        {
                            valid = false;
                            break;
                        }
                        value = (value << 6U) | (continuation & 0x3FU);
                    }
                    if (valid)
                    {
                        code_points.push_back(value);
                        index += length;
                        continue;
                    }
                }
                code_points.push_back(lead);
                ++index;
            }
            if (offsets != nullptr)
            {
                offsets->push_back(text.size());
            }
            return code_points;
        }

        // Simple case folding for file-name matching: ASCII, Latin-1 Supplement, basic Greek and
        // basic Cyrillic. Other scripts compare exactly (no locale-dependent tolower).
        char32_t fold_file_name_code_point(char32_t cp)
        {
            if (cp >= U'A' && cp <= U'Z') { return cp + 0x20U; }
            if (cp >= 0xC0U && cp <= 0xDEU && cp != 0xD7U) { return cp + 0x20U; }
            if (cp >= 0x391U && cp <= 0x3A9U && cp != 0x3A2U) { return cp + 0x20U; }
            if (cp >= 0x410U && cp <= 0x42FU) { return cp + 0x20U; }
            if (cp >= 0x400U && cp <= 0x40FU) { return cp + 0x50U; }
            return cp;
        }

        // Case-insensitive DOS-style wildcard match that also records what each `*`/`?` matched, in
        // order, so COPY FILE and RENAME can carry those segments into a destination pattern
        // (`*.bin TO *.bak` maps one.bin to one.bak; `?ne.bin TO ?ne.bak` maps the `?`). `*` takes the
        // longest run that lets the rest match, so `*.*` splits at the last dot, and, as on Windows, a
        // trailing `.*` also matches names without a dot. Runs in O(pattern x name) time, so a hostile
        // mask cannot make expansion slow.
        bool file_wildcard_captures(const std::string &pattern,
                                    const std::string &name,
                                    std::vector<std::string> &captures)
        {
            captures.clear();
            const std::vector<char32_t> pat = decode_file_name_code_points(pattern);
            std::vector<std::size_t> txt_offsets;
            const std::vector<char32_t> txt = decode_file_name_code_points(name, &txt_offsets);
            const auto slice = [&](std::size_t begin, std::size_t end)
            {
                return name.substr(txt_offsets[begin], txt_offsets[end] - txt_offsets[begin]);
            };
            const auto solve = [&](const std::vector<char32_t> &pp, std::vector<std::string> &out) -> bool
            {
                std::vector<std::vector<char>> can(pp.size() + 1U, std::vector<char>(txt.size() + 1U, 0));
                can[pp.size()][txt.size()] = 1;
                for (std::size_t i = pp.size(); i-- > 0U;)
                {
                    for (std::size_t j = txt.size() + 1U; j-- > 0U;)
                    {
                        if (pp[i] == U'*')
                        {
                            can[i][j] = can[i + 1U][j] || (j < txt.size() && can[i][j + 1U]) ? 1 : 0;
                        }
                        else if (j < txt.size() &&
                                 (pp[i] == U'?' || fold_file_name_code_point(pp[i]) == fold_file_name_code_point(txt[j])))
                        {
                            can[i][j] = can[i + 1U][j + 1U];
                        }
                    }
                }
                if (!can[0][0])
                {
                    return false;
                }
                std::size_t i = 0U;
                std::size_t j = 0U;
                while (i < pp.size())
                {
                    if (pp[i] == U'*')
                    {
                        std::size_t end = txt.size();
                        while (end > j && !can[i + 1U][end])
                        {
                            --end;
                        }
                        out.push_back(slice(j, end));
                        j = end;
                        ++i;
                    }
                    else
                    {
                        if (pp[i] == U'?')
                        {
                            out.push_back(slice(j, j + 1U));
                        }
                        ++i;
                        ++j;
                    }
                }
                return true;
            };
            if (solve(pat, captures))
            {
                return true;
            }
            captures.clear();
            if (pat.size() >= 2U && pat[pat.size() - 2U] == U'.' && pat.back() == U'*' &&
                std::find(txt.begin(), txt.end(), U'.') == txt.end())
            {
                const std::vector<char32_t> without_extension(pat.begin(), pat.end() - 2);
                if (solve(without_extension, captures))
                {
                    captures.push_back({});
                    return true;
                }
                captures.clear();
            }
            return false;
        }

        bool file_wildcard_matches(const std::string &pattern, const std::string &name)
        {
            std::vector<std::string> captures;
            return file_wildcard_captures(pattern, name, captures);
        }

        // Substitute each wildcard in `destination_pattern` with the next captured segment
        // (an exhausted capture list contributes nothing); other characters are kept.
        std::string map_wildcard_destination(const std::string &destination_pattern,
                                             const std::vector<std::string> &captures)
        {
            std::string result;
            std::size_t next_capture = 0U;
            for (const char ch : destination_pattern)
            {
                if (ch == '*' || ch == '?')
                {
                    if (next_capture < captures.size())
                    {
                        result += captures[next_capture++];
                    }
                    continue;
                }
                result.push_back(ch);
            }
            return result;
        }

        // A VFP-style relative operand separates with backslashes, which are not separators on
        // POSIX hosts, so they are normalized there (a Windows drive or UNC path is left alone).
        std::string normalize_file_operand_separators(std::string text)
        {
#if !defined(_WIN32)
            const bool drive_path = text.size() >= 3U && std::isalpha(static_cast<unsigned char>(text[0])) != 0 &&
                                    text[1] == ':' && (text[2] == '\\' || text[2] == '/');
            const bool unc_path = text.size() >= 2U && text[0] == '\\' && text[1] == '\\';
            if (!drive_path && !unc_path)
            {
                std::replace(text.begin(), text.end(), '\\', '/');
            }
#endif
            return text;
        }

        // Path for a file-command operand; a relative result is anchored at the default directory.
        std::filesystem::path file_command_path_from_operand(const std::string &raw, const std::string &default_directory)
        {
            const std::string text = normalize_file_operand_separators(raw);
            std::filesystem::path path = copperfin::platform::path_from_utf8_string(text);
            if (path.is_relative())
            {
                path = copperfin::platform::path_from_utf8_string(default_directory) / path;
            }
            return path;
        }

        // Regular files in the pattern's directory whose name matches its filename part,
        // sorted by name. Wildcards are expanded in the filename only. A missing directory
        // or no match yields an empty list, never an error.
        std::vector<std::filesystem::path> expand_file_wildcard(const std::filesystem::path &pattern_path)
        {
            std::vector<std::filesystem::path> matches;
            const std::string pattern = copperfin::platform::path_to_utf8_string(pattern_path.filename());
            std::error_code ec;
            std::filesystem::directory_iterator iterator(pattern_path.parent_path(), ec);
            if (ec)
            {
                return matches;
            }
            for (const std::filesystem::directory_iterator end; iterator != end; iterator.increment(ec))
            {
                if (ec)
                {
                    break;
                }
                std::error_code status_ec;
                if (iterator->symlink_status(status_ec).type() != std::filesystem::file_type::regular)
                {
                    continue;
                }
                if (file_wildcard_matches(pattern, copperfin::platform::path_to_utf8_string(iterator->path().filename())))
                {
                    matches.push_back(iterator->path());
                }
            }
            std::sort(matches.begin(), matches.end());
            return matches;
        }

        std::vector<std::string> split_delimited_text_records(
            const std::string &contents,
            const DelimitedTextOptions &options,
            const std::vector<vfp::DbfFieldDescriptor> *target_fields = nullptr)
        {
            std::vector<std::string> records;
            std::string current;
            bool in_quotes = false;
            bool field_was_quoted = false;
            bool field_has_unquoted_data = false;
            std::size_t field_index = 0U;
            for (std::size_t index = 0U; index < contents.size(); ++index)
            {
                const char ch = contents[index];
                if (ch == options.quote)
                {
                    current.push_back(ch);
                    const bool character_target =
                        target_fields != nullptr &&
                        field_index < target_fields->size() &&
                        std::toupper(static_cast<unsigned char>((*target_fields)[field_index].type)) == 'C';
                    if (options.preserve_unquoted_quote_bytes && character_target &&
                        !in_quotes && !field_was_quoted && field_has_unquoted_data)
                    {
                        continue;
                    }
                    if (in_quotes && index + 1U < contents.size() && contents[index + 1U] == options.quote)
                    {
                        current.push_back(contents[++index]);
                    }
                    else
                    {
                        in_quotes = !in_quotes;
                        field_was_quoted = true;
                    }
                    continue;
                }
                if (!in_quotes && (ch == '\r' || ch == '\n'))
                {
                    records.push_back(std::move(current));
                    current.clear();
                    field_was_quoted = false;
                    field_has_unquoted_data = false;
                    field_index = 0U;
                    if (ch == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n')
                    {
                        ++index;
                    }
                    continue;
                }
                if (!in_quotes && ch == options.delimiter)
                {
                    field_was_quoted = false;
                    field_has_unquoted_data = false;
                    ++field_index;
                }
                else if (!in_quotes && !std::isspace(static_cast<unsigned char>(ch)))
                {
                    field_has_unquoted_data = true;
                }
                current.push_back(ch);
            }
            if (!current.empty() || (!contents.empty() && contents.back() != '\r' && contents.back() != '\n'))
            {
                records.push_back(std::move(current));
            }
            return records;
        }

        bool delimited_text_has_valid_enclosures(
            const std::string &contents,
            const DelimitedTextOptions &options)
        {
            enum class FieldState
            {
                leading,
                unquoted,
                quoted,
                after_quote,
            };

            FieldState state = FieldState::leading;
            for (std::size_t index = 0U; index < contents.size(); ++index)
            {
                const char ch = contents[index];
                const bool record_boundary = ch == '\r' || ch == '\n';
                if (state == FieldState::quoted)
                {
                    if (ch != options.quote)
                    {
                        continue;
                    }
                    if (index + 1U < contents.size() && contents[index + 1U] == options.quote)
                    {
                        ++index;
                    }
                    else
                    {
                        state = FieldState::after_quote;
                    }
                    continue;
                }
                if (state == FieldState::after_quote)
                {
                    if (ch == options.delimiter || record_boundary)
                    {
                        state = FieldState::leading;
                        if (ch == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n')
                        {
                            ++index;
                        }
                    }
                    else if (!std::isspace(static_cast<unsigned char>(ch)))
                    {
                        return false;
                    }
                    continue;
                }
                if (state == FieldState::leading)
                {
                    if (ch == options.quote)
                    {
                        state = FieldState::quoted;
                    }
                    else if (ch == options.delimiter || record_boundary)
                    {
                        if (ch == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n')
                        {
                            ++index;
                        }
                    }
                    else if (!std::isspace(static_cast<unsigned char>(ch)))
                    {
                        state = FieldState::unquoted;
                    }
                    continue;
                }
                if (ch == options.quote)
                {
                    return false;
                }
                if (ch == options.delimiter || record_boundary)
                {
                    state = FieldState::leading;
                    if (ch == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n')
                    {
                        ++index;
                    }
                }
            }
            return state != FieldState::quoted;
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

        bool parse_dif_sylk_date(const std::string &value, int &year, int &month, int &day)
        {
            std::string digits;
            digits.reserve(8U);
            for (const char ch : trim_copy(value))
            {
                if (std::isdigit(static_cast<unsigned char>(ch)) != 0)
                {
                    digits.push_back(ch);
                }
            }
            if (digits.size() != 8U)
            {
                return false;
            }
            year = std::stoi(digits.substr(0U, 4U));
            month = std::stoi(digits.substr(4U, 2U));
            day = std::stoi(digits.substr(6U, 2U));
            return year > 0 && month >= 1 && month <= 12 &&
                   day >= 1 && day <= days_in_month(year, month);
        }

        bool parse_dif_sylk_datetime(
            const std::string &value,
            int &year,
            int &month,
            int &day,
            int &hour,
            int &minute,
            int &second)
        {
            const std::string text = trim_copy(value);
            constexpr std::string_view julian_prefix = "julian:";
            constexpr std::string_view millis_prefix = "millis:";
            const std::size_t millis_pos = text.find(millis_prefix);
            if (text.rfind(julian_prefix, 0U) != 0U || millis_pos == std::string::npos)
            {
                return false;
            }
            int julian_day = 0;
            int millis = 0;
            const std::string julian_text = trim_copy(text.substr(julian_prefix.size(), millis_pos - julian_prefix.size()));
            const std::string millis_text = trim_copy(text.substr(millis_pos + millis_prefix.size()));
            const auto julian_result = std::from_chars(
                julian_text.data(), julian_text.data() + julian_text.size(), julian_day, 10);
            const auto millis_result = std::from_chars(
                millis_text.data(), millis_text.data() + millis_text.size(), millis, 10);
            if (julian_result.ec != std::errc{} || julian_result.ptr != julian_text.data() + julian_text.size() ||
                millis_result.ec != std::errc{} || millis_result.ptr != millis_text.data() + millis_text.size() ||
                !julian_to_runtime_date(julian_day, year, month, day) ||
                millis < 0 || millis >= 24 * 60 * 60 * 1000)
            {
                return false;
            }
            hour = millis / (60 * 60 * 1000);
            minute = (millis / (60 * 1000)) % 60;
            second = (millis / 1000) % 60;
            return true;
        }

        std::string format_dif_datetime(const std::string &value)
        {
            int year = 0;
            int month = 0;
            int day = 0;
            int hour = 0;
            int minute = 0;
            int second = 0;
            if (!parse_dif_sylk_datetime(value, year, month, day, hour, minute, second))
            {
                return value;
            }
            const bool afternoon = hour >= 12;
            const int twelve_hour = hour % 12 == 0 ? 12 : hour % 12;
            std::ostringstream formatted;
            formatted << std::setfill('0') << std::setw(4) << year << '/'
                      << std::setw(2) << month << '/' << std::setw(2) << day << ' '
                      << std::setw(2) << twelve_hour << ':' << std::setw(2) << minute << ':'
                      << std::setw(2) << second << (afternoon ? " PM" : " AM");
            return formatted.str();
        }

        bool is_blank_dif_sylk_datetime(const std::string &value)
        {
            // The DBF decoder exposes VFP's all-zero DateTime storage as this
            // canonical diagnostic token.  It is a blank value, never text to
            // expose in an interchange artifact.
            return trim_copy(value) == "julian:0 millis:0";
        }

        std::string format_sylk_datetime_serial(const std::string &value)
        {
            int year = 0;
            int month = 0;
            int day = 0;
            int hour = 0;
            int minute = 0;
            int second = 0;
            if (!parse_dif_sylk_datetime(value, year, month, day, hour, minute, second))
            {
                return value;
            }
            // SYLK uses Excel's 1900 date system.  Excel reserves serial 60
            // for its compatibility-only 1900-02-29, so real dates from
            // 1900-03-01 onward need the extra day while earlier ones do not.
            int serial_day = date_to_julian(year, month, day) - date_to_julian(1899, 12, 31);
            if (date_to_julian(year, month, day) >= date_to_julian(1900, 3, 1))
            {
                ++serial_day;
            }
            const double serial = static_cast<double>(serial_day) +
                                  static_cast<double>(((hour * 60) + minute) * 60 + second) / 86400.0;
            std::ostringstream formatted;
            formatted.imbue(std::locale::classic());
            formatted << std::setprecision(15) << serial;
            return formatted.str();
        }

        std::string format_dif_cell_value(
            const vfp::DbfFieldDescriptor &field,
            std::string value,
            bool header_row)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (!header_row && (field_type == 'C' || field_type == 'V'))
            {
                // VFP removes fixed-width Character padding and significant
                // Varchar trailing spaces from DIF without stripping leading
                // whitespace (#6646/#6650).
                while (!value.empty() && value.back() == ' ')
                {
                    value.pop_back();
                }
                return value;
            }
            return trim_copy(value);
        }

        std::string serialize_dif_table(
            const std::vector<vfp::DbfFieldDescriptor> &fields,
            const std::vector<std::vector<std::string>> &rows,
            const std::vector<std::vector<bool>> &row_nulls,
            const std::vector<std::vector<std::string>> &row_raw_values)
        {
            std::ostringstream dif;
            dif.imbue(std::locale::classic());
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

            const auto write_row = [&](const std::vector<std::string> &row_values,
                                       const std::vector<bool> *nulls,
                                       const std::vector<std::string> *raw_values,
                                       bool header_row)
            {
                dif << "-1,0\n";
                dif << "BOT\n";
                for (std::size_t index = 0U; index < fields.size(); ++index)
                {
                    const char field_type = static_cast<char>(
                        std::toupper(static_cast<unsigned char>(fields[index].type)));
                    if (!header_row && field_type == 'Q')
                    {
                        const std::string &raw = (*raw_values)[index];
                        dif << "0,";
                        dif.write(raw.data(), static_cast<std::streamsize>(raw.size()));
                        dif << "\nV\n";
                        continue;
                    }
                    const std::string value = index < row_values.size()
                                                  ? format_dif_cell_value(fields[index], row_values[index], header_row)
                                                  : std::string{};
                    const bool is_null = nulls != nullptr && index < nulls->size() && (*nulls)[index];
                    if (!header_row && is_null)
                    {
                        if (field_type == 'L')
                        {
                            dif << "0,0\nFALSE\n";
                            continue;
                        }
                        if (dif_field_prefers_numeric(fields[index]))
                        {
                            dif << "0,\nV\n";
                            continue;
                        }
                        if (field_type == 'C' || field_type == 'V')
                        {
                            dif << "1,0\n\"\"\n";
                            continue;
                        }
                    }
                    if (!header_row && !is_null && field_type == 'L')
                    {
                        const std::string normalized = normalize_identifier(value);
                        const bool logical_true = normalized == "true" || normalized == "t" || normalized == "y";
                        dif << "0," << (logical_true ? "1" : "0") << "\n";
                        dif << (logical_true ? "TRUE" : "FALSE") << "\n";
                        continue;
                    }
                    if (!header_row && !is_null && field_type == 'D' && !value.empty())
                    {
                        int year = 0;
                        int month = 0;
                        int day = 0;
                        if (parse_dif_sylk_date(value, year, month, day))
                        {
                            dif << "0," << std::setfill('0') << std::setw(4) << year
                                << std::setw(2) << month << std::setw(2) << day << "\nV\n";
                            continue;
                        }
                    }
                    if (!header_row && !is_null && field_type == 'T' && !value.empty())
                    {
                        if (is_blank_dif_sylk_datetime(value))
                        {
                            dif << "1,0\n\"\"\n";
                            continue;
                        }
                        dif << "1,0\n\"" << dif_escape_string(format_dif_datetime(value)) << "\"\n";
                        continue;
                    }
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
            write_row(header_row, nullptr, nullptr, true);
            for (std::size_t row_index = 0U; row_index < rows.size(); ++row_index)
            {
                const std::vector<bool> *nulls = row_index < row_nulls.size() ? &row_nulls[row_index] : nullptr;
                const std::vector<std::string> *raw_values = &row_raw_values[row_index];
                write_row(rows[row_index], nulls, raw_values, false);
            }

            dif << "-1,0\n";
            dif << "EOD\n";
            return dif.str();
        }

        std::vector<std::vector<std::string>> parse_dif_table(
            const std::string &contents,
            const std::vector<vfp::DbfFieldDescriptor> &fields)
        {
            const std::size_t expected_columns = fields.size();
            std::vector<std::vector<std::string>> rows;
            std::vector<std::string> current_row;
            current_row.reserve(expected_columns);
            bool in_data_section = false;
            bool in_row = false;

            const std::vector<std::string_view> lines = split_dif_sylk_records(contents);
            for (std::size_t line_index = 0U; line_index < lines.size(); ++line_index)
            {
                const std::string_view line = lines[line_index];
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

                const std::string type_token = trim_copy(std::string{line.substr(0U, comma)});
                if (line_index + 1U >= lines.size())
                {
                    break;
                }
                const std::string_view payload_line = lines[++line_index];

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
                    value = trim_copy(std::string{payload_line});
                    if (value.size() >= 2U && value.front() == '"' && value.back() == '"')
                    {
                        value = value.substr(1U, value.size() - 2U);
                    }
                    value = dif_unescape_string(value);
                }
                else
                {
                    const std::size_t column_index = current_row.size();
                    const char field_type = column_index < fields.size()
                        ? static_cast<char>(std::toupper(static_cast<unsigned char>(fields[column_index].type)))
                        : '\0';
                    if (field_type == 'Q')
                    {
                        // Native VFP writes the physical Q field bytes in a
                        // numeric DIF cell. APPEND FROM accepts the row but
                        // imports that non-text cell as a blank Varbinary.
                        value.clear();
                    }
                    else
                    {
                        value = field_type == 'L'
                            ? trim_copy(std::string{payload_line})
                            : trim_copy(std::string{line.substr(comma + 1U)});
                    }
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

        std::string format_sylk_cell_value(
            const vfp::DbfFieldDescriptor &field,
            std::string value,
            bool header_row,
            std::uint8_t source_code_page_mark)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
            if (!header_row && field_type == 'C')
            {
                // VFP emits the complete fixed-width Character cell in SYLK,
                // including leading whitespace and DBF space padding (#6646).
                // The field width is measured in source DBF bytes, not in the
                // UTF-8 bytes used by the decoded runtime string.
                const vfp::DbfTextConversionResult encoded =
                    vfp::encode_dbf_text(source_code_page_mark, value);
                const std::size_t source_width = encoded.ok ? encoded.text.size() : value.size();
                if (source_width < field.length)
                {
                    value.append(field.length - source_width, ' ');
                }
                return value;
            }
            if (!header_row && field_type == 'V')
            {
                // Unlike fixed-width Character fields, Varchar cells use the
                // stored payload length and retain all payload bytes in SYLK.
                return value;
            }
            return trim_copy(value);
        }

        std::optional<std::size_t> parse_sylk_coordinate(const std::string &token)
        {
            const std::string trimmed = trim_copy(token);
            if (trimmed.empty())
            {
                return std::nullopt;
            }

            std::size_t value = 0U;
            const auto parsed = std::from_chars(
                trimmed.data(), trimmed.data() + trimmed.size(), value, 10);
            if (parsed.ec != std::errc{} || parsed.ptr != trimmed.data() + trimmed.size())
            {
                return std::nullopt;
            }
            return value;
        }

        std::string serialize_sylk_table(
            const std::vector<vfp::DbfFieldDescriptor> &fields,
            const std::vector<std::vector<std::string>> &rows,
            const std::vector<std::vector<bool>> &row_nulls,
            const std::vector<std::vector<std::string>> &row_raw_values,
            std::uint8_t source_code_page_mark)
        {
            std::ostringstream sylk;
            sylk.imbue(std::locale::classic());
            sylk << "ID;PCopperfin\n";
            sylk << "B;Y" << (rows.size() + 1U) << ";X" << fields.size() << "\n";

            const auto write_row = [&](std::size_t row_index,
                                       const std::vector<std::string> &row_values,
                                       const std::vector<bool> *nulls,
                                       const std::vector<std::string> *raw_values,
                                       bool header_row)
            {
                for (std::size_t column_index = 0U; column_index < fields.size(); ++column_index)
                {
                    sylk << "C;Y" << row_index << ";X" << (column_index + 1U) << ";K";
                    const char field_type = static_cast<char>(
                        std::toupper(static_cast<unsigned char>(fields[column_index].type)));
                    if (!header_row && field_type == 'Q')
                    {
                        const std::string &raw = (*raw_values)[column_index];
                        sylk.write(raw.data(), static_cast<std::streamsize>(raw.size()));
                        sylk << "\n";
                        continue;
                    }
                    const std::string value = column_index < row_values.size()
                                                  ? format_sylk_cell_value(
                                                        fields[column_index],
                                                        row_values[column_index],
                                                        header_row,
                                                        source_code_page_mark)
                                                  : std::string{};
                    const bool is_null = nulls != nullptr && column_index < nulls->size() && (*nulls)[column_index];
                    if (!header_row && is_null)
                    {
                        sylk << "\n";
                        continue;
                    }
                    if (!header_row && !is_null && field_type == 'L')
                    {
                        const std::string normalized = normalize_identifier(value);
                        const bool logical_true = normalized == "true" || normalized == "t" || normalized == "y";
                        sylk << "\"" << (logical_true ? "T" : "F") << "\"\n";
                        continue;
                    }
                    if (!header_row && !is_null && field_type == 'D' && !value.empty())
                    {
                        int year = 0;
                        int month = 0;
                        int day = 0;
                        if (parse_dif_sylk_date(value, year, month, day))
                        {
                            sylk << "\"" << std::setfill('0') << std::setw(4) << year
                                 << std::setw(2) << month << std::setw(2) << day << "\"\n";
                            continue;
                        }
                    }
                    if (!header_row && !is_null && field_type == 'T' && !value.empty())
                    {
                        if (is_blank_dif_sylk_datetime(value))
                        {
                            sylk << "\"\"\n";
                            continue;
                        }
                        sylk << format_sylk_datetime_serial(value) << "\n";
                        continue;
                    }
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
            write_row(1U, header_row, nullptr, nullptr, true);
            for (std::size_t row_index = 0U; row_index < rows.size(); ++row_index)
            {
                const std::vector<bool> *nulls = row_index < row_nulls.size() ? &row_nulls[row_index] : nullptr;
                const std::vector<std::string> *raw_values = &row_raw_values[row_index];
                write_row(row_index + 2U, rows[row_index], nulls, raw_values, false);
            }
            sylk << "E\n";
            return sylk.str();
        }

        std::vector<std::vector<std::string>> parse_sylk_table(
            const std::string &contents,
            const std::vector<vfp::DbfFieldDescriptor> &fields)
        {
            const std::size_t expected_columns = fields.size();
            std::map<std::size_t, std::vector<std::string>> rows_by_index;
            for (const std::string_view line_view : split_dif_sylk_records(contents))
            {
                const std::string line{line_view};
                if (!starts_with_insensitive(line, "C;"))
                {
                    continue;
                }

                std::size_t row_index = 0U;
                std::size_t column_index = 0U;
                std::string value_token;
                bool malformed_coordinate = false;
                std::size_t scan = 2U;
                while (scan < line.size())
                {
                    const std::size_t next = line.find(';', scan);
                    const std::string token = line.substr(scan, next == std::string::npos ? std::string::npos : next - scan);
                    if (starts_with_insensitive(token, "Y"))
                    {
                        const auto parsed = parse_sylk_coordinate(token.substr(1U));
                        if (!parsed.has_value())
                        {
                            malformed_coordinate = true;
                            break;
                        }
                        row_index = *parsed;
                    }
                    else if (starts_with_insensitive(token, "X"))
                    {
                        const auto parsed = parse_sylk_coordinate(token.substr(1U));
                        if (!parsed.has_value())
                        {
                            malformed_coordinate = true;
                            break;
                        }
                        column_index = *parsed;
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

                if (malformed_coordinate || row_index == 0U || column_index == 0U)
                {
                    continue;
                }

                std::string value = trim_copy(value_token);
                const bool quoted_value = value.size() >= 2U &&
                    value.front() == '"' && value.back() == '"';
                const char field_type = column_index <= fields.size()
                    ? static_cast<char>(std::toupper(static_cast<unsigned char>(fields[column_index - 1U].type)))
                    : '\0';
                if (field_type == 'Q' && !quoted_value)
                {
                    // Native physical Q cells are unquoted. VFP accepts the
                    // row but imports the binary cell as a blank Varbinary.
                    value.clear();
                }
                else if (quoted_value)
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

        // #6047: the hidden "_NullFlags" bitmap field is never user-visible
        // in real VFP9 (AFIELDS()/FCOUNT()/SELECT * all hide it), so every
        // cursor-opening call site filters it out of the field list it
        // caches, rather than the shared parse_dbf_table_from_file()
        // parser itself, which existing non-cursor consumers (schema-
        // rewrite/ALTER TABLE preservation, exporters) still expect to see
        // every physical field exactly as read.
        std::vector<vfp::DbfFieldDescriptor> visible_cursor_fields(
            const std::vector<vfp::DbfFieldDescriptor> &physical_fields)
        {
            std::vector<vfp::DbfFieldDescriptor> visible;
            visible.reserve(physical_fields.size());
            for (const auto &field : physical_fields)
            {
                if (field.type == '0' && collapse_identifier(field.name) == "NULLFLAGS")
                {
                    continue;
                }
                visible.push_back(field);
            }
            return visible;
        }

        // #6047: same filtering as visible_cursor_fields(), but for one
        // already-decoded record's per-field values (e.g. GETFLDSTATE()'s
        // -1/ordinal forms, which index directly into DbfRecord::values and
        // would otherwise see one extra hidden column).
        std::vector<vfp::DbfRecordValue> visible_record_values(
            const std::vector<vfp::DbfRecordValue> &physical_values)
        {
            std::vector<vfp::DbfRecordValue> visible;
            visible.reserve(physical_values.size());
            for (const auto &value : physical_values)
            {
                if (value.field_type == '0' && collapse_identifier(value.field_name) == "NULLFLAGS")
                {
                    continue;
                }
                visible.push_back(value);
            }
            return visible;
        }

        PrgValue record_value_to_prg_value(const vfp::DbfRecordValue &field)
        {
            const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.field_type)));
            const std::string text = trim_copy(field.display_value);
            if (field.is_null)
            {
                return make_null_value();
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
                if (const auto parsed = try_parse_invariant_double(text, field_type == 'B'); parsed.has_value())
                {
                    return make_number_value(*parsed);
                }
                return make_string_value(field.display_value);
            }
            if (field_type == 'D')
            {
                if (text.empty())
                {
                    return make_date_value("");
                }

                int year = 0;
                int month = 0;
                int day = 0;
                if (parse_runtime_or_storage_date_string(field.display_value, year, month, day))
                {
                    return make_date_value(format_runtime_date_string(year, month, day), year, month, day);
                }
            }
            if (field_type == 'T')
            {
                if (text.empty())
                {
                    return make_datetime_value("");
                }

                int julian_day = 0;
                int millis = 0;
                if (parse_datetime_storage_contract(field.display_value, julian_day, millis))
                {
                    if (julian_day == 0 && millis == 0)
                    {
                        return make_datetime_value("");
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
                            return make_datetime_value(
                                format_runtime_datetime_string(year, month, day, hour, minute, second),
                                year,
                                month,
                                day,
                                hour,
                                minute,
                                second);
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
                    return make_datetime_value(
                        format_runtime_datetime_string(year, month, day, hour, minute, second),
                        year,
                        month,
                        day,
                        hour,
                        minute,
                        second);
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
            if (field_type == 'C' || field_type == 'V' || field_type == 'D' || field_type == 'T')
            {
                if (field_type == 'D')
                {
                    return make_date_value("");
                }
                if (field_type == 'T')
                {
                    return make_datetime_value("");
                }
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
                normalized.find("ole member") != std::string::npos ||
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
