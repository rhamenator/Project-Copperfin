#include "copperfin/runtime/prg_engine.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/report_layout.h"

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
    select_command,
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

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
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
        } else if (starts_with_insensitive(line, "SELECT ")) {
            statement.kind = StatementKind::select_command;
            statement.expression = trim_copy(line.substr(7U));
        } else if (starts_with_insensitive(line, "SET DATASESSION TO ")) {
            statement.kind = StatementKind::set_datasession;
            statement.expression = trim_copy(line.substr(19U));
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
    std::string error_handler;
    std::map<std::string, std::string> set_state;
    int selected_work_area = 1;
    int current_data_session = 1;
    int next_work_area = 1;
    int next_sql_handle = 1;
    int next_ole_handle = 1;
    std::map<int, std::string> work_area_aliases;
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
        state.work_area.selected = selected_work_area;
        state.work_area.data_session = current_data_session;
        state.work_area.aliases = work_area_aliases;
        for (const auto& [_, connection] : sql_connections) {
            state.sql_connections.push_back(connection);
        }
        for (const auto& [_, object] : ole_objects) {
            state.ole_objects.push_back(object);
        }

        if (const Statement* statement = current_statement()) {
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
        return std::max(1, next_work_area++);
    }

    int select_work_area(int requested_area) {
        if (requested_area <= 0) {
            requested_area = allocate_work_area();
        } else if (requested_area >= next_work_area) {
            next_work_area = requested_area + 1;
        }
        selected_work_area = requested_area;
        return selected_work_area;
    }

    int sql_connect(const std::string& target, const std::string& provider) {
        const int handle = next_sql_handle++;
        sql_connections.emplace(handle, RuntimeSqlConnectionState{
            .handle = handle,
            .target = target,
            .provider = provider
        });
        return handle;
    }

    bool sql_disconnect(int handle) {
        return sql_connections.erase(handle) > 0;
    }

    bool sql_exec(int handle, const std::string& command) {
        if (sql_connections.find(handle) == sql_connections.end()) {
            last_error_message = "SQL handle not found: " + std::to_string(handle);
            return false;
        }
        events.push_back({
            .category = "sql.exec",
            .detail = "handle " + std::to_string(handle) + ": " + command,
            .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location
        });
        return true;
    }

    int register_ole_object(const std::string& prog_id, const std::string& source) {
        const int handle = next_ole_handle++;
        ole_objects.emplace(handle, RuntimeOleObjectState{
            .handle = handle,
            .prog_id = prog_id,
            .source = source
        });
        return handle;
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
            return {.ok = false, .message = last_error_message};
        }

        const auto open_result = studio::open_document({
            .path = asset_path.string(),
            .read_only = true,
            .load_full_table = true
        });
        if (!open_result.ok) {
            last_error_message = open_result.error;
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
        std::function<int(int)> select_work_area_callback,
        std::function<int(const std::string&, const std::string&)> sql_connect_callback,
        std::function<bool(int, const std::string&)> sql_exec_callback,
        std::function<bool(int)> sql_disconnect_callback,
        std::function<int(const std::string&, const std::string&)> register_ole_callback,
        std::function<void(const std::string&, const std::string&)> record_event_callback)
        : current_work_area_(current_work_area),
          select_work_area_callback_(std::move(select_work_area_callback)),
          sql_connect_callback_(std::move(sql_connect_callback)),
          sql_exec_callback_(std::move(sql_exec_callback)),
          sql_disconnect_callback_(std::move(sql_disconnect_callback)),
          register_ole_callback_(std::move(register_ole_callback)),
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
        if (function == "select") {
            if (arguments.empty()) {
                return make_number_value(static_cast<double>(current_work_area_));
            }
            return make_number_value(static_cast<double>(select_work_area_callback_(static_cast<int>(std::llround(value_as_number(arguments[0]))))));
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
            return make_number_value(sql_exec_callback_(handle, command) ? 1.0 : -1.0);
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
    std::function<int(int)> select_work_area_callback_;
    std::function<int(const std::string&, const std::string&)> sql_connect_callback_;
    std::function<bool(int, const std::string&)> sql_exec_callback_;
    std::function<bool(int)> sql_disconnect_callback_;
    std::function<int(const std::string&, const std::string&)> register_ole_callback_;
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
        selected_work_area,
        [this](int requested_area) {
            return select_work_area(requested_area);
        },
        [this](const std::string& target, const std::string& provider) {
            return sql_connect(target, provider);
        },
        [this](int handle, const std::string& command) {
            return sql_exec(handle, command);
        },
        [this](int handle) {
            return sql_disconnect(handle);
        },
        [this](const std::string& prog_id, const std::string& source) {
            return register_ole_object(prog_id, source);
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

    switch (statement.kind) {
        case StatementKind::assignment:
            assign_variable(frame, statement.identifier, evaluate_expression(statement.expression, frame));
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
        case StatementKind::select_command: {
            const std::string selection = trim_copy(statement.expression);
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
                const std::string normalized = normalize_identifier(selection);
                const auto found = std::find_if(work_area_aliases.begin(), work_area_aliases.end(), [&](const auto& pair) {
                    return normalize_identifier(pair.second) == normalized;
                });
                target_area = found == work_area_aliases.end() ? allocate_work_area() : found->first;
                work_area_aliases[target_area] = selection;
            }

            const int selected = select_work_area(target_area);
            events.push_back({
                .category = "runtime.select",
                .detail = "work area " + std::to_string(selected),
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
