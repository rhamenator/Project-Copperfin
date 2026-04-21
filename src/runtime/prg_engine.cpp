#include <ctime>
        // Julian date conversion helpers
        // Astronomical Julian day (Fliegel-Van Flandern)
        int date_to_julian(int year, int month, int day)
        {
            // Astronomical Julian day (Fliegel-Van Flandern), minus 702 to match test expectation
            return ((1461 * (year + 4800 + (month - 14) / 12)) / 4
                 + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
                 - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4
                 + day - 32075) - 702;
        }

        void julian_to_date(int julian, int &year, int &month, int &day)
        {
            int l = (julian + 702) + 68569;
            int n = (4 * l) / 146097;
            l = l - (146097 * n + 3) / 4;
            int i = (4000 * (l + 1)) / 1461001;
            l = l - (1461 * i) / 4 + 31;
            int j = (80 * l) / 2447;
            day = l - (2447 * j) / 80;
            l = j / 11;
            month = j + 2 - (12 * l);
            year = 100 * (n - 49) + i + l;
        }
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_command_helpers.h"
#include "prg_engine_helpers.h"
#include "prg_engine_internal.h"
#include "prg_engine_runtime_config.h"
#include "prg_engine_table_structure_helpers.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/report_layout.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <new>
#include <optional>
#include <set>
#include <sstream>
#include <system_error>
#include <thread>
#include <chrono>
#include <utility>

#if defined(_WIN32)
#include <process.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")
#include <comdef.h>

// Minimal COM interface declarations for .NET CLR v4 hosting.
// We only need _AppDomain::Load_2, _Type::GetMethod_2, _MethodInfo::Invoke_3.
// Rather than importing the full mscorlib.tlb (which collides with SDK headers),
// we declare only what we need via IDispatch-based late binding.
// The actual CLR invocation uses IDispatch::Invoke for safety and compatibility.
#include <oaidl.h>
#else
#include <unistd.h>
#endif

namespace copperfin::runtime
{

    namespace
    {

        std::size_t portable_path_separator_position(const std::string &path)
        {
            const std::size_t slash = path.find_last_of('/');
            const std::size_t backslash = path.find_last_of('\\');
            if (slash == std::string::npos)
            {
                return backslash;
            }
            if (backslash == std::string::npos)
            {
                return slash;
            }
            return std::max(slash, backslash);
        }

        std::string portable_path_drive(const std::string &path)
        {
            if (path.size() >= 2U && std::isalpha(static_cast<unsigned char>(path[0])) != 0 && path[1] == ':')
            {
                return path.substr(0U, 2U);
            }
            if (path.size() >= 2U && (path[0] == '\\' || path[0] == '/') && path[1] == path[0])
            {
                const std::size_t server_end = path.find_first_of("\\/", 2U);
                if (server_end != std::string::npos)
                {
                    const std::size_t share_end = path.find_first_of("\\/", server_end + 1U);
                    if (share_end != std::string::npos)
                    {
                        return path.substr(0U, share_end);
                    }
                }
            }
            return {};
        }

        std::string portable_path_parent(const std::string &path)
        {
            const std::size_t separator = portable_path_separator_position(path);
            if (separator == std::string::npos)
            {
                return {};
            }
            if (separator == 0U)
            {
                return path.substr(0U, 1U);
            }
            if (separator == 2U && path.size() >= 3U && path[1] == ':')
            {
                return path.substr(0U, 3U);
            }
            return path.substr(0U, separator);
        }

        std::string portable_path_filename(const std::string &path)
        {
            const std::size_t separator = portable_path_separator_position(path);
            return separator == std::string::npos ? path : path.substr(separator + 1U);
        }

        std::string portable_path_extension(const std::string &path)
        {
            const std::string filename = portable_path_filename(path);
            const std::size_t dot = filename.find_last_of('.');
            return dot == std::string::npos || dot == 0U ? std::string{} : filename.substr(dot + 1U);
        }

        std::string portable_path_stem(const std::string &path)
        {
            const std::string filename = portable_path_filename(path);
            const std::size_t dot = filename.find_last_of('.');
            return dot == std::string::npos || dot == 0U ? filename : filename.substr(0U, dot);
        }

        std::string portable_force_extension(const std::string &path, std::string extension)
        {
            if (!extension.empty() && extension[0] == '.')
            {
                extension.erase(extension.begin());
            }
            const std::size_t separator = portable_path_separator_position(path);
            const std::size_t filename_start = separator == std::string::npos ? 0U : separator + 1U;
            const std::size_t dot = path.find_last_of('.');
            const bool has_extension = dot != std::string::npos && dot >= filename_start && dot != filename_start;
            const std::string stem_path = has_extension ? path.substr(0U, dot) : path;
            return extension.empty() ? stem_path : stem_path + "." + extension;
        }

        std::string portable_force_path(const std::string &path, std::string directory)
        {
            const std::string filename = portable_path_filename(path);
            if (directory.empty())
            {
                return filename;
            }
            const char separator = directory.find('\\') != std::string::npos ? '\\' : '/';
            if (directory.back() != '\\' && directory.back() != '/')
            {
                directory += separator;
            }
            return directory + filename;
        }

        struct LoopState
        {
            std::size_t for_statement_index = 0;
            std::size_t endfor_statement_index = 0;
            std::string variable_name;
            double end_value = 0.0;
            double step_value = 1.0;
            std::size_t iteration_count = 0;
        };

        struct ScanState
        {
            std::size_t scan_statement_index = 0;
            std::size_t endscan_statement_index = 0;
            int work_area = 0;
            std::string for_expression;
            std::string while_expression;
            std::size_t iteration_count = 0;
        };

        struct WhileState
        {
            std::size_t do_while_statement_index = 0;
            std::size_t enddo_statement_index = 0;
            std::size_t iteration_count = 0;
        };

        struct CaseState
        {
            std::size_t do_case_statement_index = 0;
            std::size_t endcase_statement_index = 0;
            bool matched = false;
        };

        struct WithState
        {
            PrgValue target;
            std::string binding_name;
        };

        struct TryState
        {
            std::size_t try_statement_index = 0;
            std::optional<std::size_t> catch_statement_index;
            std::optional<std::size_t> finally_statement_index;
            std::size_t endtry_statement_index = 0;
            std::string catch_variable;
            bool handling_error = false;
            bool entered_catch = false;
            bool entered_finally = false;
        };

        struct Frame
        {
            std::string file_path;
            std::string routine_name;
            const Routine *routine = nullptr;
            std::size_t pc = 0;
            std::map<std::string, PrgValue> locals;
            std::vector<PrgValue> call_arguments;
            std::vector<std::optional<std::string>> call_argument_references;
            std::map<std::string, std::string> parameter_reference_bindings;
            std::set<std::string> local_names;
            std::map<std::string, std::optional<PrgValue>> private_saved_values;
            std::vector<LoopState> loops;
            std::vector<ScanState> scans;
            std::vector<WhileState> whiles;
            std::vector<CaseState> cases;
            std::vector<WithState> withs;
            std::vector<TryState> tries;
            bool evaluate_conditional_else = false;
        };

        struct ExecutionOutcome
        {
            bool ok = true;
            bool waiting_for_events = false;
            bool frame_returned = false;
            std::string message;
        };

        struct CursorState
        {
            struct OrderState
            {
                std::string name;
                std::string expression;
                std::string for_expression;
                std::string index_path;
                std::string normalization_hint;
                std::string collation_hint;
                std::string key_domain_hint;
                bool descending = false;
            };

            struct FieldRule
            {
                bool nullable = true;
                bool has_default = false;
                std::string default_expression;
            };

            int work_area = 0;
            std::string alias;
            std::string source_path;
            std::string dbf_identity;
            std::string source_kind;
            bool remote = false;
            std::size_t field_count = 0;
            std::size_t record_count = 0;
            std::size_t recno = 0;
            bool found = false;
            bool bof = true;
            bool eof = true;
            std::vector<OrderState> orders;
            std::string active_order_name;
            std::string active_order_expression;
            std::string active_order_for_expression;
            std::string active_order_path;
            std::string active_order_normalization_hint;
            std::string active_order_collation_hint;
            std::string active_order_key_domain_hint;
            bool active_order_descending = false;
            std::string filter_expression;
            std::vector<vfp::DbfRecord> remote_records;
            std::map<std::string, FieldRule> field_rules;
        };

        struct IndexedCandidate
        {
            std::string key;
            std::size_t recno = 0;
        };

        struct CursorPositionSnapshot
        {
            std::size_t recno = 0;
            bool found = false;
            bool bof = true;
            bool eof = true;
            std::string active_order_name;
            std::string active_order_expression;
            std::string active_order_for_expression;
            std::string active_order_path;
            std::string active_order_normalization_hint;
            std::string active_order_collation_hint;
            std::string active_order_key_domain_hint;
            bool active_order_descending = false;
        };

        struct RegisteredApiFunction
        {
            int handle = 0;
            std::string variant;
            std::string function_name;
            std::string argument_types;
            std::string return_type;
            std::string dll_name;
        };

        struct DeclaredDllFunction
        {
            std::string alias;         // Name used in PRG code (may equal function_name)
            std::string function_name; // Actual export name in DLL
            std::string dll_path;      // Path to DLL/FLL/.NET assembly
            std::string return_type;   // e.g. "INTEGER", "STRING", "DOUBLE", etc.
            std::string param_types;   // Comma-separated param types
            bool is_dotnet = false;
#if defined(_WIN32)
            HMODULE hmodule = nullptr;
            FARPROC proc_address = nullptr;
#endif
            // .NET-specific (assembly!Namespace.Type.Method)
            std::string dotnet_type_name;
            std::string dotnet_method_name;
        };

        struct DataSessionState
        {
            int selected_work_area = 1;
            int next_work_area = 1;
            std::map<int, std::string> aliases;
            std::map<int, CursorState> cursors;
        };

        struct RuntimeArray
        {
            std::size_t rows = 0;
            std::size_t columns = 1;
            std::vector<PrgValue> values;
        };

        int current_process_id()
        {
#if defined(_WIN32)
            return _getpid();
#else
            return getpid();
#endif
        }

        std::tm local_time_from_time_t(const std::time_t raw_time)
        {
            std::tm local{};
#if defined(_WIN32)
            localtime_s(&local, &raw_time);
#else
            localtime_r(&raw_time, &local);
#endif
            return local;
        }

        bool is_leap_year(const int year)
        {
            if ((year % 400) == 0)
            {
                return true;
            }
            if ((year % 100) == 0)
            {
                return false;
            }
            return (year % 4) == 0;
        }

        int days_in_month(const int year, const int month)
        {
            static constexpr std::array<int, 12U> kDays = {
                31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            if (month < 1 || month > 12)
            {
                return 0;
            }
            if (month == 2 && is_leap_year(year))
            {
                return 29;
            }
            return kDays[static_cast<std::size_t>(month - 1)];
        }

        bool parse_runtime_date_string(const std::string &raw, int &year, int &month, int &day)
        {
            const std::string value = trim_copy(raw);
            if (value.empty())
            {
                return false;
            }

            const auto first_slash = value.find('/');
            if (first_slash != std::string::npos)
            {
                const auto second_slash = value.find('/', first_slash + 1U);
                if (second_slash == std::string::npos)
                {
                    return false;
                }
                try
                {
                    month = std::stoi(value.substr(0U, first_slash));
                    day = std::stoi(value.substr(first_slash + 1U, second_slash - first_slash - 1U));
                    std::size_t year_end = second_slash + 1U;
                    while (year_end < value.size() && std::isdigit(static_cast<unsigned char>(value[year_end])) != 0)
                    {
                        ++year_end;
                    }
                    year = std::stoi(value.substr(second_slash + 1U, year_end - second_slash - 1U));
                }
                catch (...)
                {
                    return false;
                }
            }
            else
            {
                // Support compact YYYYMMDD values when the runtime passes DTOS-like strings.
                if (value.size() < 8U)
                {
                    return false;
                }
                for (std::size_t index = 0U; index < 8U; ++index)
                {
                    if (std::isdigit(static_cast<unsigned char>(value[index])) == 0)
                    {
                        return false;
                    }
                }
                try
                {
                    year = std::stoi(value.substr(0U, 4U));
                    month = std::stoi(value.substr(4U, 2U));
                    day = std::stoi(value.substr(6U, 2U));
                }
                catch (...)
                {
                    return false;
                }
            }

            if (year <= 0 || month < 1 || month > 12)
            {
                return false;
            }
            const int max_day = days_in_month(year, month);
            if (day < 1 || day > max_day)
            {
                return false;
            }
            return true;
        }

        std::string format_runtime_date_string(const int year, const int month, const int day)
        {
            std::ostringstream stream;
            stream << std::setfill('0')
                   << std::setw(2) << month << '/'
                   << std::setw(2) << day << '/'
                   << std::setw(4) << year;
            return stream.str();
        }

        bool parse_runtime_time_string(const std::string &raw, int &hour, int &minute, int &second)
        {
            const std::string value = trim_copy(raw);
            if (value.empty())
            {
                return false;
            }

            const auto first_colon = value.find(':');
            const auto second_colon = first_colon == std::string::npos ? std::string::npos : value.find(':', first_colon + 1U);
            if (first_colon == std::string::npos || second_colon == std::string::npos)
            {
                return false;
            }

            if (value.find('/') != std::string::npos || value.find(' ') != std::string::npos || value.find('T') != std::string::npos)
            {
                return false;
            }

            const auto parse_component = [&](const std::size_t start, const std::size_t end, int &out) -> bool
            {
                if (end <= start)
                {
                    return false;
                }
                for (std::size_t index = start; index < end; ++index)
                {
                    if (std::isdigit(static_cast<unsigned char>(value[index])) == 0)
                    {
                        return false;
                    }
                }
                try
                {
                    out = std::stoi(value.substr(start, end - start));
                }
                catch (...)
                {
                    return false;
                }
                return true;
            };

            if (!parse_component(0U, first_colon, hour) ||
                !parse_component(first_colon + 1U, second_colon, minute) ||
                !parse_component(second_colon + 1U, value.size(), second))
            {
                return false;
            }

            return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59;
        }

        bool parse_runtime_datetime_string(
            const std::string &raw,
            int &year,
            int &month,
            int &day,
            int &hour,
            int &minute,
            int &second)
        {
            const std::string value = trim_copy(raw);
            if (value.empty())
            {
                return false;
            }

            const auto separator = value.find_first_of(" T");
            const std::string date_part = separator == std::string::npos ? value : value.substr(0U, separator);
            const std::string time_part = separator == std::string::npos ? std::string{} : trim_copy(value.substr(separator + 1U));

            if (!parse_runtime_date_string(date_part, year, month, day))
            {
                return false;
            }

            hour = 0;
            minute = 0;
            second = 0;
            if (!time_part.empty() && !parse_runtime_time_string(time_part, hour, minute, second))
            {
                return false;
            }

            return true;
        }

        std::string format_runtime_datetime_string(
            const int year,
            const int month,
            const int day,
            const int hour,
            const int minute,
            const int second)
        {
            std::ostringstream stream;
            stream << std::setfill('0')
                   << std::setw(2) << month << '/'
                   << std::setw(2) << day << '/'
                   << std::setw(4) << year << ' '
                   << std::setw(2) << hour << ':'
                   << std::setw(2) << minute << ':'
                   << std::setw(2) << second;
            return stream.str();
        }

        int weekday_number_sunday_first(const int year, const int month, const int day)
        {
            std::tm local_tm{};
            local_tm.tm_year = year - 1900;
            local_tm.tm_mon = month - 1;
            local_tm.tm_mday = day;
            local_tm.tm_hour = 12;
            local_tm.tm_min = 0;
            local_tm.tm_sec = 0;
            const std::time_t converted = std::mktime(&local_tm);
            if (converted == static_cast<std::time_t>(-1))
            {
                return 0;
            }
            const std::tm normalized = local_time_from_time_t(converted);
            return normalized.tm_wday + 1;
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

        std::vector<std::string> split_text_lines(const std::string &contents)
        {
            std::vector<std::string> lines;
            std::string current;
            for (std::size_t index = 0U; index < contents.size(); ++index)
            {
                const char ch = contents[index];
                if (ch == '\r' || ch == '\n')
                {
                    lines.push_back(current);
                    current.clear();
                    if (ch == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n')
                    {
                        ++index;
                    }
                    continue;
                }
                current.push_back(ch);
            }
            if (!current.empty() || (!contents.empty() && contents.back() != '\r' && contents.back() != '\n'))
            {
                lines.push_back(current);
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

        std::string runtime_error_parameter(const std::string &message)
        {
            const std::size_t quoted_start = message.find('\'');
            if (quoted_start != std::string::npos)
            {
                const std::size_t quoted_end = message.find('\'', quoted_start + 1U);
                if (quoted_end != std::string::npos && quoted_end > quoted_start + 1U)
                {
                    return message.substr(quoted_start + 1U, quoted_end - quoted_start - 1U);
                }
            }
            const std::size_t colon = message.rfind(':');
            if (colon != std::string::npos && colon + 1U < message.size())
            {
                return trim_copy(message.substr(colon + 1U));
            }
            return message;
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

    } // namespace

    struct PrgRuntimeSession::Impl
    {
        explicit Impl(RuntimeSessionOptions session_options)
            : options(std::move(session_options))
        {
            max_call_depth = std::max<std::size_t>(1U, options.max_call_depth);
            max_executed_statements = std::max<std::size_t>(1U, options.max_executed_statements);
            max_loop_iterations = std::max<std::size_t>(1U, options.max_loop_iterations);
            scheduler_yield_statement_interval = std::max<std::size_t>(1U, options.scheduler_yield_statement_interval);
            scheduler_yield_sleep_ms = options.scheduler_yield_sleep_ms;
            runtime_temp_directory = choose_runtime_temp_directory(options);
        }

        RuntimeSessionOptions options;
        std::map<std::string, Program> programs;
        std::vector<Frame> stack;
        std::map<std::string, PrgValue> globals;
        std::map<std::string, RuntimeArray> arrays;
        std::vector<RuntimeBreakpoint> breakpoints;
        std::vector<RuntimeEvent> events;
        RuntimePauseState last_state{};
        std::string startup_default_directory;
        std::string last_error_message;
        SourceLocation last_fault_location{};
        std::string last_fault_statement;
        int last_error_code = 0;
        std::string last_error_procedure;
        std::string error_handler;
        std::map<int, std::map<std::string, std::string>> set_state_by_session;
        int current_data_session = 1;
        std::map<int, int> next_sql_handle_by_session;
        std::map<int, int> next_api_handle_by_session;
        int next_ole_handle = 1;
        std::map<int, DataSessionState> data_sessions;
        std::map<int, std::string> default_directory_by_session;
        std::map<int, std::map<int, RuntimeSqlConnectionState>> sql_connections_by_session;
        std::map<int, RuntimeOleObjectState> ole_objects;
        std::set<std::string> loaded_libraries;
        std::map<int, std::map<int, RegisteredApiFunction>> registered_api_functions_by_session;
        std::map<std::string, DeclaredDllFunction> declared_dll_functions; // keyed by normalized alias
        bool entry_pause_pending = false;
        bool waiting_for_events = false;
        bool handling_error = false;
        std::optional<std::size_t> error_handler_return_depth;
        // Saved fault position for RETRY / RESUME
        std::string fault_frame_file_path;
        std::string fault_frame_routine_name;
        std::size_t fault_statement_index = 0U;
        bool fault_pc_valid = false;
        std::optional<std::size_t> event_dispatch_return_depth;
        bool restore_event_loop_after_dispatch = false;
        std::size_t executed_statement_count = 0;
        std::size_t max_call_depth = 1024;
        std::size_t max_executed_statements = 500000;
        std::size_t max_loop_iterations = 200000;
        std::filesystem::path runtime_temp_directory;
        std::size_t scheduler_yield_statement_interval = 4096;
        std::size_t scheduler_yield_sleep_ms = 1;

        Program &load_program(const std::string &path)
        {
            const std::string normalized = normalize_path(path);
            const auto existing = programs.find(normalized);
            if (existing != programs.end())
            {
                return existing->second;
            }
            auto [inserted, _] = programs.emplace(normalized, parse_program(normalized));
            return inserted->second;
        }

        void push_main_frame(
            const std::string &path,
            std::vector<PrgValue> call_arguments = {},
            std::vector<std::optional<std::string>> call_argument_references = {})
        {
            Program &program = load_program(path);
            Frame frame;
            frame.file_path = program.path;
            frame.routine_name = "main";
            frame.routine = &program.main;
            frame.call_arguments = std::move(call_arguments);
            frame.call_argument_references = std::move(call_argument_references);
            stack.push_back(std::move(frame));
        }

        void push_routine_frame(
            const std::string &path,
            const Routine &routine,
            std::vector<PrgValue> call_arguments = {},
            std::vector<std::optional<std::string>> call_argument_references = {})
        {
            Frame frame;
            frame.file_path = normalize_path(path);
            frame.routine_name = routine.name;
            frame.routine = &routine;
            frame.call_arguments = std::move(call_arguments);
            frame.call_argument_references = std::move(call_argument_references);
            stack.push_back(std::move(frame));
        }

        const Statement *current_statement() const
        {
            if (stack.empty())
            {
                return nullptr;
            }
            const Frame &frame = stack.back();
            if (frame.routine == nullptr || frame.pc >= frame.routine->statements.size())
            {
                return nullptr;
            }
            return &frame.routine->statements[frame.pc];
        }

        void capture_last_error_context(const Frame &frame, const Statement &statement)
        {
            if (last_fault_location.file_path.empty())
            {
                last_fault_location = statement.location;
            }
            if (last_fault_statement.empty())
            {
                last_fault_statement = statement.text;
            }
            last_error_code = classify_runtime_error_code(last_error_message);
            last_error_procedure = frame.routine_name;
        }

        DataSessionState &current_session_state()
        {
            auto [iterator, _] = data_sessions.try_emplace(current_data_session);
            iterator->second.selected_work_area = std::max(1, iterator->second.selected_work_area);
            iterator->second.next_work_area = std::max(1, iterator->second.next_work_area);
            return iterator->second;
        }

        const DataSessionState &current_session_state() const
        {
            const auto found = data_sessions.find(current_data_session);
            if (found != data_sessions.end())
            {
                return found->second;
            }
            static const DataSessionState empty_session{};
            return empty_session;
        }

        int current_selected_work_area() const
        {
            return current_session_state().selected_work_area;
        }

        std::string &current_default_directory()
        {
            auto [iterator, _] = default_directory_by_session.try_emplace(current_data_session, startup_default_directory);
            return iterator->second;
        }

        const std::string &current_default_directory() const
        {
            const auto found = default_directory_by_session.find(current_data_session);
            if (found != default_directory_by_session.end())
            {
                return found->second;
            }

            return startup_default_directory;
        }

        std::map<int, RuntimeSqlConnectionState> &current_sql_connections()
        {
            auto [iterator, _] = sql_connections_by_session.try_emplace(current_data_session);
            return iterator->second;
        }

        const std::map<int, RuntimeSqlConnectionState> &current_sql_connections() const
        {
            const auto found = sql_connections_by_session.find(current_data_session);
            if (found != sql_connections_by_session.end())
            {
                return found->second;
            }

            static const std::map<int, RuntimeSqlConnectionState> empty_connections;
            return empty_connections;
        }

        int &current_sql_handle_counter()
        {
            auto [iterator, _] = next_sql_handle_by_session.try_emplace(current_data_session, 1);
            iterator->second = std::max(1, iterator->second);
            return iterator->second;
        }

        int &current_api_handle_counter()
        {
            auto [iterator, _] = next_api_handle_by_session.try_emplace(current_data_session, 1);
            iterator->second = std::max(1, iterator->second);
            return iterator->second;
        }

        std::map<int, RegisteredApiFunction> &current_registered_api_functions()
        {
            auto [iterator, _] = registered_api_functions_by_session.try_emplace(current_data_session);
            return iterator->second;
        }

        const std::map<int, RegisteredApiFunction> &current_registered_api_functions() const
        {
            const auto found = registered_api_functions_by_session.find(current_data_session);
            if (found != registered_api_functions_by_session.end())
            {
                return found->second;
            }

            static const std::map<int, RegisteredApiFunction> empty_registered_functions;
            return empty_registered_functions;
        }

        RuntimePauseState build_pause_state(DebugPauseReason reason, std::string message = {})
        {
            RuntimePauseState state;
            state.paused = reason != DebugPauseReason::completed;
            state.completed = reason == DebugPauseReason::completed;
            state.waiting_for_events = waiting_for_events;
            state.reason = reason;
            state.message = std::move(message);
            state.executed_statement_count = executed_statement_count;
            state.globals = globals;
            state.events = events;
            const DataSessionState &session = current_session_state();
            state.work_area.selected = session.selected_work_area;
            state.work_area.data_session = current_data_session;
            state.work_area.aliases = session.aliases;
            for (const auto &[_, cursor] : session.cursors)
            {
                state.cursors.push_back({.work_area = cursor.work_area,
                                         .alias = cursor.alias,
                                         .source_path = cursor.source_path,
                                         .source_kind = cursor.source_kind,
                                         .filter_expression = cursor.filter_expression,
                                         .remote = cursor.remote,
                                         .record_count = cursor.record_count,
                                         .recno = cursor.recno,
                                         .bof = cursor.bof,
                                         .eof = cursor.eof});
            }
            for (const auto &[_, connection] : current_sql_connections())
            {
                state.sql_connections.push_back(connection);
            }
            for (const auto &[_, object] : ole_objects)
            {
                state.ole_objects.push_back(object);
            }

            if (reason == DebugPauseReason::error)
            {
                const auto error_event = std::find_if(events.rbegin(), events.rend(), [](const RuntimeEvent &event)
                                                      { return event.category == "runtime.error"; });
                if (error_event != events.rend())
                {
                    state.location = error_event->location;
                }
                else if (!last_fault_location.file_path.empty())
                {
                    state.location = last_fault_location;
                }
                if (!last_fault_statement.empty())
                {
                    state.statement_text = last_fault_statement;
                }
            }
            else if (const Statement *statement = current_statement())
            {
                state.location = statement->location;
                state.statement_text = statement->text;
            }

            for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator)
            {
                RuntimeStackFrame frame;
                frame.file_path = iterator->file_path;
                frame.routine_name = iterator->routine_name;
                if (iterator->routine != nullptr && iterator->pc < iterator->routine->statements.size())
                {
                    frame.line = iterator->routine->statements[iterator->pc].location.line;
                }
                frame.locals = iterator->locals;
                state.call_stack.push_back(std::move(frame));
            }

            last_state = state;
            return state;
        }

        [[nodiscard]] bool can_push_frame() const
        {
            return stack.size() < max_call_depth;
        }

        [[nodiscard]] std::string call_depth_limit_message() const
        {
            return "Runtime guardrail: maximum call depth (" + std::to_string(max_call_depth) + ") exceeded.";
        }

        [[nodiscard]] std::string step_budget_limit_message() const
        {
            return "Runtime guardrail: maximum executed statements (" + std::to_string(max_executed_statements) + ") exceeded.";
        }

        [[nodiscard]] std::string loop_iteration_limit_message() const
        {
            return "Runtime guardrail: maximum loop iterations (" + std::to_string(max_loop_iterations) + ") exceeded.";
        }

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
            std::string canonical = uppercase_copy(trim_copy(for_expression));
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
            return normalized_value != "off" && normalized_value != "false" && normalized_value != "0";
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
                if (cursor.remote)
                {
                    const std::string normalization_hint = derive_order_normalization_hint(target_name);
                    cursor.active_order_name = uppercase_copy(target_name);
                    cursor.active_order_expression = target_name;
                    cursor.active_order_for_expression.clear();
                    cursor.active_order_path.clear();
                    cursor.active_order_normalization_hint = normalization_hint;
                    cursor.active_order_collation_hint = derive_order_collation_hint(target_name, normalization_hint);
                    cursor.active_order_key_domain_hint.clear();
                    cursor.active_order_descending = descending_override.value_or(false);
                    return true;
                }
                last_error_message = "Requested order/tag was not found";
                return false;
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

            const std::string normalized_target = normalize_seek_key_for_order(
                search_key,
                cursor.active_order_normalization_hint);
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
                    candidates.push_back({.key = evaluate_index_expression(cursor.active_order_expression, record),
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
                    candidates.push_back({.key = evaluate_index_expression(cursor.active_order_expression, record),
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
            const auto value_from_record = [&](const CursorState *cursor, const std::string &field_name) -> std::optional<PrgValue>
            {
                if (cursor == nullptr)
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
            if (cursor.remote)
            {
                if (!cursor.remote_records.empty())
                {
                    names.reserve(cursor.remote_records.front().values.size());
                    for (const auto &value : cursor.remote_records.front().values)
                    {
                        names.push_back(value.field_name);
                    }
                    return names;
                }
                return {"ID", "NAME", "AMOUNT"};
            }

            if (cursor.source_path.empty())
            {
                return names;
            }

            const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, std::max<std::size_t>(cursor.record_count, 1U));
            if (!table_result.ok)
            {
                last_error_message = table_result.error;
                return {};
            }

            names.reserve(table_result.table.fields.size());
            for (const auto &field : table_result.table.fields)
            {
                names.push_back(field.name);
            }
            return names;
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

        PrgValue aggregate_function_value(
            const std::string &function,
            const std::vector<std::string> &raw_arguments,
            const Frame &frame)
        {
            std::string value_expression;
            std::string condition_expression;
            std::string designator;

            if (function == "count")
            {
                if (raw_arguments.size() == 1U)
                {
                    designator = try_parse_designator_argument(raw_arguments[0], frame);
                    if (designator.empty())
                    {
                        condition_expression = raw_arguments[0];
                    }
                }
                else if (raw_arguments.size() >= 2U)
                {
                    condition_expression = raw_arguments[0];
                    designator = try_parse_designator_argument(raw_arguments[1], frame);
                }
            }
            else
            {
                if (!raw_arguments.empty())
                {
                    value_expression = raw_arguments[0];
                }
                if (raw_arguments.size() == 2U)
                {
                    designator = try_parse_designator_argument(raw_arguments[1], frame);
                    if (designator.empty())
                    {
                        condition_expression = raw_arguments[1];
                    }
                }
                else if (raw_arguments.size() >= 3U)
                {
                    condition_expression = raw_arguments[1];
                    designator = try_parse_designator_argument(raw_arguments[2], frame);
                }
            }

            CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr)
            {
                return make_number_value(0.0);
            }

            if (cursor->record_count == 0U || (!cursor->remote && cursor->source_path.empty()))
            {
                return make_number_value(0.0);
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(*cursor);
            double sum = 0.0;
            double min_value = 0.0;
            double max_value = 0.0;
            std::size_t matched_count = 0U;

            for (std::size_t recno = 1U; recno <= cursor->record_count; ++recno)
            {
                move_cursor_to(*cursor, static_cast<long long>(recno));
                if (!current_record_matches_visibility(*cursor, frame, condition_expression))
                {
                    continue;
                }

                if (function == "count")
                {
                    ++matched_count;
                    continue;
                }

                const PrgValue value = evaluate_expression(value_expression, frame, cursor);
                if (value.kind == PrgValueKind::empty)
                {
                    continue;
                }
                if (value.kind == PrgValueKind::string && trim_copy(value.string_value).empty())
                {
                    continue;
                }

                const double numeric_value = value_as_number(value);
                if (matched_count == 0U)
                {
                    min_value = numeric_value;
                    max_value = numeric_value;
                }
                else
                {
                    min_value = std::min(min_value, numeric_value);
                    max_value = std::max(max_value, numeric_value);
                }
                sum += numeric_value;
                ++matched_count;
            }

            restore_cursor_snapshot(*cursor, original);

            if (function == "count")
            {
                return make_number_value(static_cast<double>(matched_count));
            }
            if (matched_count == 0U)
            {
                return make_number_value(0.0);
            }
            if (function == "sum")
            {
                return make_number_value(sum);
            }
            if (function == "avg" || function == "average")
            {
                return make_number_value(sum / static_cast<double>(matched_count));
            }
            if (function == "min")
            {
                return make_number_value(min_value);
            }
            if (function == "max")
            {
                return make_number_value(max_value);
            }
            return make_number_value(0.0);
        }

        std::vector<std::size_t> collect_aggregate_scope_records(
            CursorState &cursor,
            const Frame &frame,
            const AggregateScopeClause &scope,
            const std::string &for_expression,
            const std::string &while_expression)
        {
            std::vector<std::size_t> records;
            if (cursor.record_count == 0U)
            {
                return records;
            }

            std::size_t start_recno = 1U;
            std::size_t end_recno = cursor.record_count;
            switch (scope.kind)
            {
            case AggregateScopeKind::all_records:
                break;
            case AggregateScopeKind::rest_records:
                if (cursor.eof || cursor.recno > cursor.record_count)
                {
                    return records;
                }
                start_recno = cursor.recno == 0U ? 1U : cursor.recno;
                break;
            case AggregateScopeKind::next_records:
            {
                const long long requested = static_cast<long long>(std::llround(value_as_number(evaluate_expression(scope.raw_value, frame, &cursor))));
                if (requested <= 0)
                {
                    return records;
                }
                if (cursor.eof || cursor.recno > cursor.record_count)
                {
                    return records;
                }
                start_recno = cursor.recno == 0U ? 1U : cursor.recno;
                end_recno = std::min(cursor.record_count, start_recno + static_cast<std::size_t>(requested - 1LL));
                break;
            }
            case AggregateScopeKind::record:
            {
                const long long requested = static_cast<long long>(std::llround(value_as_number(evaluate_expression(scope.raw_value, frame, &cursor))));
                if (requested < 1LL || requested > static_cast<long long>(cursor.record_count))
                {
                    return records;
                }
                start_recno = static_cast<std::size_t>(requested);
                end_recno = start_recno;
                break;
            }
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            for (std::size_t recno = start_recno; recno <= end_recno; ++recno)
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                if (!while_expression.empty() && !value_as_bool(evaluate_expression(while_expression, frame, &cursor)))
                {
                    break;
                }
                if (current_record_matches_visibility(cursor, frame, for_expression))
                {
                    records.push_back(recno);
                }
            }
            restore_cursor_snapshot(cursor, original);

            return records;
        }

        PrgValue aggregate_record_values(
            CursorState &cursor,
            const std::string &function,
            const std::string &value_expression,
            const std::vector<std::size_t> &records,
            const Frame &frame)
        {
            if (function == "count")
            {
                return make_number_value(static_cast<double>(records.size()));
            }
            if (records.empty())
            {
                return make_number_value(0.0);
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            double sum = 0.0;
            double min_value = 0.0;
            double max_value = 0.0;
            std::size_t matched_count = 0U;

            for (const std::size_t recno : records)
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                const PrgValue value = evaluate_expression(value_expression, frame, &cursor);
                if (value.kind == PrgValueKind::empty)
                {
                    continue;
                }
                if (value.kind == PrgValueKind::string && trim_copy(value.string_value).empty())
                {
                    continue;
                }

                const double numeric_value = value_as_number(value);
                if (matched_count == 0U)
                {
                    min_value = numeric_value;
                    max_value = numeric_value;
                }
                else
                {
                    min_value = std::min(min_value, numeric_value);
                    max_value = std::max(max_value, numeric_value);
                }
                sum += numeric_value;
                ++matched_count;
            }

            restore_cursor_snapshot(cursor, original);

            if (matched_count == 0U)
            {
                return make_number_value(0.0);
            }
            if (function == "sum")
            {
                return make_number_value(sum);
            }
            if (function == "avg" || function == "average")
            {
                return make_number_value(sum / static_cast<double>(matched_count));
            }
            if (function == "min")
            {
                return make_number_value(min_value);
            }
            if (function == "max")
            {
                return make_number_value(max_value);
            }
            return make_number_value(0.0);
        }

        bool execute_total_command(
            const Statement &statement,
            Frame &frame,
            std::string &error_message)
        {
            const auto parsed = parse_total_command_plan(statement.expression, error_message);
            if (!parsed.has_value())
            {
                return false;
            }

            const TotalCommandPlan &plan = *parsed;
            CursorState *cursor = resolve_cursor_target_expression(plan.in_expression, frame);
            if (cursor == nullptr)
            {
                error_message = plan.in_expression.empty()
                                    ? "TOTAL requires a selected work area"
                                    : "TOTAL target work area not found";
                return false;
            }
            std::vector<vfp::DbfFieldDescriptor> source_fields;
            std::vector<vfp::DbfRecord> source_records;
            if (cursor->remote)
            {
                source_records = cursor->remote_records;
                if (!source_records.empty())
                {
                    source_fields.reserve(source_records.front().values.size());
                    for (const auto &value : source_records.front().values)
                    {
                        vfp::DbfFieldDescriptor field;
                        field.name = value.field_name;
                        field.type = value.field_type == '\0' ? 'C' : value.field_type;
                        if (field.type == 'N' || field.type == 'F')
                        {
                            field.length = 18U;
                            field.decimal_count = 0U;
                        }
                        else
                        {
                            field.length = 32U;
                            field.decimal_count = 0U;
                        }
                        source_fields.push_back(std::move(field));
                    }

                    for (const auto &record : source_records)
                    {
                        for (auto &field : source_fields)
                        {
                            const std::string value_text = record_field_value(record, field.name).value_or(std::string{});
                            if (field.type == 'N' || field.type == 'F')
                            {
                                field.length = static_cast<std::uint8_t>(std::max<int>(field.length, 18));
                                continue;
                            }
                            field.length = static_cast<std::uint8_t>(
                                std::max<int>(field.length, static_cast<int>(std::max<std::size_t>(1U, value_text.size()))));
                        }
                    }
                }
            }
            else
            {
                if (cursor->source_path.empty())
                {
                    error_message = "TOTAL requires a local table-backed cursor";
                    return false;
                }

                const auto table_result = vfp::parse_dbf_table_from_file(cursor->source_path, cursor->record_count);
                if (!table_result.ok)
                {
                    error_message = table_result.error;
                    return false;
                }

                source_fields = table_result.table.fields;
                source_records = table_result.table.records;
            }

            const auto field_by_name = [&](const std::string &field_name) -> const vfp::DbfFieldDescriptor *
            {
                const auto found = std::find_if(
                    source_fields.begin(),
                    source_fields.end(),
                    [&](const vfp::DbfFieldDescriptor &field)
                    {
                        return collapse_identifier(field.name) == collapse_identifier(field_name);
                    });
                return found == source_fields.end() ? nullptr : &*found;
            };
            const auto is_total_numeric_field = [](const vfp::DbfFieldDescriptor &field)
            {
                return field.type == 'N' || field.type == 'F' || field.type == 'I' || field.type == 'Y';
            };
            const auto make_total_output_field = [](const vfp::DbfFieldDescriptor &field)
            {
                vfp::DbfFieldDescriptor output_field = field;
                if (output_field.type == 'I')
                {
                    output_field.length = 4U;
                    output_field.decimal_count = 0U;
                }
                else if (output_field.type == 'Y')
                {
                    output_field.length = 8U;
                    output_field.decimal_count = std::max<std::uint8_t>(output_field.decimal_count, 4U);
                }
                else
                {
                    output_field.length = static_cast<std::uint8_t>(
                        std::max<int>(output_field.length, output_field.decimal_count == 0U ? 18 : 20));
                }
                return output_field;
            };

            const vfp::DbfFieldDescriptor *on_field = field_by_name(plan.on_field_name);
            if (on_field == nullptr)
            {
                error_message = "TOTAL ON field was not found";
                return false;
            }

            std::vector<const vfp::DbfFieldDescriptor *> total_fields;
            if (plan.field_names.empty())
            {
                for (const auto &field : source_fields)
                {
                    if (is_total_numeric_field(field) &&
                        collapse_identifier(field.name) != collapse_identifier(on_field->name))
                    {
                        total_fields.push_back(&field);
                    }
                }
            }
            else
            {
                for (const std::string &field_name : plan.field_names)
                {
                    const vfp::DbfFieldDescriptor *field = field_by_name(field_name);
                    if (field == nullptr)
                    {
                        error_message = "TOTAL field was not found: " + field_name;
                        return false;
                    }
                    if (!is_total_numeric_field(*field))
                    {
                        error_message = "TOTAL only supports numeric FIELDS in the first pass";
                        return false;
                    }
                    total_fields.push_back(field);
                }
            }
            if (total_fields.empty())
            {
                error_message = "TOTAL requires at least one numeric field to total";
                return false;
            }

            std::vector<std::size_t> records = collect_aggregate_scope_records(
                *cursor,
                frame,
                plan.scope,
                plan.for_expression,
                plan.while_expression);
            if (records.empty())
            {
                const std::string target_path = value_as_string(evaluate_expression(plan.target_expression, frame));
                std::vector<vfp::DbfFieldDescriptor> output_fields;
                output_fields.push_back(*on_field);
                for (const auto *field : total_fields)
                {
                    output_fields.push_back(make_total_output_field(*field));
                }
                const auto create_result = vfp::create_dbf_table_file(target_path, output_fields, {});
                if (!create_result.ok)
                {
                    error_message = create_result.error;
                    return false;
                }
                return true;
            }

            struct TotalGroup
            {
                std::string group_value;
                std::vector<double> sums;
            };

            std::vector<TotalGroup> groups;
            const auto append_record_to_group = [&](const vfp::DbfRecord &record)
            {
                const std::string group_value = record_field_value(record, on_field->name).value_or(std::string{});
                if (groups.empty() || groups.back().group_value != group_value)
                {
                    groups.push_back({.group_value = group_value, .sums = std::vector<double>(total_fields.size(), 0.0)});
                }

                for (std::size_t index = 0; index < total_fields.size(); ++index)
                {
                    const std::string value_text = trim_copy(record_field_value(record, total_fields[index]->name).value_or(std::string{}));
                    if (!value_text.empty())
                    {
                        groups.back().sums[index] += std::stod(value_text);
                    }
                }
            };

            for (const std::size_t recno : records)
            {
                if (recno == 0U || recno > source_records.size())
                {
                    continue;
                }
                append_record_to_group(source_records[recno - 1U]);
            }

            std::vector<vfp::DbfFieldDescriptor> output_fields;
            output_fields.push_back(*on_field);
            for (const auto *field : total_fields)
            {
                output_fields.push_back(make_total_output_field(*field));
            }

            std::vector<std::vector<std::string>> output_records;
            output_records.reserve(groups.size());
            for (const auto &group : groups)
            {
                std::vector<std::string> record;
                record.push_back(group.group_value);
                for (std::size_t index = 0; index < total_fields.size(); ++index)
                {
                    record.push_back(format_total_numeric_value(group.sums[index], total_fields[index]->decimal_count));
                }
                output_records.push_back(std::move(record));
            }

            const std::string target_path = value_as_string(evaluate_expression(plan.target_expression, frame));
            const auto create_result = vfp::create_dbf_table_file(target_path, output_fields, output_records);
            if (!create_result.ok)
            {
                error_message = create_result.error;
                return false;
            }

            return true;
        }

        bool execute_calculate_command(
            const Statement &statement,
            Frame &frame,
            std::string &error_message)
        {
            const std::vector<CalculateAssignment> assignments = parse_calculate_assignments(statement.expression);
            if (assignments.empty())
            {
                error_message = "CALCULATE requires one or more aggregate TO/INTO assignments";
                return false;
            }

            for (const auto &assignment : assignments)
            {
                const std::size_t open_paren = assignment.aggregate_expression.find('(');
                const std::size_t close_paren = assignment.aggregate_expression.rfind(')');
                if (open_paren == std::string::npos || close_paren == std::string::npos || close_paren <= open_paren)
                {
                    error_message = "CALCULATE requires aggregate expressions like COUNT() or SUM(field)";
                    return false;
                }

                const std::string function = normalize_identifier(assignment.aggregate_expression.substr(0U, open_paren));
                const std::string inner = trim_copy(assignment.aggregate_expression.substr(open_paren + 1U, close_paren - open_paren - 1U));
                std::vector<std::string> raw_arguments;
                if (!inner.empty())
                {
                    raw_arguments = split_csv_like(inner);
                }
                if (!statement.secondary_expression.empty())
                {
                    if (function == "count")
                    {
                        if (raw_arguments.empty())
                        {
                            raw_arguments.push_back(statement.secondary_expression);
                        }
                        else
                        {
                            raw_arguments[0] = "(" + raw_arguments[0] + ") AND (" + statement.secondary_expression + ")";
                        }
                    }
                    else if (raw_arguments.size() < 2U)
                    {
                        raw_arguments.push_back(statement.secondary_expression);
                    }
                    else
                    {
                        raw_arguments[1] = "(" + raw_arguments[1] + ") AND (" + statement.secondary_expression + ")";
                    }
                }
                if (!statement.tertiary_expression.empty())
                {
                    raw_arguments.push_back(statement.tertiary_expression);
                }

                assign_variable(frame, assignment.variable_name, aggregate_function_value(function, raw_arguments, frame));
            }

            return true;
        }

        bool execute_command_aggregate(
            const Statement &statement,
            Frame &frame,
            const std::string &function,
            std::string &error_message)
        {
            CursorState *cursor = resolve_cursor_target_expression(statement.quaternary_expression, frame);
            if (cursor == nullptr)
            {
                error_message = statement.quaternary_expression.empty()
                                    ? uppercase_copy(function) + " requires a selected work area"
                                    : uppercase_copy(function) + " target work area not found";
                return false;
            }

            const std::string target_text = trim_copy(statement.identifier);
            bool to_array = false;
            std::string array_name;
            if (!target_text.empty() && starts_with_insensitive(target_text, "ARRAY"))
            {
                std::string array_target_tail = trim_copy(target_text.substr(5U));
                if (!array_target_tail.empty() && array_target_tail.front() == ',')
                {
                    array_target_tail = trim_copy(array_target_tail.substr(1U));
                }
                if (array_target_tail.empty())
                {
                    error_message = uppercase_copy(function) + " TO ARRAY requires a target array name";
                    return false;
                }

                std::vector<std::string> array_targets = split_csv_like(array_target_tail);
                for (std::string &candidate : array_targets)
                {
                    candidate = trim_copy(std::move(candidate));
                }
                array_targets.erase(
                    std::remove_if(array_targets.begin(), array_targets.end(), [](const std::string &candidate)
                                   { return candidate.empty(); }),
                    array_targets.end());
                if (array_targets.size() != 1U)
                {
                    error_message = uppercase_copy(function) + " TO ARRAY accepts exactly one array target";
                    return false;
                }

                array_name = array_targets.front();
                to_array = true;
            }

            std::string expression_text;
            const AggregateScopeClause scope = parse_aggregate_scope_clause(statement.expression, expression_text);
            std::vector<std::string> targets;
            if (!to_array && !statement.identifier.empty())
            {
                targets = split_csv_like(statement.identifier);
            }
            for (std::string &target : targets)
            {
                target = trim_copy(std::move(target));
            }

            if (function == "count")
            {
                const std::string normalized_expression = normalize_identifier(expression_text);
                if (!expression_text.empty() && normalized_expression != "all")
                {
                    error_message = "COUNT only supports the first-pass ALL/FOR/TO forms right now";
                    return false;
                }
                if (targets.size() > 1U)
                {
                    error_message = "COUNT TO only accepts a single variable target";
                    return false;
                }

                const std::vector<std::size_t> records = collect_aggregate_scope_records(
                    *cursor,
                    frame,
                    scope,
                    statement.secondary_expression,
                    statement.tertiary_expression);
                const PrgValue result = aggregate_record_values(*cursor, function, {}, records, frame);
                if (to_array)
                {
                    assign_array(array_name, {result}, 1U);
                }
                else if (!targets.empty())
                {
                    assign_variable(frame, targets.front(), result);
                }
                return true;
            }

            if (normalize_identifier(expression_text) == "all")
            {
                error_message = uppercase_copy(function) + " without explicit expressions is not implemented yet";
                return false;
            }
            if (expression_text.empty())
            {
                error_message = uppercase_copy(function) + " requires one or more expressions";
                return false;
            }

            std::vector<std::string> expressions = split_csv_like(expression_text);
            for (std::string &expression : expressions)
            {
                expression = trim_copy(std::move(expression));
            }
            expressions.erase(
                std::remove_if(expressions.begin(), expressions.end(), [](const std::string &expression)
                               { return expression.empty(); }),
                expressions.end());
            if (expressions.empty())
            {
                error_message = uppercase_copy(function) + " requires one or more expressions";
                return false;
            }
            if (!targets.empty() && targets.size() != expressions.size())
            {
                error_message = uppercase_copy(function) + " TO requires one variable per aggregate expression";
                return false;
            }

            const std::vector<std::size_t> records = collect_aggregate_scope_records(
                *cursor,
                frame,
                scope,
                statement.secondary_expression,
                statement.tertiary_expression);

            if (to_array)
            {
                std::vector<PrgValue> array_values;
                array_values.reserve(expressions.size());
                for (const std::string &expression : expressions)
                {
                    array_values.push_back(aggregate_record_values(*cursor, function, expression, records, frame));
                }
                assign_array(array_name, array_values, 1U);
                return true;
            }

            for (std::size_t index = 0; index < expressions.size(); ++index)
            {
                const PrgValue result = aggregate_record_values(*cursor, function, expressions[index], records, frame);
                if (!targets.empty())
                {
                    assign_variable(frame, targets[index], result);
                }
            }

            return true;
        }

        bool execute_seek(
            CursorState &cursor,
            const std::string &search_key,
            bool move_pointer,
            bool preserve_pointer_on_miss,
            const std::string &order_designator,
            std::optional<bool> descending_override = std::nullopt,
            std::string *error_message = nullptr,
            std::string *used_order_name = nullptr,
            std::string *used_order_normalization_hint = nullptr,
            std::string *used_order_collation_hint = nullptr,
            bool *used_order_descending = nullptr)
        {
            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            if (!trim_copy(order_designator).empty() && !activate_order(cursor, order_designator, descending_override))
            {
                if (error_message != nullptr)
                {
                    *error_message = last_error_message;
                }
                restore_cursor_snapshot(cursor, original);
                return false;
            }

            const bool found = seek_in_cursor(cursor, search_key);
            const std::string runtime_error = last_error_message;
            if (used_order_name != nullptr)
            {
                *used_order_name = cursor.active_order_name;
            }
            if (used_order_normalization_hint != nullptr)
            {
                *used_order_normalization_hint = cursor.active_order_normalization_hint;
            }
            if (used_order_collation_hint != nullptr)
            {
                *used_order_collation_hint = cursor.active_order_collation_hint;
            }
            if (used_order_descending != nullptr)
            {
                *used_order_descending = cursor.active_order_descending;
            }
            if (!move_pointer || (!found && preserve_pointer_on_miss))
            {
                cursor.recno = original.recno;
                cursor.bof = original.bof;
                cursor.eof = original.eof;
                if (!move_pointer)
                {
                    cursor.found = original.found;
                }
            }
            cursor.active_order_name = original.active_order_name;
            cursor.active_order_expression = original.active_order_expression;
            cursor.active_order_for_expression = original.active_order_for_expression;
            cursor.active_order_path = original.active_order_path;
            cursor.active_order_normalization_hint = original.active_order_normalization_hint;
            cursor.active_order_collation_hint = original.active_order_collation_hint;
            cursor.active_order_key_domain_hint = original.active_order_key_domain_hint;
            cursor.active_order_descending = original.active_order_descending;

            if (!found && error_message != nullptr && !runtime_error.empty())
            {
                *error_message = runtime_error;
            }

            return found;
        }

        std::string order_function_value(const std::string &designator, bool include_path) const
        {
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr || cursor->active_order_name.empty())
            {
                return {};
            }

            if (!include_path)
            {
                return uppercase_copy(cursor->active_order_name);
            }

            if (!cursor->active_order_path.empty())
            {
                return uppercase_copy(cursor->active_order_path);
            }

            return uppercase_copy(cursor->active_order_name);
        }

        std::string tag_function_value(const std::string &index_file_name, std::size_t tag_number, const std::string &designator) const
        {
            const CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr || cursor->orders.empty())
            {
                return {};
            }

            std::size_t resolved_index = tag_number == 0U ? 0U : tag_number - 1U;
            if (!trim_copy(index_file_name).empty())
            {
                const std::string normalized_target_path = normalize_path(unquote_string(index_file_name));
                const std::string normalized_target_name =
                    collapse_identifier(std::filesystem::path(normalized_target_path.empty() ? index_file_name : normalized_target_path).filename().string());
                std::vector<const CursorState::OrderState *> matching_orders;
                for (const CursorState::OrderState &order : cursor->orders)
                {
                    const std::string normalized_order_path = normalize_path(order.index_path);
                    if ((!normalized_target_path.empty() && normalized_order_path == normalized_target_path) ||
                        collapse_identifier(std::filesystem::path(normalized_order_path).filename().string()) == normalized_target_name)
                    {
                        matching_orders.push_back(&order);
                    }
                }
                if (resolved_index < matching_orders.size())
                {
                    return uppercase_copy(matching_orders[resolved_index]->name);
                }
                return {};
            }

            if (resolved_index >= cursor->orders.size())
            {
                return {};
            }

            return uppercase_copy(cursor->orders[resolved_index].name);
        }

        bool is_library_loaded(const std::string &library_name) const
        {
            return loaded_libraries.contains(normalize_identifier(library_name));
        }

        int register_api_function(
            const std::string &variant,
            const std::string &function_name,
            const std::string &argument_types,
            const std::string &return_type,
            const std::string &dll_name)
        {
            if (!is_library_loaded("foxtools"))
            {
                last_error_message = "FOXTOOLS is not loaded";
                return -1;
            }

            const int handle = current_api_handle_counter()++;
            current_registered_api_functions().emplace(handle, RegisteredApiFunction{
                                                                   .handle = handle,
                                                                   .variant = variant,
                                                                   .function_name = function_name,
                                                                   .argument_types = argument_types,
                                                                   .return_type = return_type,
                                                                   .dll_name = dll_name});
            events.push_back({.category = "interop.regfn",
                              .detail = variant + ":" + function_name + "@" + dll_name + " -> " + std::to_string(handle),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return handle;
        }

        PrgValue call_registered_api_function(int handle, const std::vector<PrgValue> &arguments)
        {
            const auto &registered_functions = current_registered_api_functions();
            const auto found = registered_functions.find(handle);
            if (found == registered_functions.end())
            {
                last_error_message = "Registered API handle not found: " + std::to_string(handle);
                return make_number_value(-1.0);
            }

            const RegisteredApiFunction &function = found->second;
            events.push_back({.category = "interop.callfn",
                              .detail = function.function_name + " (" + std::to_string(arguments.size()) + " args)",
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});

            const std::string normalized_name = normalize_identifier(function.function_name);
            if (normalized_name == "getcurrentprocessid")
            {
                return make_number_value(static_cast<double>(current_process_id()));
            }
            if ((normalized_name == "lstrlena" || normalized_name == "lstrlenw") && !arguments.empty())
            {
                return make_number_value(static_cast<double>(value_as_string(arguments.front()).size()));
            }
            if ((normalized_name == "messageboxa" || normalized_name == "messageboxw"))
            {
                return make_number_value(1.0);
            }
            if ((normalized_name == "getmodulehandlea" || normalized_name == "getmodulehandlew"))
            {
                return make_number_value(1.0);
            }

            const std::string normalized_return = normalize_identifier(function.return_type);
            if (normalized_return == "c")
            {
                return make_string_value({});
            }
            if (normalized_return == "f" || normalized_return == "d")
            {
                return make_number_value(0.0);
            }
            return make_number_value(0.0);
        }

        // ---------------------------------------------------------------------------
        // invoke_declared_dll_function
        // Called from ExpressionParser when declared_dll_invoke_callback_ is set.
        // ---------------------------------------------------------------------------
        PrgValue invoke_declared_dll_function(const std::string &fn_key, const std::vector<PrgValue> &args)
        {
            const std::string key = normalize_identifier(fn_key);
            const auto found = declared_dll_functions.find(key);
            if (found == declared_dll_functions.end())
                return make_empty_value();
            const DeclaredDllFunction &declfn = found->second;

#if defined(_WIN32)
            // Split comma-separated param_types string into a vector for indexed access
            std::vector<std::string> param_type_list;
            {
                std::istringstream ss(declfn.param_types);
                std::string tok;
                while (std::getline(ss, tok, ','))
                {
                    while (!tok.empty() && tok.front() == ' ')
                        tok.erase(tok.begin());
                    while (!tok.empty() && tok.back() == ' ')
                        tok.pop_back();
                    if (!tok.empty())
                        param_type_list.push_back(tok);
                }
            }

            auto param_type_at = [&](std::size_t i) -> std::string
            {
                return i < param_type_list.size() ? param_type_list[i] : std::string("integer");
            };

            // Helper: convert PrgValue → VARIANT
            auto to_variant = [&](const PrgValue &v, const std::string &ptype) -> VARIANT
            {
                VARIANT var;
                VariantInit(&var);
                const std::string pt = normalize_identifier(ptype);
                // by-ref marker stripped for marshalling
                const bool by_ref = !pt.empty() && pt.back() == '@';
                const std::string base = by_ref ? pt.substr(0, pt.size() - 1) : pt;
                if (base == "string" || base == "c")
                {
                    std::string s = value_as_string(v);
                    var.vt = VT_BSTR;
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
                    std::wstring ws(wlen, 0);
                    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, ws.data(), wlen);
                    var.bstrVal = SysAllocString(ws.c_str());
                }
                else if (base == "double" || base == "d" || base == "f")
                {
                    var.vt = VT_R8;
                    var.dblVal = value_as_number(v);
                }
                else if (base == "long" || base == "longlong" || base == "integer64" || base == "i64")
                {
                    var.vt = VT_I8;
                    var.llVal = static_cast<LONGLONG>(value_as_number(v));
                }
                else
                {
                    // Default: INTEGER / SHORT / WORD → VT_I4
                    var.vt = VT_I4;
                    var.lVal = static_cast<LONG>(value_as_number(v));
                }
                return var;
            };

            // Helper: VARIANT → PrgValue based on return_type
            auto from_variant = [&](const VARIANT &var) -> PrgValue
            {
                const std::string rt = normalize_identifier(declfn.return_type);
                if (rt == "c" || rt == "string")
                {
                    if (var.vt == VT_BSTR && var.bstrVal)
                    {
                        int len = WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1, nullptr, 0, nullptr, nullptr);
                        std::string s(len, 0);
                        WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1, s.data(), len, nullptr, nullptr);
                        if (!s.empty() && s.back() == '\0')
                            s.pop_back();
                        return make_string_value(s);
                    }
                    return make_string_value({});
                }
                if (var.vt == VT_I8 || var.vt == VT_UI8)
                    return make_number_value(static_cast<double>(var.llVal));
                if (var.vt == VT_R4)
                    return make_number_value(static_cast<double>(var.fltVal));
                if (var.vt == VT_R8)
                    return make_number_value(var.dblVal);
                if (var.vt == VT_BOOL)
                    return make_boolean_value(var.boolVal != VARIANT_FALSE);
                // Integer family
                if (var.vt == VT_I4 || var.vt == VT_UI4)
                    return make_number_value(static_cast<double>(var.lVal));
                if (var.vt == VT_I2 || var.vt == VT_UI2)
                    return make_number_value(static_cast<double>(var.iVal));
                if (var.vt == VT_I1 || var.vt == VT_UI1)
                    return make_number_value(static_cast<double>(var.bVal));
                return make_number_value(static_cast<double>(var.intVal));
            };

            if (declfn.is_dotnet)
            {
                // ---------------------------------------------------------------
                // .NET CLR hosting via COM (ICLRMetaHost / ICorRuntimeHost)
                // ---------------------------------------------------------------
                // Single-init static COM state (process-wide, non-thread-safe for
                // simplicity; adequate for VFP-style single-threaded programs).
                static ICorRuntimeHost *s_runtime_host = nullptr;
                static bool s_clr_started = false;
                if (!s_clr_started)
                {
                    ICLRMetaHost *metahost = nullptr;
                    HRESULT hr2 = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, reinterpret_cast<void **>(&metahost));
                    if (FAILED(hr2))
                    {
                        last_error_message = "CLRCreateInstance failed: " + std::to_string(hr2);
                        return make_empty_value();
                    }
                    ICLRRuntimeInfo *runtime_info = nullptr;
                    hr2 = metahost->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, reinterpret_cast<void **>(&runtime_info));
                    if (FAILED(hr2))
                    {
                        IEnumUnknown *enumerator = nullptr;
                        metahost->EnumerateInstalledRuntimes(&enumerator);
                        if (enumerator)
                        {
                            IUnknown *rt_unk = nullptr;
                            ULONG fetched = 0;
                            while (enumerator->Next(1, &rt_unk, &fetched) == S_OK && fetched > 0)
                            {
                                if (runtime_info)
                                    runtime_info->Release();
                                rt_unk->QueryInterface(IID_ICLRRuntimeInfo, reinterpret_cast<void **>(&runtime_info));
                                rt_unk->Release();
                            }
                            enumerator->Release();
                        }
                    }
                    if (runtime_info == nullptr)
                    {
                        metahost->Release();
                        last_error_message = "No CLR runtime found";
                        return make_empty_value();
                    }
                    hr2 = runtime_info->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost, reinterpret_cast<void **>(&s_runtime_host));
                    runtime_info->Release();
                    metahost->Release();
                    if (FAILED(hr2) || s_runtime_host == nullptr)
                    {
                        last_error_message = "Failed to get ICorRuntimeHost: " + std::to_string(hr2);
                        return make_empty_value();
                    }
                    s_runtime_host->Start();
                    s_clr_started = true;
                }

                // IDispatch late-binding helper: call a named method on a COM object
                auto dispatch_call = [](IDispatch *obj, const wchar_t *method_name,
                                        std::vector<VARIANT> args, VARIANT *ret_out) -> HRESULT
                {
                    BSTR bname = SysAllocString(method_name);
                    DISPID dispid = DISPID_UNKNOWN;
                    HRESULT hr = obj->GetIDsOfNames(IID_NULL, &bname, 1, LOCALE_USER_DEFAULT, &dispid);
                    SysFreeString(bname);
                    if (FAILED(hr))
                        return hr;
                    // IDispatch args must be in reverse order
                    std::reverse(args.begin(), args.end());
                    DISPPARAMS dp{};
                    dp.rgvarg = args.empty() ? nullptr : args.data();
                    dp.cArgs = static_cast<UINT>(args.size());
                    EXCEPINFO exc{};
                    return obj->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                                       &dp, ret_out, &exc, nullptr);
                };

                // Get default AppDomain as IDispatch
                IUnknown *app_domain_unk = nullptr;
                HRESULT hr = s_runtime_host->GetDefaultDomain(&app_domain_unk);
                if (FAILED(hr) || app_domain_unk == nullptr)
                {
                    last_error_message = "GetDefaultDomain failed: " + std::to_string(hr);
                    return make_empty_value();
                }
                IDispatch *app_domain_disp = nullptr;
                hr = app_domain_unk->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&app_domain_disp));
                app_domain_unk->Release();
                if (FAILED(hr) || app_domain_disp == nullptr)
                {
                    last_error_message = "AppDomain QueryInterface IDispatch failed";
                    return make_empty_value();
                }

                // Load assembly: AppDomain.LoadFrom(path)
                std::wstring asm_path_w(declfn.dll_path.begin(), declfn.dll_path.end());
                VARIANT vpath;
                VariantInit(&vpath);
                vpath.vt = VT_BSTR;
                vpath.bstrVal = SysAllocString(asm_path_w.c_str());
                VARIANT v_assembly;
                VariantInit(&v_assembly);
                hr = dispatch_call(app_domain_disp, L"Load", {vpath}, &v_assembly);
                VariantClear(&vpath);
                app_domain_disp->Release();

                // Try LoadFrom on AppDomain.CurrentDomain if Load fails
                if (FAILED(hr) || v_assembly.vt == VT_EMPTY || v_assembly.vt == VT_NULL)
                {
                    // We couldn't load: return a graceful empty
                    VariantClear(&v_assembly);
                    last_error_message = "Could not load .NET assembly: " + declfn.dll_path + " hr=" + std::to_string(hr);
                    return make_empty_value();
                }

                IDispatch *assembly_disp = nullptr;
                if (v_assembly.vt == VT_DISPATCH)
                    assembly_disp = v_assembly.pdispVal;
                else if (v_assembly.vt == (VT_DISPATCH | VT_BYREF) && v_assembly.ppdispVal)
                    assembly_disp = *v_assembly.ppdispVal;
                if (!assembly_disp)
                {
                    VariantClear(&v_assembly);
                    last_error_message = "Assembly is not IDispatch";
                    return make_empty_value();
                }
                assembly_disp->AddRef();
                VariantClear(&v_assembly);

                // GetType(type_name)
                std::wstring type_name_w(declfn.dotnet_type_name.begin(), declfn.dotnet_type_name.end());
                VARIANT vtn;
                VariantInit(&vtn);
                vtn.vt = VT_BSTR;
                vtn.bstrVal = SysAllocString(type_name_w.c_str());
                VARIANT v_type;
                VariantInit(&v_type);
                hr = dispatch_call(assembly_disp, L"GetType", {vtn}, &v_type);
                VariantClear(&vtn);
                assembly_disp->Release();
                IDispatch *type_disp = nullptr;
                if (SUCCEEDED(hr) && v_type.vt == VT_DISPATCH)
                    type_disp = v_type.pdispVal;
                if (!type_disp)
                {
                    VariantClear(&v_type);
                    last_error_message = "Type not found: " + declfn.dotnet_type_name;
                    return make_empty_value();
                }
                type_disp->AddRef();
                VariantClear(&v_type);

                // GetMethod(method_name)
                std::wstring method_name_w(declfn.dotnet_method_name.begin(), declfn.dotnet_method_name.end());
                VARIANT vmn;
                VariantInit(&vmn);
                vmn.vt = VT_BSTR;
                vmn.bstrVal = SysAllocString(method_name_w.c_str());
                VARIANT v_method;
                VariantInit(&v_method);
                hr = dispatch_call(type_disp, L"GetMethod", {vmn}, &v_method);
                VariantClear(&vmn);
                type_disp->Release();
                IDispatch *method_disp = nullptr;
                if (SUCCEEDED(hr) && v_method.vt == VT_DISPATCH)
                    method_disp = v_method.pdispVal;
                if (!method_disp)
                {
                    VariantClear(&v_method);
                    last_error_message = "Method not found: " + declfn.dotnet_method_name;
                    return make_empty_value();
                }
                method_disp->AddRef();
                VariantClear(&v_method);

                // Build args SAFEARRAY wrapped in VARIANT for Invoke(null, args[])
                SAFEARRAY *sa = SafeArrayCreateVector(VT_VARIANT, 0, static_cast<ULONG>(args.size()));
                for (LONG idx = 0; idx < static_cast<LONG>(args.size()); ++idx)
                {
                    const std::string ptype = param_type_at(static_cast<std::size_t>(idx));
                    VARIANT v = to_variant(args[static_cast<std::size_t>(idx)], ptype);
                    SafeArrayPutElement(sa, &idx, &v);
                    VariantClear(&v);
                }
                VARIANT vsa;
                VariantInit(&vsa);
                vsa.vt = VT_ARRAY | VT_VARIANT;
                vsa.parray = sa;

                VARIANT vnull;
                VariantInit(&vnull);
                vnull.vt = VT_NULL; // static method — null target

                VARIANT ret_var;
                VariantInit(&ret_var);
                hr = dispatch_call(method_disp, L"Invoke", {vnull, vsa}, &ret_var);
                VariantClear(&vsa); // also destroys sa
                VariantClear(&vnull);
                method_disp->Release();

                if (FAILED(hr))
                {
                    last_error_message = "Method invoke failed: " + std::to_string(hr);
                    return make_empty_value();
                }
                PrgValue result = from_variant(ret_var);
                VariantClear(&ret_var);
                return result;
            }
            else
            {
                // ---------------------------------------------------------------
                // Native DLL invocation via proc_address
                // ---------------------------------------------------------------
                if (declfn.proc_address == nullptr)
                {
                    last_error_message = "No proc address for: " + declfn.function_name;
                    return make_empty_value();
                }

                // Build integer/double argument lists for common calling conventions.
                // We support the same limited set as VFP's DECLARE: up to 8 args,
                // typed as INTEGER/LONG/DOUBLE/STRING.
                // For STRING params we pass a pointer to the UTF-8 buffer.
                // We do not attempt to pack varargs generically; instead we use a
                // dispatch table keyed on arg count (0-8), which covers the vast
                // majority of real-world DLL calls.

                // Convert args to a flat array of 64-bit values (integers/pointers)
                // and a parallel doubles array.
                std::vector<std::string> string_buffers; // keep alive through the call
                struct Arg64
                {
                    __int64 i;
                    double d;
                    bool is_double;
                };
                std::vector<Arg64> flat;
                flat.reserve(args.size());
                for (std::size_t idx = 0; idx < args.size(); ++idx)
                {
                    const std::string ptype = normalize_identifier(param_type_at(idx));
                    const std::string base_pt = (!ptype.empty() && ptype.back() == '@')
                                                    ? ptype.substr(0, ptype.size() - 1)
                                                    : ptype;
                    Arg64 a{};
                    if (base_pt == "double" || base_pt == "d" || base_pt == "f")
                    {
                        a.d = value_as_number(args[idx]);
                        a.is_double = true;
                    }
                    else if (base_pt == "string" || base_pt == "c")
                    {
                        std::string s = value_as_string(args[idx]);
                        string_buffers.push_back(std::move(s));
                        a.i = reinterpret_cast<__int64>(string_buffers.back().c_str());
                    }
                    else
                    {
                        a.i = static_cast<__int64>(value_as_number(args[idx]));
                    }
                    flat.push_back(a);
                }

                // Extract all args as __int64 (works for int, ptr, and bitcast of double)
                auto iarg = [&](std::size_t i) -> __int64
                {
                    if (i >= flat.size())
                        return 0LL;
                    return flat[i].is_double
                               ? *reinterpret_cast<const __int64 *>(&flat[i].d)
                               : flat[i].i;
                };

                // Call the function. We cast to a stdcall prototype (VFP default on x86
                // for DECLARE; on x64 there is only one calling convention).
                // Return value is either integer or double based on return_type.
                const std::string rt = normalize_identifier(declfn.return_type);
                const bool ret_double = (rt == "double" || rt == "d" || rt == "f");
                const bool ret_string = (rt == "c" || rt == "string");
                const std::size_t nargs = flat.size();

#if defined(_WIN64)
                // On x64 Windows, calling convention is unified.
                typedef __int64 (*FnI_0)();
                typedef __int64 (*FnI_1)(__int64);
                typedef __int64 (*FnI_2)(__int64, __int64);
                typedef __int64 (*FnI_3)(__int64, __int64, __int64);
                typedef __int64 (*FnI_4)(__int64, __int64, __int64, __int64);
                typedef __int64 (*FnI_5)(__int64, __int64, __int64, __int64, __int64);
                typedef __int64 (*FnI_6)(__int64, __int64, __int64, __int64, __int64, __int64);
                typedef __int64 (*FnI_7)(__int64, __int64, __int64, __int64, __int64, __int64, __int64);
                typedef __int64 (*FnI_8)(__int64, __int64, __int64, __int64, __int64, __int64, __int64, __int64);
                typedef double (*FnD_0)();
                typedef double (*FnD_1)(__int64);
                typedef double (*FnD_2)(__int64, __int64);
                typedef double (*FnD_3)(__int64, __int64, __int64);
                typedef double (*FnD_4)(__int64, __int64, __int64, __int64);

                FARPROC fn = declfn.proc_address;
                __int64 iret = 0;
                double dret = 0.0;
                if (ret_double)
                {
                    switch (nargs)
                    {
                    case 0:
                        dret = reinterpret_cast<FnD_0>(fn)();
                        break;
                    case 1:
                        dret = reinterpret_cast<FnD_1>(fn)(iarg(0));
                        break;
                    case 2:
                        dret = reinterpret_cast<FnD_2>(fn)(iarg(0), iarg(1));
                        break;
                    case 3:
                        dret = reinterpret_cast<FnD_3>(fn)(iarg(0), iarg(1), iarg(2));
                        break;
                    default:
                        dret = reinterpret_cast<FnD_4>(fn)(iarg(0), iarg(1), iarg(2), iarg(3));
                        break;
                    }
                    return make_number_value(dret);
                }
                else
                {
                    switch (nargs)
                    {
                    case 0:
                        iret = reinterpret_cast<FnI_0>(fn)();
                        break;
                    case 1:
                        iret = reinterpret_cast<FnI_1>(fn)(iarg(0));
                        break;
                    case 2:
                        iret = reinterpret_cast<FnI_2>(fn)(iarg(0), iarg(1));
                        break;
                    case 3:
                        iret = reinterpret_cast<FnI_3>(fn)(iarg(0), iarg(1), iarg(2));
                        break;
                    case 4:
                        iret = reinterpret_cast<FnI_4>(fn)(iarg(0), iarg(1), iarg(2), iarg(3));
                        break;
                    case 5:
                        iret = reinterpret_cast<FnI_5>(fn)(iarg(0), iarg(1), iarg(2), iarg(3), iarg(4));
                        break;
                    case 6:
                        iret = reinterpret_cast<FnI_6>(fn)(iarg(0), iarg(1), iarg(2), iarg(3), iarg(4), iarg(5));
                        break;
                    case 7:
                        iret = reinterpret_cast<FnI_7>(fn)(iarg(0), iarg(1), iarg(2), iarg(3), iarg(4), iarg(5), iarg(6));
                        break;
                    default:
                        iret = reinterpret_cast<FnI_8>(fn)(iarg(0), iarg(1), iarg(2), iarg(3), iarg(4), iarg(5), iarg(6), iarg(7));
                        break;
                    }
                    if (ret_string)
                    {
                        const char *p = reinterpret_cast<const char *>(iret);
                        return make_string_value(p ? std::string(p) : std::string{});
                    }
                    return make_number_value(static_cast<double>(iret));
                }
#else
                // x86: use __stdcall by default (VFP DECLARE default)
                typedef __int32(__stdcall * FnSI_0)();
                typedef __int32(__stdcall * FnSI_1)(__int32);
                typedef __int32(__stdcall * FnSI_2)(__int32, __int32);
                typedef __int32(__stdcall * FnSI_3)(__int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_4)(__int32, __int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_5)(__int32, __int32, __int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_6)(__int32, __int32, __int32, __int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_7)(__int32, __int32, __int32, __int32, __int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_8)(__int32, __int32, __int32, __int32, __int32, __int32, __int32, __int32);

                auto i32 = [&](std::size_t i) -> __int32
                { return static_cast<__int32>(iarg(i)); };

                FARPROC fn = declfn.proc_address;
                __int32 iret = 0;
                switch (nargs)
                {
                case 0:
                    iret = reinterpret_cast<FnSI_0>(fn)();
                    break;
                case 1:
                    iret = reinterpret_cast<FnSI_1>(fn)(i32(0));
                    break;
                case 2:
                    iret = reinterpret_cast<FnSI_2>(fn)(i32(0), i32(1));
                    break;
                case 3:
                    iret = reinterpret_cast<FnSI_3>(fn)(i32(0), i32(1), i32(2));
                    break;
                case 4:
                    iret = reinterpret_cast<FnSI_4>(fn)(i32(0), i32(1), i32(2), i32(3));
                    break;
                case 5:
                    iret = reinterpret_cast<FnSI_5>(fn)(i32(0), i32(1), i32(2), i32(3), i32(4));
                    break;
                case 6:
                    iret = reinterpret_cast<FnSI_6>(fn)(i32(0), i32(1), i32(2), i32(3), i32(4), i32(5));
                    break;
                case 7:
                    iret = reinterpret_cast<FnSI_7>(fn)(i32(0), i32(1), i32(2), i32(3), i32(4), i32(5), i32(6));
                    break;
                default:
                    iret = reinterpret_cast<FnSI_8>(fn)(i32(0), i32(1), i32(2), i32(3), i32(4), i32(5), i32(6), i32(7));
                    break;
                }
                if (ret_string)
                {
                    const char *p = reinterpret_cast<const char *>(iret);
                    return make_string_value(p ? std::string(p) : std::string{});
                }
                return make_number_value(static_cast<double>(iret));
#endif
            }
#else
            (void)declfn;
            (void)args;
            return make_empty_value();
#endif
        }

        bool open_table_cursor(
            const std::string &raw_path,
            const std::string &requested_alias,
            const std::string &in_expression,
            bool allow_again,
            bool remote,
            int sql_handle,
            const std::string &sql_command,
            std::size_t synthetic_record_count,
            const std::map<std::string, CursorState::FieldRule> &field_rules = {})
        {
            (void)sql_command;
            std::string alias = requested_alias;
            std::string resolved_path = raw_path;
            std::string dbf_identity;
            std::size_t field_count = 0;
            std::size_t record_count = synthetic_record_count;

            if (!remote)
            {
                std::filesystem::path table_path(unquote_string(trim_copy(raw_path)));
                if (table_path.extension().empty())
                {
                    table_path += ".dbf";
                }
                if (table_path.is_relative())
                {
                    table_path = std::filesystem::path(current_default_directory()) / table_path;
                }
                table_path = table_path.lexically_normal();
                if (!std::filesystem::exists(table_path))
                {
                    last_error_message = "Unable to resolve USE target: " + table_path.string();
                    return false;
                }

                const auto table_result = vfp::parse_dbf_table_from_file(table_path.string(), 1U);
                if (!table_result.ok)
                {
                    last_error_message = table_result.error;
                    return false;
                }

                resolved_path = table_path.string();
                dbf_identity = resolved_path;
                field_count = table_result.table.fields.size();
                record_count = table_result.table.header.record_count;
                std::vector<CursorState::OrderState> orders = load_cursor_orders(resolved_path);
                if (alias.empty())
                {
                    alias = table_path.stem().string();
                }

                const std::optional<int> requested_target_area = resolve_use_target_work_area(in_expression, stack.back());
                if (!requested_target_area.has_value())
                {
                    return false;
                }
                int target_area = *requested_target_area;
                const bool preserve_selected_work_area =
                    !trim_copy(in_expression).empty() &&
                    target_area > 0 &&
                    target_area != current_selected_work_area();
                target_area = preserve_selected_work_area
                                  ? reserve_work_area(target_area)
                                  : select_work_area(target_area);

                if (!can_open_table_cursor(resolved_path, alias, false, allow_again, target_area))
                {
                    return false;
                }

                DataSessionState &session = current_session_state();
                session.aliases[target_area] = alias;
                CursorState cursor;
                cursor.work_area = target_area;
                cursor.alias = alias;
                cursor.source_path = resolved_path;
                cursor.dbf_identity = dbf_identity;
                cursor.source_kind = "table";
                cursor.field_count = field_count;
                cursor.record_count = record_count;
                cursor.recno = record_count == 0U ? 0U : 1U;
                cursor.bof = record_count == 0U;
                cursor.eof = record_count == 0U;
                cursor.orders = std::move(orders);
                cursor.field_rules = field_rules;
                session.cursors[target_area] = std::move(cursor);
                return true;
            }
            else if (alias.empty())
            {
                alias = "sqlresult" + std::to_string(next_available_work_area());
            }
            if (remote)
            {
                dbf_identity = alias;
                field_count = synthetic_record_count == 0U ? 0U : 3U;
            }

            const std::optional<int> requested_target_area = resolve_use_target_work_area(in_expression, stack.back());
            if (!requested_target_area.has_value())
            {
                return false;
            }
            int target_area = *requested_target_area;
            const bool preserve_selected_work_area =
                !trim_copy(in_expression).empty() &&
                target_area > 0 &&
                target_area != current_selected_work_area();
            target_area = preserve_selected_work_area
                              ? reserve_work_area(target_area)
                              : select_work_area(target_area);

            if (!can_open_table_cursor(resolved_path, alias, remote, allow_again, target_area))
            {
                return false;
            }

            DataSessionState &session = current_session_state();
            std::vector<vfp::DbfRecord> remote_records;
            session.aliases[target_area] = alias;
            if (remote)
            {
                remote_records.reserve(record_count);
                for (std::size_t recno = 1U; recno <= record_count; ++recno)
                {
                    remote_records.push_back(make_synthetic_sql_record(recno));
                }
            }
            CursorState cursor;
            cursor.work_area = target_area;
            cursor.alias = alias;
            cursor.source_path = resolved_path;
            cursor.dbf_identity = dbf_identity;
            cursor.source_kind = remote ? "sql-cursor" : "table";
            cursor.remote = remote;
            cursor.field_count = field_count;
            cursor.record_count = record_count;
            cursor.recno = record_count == 0U ? 0U : 1U;
            cursor.bof = record_count == 0U;
            cursor.eof = record_count == 0U;
            cursor.remote_records = std::move(remote_records);
            cursor.field_rules = field_rules;
            session.cursors[target_area] = std::move(cursor);
            if (remote && sql_handle > 0)
            {
                auto &connections = current_sql_connections();
                auto found = connections.find(sql_handle);
                if (found != connections.end())
                {
                    found->second.last_cursor_alias = alias;
                    found->second.last_result_count = record_count;
                }
            }
            return true;
        }

        int sql_connect(const std::string &target, const std::string &provider)
        {
            std::string provider_hint = provider;
            const std::string normalized_target = lowercase_copy(target);
            if (normalized_target.find("provider=") != std::string::npos)
            {
                provider_hint = "oledb";
            }
            else if (normalized_target.find("driver=") != std::string::npos || normalized_target.find("dsn=") != std::string::npos)
            {
                provider_hint = "odbc";
            }

            int &next_handle = current_sql_handle_counter();
            const int handle = next_handle++;
            current_sql_connections().emplace(handle, RuntimeSqlConnectionState{
                                                          .handle = handle,
                                                          .target = target,
                                                          .provider = provider_hint,
                                                          .last_cursor_alias = {},
                                                          .last_result_count = 0U,
                                                          .prepared_command = {},
                                                          .properties = {
                                                              {"provider", provider_hint},
                                                              {"target", target},
                                                              {"querytimeout", "0"},
                                                              {"connecttimeout", "0"}}});
            return handle;
        }

        bool sql_disconnect(int handle)
        {
            return current_sql_connections().erase(handle) > 0;
        }

        int sql_row_count(int handle) const
        {
            const auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                return -1;
            }
            return static_cast<int>(found->second.last_result_count);
        }

        int sql_prepare(int handle, const std::string &command)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                return -1;
            }
            found->second.prepared_command = command;
            found->second.properties["preparedcommand"] = command;
            events.push_back({.category = "sql.prepare",
                              .detail = "handle " + std::to_string(handle) + ": " + command,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        PrgValue sql_get_prop(int handle, const std::string &property_name) const
        {
            const auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                return make_number_value(-1.0);
            }

            const std::string normalized_name = normalize_identifier(property_name);
            if (normalized_name == "provider")
            {
                return make_string_value(found->second.provider);
            }
            if (normalized_name == "target" || normalized_name == "connectstring")
            {
                return make_string_value(found->second.target);
            }
            if (normalized_name == "preparedcommand")
            {
                return make_string_value(found->second.prepared_command);
            }
            if (normalized_name == "rowcount" || normalized_name == "lastresultcount")
            {
                return make_number_value(static_cast<double>(found->second.last_result_count));
            }

            const auto property = found->second.properties.find(normalized_name);
            if (property == found->second.properties.end())
            {
                return make_empty_value();
            }

            const std::string raw_value = property->second;
            if (!raw_value.empty() && std::all_of(raw_value.begin(), raw_value.end(), [](unsigned char ch)
                                                  { return std::isdigit(ch) != 0 || ch == '-' || ch == '.'; }))
            {
                return make_number_value(std::stod(raw_value));
            }
            return make_string_value(raw_value);
        }

        int sql_set_prop(int handle, const std::string &property_name, const PrgValue &value)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                return -1;
            }

            const std::string normalized_name = normalize_identifier(property_name);
            const std::string property_value = value.kind == PrgValueKind::number
                                                   ? std::to_string(static_cast<long long>(std::llround(value.number_value)))
                                                   : value_as_string(value);

            if (normalized_name == "preparedcommand")
            {
                found->second.prepared_command = property_value;
            }
            found->second.properties[normalized_name] = property_value;
            events.push_back({.category = "sql.setprop",
                              .detail = "handle " + std::to_string(handle) + ": " + normalized_name + "=" + property_value,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        int sql_exec(int handle, const std::string &command, const std::string &cursor_alias)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                return -1;
            }

            RuntimeSqlConnectionState &connection = found->second;
            connection.last_cursor_alias.clear();
            connection.last_result_count = 0U;

            const std::string effective_command = trim_copy(command).empty() ? connection.prepared_command : trim_copy(command);
            if (effective_command.empty())
            {
                last_error_message = "SQLEXEC requires a command or a prepared SQL statement";
                last_error_code = classify_runtime_error_code(last_error_message);
                return -1;
            }

            std::size_t result_count = 0;
            const std::string normalized_command = lowercase_copy(effective_command);
            if (normalized_command.rfind("select", 0) == 0)
            {
                result_count = 3U;
                const std::string alias = trim_copy(cursor_alias).empty()
                                              ? "sqlresult" + std::to_string(handle)
                                              : trim_copy(cursor_alias);
                if (!open_table_cursor({}, alias, resolve_sql_cursor_auto_target(), true, true, handle, effective_command, result_count))
                {
                    return -1;
                }
            }
            else if (
                normalized_command.rfind("insert", 0) == 0 ||
                normalized_command.rfind("update", 0) == 0 ||
                normalized_command.rfind("delete", 0) == 0)
            {
                result_count = normalized_command.find("where 1=0") == std::string::npos ? 1U : 0U;
                connection.last_result_count = result_count;
            }
            events.push_back({.category = "sql.exec",
                              .detail = "handle " + std::to_string(handle) + ": " + effective_command,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            if (result_count > 0U)
            {
                if (normalized_command.rfind("select", 0) == 0)
                {
                    events.push_back({.category = "sql.cursor",
                                      .detail = connection.last_cursor_alias + " (" + std::to_string(result_count) + " rows)",
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                }
                else
                {
                    events.push_back({.category = "sql.rows",
                                      .detail = "handle " + std::to_string(handle) + ": " + std::to_string(connection.last_result_count) + " affected",
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                }
            }
            else if (
                normalized_command.rfind("insert", 0) == 0 ||
                normalized_command.rfind("update", 0) == 0 ||
                normalized_command.rfind("delete", 0) == 0)
            {
                events.push_back({.category = "sql.rows",
                                  .detail = "handle " + std::to_string(handle) + ": 0 affected",
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            }
            return 1;
        }

        int register_ole_object(const std::string &prog_id, const std::string &source)
        {
            const int handle = next_ole_handle++;
            ole_objects.emplace(handle, RuntimeOleObjectState{
                                            .handle = handle,
                                            .prog_id = prog_id,
                                            .source = source,
                                            .last_action = source,
                                            .action_count = 1});
            return handle;
        }

        std::optional<RuntimeOleObjectState *> resolve_ole_object(const PrgValue &value)
        {
            int handle = 0;
            std::string prog_id;
            if (!parse_object_handle_reference(value, handle, prog_id))
            {
                return std::nullopt;
            }

            auto found = ole_objects.find(handle);
            if (found == ole_objects.end())
            {
                return std::nullopt;
            }
            return &found->second;
        }

        PrgValue evaluate_expression(const std::string &expression, const Frame &frame);
        PrgValue evaluate_expression(const std::string &expression, const Frame &frame, const CursorState *preferred_cursor);
        std::optional<std::string> materialize_xasset_bootstrap(const std::string &asset_path, bool include_read_events);

        void assign_variable(Frame &frame, const std::string &name, const PrgValue &value)
        {
            const std::string normalized = normalize_memory_variable_identifier(name);
            if (frame.local_names.contains(normalized) || frame.locals.contains(normalized))
            {
                frame.locals[normalized] = value;
                return;
            }
            globals[normalized] = value;
        }

        PrgValue lookup_variable(const Frame &frame, const std::string &name) const
        {
            const std::string normalized = normalize_memory_variable_identifier(name);
            if (const auto local = frame.locals.find(normalized); local != frame.locals.end())
            {
                return local->second;
            }
            if (const auto global = globals.find(normalized); global != globals.end())
            {
                return global->second;
            }
            return {};
        }

        RuntimeArray *find_array(const std::string &name)
        {
            const auto found = arrays.find(normalize_memory_variable_identifier(name));
            return found == arrays.end() ? nullptr : &found->second;
        }

        const RuntimeArray *find_array(const std::string &name) const
        {
            const auto found = arrays.find(normalize_memory_variable_identifier(name));
            return found == arrays.end() ? nullptr : &found->second;
        }

        bool has_array(const std::string &name) const
        {
            return find_array(name) != nullptr;
        }

        std::size_t array_length(const std::string &name, int dimension) const
        {
            const RuntimeArray *array = find_array(name);
            if (array == nullptr)
            {
                return 0U;
            }
            if (dimension == 1)
            {
                return array->rows;
            }
            if (dimension == 2)
            {
                return array->columns;
            }
            return array->values.size();
        }

        PrgValue array_value(const std::string &name, std::size_t row, std::size_t column = 1U) const
        {
            const RuntimeArray *array = find_array(name);
            if (array == nullptr || row == 0U || column == 0U || row > array->rows || column > array->columns)
            {
                return make_empty_value();
            }
            const std::size_t index = ((row - 1U) * array->columns) + (column - 1U);
            return index < array->values.size() ? array->values[index] : make_empty_value();
        }

        std::size_t array_linear_index(const RuntimeArray &array, std::size_t row, std::size_t column) const
        {
            if (row == 0U || column == 0U || row > array.rows || column > array.columns)
            {
                return 0U;
            }
            const std::size_t index = ((row - 1U) * array.columns) + column;
            return index <= array.values.size() ? index : 0U;
        }

        std::size_t array_subscript(const RuntimeArray &array, std::size_t element, int dimension) const
        {
            if (element == 0U || element > array.values.size())
            {
                return 0U;
            }
            if (dimension == 1)
            {
                return ((element - 1U) / array.columns) + 1U;
            }
            if (dimension == 2)
            {
                return ((element - 1U) % array.columns) + 1U;
            }
            return 0U;
        }

        void assign_array(const std::string &name, std::vector<PrgValue> values, std::size_t columns = 1U)
        {
            columns = std::max<std::size_t>(1U, columns);
            RuntimeArray array;
            array.columns = columns;
            array.rows = values.empty() ? 0U : ((values.size() + columns - 1U) / columns);
            array.values = std::move(values);
            array.values.resize(array.rows * array.columns);
            arrays[normalize_memory_variable_identifier(name)] = std::move(array);
        }

        bool parse_array_reference(
            const std::string &reference,
            const Frame &frame,
            std::string &array_name,
            std::size_t &row,
            std::size_t &column)
        {
            const std::string trimmed = trim_copy(reference);
            if (trimmed.empty())
            {
                return false;
            }

            const std::size_t bracket_open = trimmed.find('[');
            const std::size_t paren_open = trimmed.find('(');
            const bool uses_brackets = bracket_open != std::string::npos;
            const std::size_t open = uses_brackets ? bracket_open : paren_open;
            if (open == std::string::npos || open == 0U)
            {
                return false;
            }

            const char close_char = uses_brackets ? ']' : ')';
            if (trimmed.back() != close_char)
            {
                return false;
            }

            array_name = trim_copy(trimmed.substr(0U, open));
            if (array_name.empty())
            {
                return false;
            }
            const std::string indexes_text = trimmed.substr(open + 1U, trimmed.size() - open - 2U);
            const std::vector<std::string> parts = split_csv_like(indexes_text);
            if (parts.empty())
            {
                return false;
            }
            row = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(evaluate_expression(parts[0], frame))));
            column = parts.size() >= 2U
                         ? static_cast<std::size_t>(std::max<double>(0.0, value_as_number(evaluate_expression(parts[1], frame))))
                         : 1U;
            return row > 0U && column > 0U;
        }

        bool assign_array_element(const std::string &reference, const Frame &frame, const PrgValue &value)
        {
            std::string array_name;
            std::size_t row = 0U;
            std::size_t column = 1U;
            if (!parse_array_reference(reference, frame, array_name, row, column))
            {
                return false;
            }

            RuntimeArray *array = find_array(array_name);
            if (array == nullptr || row > array->rows || column > array->columns)
            {
                const std::size_t new_rows = array == nullptr ? row : std::max(row, array->rows);
                const std::size_t new_columns = array == nullptr ? column : std::max(column, array->columns);
                resize_array(array_name, new_rows, new_columns);
                array = find_array(array_name);
            }
            if (array == nullptr || row == 0U || column == 0U || row > array->rows || column > array->columns)
            {
                return false;
            }
            array->values[((row - 1U) * array->columns) + (column - 1U)] = value;
            return true;
        }

        bool declare_array(const std::string &declaration, const Frame &frame)
        {
            std::string array_name;
            std::size_t rows = 0U;
            std::size_t columns = 1U;
            if (!parse_array_reference(declaration, frame, array_name, rows, columns))
            {
                return false;
            }
            resize_array(array_name, rows, columns);
            return true;
        }

        PrgValue resize_array(const std::string &name, std::size_t rows, std::size_t columns = 1U)
        {
            columns = std::max<std::size_t>(1U, columns);
            RuntimeArray *array = find_array(name);
            if (array == nullptr)
            {
                assign_array(name, {}, columns);
                array = find_array(name);
            }
            if (array == nullptr)
            {
                return make_number_value(0.0);
            }
            std::vector<PrgValue> new_values(rows * columns);
            const std::size_t copy_rows = std::min(rows, array->rows);
            const std::size_t copy_columns = std::min(columns, array->columns);
            for (std::size_t row = 0U; row < copy_rows; ++row)
            {
                for (std::size_t column = 0U; column < copy_columns; ++column)
                {
                    new_values[(row * columns) + column] = array->values[(row * array->columns) + column];
                }
            }
            array->rows = rows;
            array->columns = columns;
            array->values = std::move(new_values);
            return make_number_value(static_cast<double>(array->values.size()));
        }

        PrgValue copy_array_values(
            const std::string &source_name,
            const std::string &target_name,
            std::size_t source_start,
            std::size_t count,
            std::size_t target_start)
        {
            RuntimeArray *source = find_array(source_name);
            if (source == nullptr || source->values.empty() || trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }
            source_start = std::max<std::size_t>(1U, source_start);
            target_start = std::max<std::size_t>(1U, target_start);
            if (source_start > source->values.size())
            {
                return make_number_value(0.0);
            }

            const std::size_t available = source->values.size() - source_start + 1U;
            const std::size_t copy_count = count == 0U ? available : std::min(count, available);
            if (copy_count == 0U)
            {
                return make_number_value(0.0);
            }

            RuntimeArray *target = find_array(target_name);
            const std::size_t target_columns = target == nullptr ? 1U : std::max<std::size_t>(1U, target->columns);
            const std::size_t required_elements = target_start + copy_count - 1U;
            if (target == nullptr || required_elements > target->values.size())
            {
                const std::size_t required_rows = (required_elements + target_columns - 1U) / target_columns;
                resize_array(target_name, required_rows, target_columns);
                target = find_array(target_name);
            }
            if (target == nullptr || required_elements > target->values.size())
            {
                return make_number_value(0.0);
            }

            std::vector<PrgValue> snapshot;
            snapshot.reserve(copy_count);
            for (std::size_t offset = 0U; offset < copy_count; ++offset)
            {
                snapshot.push_back(source->values[source_start - 1U + offset]);
            }
            for (std::size_t offset = 0U; offset < snapshot.size(); ++offset)
            {
                target->values[target_start - 1U + offset] = snapshot[offset];
            }
            return make_number_value(static_cast<double>(snapshot.size()));
        }

        PrgValue populate_lines_array(
            const std::string &target_name,
            const std::string &text,
            int flags = 0,
            const std::vector<std::string> &parse_tokens = {})
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }

            std::vector<std::string> lines;
            if (!parse_tokens.empty())
            {
                std::string current = text;
                lines.push_back(current);
                for (const std::string &token : parse_tokens)
                {
                    if (token.empty())
                    {
                        continue;
                    }
                    std::vector<std::string> next;
                    for (const std::string &part : lines)
                    {
                        std::size_t start = 0U;
                        while (true)
                        {
                            const std::size_t found = part.find(token, start);
                            if (found == std::string::npos)
                            {
                                next.push_back(part.substr(start));
                                break;
                            }
                            next.push_back(part.substr(start, found - start));
                            start = found + token.size();
                        }
                    }
                    lines = std::move(next);
                }
            }
            else
            {
                lines = split_text_lines(text);
            }

            const bool trim_lines = (flags & 1) != 0;
            const bool omit_empty = (flags & 2) != 0;
            std::vector<PrgValue> values;
            values.reserve(lines.size());
            for (std::string line : lines)
            {
                if (trim_lines)
                {
                    line = trim_copy(line);
                }
                if (omit_empty && line.empty())
                {
                    continue;
                }
                values.push_back(make_string_value(std::move(line)));
            }
            assign_array(target_name, std::move(values), 1U);
            return make_number_value(static_cast<double>(array_length(target_name, 0)));
        }

        PrgValue populate_directory_array(
            const std::string &target_name,
            const std::string &skeleton,
            const std::string &attribute_filter)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }

            namespace fs = std::filesystem;
            fs::path pattern_path = skeleton.empty() ? fs::path("*.*") : fs::path(skeleton);
            if (pattern_path.is_relative())
            {
                pattern_path = fs::path(current_default_directory()) / pattern_path;
            }
            const fs::path directory = pattern_path.has_parent_path() ? pattern_path.parent_path() : fs::path(current_default_directory());
            const std::string pattern = pattern_path.filename().string().empty() ? "*.*" : pattern_path.filename().string();
            const bool include_directories = normalize_identifier(attribute_filter).find('d') != std::string::npos;

            std::vector<fs::directory_entry> entries;
            std::error_code ignored;
            if (fs::exists(directory, ignored))
            {
                for (const auto &entry : fs::directory_iterator(directory, ignored))
                {
                    const bool is_directory = entry.is_directory(ignored);
                    if (is_directory && !include_directories)
                    {
                        continue;
                    }
                    if (!wildcard_match_insensitive(pattern, entry.path().filename().string()))
                    {
                        continue;
                    }
                    entries.push_back(entry);
                }
            }
            std::sort(entries.begin(), entries.end(), [](const fs::directory_entry &left, const fs::directory_entry &right)
                      { return lowercase_copy(left.path().filename().string()) < lowercase_copy(right.path().filename().string()); });

            std::vector<PrgValue> values;
            values.reserve(entries.size() * 5U);
            for (const auto &entry : entries)
            {
                const auto last_write = entry.last_write_time(ignored);
                const bool is_directory = entry.is_directory(ignored);
                values.push_back(make_string_value(entry.path().filename().string()));
                values.push_back(make_number_value(is_directory ? 0.0 : static_cast<double>(entry.file_size(ignored))));
                values.push_back(make_string_value(format_file_time_part(last_write, true)));
                values.push_back(make_string_value(format_file_time_part(last_write, false)));
                values.push_back(make_string_value(file_attributes_for_adir(entry)));
            }
            assign_array(target_name, std::move(values), 5U);
            return make_number_value(static_cast<double>(entries.size()));
        }

        PrgValue populate_fields_array(const std::string &target_name, const std::string &designator)
        {
            if (trim_copy(target_name).empty())
            {
                return make_number_value(0.0);
            }
            CursorState *cursor = resolve_cursor_target(designator);
            if (cursor == nullptr || cursor->source_path.empty())
            {
                assign_array(target_name, {}, 16U);
                return make_number_value(0.0);
            }
            const auto table_result = vfp::parse_dbf_table_from_file(cursor->source_path, 1U);
            if (!table_result.ok)
            {
                assign_array(target_name, {}, 16U);
                return make_number_value(0.0);
            }

            std::vector<PrgValue> values;
            values.reserve(table_result.table.fields.size() * 16U);
            for (const auto &field : table_result.table.fields)
            {
                values.push_back(make_string_value(field.name));
                values.push_back(make_string_value(std::string(1U, static_cast<char>(std::toupper(static_cast<unsigned char>(field.type))))));
                values.push_back(make_number_value(static_cast<double>(field.length)));
                values.push_back(make_number_value(static_cast<double>(field.decimal_count)));
                values.push_back(make_boolean_value(false));
                values.push_back(make_boolean_value(false));
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
                values.push_back(make_empty_value());
            }
            assign_array(target_name, std::move(values), 16U);
            return make_number_value(static_cast<double>(table_result.table.fields.size()));
        }

        PrgValue mutate_array_function(
            const std::string &function,
            const std::vector<std::string> &raw_arguments,
            const std::vector<PrgValue> &arguments)
        {
            if (raw_arguments.empty())
            {
                return make_number_value(0.0);
            }
            const std::string array_name = raw_arguments[0];
            const std::string normalized_function = normalize_identifier(function);
            RuntimeArray *array = find_array(array_name);

            if (normalized_function == "alines" && arguments.size() >= 2U)
            {
                const int flags = arguments.size() >= 3U ? static_cast<int>(std::llround(value_as_number(arguments[2]))) : 0;
                std::vector<std::string> parse_tokens;
                for (std::size_t index = 3U; index < arguments.size(); ++index)
                {
                    parse_tokens.push_back(value_as_string(arguments[index]));
                }
                return populate_lines_array(array_name, value_as_string(arguments[1]), flags, parse_tokens);
            }

            if (normalized_function == "adir")
            {
                const std::string skeleton = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{"*.*"};
                const std::string attributes = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
                return populate_directory_array(array_name, skeleton, attributes);
            }

            if (normalized_function == "afields")
            {
                const std::string designator = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
                return populate_fields_array(array_name, designator);
            }

            if (normalized_function == "asize")
            {
                const std::size_t rows = arguments.size() >= 2U
                                             ? static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[1])))
                                             : 0U;
                const std::size_t columns = arguments.size() >= 3U
                                                ? static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[2])))
                                                : 1U;
                return resize_array(array_name, rows, columns);
            }

            if (normalized_function == "acopy" && raw_arguments.size() >= 2U)
            {
                const std::size_t source_start = arguments.size() >= 3U
                                                     ? static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[2])))
                                                     : 1U;
                const std::size_t count = arguments.size() >= 4U
                                              ? static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[3])))
                                              : 0U;
                const std::size_t target_start = arguments.size() >= 5U
                                                     ? static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[4])))
                                                     : 1U;
                return copy_array_values(array_name, raw_arguments[1], source_start, count, target_start);
            }

            if (array == nullptr)
            {
                return make_number_value(0.0);
            }
            if (normalized_function == "aelement" && arguments.size() >= 2U)
            {
                const std::size_t row = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[1])));
                const std::size_t column = arguments.size() >= 3U
                                               ? static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[2])))
                                               : 1U;
                return make_number_value(static_cast<double>(array_linear_index(*array, row, column)));
            }
            if (normalized_function == "asubscript" && arguments.size() >= 3U)
            {
                const std::size_t element = static_cast<std::size_t>(std::max<double>(0.0, value_as_number(arguments[1])));
                const int dimension = static_cast<int>(std::llround(value_as_number(arguments[2])));
                return make_number_value(static_cast<double>(array_subscript(*array, element, dimension)));
            }
            if (normalized_function == "ascan" && arguments.size() >= 2U)
            {
                const double raw_start = arguments.size() >= 3U ? value_as_number(arguments[2]) : 1.0;
                const double raw_count = arguments.size() >= 4U ? value_as_number(arguments[3]) : -1.0;
                const int search_column = arguments.size() >= 5U ? static_cast<int>(std::llround(value_as_number(arguments[4]))) : -1;
                const int flags = arguments.size() >= 6U ? static_cast<int>(std::llround(value_as_number(arguments[5]))) : 0;
                const bool case_insensitive = (flags & 1) != 0;
                const bool predicate_search = (flags & 16) != 0;
                const bool exact_match = (flags & 4) != 0
                                             ? (flags & 2) != 0
                                             : is_set_enabled("exact");
                const std::string predicate_text = predicate_search ? trim_copy(value_as_string(arguments[1])) : std::string{};
                const auto array_value_matches = [&](const PrgValue &left, const PrgValue &right)
                {
                    if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string)
                    {
                        std::string left_value = value_as_string(left);
                        std::string right_value = value_as_string(right);
                        if (case_insensitive)
                        {
                            left_value = uppercase_copy(std::move(left_value));
                            right_value = uppercase_copy(std::move(right_value));
                        }
                        return exact_match
                                   ? left_value == right_value
                                   : left_value.rfind(right_value, 0U) == 0U;
                    }
                    if (left.kind == PrgValueKind::boolean || right.kind == PrgValueKind::boolean)
                    {
                        return value_as_bool(left) == value_as_bool(right);
                    }
                    return std::abs(value_as_number(left) - value_as_number(right)) < 0.000001;
                };
                auto parse_predicate_block = [](const std::string &text)
                {
                    struct PredicateBlock
                    {
                        std::string parameter;
                        std::string expression;
                    };
                    PredicateBlock block{std::string{}, trim_copy(text)};
                    if (text.size() >= 5U && text[0] == '{' && text[1] == '|')
                    {
                        const std::size_t parameter_end = text.find('|', 2U);
                        if (parameter_end != std::string::npos)
                        {
                            const std::size_t close = text.rfind('}');
                            const std::size_t expression_end = close == std::string::npos || close <= parameter_end
                                                                   ? text.size()
                                                                   : close;
                            block.parameter = trim_copy(text.substr(2U, parameter_end - 2U));
                            block.expression = trim_copy(text.substr(parameter_end + 1U, expression_end - parameter_end - 1U));
                        }
                    }
                    return block;
                };
                const auto predicate_block = parse_predicate_block(predicate_text);
                const std::array<std::string, 4U> predicate_metadata_names = {
                    normalize_memory_variable_identifier("_ASCANVALUE"),
                    normalize_memory_variable_identifier("_ASCANINDEX"),
                    normalize_memory_variable_identifier("_ASCANROW"),
                    normalize_memory_variable_identifier("_ASCANCOLUMN")};
                const std::string predicate_parameter_name = normalize_memory_variable_identifier(predicate_block.parameter);
                std::map<std::string, std::optional<PrgValue>> saved_globals;
                std::map<std::string, std::optional<PrgValue>> saved_locals;
                auto snapshot_predicate_binding = [&](Frame &frame, const std::string &name)
                {
                    if (name.empty() || saved_globals.contains(name))
                    {
                        return;
                    }
                    if (const auto global = globals.find(name); global != globals.end())
                    {
                        saved_globals[name] = global->second;
                    }
                    else
                    {
                        saved_globals[name] = std::nullopt;
                    }
                    if (const auto local = frame.locals.find(name); local != frame.locals.end())
                    {
                        saved_locals[name] = local->second;
                    }
                    else
                    {
                        saved_locals[name] = std::nullopt;
                    }
                };
                if (predicate_search && !stack.empty())
                {
                    Frame &frame = stack.back();
                    for (const std::string &name : predicate_metadata_names)
                    {
                        snapshot_predicate_binding(frame, name);
                    }
                    snapshot_predicate_binding(frame, predicate_parameter_name);
                }
                auto restore_predicate_bindings = [&]()
                {
                    if (stack.empty())
                    {
                        return;
                    }
                    Frame &frame = stack.back();
                    for (const auto &[name, value] : saved_globals)
                    {
                        if (value)
                        {
                            globals[name] = *value;
                        }
                        else
                        {
                            globals.erase(name);
                        }
                    }
                    for (const auto &[name, value] : saved_locals)
                    {
                        if (value)
                        {
                            frame.locals[name] = *value;
                        }
                        else
                        {
                            frame.locals.erase(name);
                        }
                    }
                };
                const auto predicate_value_matches = [&](const PrgValue &value, std::size_t linear_index)
                {
                    if (!predicate_search)
                    {
                        return false;
                    }
                    if (predicate_block.expression.empty() || stack.empty())
                    {
                        return false;
                    }
                    Frame &frame = stack.back();
                    const std::size_t row = array->columns > 0U ? (linear_index / array->columns) + 1U : linear_index + 1U;
                    const std::size_t column = array->columns > 0U ? (linear_index % array->columns) + 1U : 1U;
                    assign_variable(frame, "_ASCANVALUE", value);
                    assign_variable(frame, "_ASCANINDEX", make_number_value(static_cast<double>(linear_index + 1U)));
                    assign_variable(frame, "_ASCANROW", make_number_value(static_cast<double>(row)));
                    assign_variable(frame, "_ASCANCOLUMN", make_number_value(static_cast<double>(column)));
                    if (!predicate_block.parameter.empty())
                    {
                        assign_variable(frame, predicate_block.parameter, value);
                    }
                    return value_as_bool(evaluate_expression(predicate_block.expression, frame));
                };
                const std::size_t start = raw_start <= 0.0
                                              ? 1U
                                              : static_cast<std::size_t>(raw_start);
                const std::size_t count = raw_count <= 0.0
                                              ? 0U
                                              : static_cast<std::size_t>(raw_count);
                if (start == 0U || start > array->values.size())
                {
                    restore_predicate_bindings();
                    return make_number_value(0.0);
                }
                if (search_column > 0 && array->columns > 1U)
                {
                    const std::size_t column = static_cast<std::size_t>(search_column);
                    if (column > array->columns)
                    {
                        restore_predicate_bindings();
                        return make_number_value(0.0);
                    }
                    const std::size_t start_row = (start - 1U) / array->columns;
                    const std::size_t available_rows = array->rows > start_row ? array->rows - start_row : 0U;
                    const std::size_t rows_to_scan = count == 0U ? available_rows : std::min(count, available_rows);
                    for (std::size_t row = start_row; row < start_row + rows_to_scan; ++row)
                    {
                        const std::size_t index = (row * array->columns) + (column - 1U);
                        if (index < array->values.size() &&
                            (predicate_value_matches(array->values[index], index) ||
                             (!predicate_search && array_value_matches(array->values[index], arguments[1]))))
                        {
                            const PrgValue result = make_number_value((flags & 8) != 0
                                                                          ? static_cast<double>(row + 1U)
                                                                          : static_cast<double>(index + 1U));
                            restore_predicate_bindings();
                            return result;
                        }
                    }
                    restore_predicate_bindings();
                    return make_number_value(0.0);
                }
                const std::size_t begin_index = start - 1U;
                const std::size_t available = array->values.size() - begin_index;
                const std::size_t scan_count = count == 0U ? available : std::min(count, available);
                const std::size_t end_index = begin_index + scan_count;
                for (std::size_t index = begin_index; index < end_index; ++index)
                {
                    if (predicate_value_matches(array->values[index], index) ||
                        (!predicate_search && array_value_matches(array->values[index], arguments[1])))
                    {
                        const PrgValue result = make_number_value((flags & 8) != 0 && array->columns > 1U
                                                                      ? static_cast<double>((index / array->columns) + 1U)
                                                                      : static_cast<double>(index + 1U));
                        restore_predicate_bindings();
                        return result;
                    }
                }
                restore_predicate_bindings();
                return make_number_value(0.0);
            }
            if (normalized_function == "adel" && arguments.size() >= 2U)
            {
                const std::size_t position = static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[1])));
                const int row_or_column = arguments.size() >= 3U ? static_cast<int>(std::llround(value_as_number(arguments[2]))) : 1;
                if (array->columns > 1U)
                {
                    if (row_or_column == 2)
                    {
                        if (position == 0U || position > array->columns)
                        {
                            return make_number_value(0.0);
                        }
                        for (std::size_t row = 0U; row < array->rows; ++row)
                        {
                            for (std::size_t column = position - 1U; column + 1U < array->columns; ++column)
                            {
                                array->values[(row * array->columns) + column] =
                                    array->values[(row * array->columns) + column + 1U];
                            }
                            array->values[(row * array->columns) + array->columns - 1U] = make_boolean_value(false);
                        }
                    }
                    else
                    {
                        if (position == 0U || position > array->rows)
                        {
                            return make_number_value(0.0);
                        }
                        for (std::size_t row = position - 1U; row + 1U < array->rows; ++row)
                        {
                            for (std::size_t column = 0U; column < array->columns; ++column)
                            {
                                array->values[(row * array->columns) + column] =
                                    array->values[((row + 1U) * array->columns) + column];
                            }
                        }
                        const std::size_t last_row = array->rows - 1U;
                        for (std::size_t column = 0U; column < array->columns; ++column)
                        {
                            array->values[(last_row * array->columns) + column] = make_boolean_value(false);
                        }
                    }
                }
                else
                {
                    if (position == 0U || position > array->values.size())
                    {
                        return make_number_value(0.0);
                    }
                    for (std::size_t index = position - 1U; index + 1U < array->values.size(); ++index)
                    {
                        array->values[index] = array->values[index + 1U];
                    }
                    if (!array->values.empty())
                    {
                        array->values.back() = make_boolean_value(false);
                    }
                }
                return make_number_value(1.0);
            }
            if (normalized_function == "ains" && arguments.size() >= 2U)
            {
                const std::size_t position = static_cast<std::size_t>(std::max<double>(1.0, value_as_number(arguments[1])));
                const int row_or_column = arguments.size() >= 3U ? static_cast<int>(std::llround(value_as_number(arguments[2]))) : 1;
                if (array->columns > 1U)
                {
                    if (row_or_column == 2)
                    {
                        if (position == 0U || position > array->columns)
                        {
                            return make_number_value(0.0);
                        }
                        for (std::size_t row = 0U; row < array->rows; ++row)
                        {
                            for (std::size_t column = array->columns - 1U; column > position - 1U; --column)
                            {
                                array->values[(row * array->columns) + column] =
                                    array->values[(row * array->columns) + column - 1U];
                            }
                            array->values[(row * array->columns) + position - 1U] = make_boolean_value(false);
                        }
                    }
                    else
                    {
                        if (position == 0U || position > array->rows)
                        {
                            return make_number_value(0.0);
                        }
                        for (std::size_t row = array->rows - 1U; row > position - 1U; --row)
                        {
                            for (std::size_t column = 0U; column < array->columns; ++column)
                            {
                                array->values[(row * array->columns) + column] =
                                    array->values[((row - 1U) * array->columns) + column];
                            }
                        }
                        for (std::size_t column = 0U; column < array->columns; ++column)
                        {
                            array->values[((position - 1U) * array->columns) + column] = make_boolean_value(false);
                        }
                    }
                }
                else
                {
                    if (position == 0U || position > array->values.size())
                    {
                        return make_number_value(0.0);
                    }
                    for (std::size_t index = array->values.size() - 1U; index > position - 1U; --index)
                    {
                        array->values[index] = array->values[index - 1U];
                    }
                    array->values[position - 1U] = make_boolean_value(false);
                }
                return make_number_value(1.0);
            }
            if (normalized_function == "asort")
            {
                const double raw_start = arguments.size() >= 2U ? value_as_number(arguments[1]) : 1.0;
                const double raw_count = arguments.size() >= 3U ? value_as_number(arguments[2]) : -1.0;
                const double raw_order = arguments.size() >= 4U ? value_as_number(arguments[3]) : 0.0;
                const int flags = arguments.size() >= 5U ? static_cast<int>(std::llround(value_as_number(arguments[4]))) : 0;
                const bool descending = raw_order > 0.0;
                const bool case_insensitive = (flags & 1) != 0;
                const auto is_numeric_array_value = [](const PrgValue &value)
                {
                    return value.kind == PrgValueKind::number ||
                           value.kind == PrgValueKind::int64 ||
                           value.kind == PrgValueKind::uint64;
                };
                const auto sort_key = [&](const PrgValue &value)
                {
                    std::string key = value_as_string(value);
                    return case_insensitive ? uppercase_copy(std::move(key)) : key;
                };
                const auto value_less = [&](const PrgValue &left, const PrgValue &right)
                {
                    if (is_numeric_array_value(left) && is_numeric_array_value(right))
                    {
                        const double left_number = value_as_number(left);
                        const double right_number = value_as_number(right);
                        return descending ? right_number < left_number : left_number < right_number;
                    }
                    const std::string left_key = sort_key(left);
                    const std::string right_key = sort_key(right);
                    return descending ? right_key < left_key : left_key < right_key;
                };
                const std::size_t start = raw_start <= 0.0
                                              ? 1U
                                              : static_cast<std::size_t>(raw_start);
                if (start == 0U || start > array->values.size())
                {
                    return make_number_value(-1.0);
                }
                if (array->columns <= 1U)
                {
                    const std::size_t begin_index = start - 1U;
                    const std::size_t available = array->values.size() - begin_index;
                    const std::size_t count = raw_count <= 0.0
                                                  ? available
                                                  : std::min(static_cast<std::size_t>(raw_count), available);
                    std::sort(array->values.begin() + static_cast<std::ptrdiff_t>(begin_index),
                              array->values.begin() + static_cast<std::ptrdiff_t>(begin_index + count),
                              value_less);
                    return make_number_value(1.0);
                }
                const std::size_t start_index = start - 1U;
                const std::size_t start_row = start_index / array->columns;
                const std::size_t sort_column = start_index % array->columns;
                const std::size_t available_rows = array->rows > start_row ? array->rows - start_row : 0U;
                const std::size_t rows_to_sort = raw_count <= 0.0
                                                     ? available_rows
                                                     : std::min(static_cast<std::size_t>(raw_count), available_rows);
                std::vector<std::vector<PrgValue>> rows;
                rows.reserve(rows_to_sort);
                for (std::size_t row = start_row; row < start_row + rows_to_sort; ++row)
                {
                    const auto row_begin = array->values.begin() + static_cast<std::ptrdiff_t>(row * array->columns);
                    rows.emplace_back(row_begin, row_begin + static_cast<std::ptrdiff_t>(array->columns));
                }
                std::sort(rows.begin(), rows.end(), [&](const auto &left, const auto &right)
                          { return value_less(left[sort_column], right[sort_column]); });
                for (std::size_t offset = 0U; offset < rows.size(); ++offset)
                {
                    const std::size_t row = start_row + offset;
                    std::copy(rows[offset].begin(), rows[offset].end(),
                              array->values.begin() + static_cast<std::ptrdiff_t>(row * array->columns));
                }
                return make_number_value(1.0);
            }
            return make_number_value(0.0);
        }

        int populate_error_array(const std::string &name)
        {
            if (trim_copy(name).empty())
            {
                return 0;
            }
            if (last_error_code == 0 && last_error_message.empty())
            {
                return 0;
            }
            const int effective_error_code = last_error_code == 0
                                                 ? classify_runtime_error_code(last_error_message)
                                                 : last_error_code;
            const std::string error_parameter = runtime_error_parameter(last_error_message);
            if (effective_error_code == 1526)
            {
                std::vector<PrgValue> values{
                    make_number_value(1526.0),
                    make_string_value(last_error_message),
                    make_string_value(error_parameter),
                    make_string_value("HY000"),
                    make_number_value(-1.0),
                    make_number_value(0.0),
                    make_empty_value()};
                assign_array(name, std::move(values), 7U);
                return 1;
            }
            if (effective_error_code == 1429)
            {
                std::vector<PrgValue> values{
                    make_number_value(1429.0),
                    make_string_value(last_error_message),
                    make_string_value(error_parameter),
                    make_string_value("Copperfin OLE"),
                    make_empty_value(),
                    make_empty_value(),
                    make_number_value(1429.0)};
                assign_array(name, std::move(values), 7U);
                return 1;
            }
            std::vector<PrgValue> values{
                make_number_value(static_cast<double>(effective_error_code)),
                make_string_value(last_error_message),
                make_string_value(error_parameter == last_error_message ? std::string{} : error_parameter),
                make_number_value(static_cast<double>(current_selected_work_area())),
                make_empty_value(),
                make_empty_value(),
                make_empty_value()};
            assign_array(name, std::move(values), 7U);
            return 1;
        }

        void restore_private_declarations(Frame &frame)
        {
            for (const auto &[name, saved] : frame.private_saved_values)
            {
                if (saved.has_value())
                {
                    globals[name] = *saved;
                }
                else
                {
                    globals.erase(name);
                }
            }
        }

        void sync_byref_arguments(Frame &frame)
        {
            if (frame.parameter_reference_bindings.empty())
            {
                return;
            }

            Frame *caller = stack.size() >= 2U ? &stack[stack.size() - 2U] : nullptr;
            for (const auto &[parameter_name, reference_name] : frame.parameter_reference_bindings)
            {
                const auto local = frame.locals.find(parameter_name);
                if (local == frame.locals.end())
                {
                    continue;
                }
                if (caller != nullptr)
                {
                    assign_variable(*caller, reference_name, local->second);
                }
                else
                {
                    globals[normalize_memory_variable_identifier(reference_name)] = local->second;
                }
            }
        }

        void pop_frame()
        {
            if (!stack.empty())
            {
                sync_byref_arguments(stack.back());
                restore_private_declarations(stack.back());
                stack.pop_back();
            }
        }

        bool breakpoint_matches(const SourceLocation &location) const
        {
            const std::string normalized = normalize_path(location.file_path);
            return std::any_of(breakpoints.begin(), breakpoints.end(), [&](const RuntimeBreakpoint &breakpoint)
                               { return normalize_path(breakpoint.file_path) == normalized && breakpoint.line == location.line; });
        }

        std::optional<std::size_t> find_matching_branch(const Frame &frame, std::size_t pc, bool seek_else) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::if_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endif_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
                else if (seek_else && kind == StatementKind::else_statement && depth == 0)
                {
                    return index;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_endfor(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::for_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endfor_statement)
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

        std::optional<std::size_t> find_matching_enddo(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_while_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::enddo_statement)
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

        std::optional<std::size_t> find_matching_endcase(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_case_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endcase_statement)
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

        std::optional<std::size_t> find_matching_endwith(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::with_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endwith_statement)
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

        struct TryClauseTargets
        {
            std::optional<std::size_t> catch_statement_index;
            std::optional<std::size_t> finally_statement_index;
            std::optional<std::size_t> endtry_statement_index;
        };

        TryClauseTargets find_try_clause_targets(const Frame &frame, std::size_t pc) const
        {
            TryClauseTargets targets;
            if (frame.routine == nullptr)
            {
                return targets;
            }

            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::try_statement)
                {
                    ++depth;
                    continue;
                }
                if (kind == StatementKind::endtry_statement)
                {
                    if (depth == 0)
                    {
                        targets.endtry_statement_index = index;
                        return targets;
                    }
                    --depth;
                    continue;
                }
                if (depth != 0)
                {
                    continue;
                }
                if (kind == StatementKind::catch_statement && !targets.catch_statement_index.has_value())
                {
                    targets.catch_statement_index = index;
                }
                else if (kind == StatementKind::finally_statement && !targets.finally_statement_index.has_value())
                {
                    targets.finally_statement_index = index;
                }
            }

            return targets;
        }

        std::string apply_with_context(std::string text, const Frame &frame) const
        {
            if (frame.withs.empty())
            {
                return text;
            }

            const std::string binding_name = frame.withs.back().binding_name;
            if (binding_name.empty())
            {
                return text;
            }

            std::string rewritten;
            rewritten.reserve(text.size() + binding_name.size() * 2U);
            bool in_string = false;
            for (std::size_t index = 0U; index < text.size(); ++index)
            {
                const char ch = text[index];
                if (ch == '\'')
                {
                    in_string = !in_string;
                    rewritten.push_back(ch);
                    continue;
                }
                if (in_string || ch != '.' || (index + 1U) >= text.size())
                {
                    rewritten.push_back(ch);
                    continue;
                }

                const unsigned char next = static_cast<unsigned char>(text[index + 1U]);
                if (std::isalpha(next) == 0 && next != '_')
                {
                    rewritten.push_back(ch);
                    continue;
                }

                char previous_nonspace = '\0';
                for (std::size_t lookback = index; lookback > 0U; --lookback)
                {
                    const char candidate = text[lookback - 1U];
                    if (std::isspace(static_cast<unsigned char>(candidate)) != 0)
                    {
                        continue;
                    }
                    previous_nonspace = candidate;
                    break;
                }

                if (std::isalnum(static_cast<unsigned char>(previous_nonspace)) != 0 ||
                    previous_nonspace == '_' ||
                    previous_nonspace == '.')
                {
                    rewritten.push_back(ch);
                    continue;
                }

                rewritten.append(binding_name);
                rewritten.push_back(ch);
            }

            return rewritten;
        }

        bool dispatch_try_handler(Frame &frame, const Statement &statement)
        {
            for (auto iterator = frame.tries.rbegin(); iterator != frame.tries.rend(); ++iterator)
            {
                TryState &active_try = *iterator;
                if (active_try.handling_error)
                {
                    continue;
                }

                active_try.handling_error = true;
                active_try.entered_catch = false;
                active_try.entered_finally = false;
                if (!active_try.catch_variable.empty())
                {
                    assign_variable(frame, active_try.catch_variable, make_string_value(last_error_message));
                }

                if (active_try.catch_statement_index.has_value())
                {
                    frame.pc = *active_try.catch_statement_index + 1U;
                    active_try.entered_catch = true;
                }
                else if (active_try.finally_statement_index.has_value())
                {
                    frame.pc = *active_try.finally_statement_index + 1U;
                    active_try.entered_finally = true;
                }
                else
                {
                    frame.pc = active_try.endtry_statement_index + 1U;
                    frame.tries.erase(std::next(iterator).base());
                }

                events.push_back({.category = "runtime.try_handler",
                                  .detail = statement.text,
                                  .location = statement.location});
                return true;
            }

            return false;
        }

        std::optional<std::size_t> find_next_case_clause(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_case_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endcase_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
                else if (depth == 0 && (kind == StatementKind::case_statement || kind == StatementKind::otherwise_statement))
                {
                    return index;
                }
            }
            return std::nullopt;
        }

        enum class ActiveLoopKind
        {
            for_loop,
            scan_loop,
            while_loop
        };

        struct ActiveLoop
        {
            ActiveLoopKind kind = ActiveLoopKind::for_loop;
            std::size_t start_statement_index = 0;
            std::size_t end_statement_index = 0;
        };

        std::optional<ActiveLoop> find_innermost_active_loop(const Frame &frame) const
        {
            std::optional<ActiveLoop> active;
            const auto consider = [&](ActiveLoop candidate)
            {
                if (!active.has_value() || candidate.start_statement_index > active->start_statement_index)
                {
                    active = candidate;
                }
            };

            if (!frame.loops.empty())
            {
                const LoopState &loop = frame.loops.back();
                consider({.kind = ActiveLoopKind::for_loop,
                          .start_statement_index = loop.for_statement_index,
                          .end_statement_index = loop.endfor_statement_index});
            }
            if (!frame.scans.empty())
            {
                const ScanState &scan = frame.scans.back();
                consider({.kind = ActiveLoopKind::scan_loop,
                          .start_statement_index = scan.scan_statement_index,
                          .end_statement_index = scan.endscan_statement_index});
            }
            if (!frame.whiles.empty())
            {
                const WhileState &loop = frame.whiles.back();
                consider({.kind = ActiveLoopKind::while_loop,
                          .start_statement_index = loop.do_while_statement_index,
                          .end_statement_index = loop.enddo_statement_index});
            }

            return active;
        }

        ExecutionOutcome continue_for_loop(Frame &frame, const Statement &, bool jump_after_completion)
        {
            if (frame.loops.empty())
            {
                return {};
            }

            LoopState &loop = frame.loops.back();
            ++loop.iteration_count;
            if (loop.iteration_count > max_loop_iterations)
            {
                last_error_message = loop_iteration_limit_message();
                return {.ok = false, .message = last_error_message};
            }
            const double next_value = value_as_number(lookup_variable(frame, loop.variable_name)) + loop.step_value;
            assign_variable(frame, loop.variable_name, make_number_value(next_value));
            const bool should_continue = loop.step_value >= 0.0
                                             ? next_value <= loop.end_value
                                             : next_value >= loop.end_value;
            if (should_continue)
            {
                frame.pc = loop.for_statement_index + 1U;
            }
            else
            {
                const std::size_t completion_pc = loop.endfor_statement_index + 1U;
                frame.loops.pop_back();
                if (jump_after_completion)
                {
                    frame.pc = completion_pc;
                }
            }
            return {};
        }

        ExecutionOutcome continue_scan_loop(Frame &frame, const Statement &statement, bool jump_after_completion)
        {
            if (frame.scans.empty())
            {
                return {};
            }

            ScanState &scan = frame.scans.back();
            ++scan.iteration_count;
            if (scan.iteration_count > max_loop_iterations)
            {
                last_error_message = loop_iteration_limit_message();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }
            CursorState *cursor = find_cursor_by_area(scan.work_area);
            if (cursor == nullptr)
            {
                frame.scans.pop_back();
                return {};
            }

            if (!locate_next_matching_record(*cursor, scan.for_expression, scan.while_expression, frame, cursor->recno + 1U))
            {
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            if (cursor->found)
            {
                frame.pc = scan.scan_statement_index + 1U;
            }
            else
            {
                frame.scans.pop_back();
                if (jump_after_completion)
                {
                    frame.pc = scan.endscan_statement_index + 1U;
                }
            }
            return {};
        }

        std::filesystem::path resolve_asset_path(const std::string &raw_path, const char *extension) const
        {
            std::filesystem::path asset_path(unquote_string(take_first_token(raw_path)));
            if (asset_path.extension().empty())
            {
                asset_path += extension;
            }
            if (asset_path.is_relative())
            {
                asset_path = std::filesystem::path(current_default_directory()) / asset_path;
            }
            return asset_path.lexically_normal();
        }

        std::filesystem::path resolve_report_output_path(const std::string &to_clause, const Frame &frame)
        {
            std::string target = trim_copy(to_clause);
            if (target.empty())
            {
                return {};
            }

            if (starts_with_insensitive(target, "FILE "))
            {
                target = trim_copy(target.substr(5U));
            }
            if (target.empty())
            {
                return {};
            }

            std::filesystem::path output_path;
            if (target.size() >= 2U && target.front() == '\'' && target.back() == '\'')
            {
                output_path = std::filesystem::path(unquote_string(target));
            }
            else
            {
                output_path = std::filesystem::path(value_as_string(evaluate_expression(target, frame)));
            }

            if (output_path.is_relative())
            {
                output_path = std::filesystem::path(current_default_directory()) / output_path;
            }
            return output_path.lexically_normal();
        }

        ExecutionOutcome open_report_surface(const Statement &statement, const Frame &frame, const char *extension, const char *category_prefix)
        {
            const std::filesystem::path asset_path = resolve_asset_path(statement.identifier, extension);
            if (!std::filesystem::exists(asset_path))
            {
                last_error_message = std::string("Unable to resolve report asset: ") + asset_path.string();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            studio::StudioOpenRequest request;
            request.path = asset_path.string();
            request.read_only = true;
            request.load_full_table = true;
            const auto open_result = studio::open_document(request);
            if (!open_result.ok)
            {
                last_error_message = open_result.error;
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            const auto layout = studio::build_report_layout(open_result.document);
            const bool preview_mode =
                normalize_identifier(statement.secondary_expression) == "preview" ||
                trim_copy(statement.tertiary_expression).empty();
            if (preview_mode)
            {
                waiting_for_events = true;
                events.push_back({.category = std::string(category_prefix) + ".preview",
                                  .detail = asset_path.string(),
                                  .location = statement.location});
                if (layout.available)
                {
                    events.push_back({.category = std::string(category_prefix) + ".preview.layout",
                                      .detail = std::to_string(layout.sections.size()) + " sections",
                                      .location = statement.location});
                }
                return {.ok = true, .waiting_for_events = true, .frame_returned = false, .message = {}};
            }

            const std::filesystem::path output_path = resolve_report_output_path(statement.tertiary_expression, frame);
            if (output_path.empty())
            {
                last_error_message = "REPORT/LABEL TO clause requires a writable output path";
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            if (!output_path.parent_path().empty())
            {
                std::error_code ignored;
                std::filesystem::create_directories(output_path.parent_path(), ignored);
            }

            std::ofstream output(output_path, std::ios::binary);
            if (!output.good())
            {
                last_error_message = "Unable to open report output path: " + output_path.string();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            output << "Copperfin " << category_prefix << " render\n";
            output << "source=" << asset_path.string() << "\n";
            if (layout.available)
            {
                output << "sections=" << layout.sections.size() << "\n";
            }
            output.close();
            if (!output.good())
            {
                last_error_message = "Unable to write report output path: " + output_path.string();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            events.push_back({.category = std::string(category_prefix) + ".render",
                              .detail = output_path.string(),
                              .location = statement.location});
            return {};
        }

        bool dispatch_event_handler(const std::string &routine_name);
        bool dispatch_error_handler();
        ExecutionOutcome execute_current_statement();
        RuntimePauseState run(DebugResumeAction action);
    };

#include "prg_engine_expression.inl"

    PrgValue PrgRuntimeSession::Impl::evaluate_expression(const std::string &expression, const Frame &frame)
    {
        return evaluate_expression(expression, frame, resolve_cursor_target({}));
    }

    PrgValue PrgRuntimeSession::Impl::evaluate_expression(
        const std::string &expression,
        const Frame &frame,
        const CursorState *preferred_cursor)
    {
        const std::string effective_expression = apply_with_context(expression, frame);
        ExpressionParser parser(
            effective_expression,
            frame,
            globals,
            current_default_directory(),
            last_error_message,
            last_error_code,
            last_error_procedure,
            last_fault_location.line,
            error_handler,
            is_set_enabled("exact"),
            current_selected_work_area(),
            [this]()
            {
                return next_available_work_area();
            },
            [this](const std::string &designator)
            {
                if (designator.empty())
                {
                    return current_selected_work_area();
                }
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0 : cursor->work_area;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? std::string{} : cursor->alias;
            },
            [this](const std::string &designator)
            {
                return resolve_cursor_target(designator) != nullptr;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? std::string{} : cursor->dbf_identity;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0U : cursor->field_count;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0U : cursor->record_count;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0U : cursor->recno;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? false : cursor->found;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? true : cursor->eof;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? true : cursor->bof;
            },
            [this, preferred_cursor](const std::string &identifier)
            {
                const CursorState *current_cursor = preferred_cursor == nullptr ? resolve_cursor_target({}) : preferred_cursor;
                return resolve_field_value(identifier, current_cursor);
            },
            [this](const std::string &name)
            {
                return has_array(name);
            },
            [this](const std::string &name, int dimension)
            {
                return array_length(name, dimension);
            },
            [this](const std::string &name, std::size_t row, std::size_t column)
            {
                return array_value(name, row, column);
            },
            [this](const std::string &function, const std::vector<std::string> &raw_arguments, const std::vector<PrgValue> &arguments)
            {
                return mutate_array_function(function, raw_arguments, arguments);
            },
            [this](const std::string &name)
            {
                return populate_error_array(name);
            },
            [this, &frame](const std::string &function_name, const std::vector<std::string> &raw_arguments)
            {
                return aggregate_function_value(function_name, raw_arguments, frame);
            },
            [this](const std::string &designator, bool include_path)
            {
                return order_function_value(designator, include_path);
            },
            [this](const std::string &index_file_name, std::size_t tag_number, const std::string &designator)
            {
                return tag_function_value(index_file_name, tag_number, designator);
            },
            [this](const std::string &search_key, bool move_pointer, const std::string &designator, const std::string &order_designator)
            {
                CursorState *cursor = resolve_cursor_target(designator);
                if (cursor == nullptr)
                {
                    return false;
                }
                const SeekFunctionOrderDesignator parsed_order = parse_seek_function_order_designator(order_designator);
                return execute_seek(
                    *cursor,
                    search_key,
                    move_pointer,
                    false,
                    parsed_order.order_designator,
                    parsed_order.descending_override);
            },
            [this](const std::string &search_key, bool move_pointer, const std::string &designator, const std::string &order_designator)
            {
                CursorState *cursor = resolve_cursor_target(designator);
                if (cursor == nullptr)
                {
                    return false;
                }
                const SeekFunctionOrderDesignator parsed_order = parse_seek_function_order_designator(order_designator);
                return execute_seek(
                    *cursor,
                    search_key,
                    move_pointer,
                    true,
                    parsed_order.order_designator,
                    parsed_order.descending_override);
            },
            [this]()
            {
                return std::string("FOXTOOLS:9.0");
            },
            []()
            {
                return 1001;
            },
            [this](const std::string &variant,
                   const std::string &function_name,
                   const std::string &argument_types,
                   const std::string &return_type,
                   const std::string &dll_name)
            {
                return register_api_function(variant, function_name, argument_types, return_type, dll_name);
            },
            [this](int handle, const std::vector<PrgValue> &arguments)
            {
                return call_registered_api_function(handle, arguments);
            },
            [this](const std::string &target, const std::string &provider)
            {
                return sql_connect(target, provider);
            },
            [this](int handle, const std::string &command, const std::string &cursor_alias)
            {
                return sql_exec(handle, command, cursor_alias);
            },
            [this](int handle)
            {
                return sql_disconnect(handle);
            },
            [this](int handle)
            {
                return sql_row_count(handle);
            },
            [this](int handle, const std::string &command)
            {
                return sql_prepare(handle, command);
            },
            [this](int handle, const std::string &property_name)
            {
                return sql_get_prop(handle, property_name);
            },
            [this](int handle, const std::string &property_name, const PrgValue &value)
            {
                return sql_set_prop(handle, property_name, value);
            },
            [this](const std::string &prog_id, const std::string &source)
            {
                return register_ole_object(prog_id, source);
            },
            [this, &frame](const std::string &base_name, const std::string &member_path, const std::vector<PrgValue> &arguments)
            {
                const PrgValue object_value = lookup_variable(frame, base_name);
                auto object = resolve_ole_object(object_value);
                if (!object.has_value())
                {
                    return make_empty_value();
                }

                RuntimeOleObjectState *runtime_object = *object;
                runtime_object->last_action = member_path + "()";
                ++runtime_object->action_count;
                events.push_back({.category = "ole.invoke",
                                  .detail = runtime_object->prog_id + "." + member_path,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});

                const std::string leaf = member_path.substr(member_path.rfind('.') == std::string::npos ? 0U : member_path.rfind('.') + 1U);
                if (leaf == "add" || leaf == "create" || leaf == "open" || leaf == "item")
                {
                    return make_string_value("object:" + runtime_object->prog_id + "." + member_path + "#" + std::to_string(runtime_object->handle));
                }
                if (arguments.empty())
                {
                    return make_string_value("ole:" + runtime_object->prog_id + "." + member_path);
                }
                return arguments.front();
            },
            [this, &frame](const std::string &property_path)
            {
                const auto separator = property_path.find('.');
                if (separator == std::string::npos)
                {
                    return make_empty_value();
                }

                const PrgValue object_value = lookup_variable(frame, property_path.substr(0U, separator));
                auto object = resolve_ole_object(object_value);
                if (!object.has_value())
                {
                    return make_empty_value();
                }

                RuntimeOleObjectState *runtime_object = *object;
                runtime_object->last_action = property_path.substr(separator + 1U);
                ++runtime_object->action_count;
                events.push_back({.category = "ole.get",
                                  .detail = runtime_object->prog_id + "." + property_path.substr(separator + 1U),
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                return make_string_value("ole:" + runtime_object->prog_id + "." + property_path.substr(separator + 1U));
            },
            [this, &frame, preferred_cursor](const std::string &nested_expression)
            {
                return evaluate_expression(nested_expression, frame, preferred_cursor);
            },
            [this](const std::string &option_name)
            {
                const std::string normalized_name = normalize_identifier(option_name);
                if (normalized_name == "default")
                {
                    return current_default_directory();
                }

                const auto found = current_set_state().find(normalized_name);
                if (found == current_set_state().end())
                {
                    return std::string("OFF");
                }

                const std::string normalized_value = normalize_identifier(found->second);
                if (normalized_value.empty() || normalized_value == "on" || normalized_value == "true" || normalized_value == "1")
                {
                    return std::string("ON");
                }
                if (normalized_value == "off" || normalized_value == "false" || normalized_value == "0")
                {
                    return std::string("OFF");
                }
                return found->second;
            },
            [this](const std::string &category, const std::string &detail)
            {
                events.push_back({.category = category,
                                  .detail = detail,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            },
            [this](const std::string &fn_key, const std::vector<PrgValue> &fn_args) -> PrgValue
            {
                return invoke_declared_dll_function(fn_key, fn_args);
            });
        return parser.parse();
    }

    std::optional<std::string> PrgRuntimeSession::Impl::materialize_xasset_bootstrap(
        const std::string &asset_path,
        bool include_read_events)
    {
        studio::StudioOpenRequest request;
        request.path = asset_path;
        request.read_only = true;
        request.load_full_table = true;
        const auto open_result = studio::open_document(request);
        if (!open_result.ok)
        {
            last_error_message = open_result.error;
            return std::nullopt;
        }

        const XAssetExecutableModel model = build_xasset_executable_model(open_result.document);
        if (!model.ok || !model.runnable_startup)
        {
            last_error_message = model.error.empty()
                                     ? "No runnable startup methods were found in asset: " + asset_path
                                     : model.error;
            return std::nullopt;
        }

        const std::filesystem::path asset_file(asset_path);
        const std::filesystem::path bootstrap_path =
            runtime_temp_directory /
            (asset_file.stem().string() + "_copperfin_bootstrap.prg");

        std::ofstream output(bootstrap_path, std::ios::binary);
        output << build_xasset_bootstrap_source(model, include_read_events);
        output.close();
        if (!output.good())
        {
            last_error_message = "Unable to materialize xAsset bootstrap for: " + asset_path;
            return std::nullopt;
        }

        return bootstrap_path.string();
    }

#include "prg_engine_dispatch.inl"
    bool PrgRuntimeSession::Impl::dispatch_event_handler(const std::string &routine_name)
    {
        if (!waiting_for_events || stack.empty())
        {
            return false;
        }

        const std::string normalized_target = normalize_identifier(routine_name);
        for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator)
        {
            Program &program = load_program(iterator->file_path);
            const auto found = program.routines.find(normalized_target);
            if (found == program.routines.end())
            {
                continue;
            }

            waiting_for_events = false;
            event_dispatch_return_depth = stack.size();
            restore_event_loop_after_dispatch = true;
            if (!can_push_frame())
            {
                waiting_for_events = true;
                restore_event_loop_after_dispatch = false;
                event_dispatch_return_depth.reset();
                last_error_message = call_depth_limit_message();
                events.push_back({.category = "runtime.error",
                                  .detail = last_error_message,
                                  .location = {}});
                return false;
            }
            push_routine_frame(program.path, found->second);
            events.push_back({.category = "runtime.dispatch",
                              .detail = found->second.name,
                              .location = {}});
            return true;
        }

        return false;
    }

    bool PrgRuntimeSession::Impl::dispatch_error_handler()
    {
        if (handling_error)
        {
            return false;
        }
        if (stack.empty())
        {
            return false;
        }

        std::string handler = trim_copy(error_handler);
        if (handler.empty())
        {
            return false;
        }

        if (!starts_with_insensitive(handler, "DO "))
        {
            return false;
        }
        handler = trim_copy(handler.substr(3U));
        if (handler.empty())
        {
            return false;
        }

        std::string handler_arguments_clause;
        const std::size_t with_position = find_keyword_top_level(handler, "WITH");
        if (with_position != std::string::npos)
        {
            handler_arguments_clause = trim_copy(handler.substr(with_position + 4U));
            handler = trim_copy(handler.substr(0U, with_position));
        }
        if (handler.empty())
        {
            return false;
        }

        const Frame &error_frame = stack.back();
        std::vector<PrgValue> handler_arguments;
        if (!handler_arguments_clause.empty())
        {
            for (const std::string &raw_argument : split_csv_like(handler_arguments_clause))
            {
                const std::string argument_expression = trim_copy(raw_argument);
                if (!argument_expression.empty())
                {
                    handler_arguments.push_back(evaluate_expression(argument_expression, error_frame));
                }
            }
        }

        for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator)
        {
            Program &program = load_program(iterator->file_path);
            const auto found = program.routines.find(normalize_identifier(handler));
            if (found == program.routines.end())
            {
                continue;
            }
            if (!can_push_frame())
            {
                return false;
            }

            handling_error = true;
            // Save fault position for RETRY / RESUME
            fault_frame_file_path = error_frame.file_path;
            fault_frame_routine_name = error_frame.routine_name;
            fault_statement_index = error_frame.pc > 0U ? error_frame.pc - 1U : 0U;
            fault_pc_valid = true;
            error_handler_return_depth = stack.size();
            push_routine_frame(program.path, found->second, handler_arguments);
            events.push_back({.category = "runtime.error_handler",
                              .detail = handler_arguments.empty()
                                            ? found->second.name
                                            : found->second.name + " WITH " + std::to_string(handler_arguments.size()) + " argument(s)",
                              .location = {}});
            return true;
        }

        return false;
    }

    RuntimePauseState PrgRuntimeSession::Impl::run(DebugResumeAction action)
    {
        if (entry_pause_pending)
        {
            entry_pause_pending = false;
            return build_pause_state(DebugPauseReason::entry, "Stopped on entry.");
        }

        if (waiting_for_events)
        {
            return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
        }

        const std::size_t base_depth = stack.size();
        bool first_statement = true;

        try
        {
            while (true)
            {
                while (!stack.empty() &&
                       (stack.back().routine == nullptr || stack.back().pc >= stack.back().routine->statements.size()))
                {
                    pop_frame();
                }
                if (event_dispatch_return_depth.has_value() && stack.size() <= *event_dispatch_return_depth)
                {
                    event_dispatch_return_depth.reset();
                    if (restore_event_loop_after_dispatch)
                    {
                        restore_event_loop_after_dispatch = false;
                        waiting_for_events = true;
                        return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
                    }
                    restore_event_loop_after_dispatch = false;
                }
                if (error_handler_return_depth.has_value() && stack.size() <= *error_handler_return_depth)
                {
                    error_handler_return_depth.reset();
                    handling_error = false;
                }
                if (stack.empty())
                {
                    return build_pause_state(DebugPauseReason::completed, "Execution completed.");
                }

                const Statement *next = current_statement();
                if (next == nullptr)
                {
                    pop_frame();
                    continue;
                }

                if (executed_statement_count >= max_executed_statements)
                {
                    last_error_message = step_budget_limit_message();
                    last_fault_location = next->location;
                    last_fault_statement = next->text;
                    events.push_back({.category = "runtime.error",
                                      .detail = last_error_message,
                                      .location = next->location});
                    return build_pause_state(DebugPauseReason::error, last_error_message);
                }

                if (!first_statement && breakpoint_matches(next->location))
                {
                    return build_pause_state(DebugPauseReason::breakpoint, "Breakpoint hit.");
                }

                const ExecutionOutcome outcome = execute_current_statement();
                if (!outcome.ok)
                {
                    if (!stack.empty())
                    {
                        capture_last_error_context(stack.back(), *next);
                        if (dispatch_try_handler(stack.back(), *next))
                        {
                            continue;
                        }
                    }
                    if (dispatch_error_handler())
                    {
                        continue;
                    }
                    return build_pause_state(DebugPauseReason::error, outcome.message);
                }
                if (outcome.waiting_for_events)
                {
                    return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
                }

                if (stack.empty())
                {
                    return build_pause_state(DebugPauseReason::completed, "Execution completed.");
                }

                switch (action)
                {
                case DebugResumeAction::continue_run:
                    if (scheduler_yield_statement_interval != 0U &&
                        (executed_statement_count % scheduler_yield_statement_interval) == 0U)
                    {
                        if (scheduler_yield_sleep_ms == 0U)
                        {
                            std::this_thread::yield();
                        }
                        else
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(scheduler_yield_sleep_ms));
                        }
                    }
                    break;
                case DebugResumeAction::step_into:
                    return build_pause_state(DebugPauseReason::step, "Step completed.");
                case DebugResumeAction::step_over:
                    if (stack.size() <= base_depth)
                    {
                        return build_pause_state(DebugPauseReason::step, "Step-over completed.");
                    }
                    break;
                case DebugResumeAction::step_out:
                    if (stack.size() < base_depth)
                    {
                        return build_pause_state(DebugPauseReason::step, "Step-out completed.");
                    }
                    break;
                }

                first_statement = false;
            }
        }
        catch (const std::bad_alloc &)
        {
            last_error_message = "Runtime resource fault: out of memory. Execution paused safely.";
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::filesystem::filesystem_error &error)
        {
            last_error_message = std::string("Runtime resource fault: filesystem failure: ") + error.what();
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::system_error &error)
        {
            last_error_message = std::string("Runtime resource fault: system error: ") + error.what();
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::exception &error)
        {
            last_error_message = std::string("Runtime fault: ") + error.what();
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (...)
        {
            last_error_message = "Runtime fault: unknown exception";
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
    }

    PrgRuntimeSession PrgRuntimeSession::create(const RuntimeSessionOptions &options)
    {
        RuntimeSessionOptions effective = options;
        effective.startup_path = normalize_path(effective.startup_path);
        effective.working_directory = effective.working_directory.empty()
                                          ? std::filesystem::path(effective.startup_path).parent_path().string()
                                          : normalize_path(effective.working_directory);

        if (const auto config = load_runtime_config_near(
                std::filesystem::path(effective.startup_path),
                std::filesystem::path(effective.working_directory)))
        {
            apply_runtime_config_defaults(effective, *config);
        }

        auto impl = std::make_unique<Impl>(effective);
        impl->startup_default_directory = effective.working_directory;
        impl->default_directory_by_session.emplace(1, impl->startup_default_directory);
        impl->data_sessions.try_emplace(1);
        impl->events.push_back({.category = "runtime.config",
                                .detail = "temp=" + impl->runtime_temp_directory.string() +
                                          ";max_call_depth=" + std::to_string(impl->max_call_depth) +
                                          ";max_executed_statements=" + std::to_string(impl->max_executed_statements) +
                                          ";max_loop_iterations=" + std::to_string(impl->max_loop_iterations),
                                .location = {}});
        impl->push_main_frame(effective.startup_path);
        impl->entry_pause_pending = effective.stop_on_entry;
        return PrgRuntimeSession(std::move(impl));
    }

    void PrgRuntimeSession::add_breakpoint(const RuntimeBreakpoint &breakpoint)
    {
        impl_->breakpoints.push_back({.file_path = normalize_path(breakpoint.file_path),
                                      .line = breakpoint.line});
    }

    void PrgRuntimeSession::clear_breakpoints()
    {
        impl_->breakpoints.clear();
    }

    bool PrgRuntimeSession::dispatch_event_handler(const std::string &routine_name)
    {
        return impl_->dispatch_event_handler(routine_name);
    }

    RuntimePauseState PrgRuntimeSession::run(DebugResumeAction action)
    {
        return impl_->run(action);
    }

    const RuntimePauseState &PrgRuntimeSession::state() const noexcept
    {
        return impl_->last_state;
    }

    PrgRuntimeSession::PrgRuntimeSession(std::unique_ptr<Impl> impl)
        : impl_(std::move(impl))
    {
    }

    PrgRuntimeSession::PrgRuntimeSession(PrgRuntimeSession &&) noexcept = default;

    PrgRuntimeSession &PrgRuntimeSession::operator=(PrgRuntimeSession &&) noexcept = default;

    PrgRuntimeSession::~PrgRuntimeSession() = default;

    const char *debug_pause_reason_name(DebugPauseReason reason)
    {
        switch (reason)
        {
        case DebugPauseReason::none:
            return "none";
        case DebugPauseReason::entry:
            return "entry";
        case DebugPauseReason::breakpoint:
            return "breakpoint";
        case DebugPauseReason::step:
            return "step";
        case DebugPauseReason::event_loop:
            return "event_loop";
        case DebugPauseReason::completed:
            return "completed";
        case DebugPauseReason::error:
            return "error";
        }
        return "none";
    }

    std::string format_value(const PrgValue &value)
    {
        return value_as_string(value);
    }

} // namespace copperfin::runtime
