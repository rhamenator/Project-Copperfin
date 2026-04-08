#include "copperfin/runtime/prg_engine.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/report_layout.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <set>
#include <sstream>
#include <utility>

namespace copperfin::runtime {

namespace {

enum class StatementKind {
    assignment,
    expression,
    do_command,
    do_form,
    report_form,
    label_form,
    activate_surface,
    release_surface,
    return_statement,
    if_statement,
    else_statement,
    endif_statement,
    for_statement,
    endfor_statement,
    read_events,
    clear_events,
    seek_command,
    go_command,
    skip_command,
    select_command,
    use_command,
    set_order,
    set_command,
    set_datasession,
    set_default,
    on_error,
    public_declaration,
    local_declaration,
    no_op
};

struct Statement {
    StatementKind kind = StatementKind::no_op;
    SourceLocation location{};
    std::string text;
    std::string identifier;
    std::string expression;
    std::string secondary_expression;
    std::string tertiary_expression;
    std::vector<std::string> names;
};

struct Routine {
    std::string name;
    std::vector<Statement> statements;
};

struct Program {
    std::string path;
    Routine main{};
    std::map<std::string, Routine> routines;
};

struct LoopState {
    std::size_t for_statement_index = 0;
    std::string variable_name;
    double end_value = 0.0;
    double step_value = 1.0;
};

struct Frame {
    std::string file_path;
    std::string routine_name;
    const Routine* routine = nullptr;
    std::size_t pc = 0;
    std::map<std::string, PrgValue> locals;
    std::set<std::string> local_names;
    std::vector<LoopState> loops;
};

struct ExecutionOutcome {
    bool ok = true;
    bool waiting_for_events = false;
    bool frame_returned = false;
    std::string message;
};

struct CursorState {
    struct OrderState {
        std::string name;
        std::string expression;
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
};

struct DataSessionState {
    int selected_work_area = 1;
    int next_work_area = 1;
    std::map<int, std::string> aliases;
    std::map<int, CursorState> cursors;
};

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool starts_with_insensitive(const std::string& value, const std::string& prefix) {
    if (value.size() < prefix.size()) {
        return false;
    }
    for (std::size_t index = 0; index < prefix.size(); ++index) {
        if (std::tolower(static_cast<unsigned char>(value[index])) !=
            std::tolower(static_cast<unsigned char>(prefix[index]))) {
            return false;
        }
    }
    return true;
}

std::string normalize_identifier(std::string value) {
    return lowercase_copy(trim_copy(std::move(value)));
}

std::string normalize_path(const std::string& value) {
    if (value.empty()) {
        return {};
    }
    return std::filesystem::path(value).lexically_normal().string();
}

std::string unquote_string(std::string value) {
    value = trim_copy(std::move(value));
    if (value.size() >= 2U && value.front() == '\'' && value.back() == '\'') {
        return value.substr(1U, value.size() - 2U);
    }
    return value;
}

std::string take_first_token(std::string value) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return value;
    }
    if (value.front() == '\'') {
        const auto closing = value.find('\'', 1U);
        if (closing != std::string::npos) {
            return value.substr(0U, closing + 1U);
        }
        return value;
    }

    const auto separator = value.find(' ');
    return separator == std::string::npos ? value : value.substr(0U, separator);
}

std::string uppercase_copy(std::string value);

std::string take_keyword_value(const std::string& text, const std::string& keyword) {
    const std::string upper = uppercase_copy(text);
    const std::string pattern = " " + uppercase_copy(keyword) + " ";
    const auto position = upper.find(pattern);
    if (position == std::string::npos) {
        return {};
    }
    return take_first_token(text.substr(position + pattern.size()));
}

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

std::string collapse_identifier(const std::string& value) {
    std::string normalized;
    normalized.reserve(value.size());

    for (char ch : value) {
        const auto raw = static_cast<unsigned char>(ch);
        if (std::isalnum(raw) == 0) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(raw)));
    }

    return normalized;
}

std::string unquote_identifier(std::string value) {
    value = trim_copy(std::move(value));
    if (value.size() >= 2U) {
        if ((value.front() == '\'' && value.back() == '\'') ||
            (value.front() == '"' && value.back() == '"')) {
            return value.substr(1U, value.size() - 2U);
        }
    }
    return value;
}

std::string normalize_index_value(std::string value) {
    value = trim_copy(std::move(value));
    if (value == ".T.") {
        return "true";
    }
    if (value == ".F.") {
        return "false";
    }
    return value;
}

std::optional<std::string> record_field_value(const vfp::DbfRecord& record, const std::string& field_name) {
    const std::string normalized_field = collapse_identifier(field_name);
    for (const auto& value : record.values) {
        if (collapse_identifier(value.field_name) == normalized_field) {
            return value.display_value;
        }
    }
    return std::nullopt;
}

std::string evaluate_index_expression(const std::string& expression, const vfp::DbfRecord& record) {
    const std::string trimmed = trim_copy(expression);
    const std::string upper = uppercase_copy(trimmed);

    const auto apply_unary = [&](const std::string& prefix, auto&& transform) -> std::optional<std::string> {
        if (!starts_with_insensitive(trimmed, prefix + "(") || trimmed.back() != ')') {
            return std::nullopt;
        }

        const std::string inner = trim_copy(trimmed.substr(prefix.size() + 1U, trimmed.size() - prefix.size() - 2U));
        std::string value = evaluate_index_expression(inner, record);
        transform(value);
        return value;
    };

    if (const auto upper_value = apply_unary("UPPER", [](std::string& value) {
            value = uppercase_copy(value);
        })) {
        return *upper_value;
    }
    if (const auto lower_value = apply_unary("LOWER", [](std::string& value) {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        })) {
        return *lower_value;
    }
    if (const auto trim_value = apply_unary("ALLTRIM", [](std::string& value) {
            value = trim_copy(std::move(value));
        })) {
        return *trim_value;
    }
    if (const auto ltrim_value = apply_unary("LTRIM", [](std::string& value) {
            const auto first = std::find_if(value.begin(), value.end(), [](unsigned char ch) {
                return std::isspace(ch) == 0;
            });
            value.erase(value.begin(), first);
        })) {
        return *ltrim_value;
    }
    if (const auto rtrim_value = apply_unary("RTRIM", [](std::string& value) {
            while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
                value.pop_back();
            }
        })) {
        return *rtrim_value;
    }

    if (trimmed.size() >= 2U &&
        ((trimmed.front() == '\'' && trimmed.back() == '\'') ||
         (trimmed.front() == '"' && trimmed.back() == '"'))) {
        return unquote_identifier(trimmed);
    }

    if (const auto field_value = record_field_value(record, trimmed)) {
        return normalize_index_value(*field_value);
    }

    return normalize_index_value(trimmed);
}

bool has_keyword(const std::string& text, const std::string& keyword) {
    const std::string upper = " " + uppercase_copy(text) + " ";
    const std::string pattern = " " + uppercase_copy(keyword) + " ";
    return upper.find(pattern) != std::string::npos;
}

bool parse_object_handle_reference(const PrgValue& value, int& handle, std::string& prog_id) {
    if (value.kind != PrgValueKind::string) {
        return false;
    }

    const std::string prefix = "object:";
    if (value.string_value.rfind(prefix, 0) != 0) {
        return false;
    }

    const auto separator = value.string_value.rfind('#');
    if (separator == std::string::npos || separator <= prefix.size()) {
        return false;
    }

    prog_id = value.string_value.substr(prefix.size(), separator - prefix.size());
    try {
        handle = std::stoi(value.string_value.substr(separator + 1U));
    } catch (...) {
        return false;
    }
    return true;
}

std::string strip_inline_comment(const std::string& line) {
    bool in_string = false;
    for (std::size_t index = 0; index < line.size(); ++index) {
        const char ch = line[index];
        if (ch == '\'') {
            in_string = !in_string;
            continue;
        }
        if (!in_string && ch == '&' && (index + 1U) < line.size() && line[index + 1U] == '&') {
            return line.substr(0U, index);
        }
    }
    return line;
}

std::vector<std::pair<std::size_t, std::string>> load_logical_lines(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    std::vector<std::pair<std::size_t, std::string>> lines;
    if (!input) {
        return lines;
    }

    std::string raw_line;
    std::size_t line_number = 0;
    std::size_t current_start = 0;
    std::string current_text;
    bool continuing = false;

    while (std::getline(input, raw_line)) {
        ++line_number;
        if (!raw_line.empty() && raw_line.back() == '\r') {
            raw_line.pop_back();
        }

        std::string line = strip_inline_comment(raw_line);
        if (!continuing) {
            current_start = line_number;
            current_text.clear();
        }

        const std::string trimmed = trim_copy(line);
        if (!current_text.empty() && !trimmed.empty()) {
            current_text += " ";
        }
        current_text += trimmed;

        if (!trimmed.empty() && trimmed.back() == ';') {
            current_text.pop_back();
            continuing = true;
            continue;
        }

        continuing = false;
        lines.emplace_back(current_start, trim_copy(current_text));
    }

    if (continuing && !current_text.empty()) {
        lines.emplace_back(current_start, trim_copy(current_text));
    }

    return lines;
}

Statement make_statement(StatementKind kind, const std::string& path, std::size_t line, const std::string& text) {
    Statement statement;
    statement.kind = kind;
    statement.location = {.file_path = normalize_path(path), .line = line};
    statement.text = text;
    return statement;
}

std::vector<std::string> split_csv_like(const std::string& text) {
    std::vector<std::string> parts;
    std::string current;
    int nesting = 0;
    bool in_string = false;

    for (char ch : text) {
        if (ch == '\'') {
            in_string = !in_string;
            current.push_back(ch);
            continue;
        }
        if (!in_string) {
            if (ch == '(') {
                ++nesting;
            } else if (ch == ')' && nesting > 0) {
                --nesting;
            } else if (ch == ',' && nesting == 0) {
                parts.push_back(trim_copy(current));
                current.clear();
                continue;
            }
        }
        current.push_back(ch);
    }

    if (!current.empty()) {
        parts.push_back(trim_copy(current));
    }
    return parts;
}

Program parse_program(const std::string& path) {
    Program program;
    program.path = normalize_path(path);
    program.main.name = "main";

    Routine* current = &program.main;
    for (const auto& [line_number, raw_line] : load_logical_lines(path)) {
        const std::string line = trim_copy(raw_line);
        if (line.empty()) {
            continue;
        }
        if (line[0] == '*' || starts_with_insensitive(line, "* ") || starts_with_insensitive(line, "#")) {
            continue;
        }

        const std::string upper = uppercase_copy(line);
        if (starts_with_insensitive(line, "PROCEDURE ") || starts_with_insensitive(line, "FUNCTION ")) {
            const auto separator = line.find(' ');
            Routine routine;
            routine.name = trim_copy(line.substr(separator + 1U));
            current = &program.routines[normalize_identifier(routine.name)];
            *current = std::move(routine);
            continue;
        }
        if (starts_with_insensitive(line, "ENDPROC") || starts_with_insensitive(line, "ENDFUNC") || starts_with_insensitive(line, "END FUNC")) {
            current = &program.main;
            continue;
        }

        Statement statement = make_statement(StatementKind::no_op, path, line_number, line);
        if (starts_with_insensitive(line, "IF ")) {
            statement.kind = StatementKind::if_statement;
            statement.expression = trim_copy(line.substr(3U));
        } else if (upper == "ELSE") {
            statement.kind = StatementKind::else_statement;
        } else if (upper == "ENDIF") {
            statement.kind = StatementKind::endif_statement;
        } else if (starts_with_insensitive(line, "FOR ")) {
            statement.kind = StatementKind::for_statement;
            const std::string body = trim_copy(line.substr(4U));
            const auto equals = body.find('=');
            const auto to_position = uppercase_copy(body).find(" TO ");
            if (equals != std::string::npos && to_position != std::string::npos && to_position > equals) {
                statement.identifier = trim_copy(body.substr(0U, equals));
                statement.expression = trim_copy(body.substr(equals + 1U, to_position - equals - 1U));
                const auto step_position = uppercase_copy(body).find(" STEP ", to_position + 4U);
                if (step_position == std::string::npos) {
                    statement.secondary_expression = trim_copy(body.substr(to_position + 4U));
                } else {
                    statement.secondary_expression = trim_copy(body.substr(to_position + 4U, step_position - to_position - 4U));
                    statement.tertiary_expression = trim_copy(body.substr(step_position + 6U));
                }
            }
        } else if (upper == "ENDFOR") {
            statement.kind = StatementKind::endfor_statement;
        } else if (starts_with_insensitive(line, "DO FORM ")) {
            statement.kind = StatementKind::do_form;
            statement.identifier = trim_copy(line.substr(8U));
        } else if (starts_with_insensitive(line, "REPORT FORM ")) {
            statement.kind = StatementKind::report_form;
            statement.identifier = trim_copy(line.substr(12U));
        } else if (starts_with_insensitive(line, "LABEL FORM ")) {
            statement.kind = StatementKind::label_form;
            statement.identifier = trim_copy(line.substr(11U));
        } else if (starts_with_insensitive(line, "ACTIVATE POPUP ")) {
            statement.kind = StatementKind::activate_surface;
            statement.identifier = "popup";
            statement.expression = trim_copy(line.substr(15U));
        } else if (starts_with_insensitive(line, "ACTIVATE MENU ")) {
            statement.kind = StatementKind::activate_surface;
            statement.identifier = "menu";
            statement.expression = trim_copy(line.substr(14U));
        } else if (starts_with_insensitive(line, "RELEASE POPUP ")) {
            statement.kind = StatementKind::release_surface;
            statement.identifier = "popup";
            statement.expression = trim_copy(line.substr(14U));
        } else if (starts_with_insensitive(line, "RELEASE MENU ")) {
            statement.kind = StatementKind::release_surface;
            statement.identifier = "menu";
            statement.expression = trim_copy(line.substr(13U));
        } else if (starts_with_insensitive(line, "DO ")) {
            statement.kind = StatementKind::do_command;
            statement.identifier = trim_copy(line.substr(3U));
        } else if (upper == "READ EVENTS") {
            statement.kind = StatementKind::read_events;
        } else if (upper == "CLEAR EVENTS") {
            statement.kind = StatementKind::clear_events;
        } else if (starts_with_insensitive(line, "SEEK ")) {
            statement.kind = StatementKind::seek_command;
            const std::string body = trim_copy(line.substr(5U));
            const std::string upper_body = uppercase_copy(body);
            const auto in_position = upper_body.find(" IN ");
            if (in_position == std::string::npos) {
                statement.expression = body;
            } else {
                statement.expression = trim_copy(body.substr(0U, in_position));
                statement.secondary_expression = trim_copy(body.substr(in_position + 4U));
            }
        } else if (starts_with_insensitive(line, "GOTO ") || starts_with_insensitive(line, "GO ")) {
            statement.kind = StatementKind::go_command;
            const std::string body = starts_with_insensitive(line, "GOTO ")
                ? trim_copy(line.substr(5U))
                : trim_copy(line.substr(3U));
            statement.expression = take_first_token(body);
            statement.secondary_expression = take_keyword_value(body, "IN");
        } else if (upper == "SKIP" || starts_with_insensitive(line, "SKIP ")) {
            statement.kind = StatementKind::skip_command;
            const std::string body = upper == "SKIP" ? std::string{} : trim_copy(line.substr(5U));
            if (starts_with_insensitive(body, "IN ")) {
                statement.expression = "1";
                statement.secondary_expression = trim_copy(body.substr(3U));
            } else {
                statement.expression = body.empty() ? "1" : take_first_token(body);
                statement.secondary_expression = take_keyword_value(body, "IN");
            }
        } else if (starts_with_insensitive(line, "SELECT ")) {
            statement.kind = StatementKind::select_command;
            statement.expression = trim_copy(line.substr(7U));
        } else if (upper == "USE" || starts_with_insensitive(line, "USE ")) {
            statement.kind = StatementKind::use_command;
            const std::string body = upper == "USE" ? std::string{} : trim_copy(line.substr(4U));
            if (starts_with_insensitive(body, "IN ")) {
                statement.secondary_expression = trim_copy(body.substr(3U));
            } else if (!body.empty()) {
                statement.expression = take_first_token(body);
                statement.identifier = take_keyword_value(body, "ALIAS");
                statement.secondary_expression = take_keyword_value(body, "IN");
                statement.tertiary_expression = has_keyword(body, "AGAIN") ? "again" : std::string{};
            }
        } else if (starts_with_insensitive(line, "SET DATASESSION TO ")) {
            statement.kind = StatementKind::set_datasession;
            statement.expression = trim_copy(line.substr(19U));
        } else if (starts_with_insensitive(line, "SET ORDER TO ")) {
            statement.kind = StatementKind::set_order;
            const std::string body = trim_copy(line.substr(13U));
            const std::string upper_body = uppercase_copy(body);
            const auto in_position = upper_body.find(" IN ");
            if (in_position == std::string::npos) {
                statement.expression = body;
            } else {
                statement.expression = trim_copy(body.substr(0U, in_position));
                statement.secondary_expression = trim_copy(body.substr(in_position + 4U));
            }
        } else if (starts_with_insensitive(line, "SET DEFAULT TO ")) {
            statement.kind = StatementKind::set_default;
            statement.expression = trim_copy(line.substr(15U));
        } else if (starts_with_insensitive(line, "SET ")) {
            statement.kind = StatementKind::set_command;
            statement.expression = trim_copy(line.substr(4U));
        } else if (starts_with_insensitive(line, "ON ERROR ")) {
            statement.kind = StatementKind::on_error;
            statement.expression = trim_copy(line.substr(9U));
        } else if (starts_with_insensitive(line, "PUBLIC ")) {
            statement.kind = StatementKind::public_declaration;
            statement.names = split_csv_like(line.substr(7U));
        } else if (starts_with_insensitive(line, "LOCAL ")) {
            statement.kind = StatementKind::local_declaration;
            statement.names = split_csv_like(line.substr(6U));
        } else if (upper == "RETURN" || starts_with_insensitive(line, "RETURN ")) {
            statement.kind = StatementKind::return_statement;
        } else {
            const auto equals = line.find('=');
            if (!line.empty() && line[0] == '=') {
                statement.kind = StatementKind::expression;
                statement.expression = trim_copy(line.substr(1U));
            } else if (equals != std::string::npos) {
                statement.kind = StatementKind::assignment;
                statement.identifier = trim_copy(line.substr(0U, equals));
                statement.expression = trim_copy(line.substr(equals + 1U));
            } else {
                statement.kind = StatementKind::expression;
                statement.expression = line;
            }
        }

        current->statements.push_back(std::move(statement));
    }

    return program;
}

PrgValue make_empty_value() {
    return {};
}

PrgValue make_boolean_value(bool value) {
    PrgValue result;
    result.kind = PrgValueKind::boolean;
    result.boolean_value = value;
    return result;
}

PrgValue make_number_value(double value) {
    PrgValue result;
    result.kind = PrgValueKind::number;
    result.number_value = value;
    return result;
}

PrgValue make_string_value(std::string value) {
    PrgValue result;
    result.kind = PrgValueKind::string;
    result.string_value = std::move(value);
    return result;
}

bool value_as_bool(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::boolean:
            return value.boolean_value;
        case PrgValueKind::number:
            return std::abs(value.number_value) > 0.000001;
        case PrgValueKind::string:
            return !value.string_value.empty();
        case PrgValueKind::empty:
            return false;
    }
    return false;
}

double value_as_number(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::boolean:
            return value.boolean_value ? 1.0 : 0.0;
        case PrgValueKind::number:
            return value.number_value;
        case PrgValueKind::string:
            return value.string_value.empty() ? 0.0 : std::stod(value.string_value);
        case PrgValueKind::empty:
            return 0.0;
    }
    return 0.0;
}

std::string value_as_string(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::boolean:
            return value.boolean_value ? "true" : "false";
        case PrgValueKind::number: {
            std::ostringstream stream;
            if (std::abs(value.number_value - std::round(value.number_value)) < 0.000001) {
                stream << std::llround(value.number_value);
            } else {
                stream << value.number_value;
            }
            return stream.str();
        }
        case PrgValueKind::string:
            return value.string_value;
        case PrgValueKind::empty:
            return {};
    }
    return {};
}

}  // namespace

struct PrgRuntimeSession::Impl {
    explicit Impl(RuntimeSessionOptions session_options)
        : options(std::move(session_options)) {
    }

    RuntimeSessionOptions options;
    std::map<std::string, Program> programs;
    std::vector<Frame> stack;
    std::map<std::string, PrgValue> globals;
    std::vector<RuntimeBreakpoint> breakpoints;
    std::vector<RuntimeEvent> events;
    RuntimePauseState last_state{};
    std::string default_directory;
    std::string last_error_message;
    SourceLocation last_fault_location{};
    std::string last_fault_statement;
    std::string error_handler;
    std::map<std::string, std::string> set_state;
    int current_data_session = 1;
    int next_sql_handle = 1;
    int next_ole_handle = 1;
    std::map<int, DataSessionState> data_sessions;
    std::map<int, RuntimeSqlConnectionState> sql_connections;
    std::map<int, RuntimeOleObjectState> ole_objects;
    bool entry_pause_pending = false;
    bool waiting_for_events = false;
    std::optional<std::size_t> event_dispatch_return_depth;
    bool restore_event_loop_after_dispatch = false;
    std::size_t executed_statement_count = 0;

    Program& load_program(const std::string& path) {
        const std::string normalized = normalize_path(path);
        const auto existing = programs.find(normalized);
        if (existing != programs.end()) {
            return existing->second;
        }
        auto [inserted, _] = programs.emplace(normalized, parse_program(normalized));
        return inserted->second;
    }

    void push_main_frame(const std::string& path) {
        Program& program = load_program(path);
        stack.push_back({
            .file_path = program.path,
            .routine_name = "main",
            .routine = &program.main,
            .pc = 0
        });
    }

    void push_routine_frame(const std::string& path, const Routine& routine) {
        stack.push_back({
            .file_path = normalize_path(path),
            .routine_name = routine.name,
            .routine = &routine,
            .pc = 0
        });
    }

    const Statement* current_statement() const {
        if (stack.empty()) {
            return nullptr;
        }
        const Frame& frame = stack.back();
        if (frame.routine == nullptr || frame.pc >= frame.routine->statements.size()) {
            return nullptr;
        }
        return &frame.routine->statements[frame.pc];
    }

    DataSessionState& current_session_state() {
        auto [iterator, _] = data_sessions.try_emplace(current_data_session);
        iterator->second.selected_work_area = std::max(1, iterator->second.selected_work_area);
        iterator->second.next_work_area = std::max(1, iterator->second.next_work_area);
        return iterator->second;
    }

    const DataSessionState& current_session_state() const {
        const auto found = data_sessions.find(current_data_session);
        if (found != data_sessions.end()) {
            return found->second;
        }
        static const DataSessionState empty_session{};
        return empty_session;
    }

    int current_selected_work_area() const {
        return current_session_state().selected_work_area;
    }

    RuntimePauseState build_pause_state(DebugPauseReason reason, std::string message = {}) {
        RuntimePauseState state;
        state.paused = reason != DebugPauseReason::completed;
        state.completed = reason == DebugPauseReason::completed;
        state.waiting_for_events = waiting_for_events;
        state.reason = reason;
        state.message = std::move(message);
        state.executed_statement_count = executed_statement_count;
        state.globals = globals;
        state.events = events;
        const DataSessionState& session = current_session_state();
        state.work_area.selected = session.selected_work_area;
        state.work_area.data_session = current_data_session;
        state.work_area.aliases = session.aliases;
        for (const auto& [_, cursor] : session.cursors) {
            state.cursors.push_back({
                .work_area = cursor.work_area,
                .alias = cursor.alias,
                .source_path = cursor.source_path,
                .source_kind = cursor.source_kind,
                .remote = cursor.remote,
                .record_count = cursor.record_count,
                .recno = cursor.recno,
                .bof = cursor.bof,
                .eof = cursor.eof
            });
        }
        for (const auto& [_, connection] : sql_connections) {
            state.sql_connections.push_back(connection);
        }
        for (const auto& [_, object] : ole_objects) {
            state.ole_objects.push_back(object);
        }

        if (reason == DebugPauseReason::error) {
            const auto error_event = std::find_if(events.rbegin(), events.rend(), [](const RuntimeEvent& event) {
                return event.category == "runtime.error";
            });
            if (error_event != events.rend()) {
                state.location = error_event->location;
            } else if (!last_fault_location.file_path.empty()) {
                state.location = last_fault_location;
            }
            if (!last_fault_statement.empty()) {
                state.statement_text = last_fault_statement;
            }
        } else if (const Statement* statement = current_statement()) {
            state.location = statement->location;
            state.statement_text = statement->text;
        }

        for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator) {
            RuntimeStackFrame frame;
            frame.file_path = iterator->file_path;
            frame.routine_name = iterator->routine_name;
            if (iterator->routine != nullptr && iterator->pc < iterator->routine->statements.size()) {
                frame.line = iterator->routine->statements[iterator->pc].location.line;
            }
            frame.locals = iterator->locals;
            state.call_stack.push_back(std::move(frame));
        }

        last_state = state;
        return state;
    }

    int allocate_work_area() {
        DataSessionState& session = current_session_state();
        const int allocated = std::max(1, session.next_work_area);
        session.next_work_area = allocated + 1;
        return allocated;
    }

    int select_work_area(int requested_area) {
        DataSessionState& session = current_session_state();
        if (requested_area <= 0) {
            requested_area = allocate_work_area();
        } else if (requested_area >= session.next_work_area) {
            session.next_work_area = requested_area + 1;
        }
        session.selected_work_area = requested_area;
        return session.selected_work_area;
    }

    CursorState* find_cursor_by_area(int area) {
        auto& session = current_session_state();
        const auto found = session.cursors.find(area);
        return found == session.cursors.end() ? nullptr : &found->second;
    }

    const CursorState* find_cursor_by_area(int area) const {
        const auto& session = current_session_state();
        const auto found = session.cursors.find(area);
        return found == session.cursors.end() ? nullptr : &found->second;
    }

    CursorState* find_cursor_by_alias(const std::string& alias) {
        const std::string normalized = normalize_identifier(alias);
        if (normalized.empty()) {
            return find_cursor_by_area(current_selected_work_area());
        }

        auto& session = current_session_state();
        const auto found = std::find_if(session.aliases.begin(), session.aliases.end(), [&](const auto& pair) {
            return normalize_identifier(pair.second) == normalized;
        });
        if (found == session.aliases.end()) {
            return nullptr;
        }
        return find_cursor_by_area(found->first);
    }

    const CursorState* find_cursor_by_alias(const std::string& alias) const {
        const std::string normalized = normalize_identifier(alias);
        if (normalized.empty()) {
            return find_cursor_by_area(current_selected_work_area());
        }

        const auto& session = current_session_state();
        const auto found = std::find_if(session.aliases.begin(), session.aliases.end(), [&](const auto& pair) {
            return normalize_identifier(pair.second) == normalized;
        });
        if (found == session.aliases.end()) {
            return nullptr;
        }
        return find_cursor_by_area(found->first);
    }

    CursorState* resolve_cursor_target(const std::string& designator) {
        const std::string trimmed = trim_copy(designator);
        if (trimmed.empty()) {
            return find_cursor_by_area(current_selected_work_area());
        }

        const bool numeric_selection = std::all_of(trimmed.begin(), trimmed.end(), [](unsigned char ch) {
            return std::isdigit(ch) != 0;
        });
        if (numeric_selection) {
            return find_cursor_by_area(std::stoi(trimmed));
        }
        return find_cursor_by_alias(trimmed);
    }

    const CursorState* resolve_cursor_target(const std::string& designator) const {
        const std::string trimmed = trim_copy(designator);
        if (trimmed.empty()) {
            return find_cursor_by_area(current_selected_work_area());
        }

        const bool numeric_selection = std::all_of(trimmed.begin(), trimmed.end(), [](unsigned char ch) {
            return std::isdigit(ch) != 0;
        });
        if (numeric_selection) {
            return find_cursor_by_area(std::stoi(trimmed));
        }
        return find_cursor_by_alias(trimmed);
    }

    void close_cursor(const std::string& designator) {
        DataSessionState& session = current_session_state();
        CursorState* cursor = resolve_cursor_target(designator);
        if (cursor == nullptr) {
            return;
        }
        if (session.selected_work_area == cursor->work_area) {
            session.selected_work_area = cursor->work_area;
        }
        session.aliases.erase(cursor->work_area);
        session.cursors.erase(cursor->work_area);
    }

    std::vector<CursorState::OrderState> load_cursor_orders(const std::string& table_path) const {
        std::vector<CursorState::OrderState> orders;
        const auto inspection = vfp::inspect_asset(table_path);
        if (!inspection.ok) {
            return orders;
        }

        for (const auto& index_asset : inspection.indexes) {
            if (!index_asset.probe.tags.empty()) {
                for (const auto& tag : index_asset.probe.tags) {
                    if (tag.key_expression_hint.empty()) {
                        continue;
                    }
                    orders.push_back({
                        .name = tag.name_hint.empty() ? collapse_identifier(tag.key_expression_hint) : tag.name_hint,
                        .expression = tag.key_expression_hint
                    });
                }
                continue;
            }

            if (!index_asset.probe.key_expression_hint.empty()) {
                const std::string fallback_name = std::filesystem::path(index_asset.path).stem().string();
                orders.push_back({
                    .name = fallback_name.empty() ? collapse_identifier(index_asset.probe.key_expression_hint) : fallback_name,
                    .expression = index_asset.probe.key_expression_hint
                });
            }
        }

        return orders;
    }

    bool can_open_table_cursor(
        const std::string& resolved_path,
        const std::string& alias,
        bool remote,
        bool allow_again,
        int target_area) {
        DataSessionState& session = current_session_state();
        const std::string normalized_alias = normalize_identifier(alias);
        const std::string normalized_path = normalize_path(resolved_path);

        for (const auto& [work_area, cursor] : session.cursors) {
            if (work_area == target_area) {
                continue;
            }

            if (!normalized_alias.empty() && normalize_identifier(cursor.alias) == normalized_alias) {
                last_error_message = "Alias already open in this data session: " + alias;
                return false;
            }

            if (!remote && !allow_again && !normalized_path.empty() &&
                normalize_path(cursor.source_path) == normalized_path) {
                last_error_message = "Table already open in this data session; USE AGAIN is required: " + resolved_path;
                return false;
            }
        }

        return true;
    }

    void move_cursor_to(CursorState& cursor, long long target_recno) {
        if (cursor.record_count == 0U) {
            cursor.recno = 0U;
            cursor.bof = true;
            cursor.eof = true;
            return;
        }

        if (target_recno <= 0) {
            cursor.recno = 0U;
            cursor.bof = true;
            cursor.eof = false;
            return;
        }

        const auto record_count = static_cast<long long>(cursor.record_count);
        if (target_recno > record_count) {
            cursor.recno = static_cast<std::size_t>(record_count + 1);
            cursor.bof = false;
            cursor.eof = true;
            return;
        }

        cursor.recno = static_cast<std::size_t>(target_recno);
        cursor.bof = false;
        cursor.eof = false;
    }

    bool activate_order(CursorState& cursor, const std::string& order_designator) {
        const std::string trimmed = trim_copy(order_designator);
        if (trimmed.empty() || trimmed == "0") {
            cursor.active_order_name.clear();
            cursor.active_order_expression.clear();
            return true;
        }

        std::string target_name = trimmed;
        if (starts_with_insensitive(target_name, "TAG ")) {
            target_name = trim_copy(target_name.substr(4U));
        }
        target_name = unquote_identifier(target_name);

        const bool numeric_selection = !target_name.empty() &&
            std::all_of(target_name.begin(), target_name.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; });
        if (numeric_selection) {
            const std::size_t index = static_cast<std::size_t>(std::max(1, std::stoi(target_name))) - 1U;
            if (index >= cursor.orders.size()) {
                last_error_message = "Requested order does not exist";
                return false;
            }

            cursor.active_order_name = cursor.orders[index].name;
            cursor.active_order_expression = cursor.orders[index].expression;
            return true;
        }

        const std::string normalized_target = collapse_identifier(target_name);
        const auto found = std::find_if(cursor.orders.begin(), cursor.orders.end(), [&](const CursorState::OrderState& order) {
            return collapse_identifier(order.name) == normalized_target;
        });
        if (found == cursor.orders.end()) {
            last_error_message = "Requested order/tag was not found";
            return false;
        }

        cursor.active_order_name = found->name;
        cursor.active_order_expression = found->expression;
        return true;
    }

    bool seek_in_cursor(CursorState& cursor, const std::string& search_key) {
        cursor.found = false;
        if (cursor.remote) {
            last_error_message = "SEEK is not yet supported on remote SQL cursors";
            move_cursor_to(cursor, static_cast<long long>(cursor.record_count + 1U));
            return false;
        }

        if (cursor.source_path.empty()) {
            last_error_message = "SEEK requires a local table-backed cursor";
            return false;
        }

        if (cursor.active_order_expression.empty()) {
            if (!cursor.orders.empty()) {
                cursor.active_order_name = cursor.orders.front().name;
                cursor.active_order_expression = cursor.orders.front().expression;
            } else {
                last_error_message = "SEEK requires an active order";
                return false;
            }
        }

        const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, cursor.record_count);
        if (!table_result.ok) {
            last_error_message = table_result.error;
            return false;
        }

        const std::string normalized_target = evaluate_index_expression(search_key, vfp::DbfRecord{});
        for (const auto& record : table_result.table.records) {
            const std::string candidate = evaluate_index_expression(cursor.active_order_expression, record);
            if (candidate == normalized_target) {
                move_cursor_to(cursor, static_cast<long long>(record.record_index + 1U));
                cursor.found = true;
                return true;
            }
        }

        move_cursor_to(cursor, static_cast<long long>(cursor.record_count + 1U));
        return false;
    }

    bool open_table_cursor(
        const std::string& raw_path,
        const std::string& requested_alias,
        const std::string& in_expression,
        bool allow_again,
        bool remote,
        int sql_handle,
        const std::string& sql_command,
        std::size_t synthetic_record_count) {
        (void)sql_command;
        std::string alias = requested_alias;
        std::string resolved_path = raw_path;
        std::string dbf_identity;
        std::size_t field_count = 0;
        std::size_t record_count = synthetic_record_count;

        if (!remote) {
            std::filesystem::path table_path(unquote_string(trim_copy(raw_path)));
            if (table_path.extension().empty()) {
                table_path += ".dbf";
            }
            if (table_path.is_relative()) {
                table_path = std::filesystem::path(default_directory) / table_path;
            }
            table_path = table_path.lexically_normal();
            if (!std::filesystem::exists(table_path)) {
                last_error_message = "Unable to resolve USE target: " + table_path.string();
                return false;
            }

            const auto table_result = vfp::parse_dbf_table_from_file(table_path.string(), 1U);
            if (!table_result.ok) {
                last_error_message = table_result.error;
                return false;
            }

            resolved_path = table_path.string();
            dbf_identity = resolved_path;
            field_count = table_result.table.fields.size();
            record_count = table_result.table.header.record_count;
            std::vector<CursorState::OrderState> orders = load_cursor_orders(resolved_path);
            if (alias.empty()) {
                alias = table_path.stem().string();
            }

            int target_area = 0;
            if (!trim_copy(in_expression).empty()) {
                const PrgValue area_value = evaluate_expression(in_expression, stack.back());
                const std::string area_text = trim_copy(value_as_string(area_value));
                if (std::all_of(area_text.begin(), area_text.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
                    target_area = std::stoi(area_text);
                } else if (!area_text.empty()) {
                    CursorState* existing = find_cursor_by_alias(area_text);
                    target_area = existing == nullptr ? 0 : existing->work_area;
                }
            }
            target_area = select_work_area(target_area);

            if (!can_open_table_cursor(resolved_path, alias, false, allow_again, target_area)) {
                return false;
            }

            DataSessionState& session = current_session_state();
            session.aliases[target_area] = alias;
            session.cursors[target_area] = CursorState{
                .work_area = target_area,
                .alias = alias,
                .source_path = resolved_path,
                .dbf_identity = dbf_identity,
                .source_kind = "table",
                .remote = false,
                .field_count = field_count,
                .record_count = record_count,
                .recno = record_count == 0U ? 0U : 1U,
                .found = false,
                .bof = record_count == 0U,
                .eof = record_count == 0U,
                .orders = std::move(orders)
            };
            return true;
        } else if (alias.empty()) {
            alias = "sqlresult" + std::to_string(current_session_state().next_work_area);
        }
        if (remote) {
            dbf_identity = alias;
            field_count = synthetic_record_count == 0U ? 0U : 3U;
        }

        int target_area = 0;
        if (!trim_copy(in_expression).empty()) {
            const PrgValue area_value = evaluate_expression(in_expression, stack.back());
            const std::string area_text = trim_copy(value_as_string(area_value));
            if (std::all_of(area_text.begin(), area_text.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
                target_area = std::stoi(area_text);
            } else if (!area_text.empty()) {
                CursorState* existing = find_cursor_by_alias(area_text);
                target_area = existing == nullptr ? 0 : existing->work_area;
            }
        }
        target_area = select_work_area(target_area);

        if (!can_open_table_cursor(resolved_path, alias, remote, allow_again, target_area)) {
            return false;
        }

        DataSessionState& session = current_session_state();
        session.aliases[target_area] = alias;
        session.cursors[target_area] = CursorState{
            .work_area = target_area,
            .alias = alias,
            .source_path = resolved_path,
            .dbf_identity = dbf_identity,
            .source_kind = remote ? "sql-cursor" : "table",
            .remote = remote,
            .field_count = field_count,
            .record_count = record_count,
            .recno = record_count == 0U ? 0U : 1U,
            .found = false,
            .bof = record_count == 0U,
            .eof = record_count == 0U
        };
        if (remote && sql_handle > 0) {
            auto found = sql_connections.find(sql_handle);
            if (found != sql_connections.end()) {
                found->second.last_cursor_alias = alias;
                found->second.last_result_count = record_count;
            }
        }
        return true;
    }

    int sql_connect(const std::string& target, const std::string& provider) {
        const int handle = next_sql_handle++;
        sql_connections.emplace(handle, RuntimeSqlConnectionState{
            .handle = handle,
            .target = target,
            .provider = provider,
            .last_cursor_alias = {},
            .last_result_count = 0U
        });
        return handle;
    }

    bool sql_disconnect(int handle) {
        return sql_connections.erase(handle) > 0;
    }

    int sql_exec(int handle, const std::string& command, const std::string& cursor_alias) {
        const auto found = sql_connections.find(handle);
        if (found == sql_connections.end()) {
            last_error_message = "SQL handle not found: " + std::to_string(handle);
            return -1;
        }

        std::size_t result_count = 0;
        const std::string normalized_command = lowercase_copy(trim_copy(command));
        if (normalized_command.rfind("select", 0) == 0) {
            result_count = 3U;
            const std::string alias = trim_copy(cursor_alias).empty()
                ? "sqlresult" + std::to_string(handle)
                : trim_copy(cursor_alias);
            if (!open_table_cursor({}, alias, "0", true, true, handle, command, result_count)) {
                return -1;
            }
        }
        events.push_back({
            .category = "sql.exec",
            .detail = "handle " + std::to_string(handle) + ": " + command,
            .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location
        });
        if (result_count > 0U) {
            events.push_back({
                .category = "sql.cursor",
                .detail = found->second.last_cursor_alias + " (" + std::to_string(result_count) + " rows)",
                .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location
            });
        }
        return 1;
    }

    int register_ole_object(const std::string& prog_id, const std::string& source) {
        const int handle = next_ole_handle++;
        ole_objects.emplace(handle, RuntimeOleObjectState{
            .handle = handle,
            .prog_id = prog_id,
            .source = source,
            .last_action = source,
            .action_count = 1
        });
        return handle;
    }

    std::optional<RuntimeOleObjectState*> resolve_ole_object(const PrgValue& value) {
        int handle = 0;
        std::string prog_id;
        if (!parse_object_handle_reference(value, handle, prog_id)) {
            return std::nullopt;
        }

        auto found = ole_objects.find(handle);
        if (found == ole_objects.end()) {
            return std::nullopt;
        }
        return &found->second;
    }

    PrgValue evaluate_expression(const std::string& expression, const Frame& frame);
    std::optional<std::string> materialize_xasset_bootstrap(const std::string& asset_path, bool include_read_events);

    void assign_variable(Frame& frame, const std::string& name, const PrgValue& value) {
        const std::string normalized = normalize_identifier(name);
        if (frame.local_names.contains(normalized) || frame.locals.contains(normalized)) {
            frame.locals[normalized] = value;
            return;
        }
        globals[normalized] = value;
    }

    PrgValue lookup_variable(const Frame& frame, const std::string& name) const {
        const std::string normalized = normalize_identifier(name);
        if (const auto local = frame.locals.find(normalized); local != frame.locals.end()) {
            return local->second;
        }
        if (const auto global = globals.find(normalized); global != globals.end()) {
            return global->second;
        }
        return {};
    }

    bool breakpoint_matches(const SourceLocation& location) const {
        const std::string normalized = normalize_path(location.file_path);
        return std::any_of(breakpoints.begin(), breakpoints.end(), [&](const RuntimeBreakpoint& breakpoint) {
            return normalize_path(breakpoint.file_path) == normalized && breakpoint.line == location.line;
        });
    }

    std::optional<std::size_t> find_matching_branch(const Frame& frame, std::size_t pc, bool seek_else) const {
        if (frame.routine == nullptr) {
            return std::nullopt;
        }
        int depth = 0;
        for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index) {
            const auto kind = frame.routine->statements[index].kind;
            if (kind == StatementKind::if_statement) {
                ++depth;
            } else if (kind == StatementKind::endif_statement) {
                if (depth == 0) {
                    return index;
                }
                --depth;
            } else if (seek_else && kind == StatementKind::else_statement && depth == 0) {
                return index;
            }
        }
        return std::nullopt;
    }

    std::optional<std::size_t> find_matching_endfor(const Frame& frame, std::size_t pc) const {
        if (frame.routine == nullptr) {
            return std::nullopt;
        }
        int depth = 0;
        for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index) {
            const auto kind = frame.routine->statements[index].kind;
            if (kind == StatementKind::for_statement) {
                ++depth;
            } else if (kind == StatementKind::endfor_statement) {
                if (depth == 0) {
                    return index;
                }
                --depth;
            }
        }
        return std::nullopt;
    }

    std::filesystem::path resolve_asset_path(const std::string& raw_path, const char* extension) const {
        std::filesystem::path asset_path(unquote_string(take_first_token(raw_path)));
        if (asset_path.extension().empty()) {
            asset_path += extension;
        }
        if (asset_path.is_relative()) {
            asset_path = std::filesystem::path(default_directory) / asset_path;
        }
        return asset_path.lexically_normal();
    }

    ExecutionOutcome open_report_surface(const Statement& statement, const char* extension, const char* category) {
        const std::filesystem::path asset_path = resolve_asset_path(statement.identifier, extension);
        if (!std::filesystem::exists(asset_path)) {
            last_error_message = std::string("Unable to resolve report asset: ") + asset_path.string();
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            return {.ok = false, .message = last_error_message};
        }

        const auto open_result = studio::open_document({
            .path = asset_path.string(),
            .read_only = true,
            .load_full_table = true
        });
        if (!open_result.ok) {
            last_error_message = open_result.error;
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            return {.ok = false, .message = last_error_message};
        }

        const auto layout = studio::build_report_layout(open_result.document);
        waiting_for_events = true;
        events.push_back({
            .category = category,
            .detail = asset_path.string(),
            .location = statement.location
        });
        if (layout.available) {
            events.push_back({
                .category = std::string(category) + ".layout",
                .detail = std::to_string(layout.sections.size()) + " sections",
                .location = statement.location
            });
        }
        return {.waiting_for_events = true};
    }

    bool dispatch_event_handler(const std::string& routine_name);
    ExecutionOutcome execute_current_statement();
    RuntimePauseState run(DebugResumeAction action);
};

namespace {

class ExpressionParser {
public:
    ExpressionParser(
        const std::string& text,
        const Frame& frame,
        const std::map<std::string, PrgValue>& globals,
        const std::string& default_directory,
        const std::string& last_error_message,
        const std::string& error_handler,
        int current_work_area,
        std::function<int()> next_free_work_area_callback,
        std::function<int(const std::string&)> resolve_work_area_callback,
        std::function<std::string(const std::string&)> alias_lookup_callback,
        std::function<bool(const std::string&)> used_callback,
        std::function<std::string(const std::string&)> dbf_lookup_callback,
        std::function<std::size_t(const std::string&)> field_count_callback,
        std::function<std::size_t(const std::string&)> record_count_callback,
        std::function<std::size_t(const std::string&)> recno_callback,
        std::function<bool(const std::string&)> found_callback,
        std::function<bool(const std::string&)> eof_callback,
        std::function<bool(const std::string&)> bof_callback,
        std::function<int(const std::string&, const std::string&)> sql_connect_callback,
        std::function<int(int, const std::string&, const std::string&)> sql_exec_callback,
        std::function<bool(int)> sql_disconnect_callback,
        std::function<int(const std::string&, const std::string&)> register_ole_callback,
        std::function<PrgValue(const std::string&, const std::string&, const std::vector<PrgValue>&)> ole_invoke_callback,
        std::function<PrgValue(const std::string&)> ole_property_callback,
        std::function<void(const std::string&, const std::string&)> record_event_callback)
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
          sql_connect_callback_(std::move(sql_connect_callback)),
          sql_exec_callback_(std::move(sql_exec_callback)),
          sql_disconnect_callback_(std::move(sql_disconnect_callback)),
          register_ole_callback_(std::move(register_ole_callback)),
          ole_invoke_callback_(std::move(ole_invoke_callback)),
          ole_property_callback_(std::move(ole_property_callback)),
          record_event_callback_(std::move(record_event_callback)),
          text_(text),
          frame_(frame),
          globals_(globals),
          default_directory_(default_directory),
          last_error_message_(last_error_message),
          error_handler_(error_handler) {
    }

    PrgValue parse() {
        position_ = 0;
        PrgValue value = parse_comparison();
        skip_whitespace();
        return value;
    }

private:
    PrgValue parse_comparison() {
        PrgValue left = parse_additive();
        while (true) {
            skip_whitespace();
            if (match("<>")) {
                left = make_boolean_value(!values_equal(left, parse_additive()));
            } else if (match("<=")) {
                left = make_boolean_value(value_as_number(left) <= value_as_number(parse_additive()));
            } else if (match(">=")) {
                left = make_boolean_value(value_as_number(left) >= value_as_number(parse_additive()));
            } else if (match("==") || match("=")) {
                left = make_boolean_value(values_equal(left, parse_additive()));
            } else if (match("<")) {
                left = make_boolean_value(value_as_number(left) < value_as_number(parse_additive()));
            } else if (match(">")) {
                left = make_boolean_value(value_as_number(left) > value_as_number(parse_additive()));
            } else {
                return left;
            }
        }
    }

    PrgValue parse_additive() {
        PrgValue left = parse_unary();
        while (true) {
            skip_whitespace();
            if (match("+")) {
                PrgValue right = parse_unary();
                if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string) {
                    left = make_string_value(value_as_string(left) + value_as_string(right));
                } else {
                    left = make_number_value(value_as_number(left) + value_as_number(right));
                }
            } else if (match("-")) {
                left = make_number_value(value_as_number(left) - value_as_number(parse_unary()));
            } else {
                return left;
            }
        }
    }

    PrgValue parse_unary() {
        skip_whitespace();
        if (match("!")) {
            return make_boolean_value(!value_as_bool(parse_unary()));
        }
        if (match("-")) {
            return make_number_value(-value_as_number(parse_unary()));
        }
        return parse_primary();
    }

    PrgValue parse_primary() {
        skip_whitespace();
        if (match("(")) {
            PrgValue value = parse_comparison();
            match(")");
            return value;
        }
        if (peek() == '\'') {
            return make_string_value(parse_string());
        }
        if (peek() == '.') {
            if (match(".T.") || match(".t.")) {
                return make_boolean_value(true);
            }
            if (match(".F.") || match(".f.")) {
                return make_boolean_value(false);
            }
        }
        if (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            return make_number_value(parse_number());
        }

        const std::string identifier = parse_identifier();
        if (identifier.empty()) {
            return {};
        }

        skip_whitespace();
        if (match("(")) {
            std::vector<PrgValue> arguments;
            skip_whitespace();
            if (!match(")")) {
                while (true) {
                    arguments.push_back(parse_comparison());
                    skip_whitespace();
                    if (match(")")) {
                        break;
                    }
                    match(",");
                }
            }
            return evaluate_function(identifier, arguments);
        }

        return resolve_identifier(identifier);
    }

    PrgValue evaluate_function(const std::string& identifier, const std::vector<PrgValue>& arguments) {
        const std::string function = normalize_identifier(identifier);
        const auto member_separator = function.find('.');
        if (member_separator != std::string::npos) {
            const std::string base_name = function.substr(0U, member_separator);
            const std::string member_path = function.substr(member_separator + 1U);
            return ole_invoke_callback_(base_name, member_path, arguments);
        }
        if (function == "select") {
            if (arguments.empty()) {
                return make_number_value(static_cast<double>(current_work_area_));
            }
            if (arguments[0].kind == PrgValueKind::string) {
                return make_number_value(static_cast<double>(resolve_work_area_callback_(value_as_string(arguments[0]))));
            }
            const int requested = static_cast<int>(std::llround(value_as_number(arguments[0])));
            return make_number_value(static_cast<double>(requested == 0 ? next_free_work_area_callback_() : resolve_work_area_callback_(std::to_string(requested))));
        }
        if (function == "alias") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_string_value(alias_lookup_callback_(designator));
        }
        if (function == "used") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_boolean_value(used_callback_(designator));
        }
        if (function == "dbf") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_string_value(dbf_lookup_callback_(designator));
        }
        if (function == "fcount") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_number_value(static_cast<double>(field_count_callback_(designator)));
        }
        if (function == "reccount") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_number_value(static_cast<double>(record_count_callback_(designator)));
        }
        if (function == "recno") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_number_value(static_cast<double>(recno_callback_(designator)));
        }
        if (function == "found") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_boolean_value(found_callback_(designator));
        }
        if (function == "eof") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_boolean_value(eof_callback_(designator));
        }
        if (function == "bof") {
            const std::string designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
            return make_boolean_value(bof_callback_(designator));
        }
        if (function == "file" && !arguments.empty()) {
            std::filesystem::path path(value_as_string(arguments[0]));
            if (path.is_relative()) {
                path = std::filesystem::path(default_directory_) / path;
            }
            return make_boolean_value(std::filesystem::exists(path));
        }
        if (function == "sys") {
            if (!arguments.empty() && std::llround(value_as_number(arguments[0])) == 16) {
                return make_string_value(frame_.file_path);
            }
            return make_string_value("0");
        }
        if (function == "at" && arguments.size() >= 2U) {
            const auto found = value_as_string(arguments[1]).find(value_as_string(arguments[0]));
            return make_number_value(found == std::string::npos ? 0.0 : static_cast<double>(found + 1U));
        }
        if (function == "rat" && arguments.size() >= 2U) {
            const auto found = value_as_string(arguments[1]).rfind(value_as_string(arguments[0]));
            return make_number_value(found == std::string::npos ? 0.0 : static_cast<double>(found + 1U));
        }
        if (function == "substr" && arguments.size() >= 2U) {
            const std::string source = value_as_string(arguments[0]);
            const std::size_t start = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1]) - 1.0));
            const std::size_t length = arguments.size() >= 3U
                ? static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])))
                : std::string::npos;
            return make_string_value(start >= source.size() ? std::string{} : source.substr(start, length));
        }
        if (function == "justpath" && !arguments.empty()) {
            return make_string_value(std::filesystem::path(value_as_string(arguments[0])).parent_path().string());
        }
        if (function == "str" && !arguments.empty()) {
            std::ostringstream stream;
            stream << std::llround(value_as_number(arguments[0]));
            return make_string_value(stream.str());
        }
        if (function == "alltrim" && !arguments.empty()) {
            return make_string_value(trim_copy(value_as_string(arguments[0])));
        }
        if (function == "chr" && !arguments.empty()) {
            return make_string_value(std::string(1U, static_cast<char>(std::llround(value_as_number(arguments[0])))));
        }
        if (function == "message") {
            return make_string_value(last_error_message_);
        }
        if (function == "error") {
            return make_number_value(0.0);
        }
        if (function == "version") {
            return make_number_value(arguments.empty() ? 9.0 : 0.0);
        }
        if (function == "on" && !arguments.empty()) {
            return make_string_value(uppercase_copy(value_as_string(arguments[0])) == "ERROR" ? error_handler_ : std::string{});
        }
        if (function == "messagebox" && !arguments.empty()) {
            return make_number_value(1.0);
        }
        if (function == "createobject" && !arguments.empty()) {
            const std::string prog_id = value_as_string(arguments[0]);
            const int handle = register_ole_callback_(prog_id, "createobject");
            record_event_callback_("ole.createobject", prog_id);
            return make_string_value("object:" + prog_id + "#" + std::to_string(handle));
        }
        if (function == "getobject" && !arguments.empty()) {
            const std::string source = value_as_string(arguments[0]);
            const int handle = register_ole_callback_(source, "getobject");
            record_event_callback_("ole.getobject", source);
            return make_string_value("object:" + source + "#" + std::to_string(handle));
        }
        if ((function == "sqlconnect" || function == "sqlstringconnect") && !arguments.empty()) {
            const std::string target = value_as_string(arguments[0]);
            const int handle = sql_connect_callback_(target, function);
            record_event_callback_("sql.connect", function + ":" + target + " -> " + std::to_string(handle));
            return make_number_value(static_cast<double>(handle));
        }
        if (function == "sqlexec" && arguments.size() >= 2U) {
            const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
            const std::string command = value_as_string(arguments[1]);
            const std::string cursor_alias = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
            return make_number_value(static_cast<double>(sql_exec_callback_(handle, command, cursor_alias)));
        }
        if (function == "sqldisconnect" && !arguments.empty()) {
            const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
            const bool ok = sql_disconnect_callback_(handle);
            if (ok) {
                record_event_callback_("sql.disconnect", std::to_string(handle));
            }
            return make_number_value(ok ? 1.0 : -1.0);
        }
        return make_string_value(function);
    }

    PrgValue resolve_identifier(const std::string& identifier) const {
        const std::string normalized = normalize_identifier(identifier);
        const auto local = frame_.locals.find(normalized);
        if (local != frame_.locals.end()) {
            return local->second;
        }
        const auto global = globals_.find(normalized);
        if (global != globals_.end()) {
            return global->second;
        }
        if (normalized.find('.') != std::string::npos) {
            return ole_property_callback_(normalized);
        }
        return {};
    }

    std::string parse_identifier() {
        skip_whitespace();
        const std::size_t start = position_;
        while (position_ < text_.size()) {
            const char ch = text_[position_];
            if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_' || ch == '.') {
                ++position_;
                continue;
            }
            break;
        }
        return text_.substr(start, position_ - start);
    }

    std::string parse_string() {
        std::string result;
        if (!match("'")) {
            return result;
        }
        while (position_ < text_.size()) {
            const char ch = text_[position_++];
            if (ch == '\'') {
                break;
            }
            result.push_back(ch);
        }
        return result;
    }

    double parse_number() {
        const std::size_t start = position_;
        while (position_ < text_.size()) {
            const char ch = text_[position_];
            if (std::isdigit(static_cast<unsigned char>(ch)) != 0 || ch == '.') {
                ++position_;
                continue;
            }
            break;
        }
        return std::stod(text_.substr(start, position_ - start));
    }

    bool match(const std::string& value) {
        skip_whitespace();
        if (text_.compare(position_, value.size(), value) == 0) {
            position_ += value.size();
            return true;
        }
        return false;
    }

    char peek() const {
        return position_ < text_.size() ? text_[position_] : '\0';
    }

    void skip_whitespace() {
        while (position_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[position_])) != 0) {
            ++position_;
        }
    }

    static bool values_equal(const PrgValue& left, const PrgValue& right) {
        if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string) {
            return value_as_string(left) == value_as_string(right);
        }
        if (left.kind == PrgValueKind::boolean || right.kind == PrgValueKind::boolean) {
            return value_as_bool(left) == value_as_bool(right);
        }
        return std::abs(value_as_number(left) - value_as_number(right)) < 0.000001;
    }

    int current_work_area_ = 1;
    std::function<int()> next_free_work_area_callback_;
    std::function<int(const std::string&)> resolve_work_area_callback_;
    std::function<std::string(const std::string&)> alias_lookup_callback_;
    std::function<bool(const std::string&)> used_callback_;
    std::function<std::string(const std::string&)> dbf_lookup_callback_;
    std::function<std::size_t(const std::string&)> field_count_callback_;
    std::function<std::size_t(const std::string&)> record_count_callback_;
    std::function<std::size_t(const std::string&)> recno_callback_;
    std::function<bool(const std::string&)> found_callback_;
    std::function<bool(const std::string&)> eof_callback_;
    std::function<bool(const std::string&)> bof_callback_;
    std::function<int(const std::string&, const std::string&)> sql_connect_callback_;
    std::function<int(int, const std::string&, const std::string&)> sql_exec_callback_;
    std::function<bool(int)> sql_disconnect_callback_;
    std::function<int(const std::string&, const std::string&)> register_ole_callback_;
    std::function<PrgValue(const std::string&, const std::string&, const std::vector<PrgValue>&)> ole_invoke_callback_;
    std::function<PrgValue(const std::string&)> ole_property_callback_;
    std::function<void(const std::string&, const std::string&)> record_event_callback_;
    const std::string& text_;
    const Frame& frame_;
    const std::map<std::string, PrgValue>& globals_;
    const std::string& default_directory_;
    const std::string& last_error_message_;
    const std::string& error_handler_;
    std::size_t position_ = 0;
};

}  // namespace

PrgValue PrgRuntimeSession::Impl::evaluate_expression(const std::string& expression, const Frame& frame) {
    ExpressionParser parser(
        expression,
        frame,
        globals,
        default_directory,
        last_error_message,
        error_handler,
        current_selected_work_area(),
        [this]() {
            return allocate_work_area();
        },
        [this](const std::string& designator) {
            if (designator.empty()) {
                return current_selected_work_area();
            }
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? 0 : cursor->work_area;
        },
        [this](const std::string& designator) {
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? std::string{} : cursor->alias;
        },
        [this](const std::string& designator) {
            return resolve_cursor_target(designator) != nullptr;
        },
        [this](const std::string& designator) {
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? std::string{} : cursor->dbf_identity;
        },
        [this](const std::string& designator) {
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? 0U : cursor->field_count;
        },
        [this](const std::string& designator) {
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? 0U : cursor->record_count;
        },
        [this](const std::string& designator) {
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? 0U : cursor->recno;
        },
        [this](const std::string& designator) {
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? false : cursor->found;
        },
        [this](const std::string& designator) {
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? true : cursor->eof;
        },
        [this](const std::string& designator) {
            const CursorState* cursor = resolve_cursor_target(designator);
            return cursor == nullptr ? true : cursor->bof;
        },
        [this](const std::string& target, const std::string& provider) {
            return sql_connect(target, provider);
        },
        [this](int handle, const std::string& command, const std::string& cursor_alias) {
            return sql_exec(handle, command, cursor_alias);
        },
        [this](int handle) {
            return sql_disconnect(handle);
        },
        [this](const std::string& prog_id, const std::string& source) {
            return register_ole_object(prog_id, source);
        },
        [this, &frame](const std::string& base_name, const std::string& member_path, const std::vector<PrgValue>& arguments) {
            const PrgValue object_value = lookup_variable(frame, base_name);
            auto object = resolve_ole_object(object_value);
            if (!object.has_value()) {
                return make_empty_value();
            }

            RuntimeOleObjectState* runtime_object = *object;
            runtime_object->last_action = member_path + "()";
            ++runtime_object->action_count;
            events.push_back({
                .category = "ole.invoke",
                .detail = runtime_object->prog_id + "." + member_path,
                .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location
            });

            const std::string leaf = member_path.substr(member_path.rfind('.') == std::string::npos ? 0U : member_path.rfind('.') + 1U);
            if (leaf == "add" || leaf == "create" || leaf == "open" || leaf == "item") {
                return make_string_value("object:" + runtime_object->prog_id + "." + member_path + "#" + std::to_string(runtime_object->handle));
            }
            if (arguments.empty()) {
                return make_string_value("ole:" + runtime_object->prog_id + "." + member_path);
            }
            return arguments.front();
        },
        [this, &frame](const std::string& property_path) {
            const auto separator = property_path.find('.');
            if (separator == std::string::npos) {
                return make_empty_value();
            }

            const PrgValue object_value = lookup_variable(frame, property_path.substr(0U, separator));
            auto object = resolve_ole_object(object_value);
            if (!object.has_value()) {
                return make_empty_value();
            }

            RuntimeOleObjectState* runtime_object = *object;
            runtime_object->last_action = property_path.substr(separator + 1U);
            ++runtime_object->action_count;
            events.push_back({
                .category = "ole.get",
                .detail = runtime_object->prog_id + "." + property_path.substr(separator + 1U),
                .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location
            });
            return make_string_value("ole:" + runtime_object->prog_id + "." + property_path.substr(separator + 1U));
        },
        [this](const std::string& category, const std::string& detail) {
            events.push_back({
                .category = category,
                .detail = detail,
                .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location
            });
        });
    return parser.parse();
}

std::optional<std::string> PrgRuntimeSession::Impl::materialize_xasset_bootstrap(
    const std::string& asset_path,
    bool include_read_events) {
    const auto open_result = studio::open_document({
        .path = asset_path,
        .read_only = true,
        .load_full_table = true
    });
    if (!open_result.ok) {
        last_error_message = open_result.error;
        return std::nullopt;
    }

    const XAssetExecutableModel model = build_xasset_executable_model(open_result.document);
    if (!model.ok || !model.runnable_startup) {
        last_error_message = model.error.empty()
            ? "No runnable startup methods were found in asset: " + asset_path
            : model.error;
        return std::nullopt;
    }

    const std::filesystem::path asset_file(asset_path);
    const std::filesystem::path bootstrap_path =
        std::filesystem::temp_directory_path() /
        (asset_file.stem().string() + "_copperfin_bootstrap.prg");

    std::ofstream output(bootstrap_path, std::ios::binary);
    output << build_xasset_bootstrap_source(model, include_read_events);
    output.close();
    if (!output.good()) {
        last_error_message = "Unable to materialize xAsset bootstrap for: " + asset_path;
        return std::nullopt;
    }

    return bootstrap_path.string();
}

ExecutionOutcome PrgRuntimeSession::Impl::execute_current_statement() {
    if (stack.empty()) {
        return {};
    }

    Frame& frame = stack.back();
    if (frame.routine == nullptr || frame.pc >= frame.routine->statements.size()) {
        stack.pop_back();
        return {};
    }

    const Statement statement = frame.routine->statements[frame.pc];
    ++frame.pc;
    ++executed_statement_count;

    events.push_back({
        .category = "execute",
        .detail = statement.text,
        .location = statement.location
    });

    try {
    switch (statement.kind) {
        case StatementKind::assignment:
            if (statement.identifier.find('.') != std::string::npos) {
                const auto separator = statement.identifier.find('.');
                const PrgValue object_value = lookup_variable(frame, statement.identifier.substr(0U, separator));
                auto object = resolve_ole_object(object_value);
                if (!object.has_value()) {
                    last_error_message = "OLE object not found for property assignment: " + statement.identifier;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                RuntimeOleObjectState* runtime_object = *object;
                runtime_object->last_action = statement.identifier.substr(separator + 1U) + " = " + value_as_string(evaluate_expression(statement.expression, frame));
                ++runtime_object->action_count;
                events.push_back({
                    .category = "ole.set",
                    .detail = runtime_object->prog_id + "." + runtime_object->last_action,
                    .location = statement.location
                });
            } else {
                assign_variable(frame, statement.identifier, evaluate_expression(statement.expression, frame));
            }
            return {};
        case StatementKind::expression:
            if (!statement.expression.empty()) {
                if (starts_with_insensitive(statement.expression, "WAIT WINDOW ")) {
                    events.push_back({
                        .category = "ui.wait_window",
                        .detail = value_as_string(evaluate_expression(statement.expression.substr(12U), frame)),
                        .location = statement.location
                    });
                } else {
                    (void)evaluate_expression(statement.expression, frame);
                }
            }
            return {};
        case StatementKind::do_command: {
            const std::string target = trim_copy(statement.identifier);
            Program& program = load_program(frame.file_path);
            if (const auto routine = program.routines.find(normalize_identifier(target)); routine != program.routines.end()) {
                push_routine_frame(program.path, routine->second);
                return {};
            }

            std::filesystem::path target_path(target);
            if (target_path.extension().empty()) {
                target_path += ".prg";
            }
            if (target_path.is_relative()) {
                target_path = std::filesystem::path(default_directory) / target_path;
            }
            if (!std::filesystem::exists(target_path)) {
                last_error_message = "Unable to resolve DO target: " + target;
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            push_main_frame(target_path.string());
            return {};
        }
        case StatementKind::do_form: {
            const std::filesystem::path form_path = resolve_asset_path(statement.identifier, ".scx");
            events.push_back({
                .category = "form.open",
                .detail = form_path.lexically_normal().string(),
                .location = statement.location
            });
            if (std::filesystem::exists(form_path)) {
                if (const auto bootstrap_path = materialize_xasset_bootstrap(form_path.string(), true)) {
                    push_main_frame(*bootstrap_path);
                }
            }
            return {};
        }
        case StatementKind::report_form:
            return open_report_surface(statement, ".frx", "report.preview");
        case StatementKind::label_form:
            return open_report_surface(statement, ".lbx", "label.preview");
        case StatementKind::activate_surface:
            waiting_for_events = true;
            events.push_back({
                .category = statement.identifier + ".activate",
                .detail = statement.expression,
                .location = statement.location
            });
            return {.waiting_for_events = true};
        case StatementKind::release_surface:
            waiting_for_events = false;
            events.push_back({
                .category = statement.identifier + ".release",
                .detail = statement.expression,
                .location = statement.location
            });
            return {};
        case StatementKind::return_statement:
            stack.pop_back();
            return {.frame_returned = true};
        case StatementKind::if_statement:
            if (!value_as_bool(evaluate_expression(statement.expression, frame))) {
                if (const auto destination = find_matching_branch(frame, frame.pc - 1U, true)) {
                    frame.pc = *destination + 1U;
                }
            }
            return {};
        case StatementKind::else_statement:
            if (const auto destination = find_matching_branch(frame, frame.pc - 1U, false)) {
                frame.pc = *destination + 1U;
            }
            return {};
        case StatementKind::endif_statement:
            return {};
        case StatementKind::for_statement: {
            const double start_value = value_as_number(evaluate_expression(statement.expression, frame));
            const double end_value = value_as_number(evaluate_expression(statement.secondary_expression, frame));
            const double step_value = statement.tertiary_expression.empty()
                ? 1.0
                : value_as_number(evaluate_expression(statement.tertiary_expression, frame));
            assign_variable(frame, statement.identifier, make_number_value(start_value));
            const bool should_enter = step_value >= 0.0 ? start_value <= end_value : start_value >= end_value;
            if (!should_enter) {
                if (const auto destination = find_matching_endfor(frame, frame.pc - 1U)) {
                    frame.pc = *destination + 1U;
                }
                return {};
            }
            frame.loops.push_back({
                .for_statement_index = frame.pc - 1U,
                .variable_name = normalize_identifier(statement.identifier),
                .end_value = end_value,
                .step_value = step_value
            });
            return {};
        }
        case StatementKind::endfor_statement: {
            if (frame.loops.empty()) {
                return {};
            }
            LoopState& loop = frame.loops.back();
            const double next_value = value_as_number(lookup_variable(frame, loop.variable_name)) + loop.step_value;
            assign_variable(frame, loop.variable_name, make_number_value(next_value));
            const bool should_continue = loop.step_value >= 0.0
                ? next_value <= loop.end_value
                : next_value >= loop.end_value;
            if (should_continue) {
                frame.pc = loop.for_statement_index + 1U;
            } else {
                frame.loops.pop_back();
            }
            return {};
        }
        case StatementKind::read_events:
            waiting_for_events = true;
            events.push_back({
                .category = "runtime.event_loop",
                .detail = "READ EVENTS entered",
                .location = statement.location
            });
            return {.waiting_for_events = true};
        case StatementKind::clear_events:
            waiting_for_events = false;
            restore_event_loop_after_dispatch = false;
            events.push_back({
                .category = "runtime.event_loop",
                .detail = "CLEAR EVENTS",
                .location = statement.location
            });
            return {};
        case StatementKind::seek_command: {
            CursorState* cursor = resolve_cursor_target(statement.secondary_expression);
            if (cursor == nullptr) {
                last_error_message = "SEEK target work area not found";
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            const std::string search_key = value_as_string(evaluate_expression(statement.expression, frame));
            const bool found = seek_in_cursor(*cursor, search_key);
            events.push_back({
                .category = "runtime.seek",
                .detail = (cursor->active_order_name.empty() ? std::string{"<default>"} : cursor->active_order_name) +
                    ": " + search_key + (found ? " -> found" : " -> not found"),
                .location = statement.location
            });
            return {};
        }
        case StatementKind::go_command: {
            CursorState* cursor = resolve_cursor_target(statement.secondary_expression);
            if (cursor == nullptr) {
                last_error_message = "GO target work area not found";
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            const std::string destination = uppercase_copy(trim_copy(statement.expression));
            if (destination == "TOP") {
                move_cursor_to(*cursor, 1);
            } else if (destination == "BOTTOM") {
                move_cursor_to(*cursor, static_cast<long long>(cursor->record_count));
            } else {
                const long long requested = std::llround(value_as_number(evaluate_expression(statement.expression, frame)));
                move_cursor_to(*cursor, requested);
            }

            events.push_back({
                .category = "runtime.go",
                .detail = destination.empty() ? statement.expression : destination,
                .location = statement.location
            });
            return {};
        }
        case StatementKind::skip_command: {
            CursorState* cursor = resolve_cursor_target(statement.secondary_expression);
            if (cursor == nullptr) {
                last_error_message = "SKIP target work area not found";
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            const long long delta = std::llround(value_as_number(evaluate_expression(statement.expression, frame)));
            move_cursor_to(*cursor, static_cast<long long>(cursor->recno) + delta);
            events.push_back({
                .category = "runtime.skip",
                .detail = statement.expression,
                .location = statement.location
            });
            return {};
        }
        case StatementKind::set_order: {
            CursorState* cursor = resolve_cursor_target(statement.secondary_expression);
            if (cursor == nullptr) {
                last_error_message = "SET ORDER target work area not found";
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            if (!activate_order(*cursor, statement.expression)) {
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            events.push_back({
                .category = "runtime.order",
                .detail = cursor->active_order_name.empty() ? "0" : cursor->active_order_name,
                .location = statement.location
            });
            return {};
        }
        case StatementKind::select_command: {
            std::string selection = trim_copy(value_as_string(evaluate_expression(statement.expression, frame)));
            if (selection.empty()) {
                selection = trim_copy(statement.expression);
            }
            if (selection.empty()) {
                return {};
            }

            int target_area = 0;
            const bool numeric_selection = std::all_of(selection.begin(), selection.end(), [](unsigned char ch) {
                return std::isdigit(ch) != 0;
            });
            if (numeric_selection) {
                target_area = std::stoi(selection);
            } else {
                const CursorState* existing = find_cursor_by_alias(selection);
                if (existing == nullptr) {
                    last_error_message = "SELECT target work area not found: " + selection;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                target_area = existing->work_area;
            }

            const int selected = select_work_area(target_area);
            events.push_back({
                .category = "runtime.select",
                .detail = "work area " + std::to_string(selected),
                .location = statement.location
            });
            return {};
        }
        case StatementKind::use_command: {
            if (statement.expression.empty() && statement.secondary_expression.empty()) {
                close_cursor(std::to_string(current_selected_work_area()));
                events.push_back({
                    .category = "runtime.use.close",
                    .detail = "current work area",
                    .location = statement.location
                });
                return {};
            }
            if (statement.expression.empty()) {
                close_cursor(statement.secondary_expression);
                events.push_back({
                    .category = "runtime.use.close",
                    .detail = trim_copy(statement.secondary_expression),
                    .location = statement.location
                });
                return {};
            }

            const std::string target = value_as_string(evaluate_expression(statement.expression, frame));
            std::string alias = statement.identifier.empty()
                ? std::filesystem::path(unquote_string(target)).stem().string()
                : value_as_string(evaluate_expression(statement.identifier, frame));
            if (alias.empty() && !statement.identifier.empty()) {
                alias = unquote_string(statement.identifier);
            }
            const bool allow_again = normalize_identifier(statement.tertiary_expression) == "again";
            if (!open_table_cursor(target, alias, statement.secondary_expression, allow_again, false, 0, {}, 0U)) {
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }
            events.push_back({
                .category = "runtime.use.open",
                .detail = alias.empty() ? target : alias + " <- " + target,
                .location = statement.location
            });
            return {};
        }
        case StatementKind::set_command:
            set_state[normalize_identifier(statement.expression)] = "true";
            events.push_back({
                .category = "runtime.set",
                .detail = statement.expression,
                .location = statement.location
            });
            return {};
        case StatementKind::set_datasession: {
            const int session_id = static_cast<int>(std::llround(value_as_number(evaluate_expression(statement.expression, frame))));
            current_data_session = std::max(1, session_id);
            (void)current_session_state();
            events.push_back({
                .category = "runtime.datasession",
                .detail = "SET DATASESSION TO " + std::to_string(current_data_session),
                .location = statement.location
            });
            return {};
        }
        case StatementKind::set_default: {
            const std::string evaluated = value_as_string(evaluate_expression(statement.expression, frame));
            if (!evaluated.empty()) {
                default_directory = normalize_path(evaluated);
            }
            return {};
        }
        case StatementKind::on_error:
            error_handler = statement.expression;
            return {};
        case StatementKind::public_declaration:
            for (const auto& name : statement.names) {
                globals.try_emplace(normalize_identifier(name), make_empty_value());
            }
            return {};
        case StatementKind::local_declaration:
            for (const auto& name : statement.names) {
                const std::string normalized = normalize_identifier(name);
                frame.local_names.insert(normalized);
                frame.locals.try_emplace(normalized, make_empty_value());
            }
            return {};
        case StatementKind::no_op:
            return {};
    }
    } catch (const std::exception& error) {
        last_error_message = std::string("Runtime fault: ") + error.what();
        last_fault_location = statement.location;
        last_fault_statement = statement.text;
        events.push_back({
            .category = "runtime.error",
            .detail = last_error_message,
            .location = statement.location
        });
        return {.ok = false, .message = last_error_message};
    } catch (...) {
        last_error_message = "Runtime fault: unknown exception";
        last_fault_location = statement.location;
        last_fault_statement = statement.text;
        events.push_back({
            .category = "runtime.error",
            .detail = last_error_message,
            .location = statement.location
        });
        return {.ok = false, .message = last_error_message};
    }

    return {};
}

bool PrgRuntimeSession::Impl::dispatch_event_handler(const std::string& routine_name) {
    if (!waiting_for_events || stack.empty()) {
        return false;
    }

    const std::string normalized_target = normalize_identifier(routine_name);
    for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator) {
        Program& program = load_program(iterator->file_path);
        const auto found = program.routines.find(normalized_target);
        if (found == program.routines.end()) {
            continue;
        }

        waiting_for_events = false;
        event_dispatch_return_depth = stack.size();
        restore_event_loop_after_dispatch = true;
        push_routine_frame(program.path, found->second);
        events.push_back({
            .category = "runtime.dispatch",
            .detail = found->second.name,
            .location = {}
        });
        return true;
    }

    return false;
}

RuntimePauseState PrgRuntimeSession::Impl::run(DebugResumeAction action) {
    if (entry_pause_pending) {
        entry_pause_pending = false;
        return build_pause_state(DebugPauseReason::entry, "Stopped on entry.");
    }

    if (waiting_for_events) {
        return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
    }

    const std::size_t base_depth = stack.size();
    bool first_statement = true;

    while (true) {
        while (!stack.empty() &&
               (stack.back().routine == nullptr || stack.back().pc >= stack.back().routine->statements.size())) {
            stack.pop_back();
        }
        if (event_dispatch_return_depth.has_value() && stack.size() <= *event_dispatch_return_depth) {
            event_dispatch_return_depth.reset();
            if (restore_event_loop_after_dispatch) {
                restore_event_loop_after_dispatch = false;
                waiting_for_events = true;
                return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
            }
            restore_event_loop_after_dispatch = false;
        }
        if (stack.empty()) {
            return build_pause_state(DebugPauseReason::completed, "Execution completed.");
        }

        const Statement* next = current_statement();
        if (next == nullptr) {
            stack.pop_back();
            continue;
        }

        if (!first_statement && breakpoint_matches(next->location)) {
            return build_pause_state(DebugPauseReason::breakpoint, "Breakpoint hit.");
        }

        const ExecutionOutcome outcome = execute_current_statement();
        if (!outcome.ok) {
            return build_pause_state(DebugPauseReason::error, outcome.message);
        }
        if (outcome.waiting_for_events) {
            return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
        }

        if (stack.empty()) {
            return build_pause_state(DebugPauseReason::completed, "Execution completed.");
        }

        switch (action) {
            case DebugResumeAction::continue_run:
                break;
            case DebugResumeAction::step_into:
                return build_pause_state(DebugPauseReason::step, "Step completed.");
            case DebugResumeAction::step_over:
                if (stack.size() <= base_depth) {
                    return build_pause_state(DebugPauseReason::step, "Step-over completed.");
                }
                break;
            case DebugResumeAction::step_out:
                if (stack.size() < base_depth) {
                    return build_pause_state(DebugPauseReason::step, "Step-out completed.");
                }
                break;
        }

        first_statement = false;
    }
}

PrgRuntimeSession PrgRuntimeSession::create(const RuntimeSessionOptions& options) {
    auto impl = std::make_unique<Impl>(options);
    impl->default_directory = options.working_directory.empty()
        ? std::filesystem::path(options.startup_path).parent_path().string()
        : normalize_path(options.working_directory);
    impl->data_sessions.try_emplace(1);
    impl->push_main_frame(options.startup_path);
    impl->entry_pause_pending = options.stop_on_entry;
    return PrgRuntimeSession(std::move(impl));
}

void PrgRuntimeSession::add_breakpoint(const RuntimeBreakpoint& breakpoint) {
    impl_->breakpoints.push_back({
        .file_path = normalize_path(breakpoint.file_path),
        .line = breakpoint.line
    });
}

void PrgRuntimeSession::clear_breakpoints() {
    impl_->breakpoints.clear();
}

bool PrgRuntimeSession::dispatch_event_handler(const std::string& routine_name) {
    return impl_->dispatch_event_handler(routine_name);
}

RuntimePauseState PrgRuntimeSession::run(DebugResumeAction action) {
    return impl_->run(action);
}

const RuntimePauseState& PrgRuntimeSession::state() const noexcept {
    return impl_->last_state;
}

PrgRuntimeSession::PrgRuntimeSession(std::unique_ptr<Impl> impl)
    : impl_(std::move(impl)) {
}

PrgRuntimeSession::PrgRuntimeSession(PrgRuntimeSession&&) noexcept = default;

PrgRuntimeSession& PrgRuntimeSession::operator=(PrgRuntimeSession&&) noexcept = default;

PrgRuntimeSession::~PrgRuntimeSession() = default;

const char* debug_pause_reason_name(DebugPauseReason reason) {
    switch (reason) {
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

std::string format_value(const PrgValue& value) {
    return value_as_string(value);
}

}  // namespace copperfin::runtime
