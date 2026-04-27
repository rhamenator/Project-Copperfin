#include "prg_engine_internal.h"

#include "prg_engine_command_helpers.h"
#include "prg_engine_helpers.h"

#include <cctype>
#include <fstream>

namespace copperfin::runtime {

namespace {

struct LogicalLine {
    std::size_t line_number = 0;
    std::string text;
    bool is_text_block = false;
    std::string block_text;
};

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

bool looks_like_array_declaration_body(const std::string& body) {
    const std::string trimmed = trim_copy(body);
    for (const char ch : trimmed) {
        if (ch == '(' || ch == '[') {
            return true;
        }
        if (std::isspace(static_cast<unsigned char>(ch)) != 0 || ch == ',') {
            return false;
        }
    }
    return false;
}

std::vector<LogicalLine> load_logical_lines(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    std::vector<LogicalLine> lines;
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

        const std::string text_probe = trim_copy(strip_inline_comment(raw_line));
        if (!continuing && (uppercase_copy(text_probe) == "TEXT" || starts_with_insensitive(text_probe, "TEXT "))) {
            const std::size_t block_start = line_number;
            std::string block_text;
            while (std::getline(input, raw_line)) {
                ++line_number;
                if (!raw_line.empty() && raw_line.back() == '\r') {
                    raw_line.pop_back();
                }

                if (uppercase_copy(trim_copy(strip_inline_comment(raw_line))) == "ENDTEXT") {
                    break;
                }

                block_text += raw_line;
                block_text += "\n";
            }

            lines.push_back({
                .line_number = block_start,
                .text = text_probe,
                .is_text_block = true,
                .block_text = std::move(block_text)
            });
            continuing = false;
            current_text.clear();
            continue;
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
        lines.push_back({
            .line_number = current_start,
            .text = trim_copy(current_text)
        });
    }

    if (continuing && !current_text.empty()) {
        lines.push_back({
            .line_number = current_start,
            .text = trim_copy(current_text)
        });
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

}  // namespace

Program parse_program(const std::string& path) {
    Program program;
    program.path = normalize_path(path);
    program.main.name = "main";

    Routine* current = &program.main;
    for (const auto& logical_line : load_logical_lines(path)) {
        const std::size_t line_number = logical_line.line_number;
        const std::string line = trim_copy(logical_line.text);
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
        } else if (starts_with_insensitive(line, "ELSEIF ")) {
            statement.kind = StatementKind::else_statement;
            statement.expression = trim_copy(line.substr(7U));
        } else if (upper == "DO CASE") {
            statement.kind = StatementKind::do_case_statement;
        } else if (starts_with_insensitive(line, "CASE ")) {
            statement.kind = StatementKind::case_statement;
            statement.expression = trim_copy(line.substr(5U));
        } else if (upper == "OTHERWISE") {
            statement.kind = StatementKind::otherwise_statement;
        } else if (upper == "ELSE") {
            statement.kind = StatementKind::else_statement;
        } else if (upper == "ENDIF") {
            statement.kind = StatementKind::endif_statement;
        } else if (starts_with_insensitive(line, "FOR EACH ")) {
            // FOR EACH <element> IN <collection>
            statement.kind = StatementKind::for_each_statement;
            const std::string body = trim_copy(line.substr(9U));
            const std::size_t in_pos = find_keyword_top_level(body, "IN");
            if (in_pos != std::string::npos) {
                statement.identifier = trim_copy(body.substr(0U, in_pos));
                statement.expression = trim_copy(body.substr(in_pos + 2U));
            }
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
        } else if (starts_with_insensitive(line, "DO WHILE ")) {
            statement.kind = StatementKind::do_while_statement;
            statement.expression = trim_copy(line.substr(9U));
        } else if (upper == "ENDFOR") {
            statement.kind = StatementKind::endfor_statement;
        } else if (upper == "ENDDO") {
            statement.kind = StatementKind::enddo_statement;
        } else if (upper == "ENDCASE") {
            statement.kind = StatementKind::endcase_statement;
        } else if (upper == "LOOP" || upper == "CONTINUE") {
            statement.kind = StatementKind::loop_statement;
        } else if (upper == "EXIT") {
            statement.kind = StatementKind::exit_statement;
        } else if (starts_with_insensitive(line, "WITH ")) {
            statement.kind = StatementKind::with_statement;
            statement.expression = trim_copy(line.substr(5U));
        } else if (upper == "ENDWITH") {
            statement.kind = StatementKind::endwith_statement;
        } else if (upper == "TRY") {
            statement.kind = StatementKind::try_statement;
        } else if (starts_with_insensitive(line, "CATCH")) {
            statement.kind = StatementKind::catch_statement;
            statement.identifier = trim_copy(line.substr(5U));
            if (starts_with_insensitive(statement.identifier, "TO ")) {
                statement.identifier = trim_copy(statement.identifier.substr(3U));
            }
        } else if (upper == "FINALLY") {
            statement.kind = StatementKind::finally_statement;
        } else if (upper == "ENDTRY") {
            statement.kind = StatementKind::endtry_statement;
        } else if (starts_with_insensitive(line, "DO FORM ")) {
            statement.kind = StatementKind::do_form;
            statement.identifier = trim_copy(line.substr(8U));
        } else if (starts_with_insensitive(line, "REPORT FORM ")) {
            statement.kind = StatementKind::report_form;
            const std::string body = trim_copy(line.substr(12U));
            statement.identifier = take_first_token(body);
            statement.secondary_expression = has_keyword(body, "PREVIEW") ? "preview" : std::string{};
            statement.tertiary_expression = extract_command_clause(body, "TO", {"PREVIEW", "NOCONSOLE", "PLAIN", "NOWAIT"});
        } else if (starts_with_insensitive(line, "LABEL FORM ")) {
            statement.kind = StatementKind::label_form;
            const std::string body = trim_copy(line.substr(11U));
            statement.identifier = take_first_token(body);
            statement.secondary_expression = has_keyword(body, "PREVIEW") ? "preview" : std::string{};
            statement.tertiary_expression = extract_command_clause(body, "TO", {"PREVIEW", "NOCONSOLE", "PLAIN", "NOWAIT"});
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
        } else if (upper == "RELEASE ALL" || starts_with_insensitive(line, "RELEASE ALL ")) {
            // RELEASE ALL [LIKE <pattern> | EXCEPT <pattern>]
            statement.kind = StatementKind::release_command;
            statement.identifier = "all";
            const std::string rest = trim_copy(line.substr(11U));
            const std::string rest_upper = uppercase_copy(rest);
            if (starts_with_insensitive(rest, "LIKE ")) {
                statement.expression = "like";
                statement.secondary_expression = trim_copy(rest.substr(5U));
            } else if (starts_with_insensitive(rest, "EXCEPT ")) {
                statement.expression = "except";
                statement.secondary_expression = trim_copy(rest.substr(7U));
            }
        } else if (starts_with_insensitive(line, "RELEASE ")) {
            // RELEASE <varlist>
            statement.kind = StatementKind::release_command;
            statement.identifier = "vars";
            statement.names = split_csv_like(trim_copy(line.substr(8U)));
        } else if (upper == "CLEAR MEMORY" || upper == "CLEAR ALL") {
            statement.kind = StatementKind::clear_memory_command;
            statement.identifier = upper == "CLEAR ALL" ? "all" : "memory";
        } else if (upper == "CANCEL") {
            statement.kind = StatementKind::cancel_statement;
        } else if (upper == "QUIT") {
            statement.kind = StatementKind::quit_statement;
        } else if (starts_with_insensitive(line, "DO ")) {
            statement.kind = StatementKind::do_command;
            const std::string body = trim_copy(line.substr(3U));
            const std::size_t with_position = find_keyword_top_level(body, "WITH");
            if (with_position == std::string::npos) {
                statement.identifier = body;
            } else {
                statement.identifier = trim_copy(body.substr(0U, with_position));
                statement.expression = trim_copy(body.substr(with_position + 4U));
            }
            } else if (starts_with_insensitive(line, "CALL ")) {
                statement.kind = StatementKind::call_command;
                const std::string body = trim_copy(line.substr(5U));
                statement.identifier = body;
        } else if (upper == "READ EVENTS") {
            statement.kind = StatementKind::read_events;
        } else if (upper == "CLEAR EVENTS") {
            statement.kind = StatementKind::clear_events;
        } else if (upper == "BEGIN TRANSACTION") {
            statement.kind = StatementKind::begin_transaction;
        } else if (upper == "END TRANSACTION") {
            statement.kind = StatementKind::end_transaction;
        } else if (upper == "ROLLBACK" || upper == "ROLLBACK TRANSACTION") {
            statement.kind = StatementKind::rollback_transaction;
        } else if (upper == "DOEVENTS") {
            statement.kind = StatementKind::doevents_command;
        } else if (starts_with_insensitive(line, "LPARAMETERS ")) {
            statement.kind = StatementKind::lparameters_declaration;
            statement.names = split_csv_like(line.substr(12U));
        } else if (starts_with_insensitive(line, "PARAMETERS ")) {
            statement.kind = StatementKind::parameters_declaration;
            statement.names = split_csv_like(line.substr(11U));
        } else if (starts_with_insensitive(line, "SEEK ")) {
            statement.kind = StatementKind::seek_command;
            const std::string body = trim_copy(line.substr(5U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"ORDER", "TAG", "IN", "ASCENDING", "DESCENDING"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "IN", {"ORDER", "TAG", "ASCENDING", "DESCENDING"});
            statement.tertiary_expression = extract_command_clause(body, "ORDER", {"TAG", "IN", "ASCENDING", "DESCENDING"});
            if (statement.tertiary_expression.empty()) {
                const std::string tag_name = extract_command_clause(body, "TAG", {"ORDER", "IN", "ASCENDING", "DESCENDING"});
                if (!tag_name.empty()) {
                    statement.tertiary_expression = "TAG " + tag_name;
                }
            }
            if (find_keyword_top_level(body, "DESCENDING") != std::string::npos) {
                statement.quaternary_expression = "DESCENDING";
            } else if (find_keyword_top_level(body, "ASCENDING") != std::string::npos) {
                statement.quaternary_expression = "ASCENDING";
            }
        } else if (starts_with_insensitive(line, "CALCULATE ")) {
            statement.kind = StatementKind::calculate_command;
            const std::string body = trim_copy(line.substr(10U));
            const std::size_t for_position = find_keyword_top_level(body, "FOR");
            const std::size_t in_position = find_keyword_top_level(body, "IN");
            std::size_t tail_start = std::string::npos;
            if (for_position != std::string::npos && in_position != std::string::npos) {
                tail_start = std::min(for_position, in_position);
            } else if (for_position != std::string::npos) {
                tail_start = for_position;
            } else if (in_position != std::string::npos) {
                tail_start = in_position;
            }
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "FOR", {"IN"});
            statement.tertiary_expression = extract_command_clause(body, "IN");
        } else if (upper == "COUNT" || starts_with_insensitive(line, "COUNT ")) {
            statement.kind = StatementKind::count_command;
            const std::string body = upper == "COUNT" ? std::string{} : trim_copy(line.substr(6U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"FOR", "TO", "INTO", "WHILE", "IN", "NOOPTIMIZE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "FOR", {"WHILE", "TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.quaternary_expression = extract_command_clause(body, "IN", {"NOOPTIMIZE"});
            statement.identifier = extract_command_clause(body, "INTO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            if (statement.identifier.empty()) {
                statement.identifier = extract_command_clause(body, "TO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            }
        } else if (upper == "SUM" || starts_with_insensitive(line, "SUM ")) {
            statement.kind = StatementKind::sum_command;
            const std::string body = upper == "SUM" ? std::string{} : trim_copy(line.substr(4U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"FOR", "TO", "INTO", "WHILE", "IN", "NOOPTIMIZE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "FOR", {"WHILE", "TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.quaternary_expression = extract_command_clause(body, "IN", {"NOOPTIMIZE"});
            statement.identifier = extract_command_clause(body, "INTO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            if (statement.identifier.empty()) {
                statement.identifier = extract_command_clause(body, "TO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            }
        } else if (upper == "AVERAGE" || starts_with_insensitive(line, "AVERAGE ")) {
            statement.kind = StatementKind::average_command;
            const std::string body = upper == "AVERAGE" ? std::string{} : trim_copy(line.substr(8U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"FOR", "TO", "INTO", "WHILE", "IN", "NOOPTIMIZE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "FOR", {"WHILE", "TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"TO", "INTO", "IN", "NOOPTIMIZE"});
            statement.quaternary_expression = extract_command_clause(body, "IN", {"NOOPTIMIZE"});
            statement.identifier = extract_command_clause(body, "INTO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            if (statement.identifier.empty()) {
                statement.identifier = extract_command_clause(body, "TO", {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
            }
        } else if (upper == "TOTAL" || starts_with_insensitive(line, "TOTAL ")) {
            statement.kind = StatementKind::total_command;
            statement.expression = upper == "TOTAL" ? std::string{} : trim_copy(line.substr(6U));
        } else if (logical_line.is_text_block && (upper == "TEXT" || starts_with_insensitive(line, "TEXT "))) {
            statement.kind = StatementKind::text_command;
            const std::string body = upper == "TEXT" ? std::string{} : trim_copy(line.substr(4U));
            statement.identifier = extract_command_clause(body, "TO", {"ADDITIVE", "NOSHOW", "TEXTMERGE", "PRETEXT"});
            statement.expression = logical_line.block_text;
            statement.secondary_expression = has_keyword(body, "ADDITIVE") ? "additive" : std::string{};
            statement.tertiary_expression = has_keyword(body, "TEXTMERGE") ? "textmerge" : std::string{};
            statement.quaternary_expression = has_keyword(body, "NOSHOW") ? "noshow" : std::string{};
        } else if (upper == "LOCATE" || starts_with_insensitive(line, "LOCATE ")) {
            statement.kind = StatementKind::locate_command;
            const std::string body = upper == "LOCATE" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (upper == "SCAN" || starts_with_insensitive(line, "SCAN ")) {
            statement.kind = StatementKind::scan_statement;
            const std::string body = upper == "SCAN" ? std::string{} : trim_copy(line.substr(5U));
            statement.expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (upper == "ENDSCAN") {
            statement.kind = StatementKind::endscan_statement;
        } else if (upper == "APPEND BLANK" || starts_with_insensitive(line, "APPEND BLANK ")) {
            statement.kind = StatementKind::append_blank_command;
            const std::string body = upper == "APPEND BLANK" ? std::string{} : trim_copy(line.substr(12U));
            statement.secondary_expression = trim_command_keyword(body, "IN");
        } else if (starts_with_insensitive(line, "REPLACE ")) {
            statement.kind = StatementKind::replace_command;
            const std::string body = trim_copy(line.substr(8U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"FOR", "WHILE", "IN"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.tertiary_expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.quaternary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (starts_with_insensitive(line, "UPDATE ")) {
            statement.kind = StatementKind::update_command;
            const std::string body = trim_copy(line.substr(7U));
            const std::size_t set_position = find_keyword_top_level(body, "SET");
            if (set_position == std::string::npos) {
                statement.identifier = body;
            } else {
                std::string target = trim_copy(body.substr(0U, set_position));
                if (starts_with_insensitive(target, "IN ")) {
                    statement.secondary_expression = trim_copy(target.substr(3U));
                    target.clear();
                }
                statement.identifier = target;
                const std::string update_tail = trim_copy(body.substr(set_position + 3U));
                const std::size_t tail_start = find_first_keyword_top_level(update_tail, {"WHERE", "FOR", "WHILE", "IN"});
                statement.expression = tail_start == std::string::npos ? update_tail : trim_copy(update_tail.substr(0U, tail_start));
                statement.tertiary_expression = extract_command_clause(update_tail, "WHERE", {"FOR", "WHILE", "IN"});
                if (statement.tertiary_expression.empty()) {
                    statement.tertiary_expression = extract_command_clause(update_tail, "FOR", {"WHERE", "WHILE", "IN"});
                }
                statement.quaternary_expression = extract_command_clause(update_tail, "WHILE", {"WHERE", "FOR", "IN"});
                const std::string in_clause = extract_command_clause(update_tail, "IN", {"WHERE", "FOR", "WHILE"});
                if (!in_clause.empty()) {
                    statement.secondary_expression = in_clause;
                }
            }
        } else if (starts_with_insensitive(line, "DELETE FROM ")) {
            statement.kind = StatementKind::delete_from_command;
            const std::string body = trim_copy(line.substr(12U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"WHERE", "FOR", "WHILE"});
            statement.identifier = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.expression = extract_command_clause(body, "WHERE", {"FOR", "WHILE"});
            if (statement.expression.empty()) {
                statement.expression = extract_command_clause(body, "FOR", {"WHERE", "WHILE"});
            }
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"WHERE", "FOR"});
        } else if (starts_with_insensitive(line, "INSERT INTO ")) {
            statement.kind = StatementKind::insert_into_command;
            const std::string body = trim_copy(line.substr(12U));
            const std::size_t values_position = find_keyword_top_level(body, "VALUES");
            if (values_position == std::string::npos) {
                statement.identifier = body;
            } else {
                std::string target = trim_copy(body.substr(0U, values_position));
                const std::size_t open_paren = target.find('(');
                if (open_paren != std::string::npos) {
                    const std::size_t close_paren = target.rfind(')');
                    if (close_paren != std::string::npos && close_paren > open_paren) {
                        statement.expression = trim_copy(target.substr(open_paren + 1U, close_paren - open_paren - 1U));
                        target = trim_copy(target.substr(0U, open_paren));
                    }
                }
                statement.identifier = target;
                std::string values = trim_copy(body.substr(values_position + 6U));
                if (values.size() >= 2U && values.front() == '(' && values.back() == ')') {
                    values = trim_copy(values.substr(1U, values.size() - 2U));
                }
                statement.secondary_expression = values;
            }
        } else if (upper == "DELETE" || starts_with_insensitive(line, "DELETE ")) {
            statement.kind = StatementKind::delete_command;
            const std::string body = upper == "DELETE" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (upper == "RECALL" || starts_with_insensitive(line, "RECALL ")) {
            statement.kind = StatementKind::recall_command;
            const std::string body = upper == "RECALL" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            statement.tertiary_expression = extract_command_clause(body, "WHILE", {"IN"});
            statement.secondary_expression = extract_command_clause(body, "IN");
        } else if (upper == "PACK" || starts_with_insensitive(line, "PACK ")) {
            statement.kind = StatementKind::pack_command;
            const std::string body = upper == "PACK" ? std::string{} : trim_copy(line.substr(5U));
            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "IN", {"MEMO", "DBF"});
            if (statement.secondary_expression.empty() && starts_with_insensitive(body, "IN ")) {
                statement.secondary_expression = trim_copy(body.substr(3U));
            }
        } else if (upper == "ZAP" || starts_with_insensitive(line, "ZAP ")) {
            statement.kind = StatementKind::zap_command;
            const std::string body = upper == "ZAP" ? std::string{} : trim_copy(line.substr(4U));
            statement.secondary_expression = extract_command_clause(body, "IN");
            if (statement.secondary_expression.empty() && starts_with_insensitive(body, "IN ")) {
                statement.secondary_expression = trim_copy(body.substr(3U));
            }
        } else if (upper == "UNLOCK" || starts_with_insensitive(line, "UNLOCK ")) {
            statement.kind = StatementKind::unlock_command;
            const std::string body = upper == "UNLOCK" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "IN");
            if (statement.secondary_expression.empty() && starts_with_insensitive(body, "IN ")) {
                statement.secondary_expression = trim_copy(body.substr(3U));
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
        } else if (upper == "BROWSE" || starts_with_insensitive(line, "BROWSE ")) {
            statement.kind = StatementKind::browse_command;
            const std::string body = upper == "BROWSE" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = body;
            statement.secondary_expression = extract_command_clause(body, "IN", {"FIELDS", "FOR", "WHILE"});
            statement.tertiary_expression = extract_command_clause(body, "FIELDS", {"FOR", "WHILE", "IN"});
            statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE", "IN", "FIELDS"});
        } else if (upper == "EDIT" || starts_with_insensitive(line, "EDIT ")) {
            statement.kind = StatementKind::edit_command;
            const std::string body = upper == "EDIT" ? std::string{} : trim_copy(line.substr(5U));
            statement.expression = extract_command_clause(body, "MEMO", {});
        } else if (upper == "CHANGE" || starts_with_insensitive(line, "CHANGE ")) {
            statement.kind = StatementKind::change_command;
            const std::string body = upper == "CHANGE" ? std::string{} : trim_copy(line.substr(7U));
            statement.expression = extract_command_clause(body, "FIELD", {});
        } else if (starts_with_insensitive(line, "INPUT ") || upper == "INPUT") {
            statement.kind = StatementKind::input_command;
            const std::string body = upper == "INPUT" ? std::string{} : trim_copy(line.substr(6U));
            // Body: ["<prompt>"] TO <var>
            const std::size_t to_pos = find_keyword_top_level(body, "TO");
            if (to_pos != std::string::npos) {
                const std::string prompt_part = trim_copy(body.substr(0U, to_pos));
                const std::string var_part = trim_copy(body.substr(to_pos + 2U));
                statement.expression = prompt_part;
                statement.identifier = var_part;
            } else {
                statement.identifier = trim_copy(body);
            }
        } else if (starts_with_insensitive(line, "ACCEPT ") || upper == "ACCEPT") {
            statement.kind = StatementKind::accept_command;
            const std::string body = upper == "ACCEPT" ? std::string{} : trim_copy(line.substr(7U));
            // Body: ["<prompt>"] TO <var>
            const std::size_t to_pos = find_keyword_top_level(body, "TO");
            if (to_pos != std::string::npos) {
                const std::string prompt_part = trim_copy(body.substr(0U, to_pos));
                const std::string var_part = trim_copy(body.substr(to_pos + 2U));
                statement.expression = prompt_part;
                statement.identifier = var_part;
            } else {
                statement.identifier = trim_copy(body);
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
                if (has_keyword(body, "EXCLUSIVE")) {
                    statement.quaternary_expression = "exclusive";
                } else if (has_keyword(body, "SHARED")) {
                    statement.quaternary_expression = "shared";
                }
            }
        } else if (starts_with_insensitive(line, "SET DATASESSION TO ")) {
            statement.kind = StatementKind::set_datasession;
            statement.expression = trim_copy(line.substr(19U));
        } else if (starts_with_insensitive(line, "SET LIBRARY TO ")) {
            statement.kind = StatementKind::set_library;
            statement.expression = trim_copy(line.substr(15U));
        } else if (starts_with_insensitive(line, "SET ORDER TO ")) {
            statement.kind = StatementKind::set_order;
            const std::string body = trim_copy(line.substr(13U));
            const std::size_t tail_start = find_first_keyword_top_level(body, {"IN", "ASCENDING", "DESCENDING"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "IN", {"ASCENDING", "DESCENDING"});
            if (find_keyword_top_level(body, "DESCENDING") != std::string::npos) {
                statement.tertiary_expression = "DESCENDING";
            } else if (find_keyword_top_level(body, "ASCENDING") != std::string::npos) {
                statement.tertiary_expression = "ASCENDING";
            }
        } else if (starts_with_insensitive(line, "SET DEFAULT TO ")) {
            statement.kind = StatementKind::set_default;
            statement.expression = trim_copy(line.substr(15U));
        } else if (starts_with_insensitive(line, "SET MEMOWIDTH TO ")) {
            statement.kind = StatementKind::set_memowidth;
            statement.expression = trim_copy(line.substr(16U));
        } else if (starts_with_insensitive(line, "SET ")) {
            statement.kind = StatementKind::set_command;
            statement.expression = trim_copy(line.substr(4U));
        } else if (starts_with_insensitive(line, "ON ERROR ")) {
            statement.kind = StatementKind::on_error;
            statement.expression = trim_copy(line.substr(9U));
        } else if (starts_with_insensitive(line, "ON SHUTDOWN ")) {
            statement.kind = StatementKind::on_shutdown;
            statement.expression = trim_copy(line.substr(12U));
        } else if (starts_with_insensitive(line, "PUBLIC ")) {
            statement.kind = StatementKind::public_declaration;
            statement.names = split_csv_like(line.substr(7U));
        } else if (starts_with_insensitive(line, "LOCAL ")) {
            statement.kind = StatementKind::local_declaration;
            statement.names = split_csv_like(line.substr(6U));
        } else if (starts_with_insensitive(line, "PRIVATE ")) {
            statement.kind = StatementKind::private_declaration;
            statement.names = split_csv_like(line.substr(8U));
        } else if (starts_with_insensitive(line, "DIMENSION ")) {
            statement.kind = StatementKind::dimension_command;
            statement.names = split_csv_like(line.substr(10U));
        } else if (starts_with_insensitive(line, "DECLARE ") &&
                   looks_like_array_declaration_body(line.substr(8U))) {
            statement.kind = StatementKind::dimension_command;
            statement.names = split_csv_like(line.substr(8U));
        } else if (starts_with_insensitive(line, "STORE ")) {
            statement.kind = StatementKind::store_command;
            const std::string body = trim_copy(line.substr(6U));
            const std::size_t to_position = find_keyword_top_level(body, "TO");
            if (to_position != std::string::npos) {
                statement.expression = trim_copy(body.substr(0U, to_position));
                statement.names = split_csv_like(trim_copy(body.substr(to_position + 2U)));
            }
        } else if (upper == "RETURN" || starts_with_insensitive(line, "RETURN ")) {
            statement.kind = StatementKind::return_statement;
        } else if (upper == "CLOSE ALL" || upper == "CLOSE TABLES"
            || upper == "CLOSE DATABASES" || upper == "CLOSE DATABASE"
            || starts_with_insensitive(line, "CLOSE ALL")
            || starts_with_insensitive(line, "CLOSE TABLES")
            || starts_with_insensitive(line, "CLOSE DATABASES")) {
            statement.kind = StatementKind::close_command;
            // Store just the scope keyword (ALL, TABLES, DATABASES) as detail
            const std::size_t space_pos = upper.find(' ');
            statement.expression = space_pos != std::string::npos ? trim_copy(upper.substr(space_pos + 1U)) : "ALL";
        } else if (starts_with_insensitive(line, "ERASE ") || starts_with_insensitive(line, "DELETE FILE ")) {
            statement.kind = StatementKind::erase_command;
            const bool starts_delete = starts_with_insensitive(line, "DELETE FILE ");
            statement.expression = trim_copy(line.substr(starts_delete ? 12U : 6U));
        } else if (starts_with_insensitive(line, "COPY FILE ")) {
            // COPY FILE <src> TO <dest>
            statement.kind = StatementKind::copy_file_command;
            const std::string body = trim_copy(line.substr(10U));
            const std::size_t to_pos = find_keyword_top_level(body, "TO");
            if (to_pos != std::string::npos) {
                statement.expression = trim_copy(body.substr(0U, to_pos));
                statement.secondary_expression = trim_copy(body.substr(to_pos + 2U));
            } else {
                statement.expression = body;
            }
        } else if (starts_with_insensitive(line, "RENAME ")) {
            // RENAME <old> TO <new>
            statement.kind = StatementKind::rename_file_command;
            const std::string body = trim_copy(line.substr(7U));
            const std::size_t to_pos = find_keyword_top_level(body, "TO");
            if (to_pos != std::string::npos) {
                statement.expression = trim_copy(body.substr(0U, to_pos));
                statement.secondary_expression = trim_copy(body.substr(to_pos + 2U));
            } else {
                statement.expression = body;
            }
        } else if (!line.empty() && (line[0] == '?' || (line.size() >= 2U && line[0] == '?' && line[1] == '?'))) {
            statement.kind = StatementKind::print_command;
            // ??, ??? all treat what follows as expression
            std::size_t start = 1U;
            while (start < line.size() && line[start] == '?') ++start;
            statement.expression = trim_copy(line.substr(start));
        } else if (starts_with_insensitive(line, "ALTER TABLE ")) {
            statement.kind = StatementKind::alter_table_command;
            const std::string body = trim_copy(line.substr(12U));
            const std::size_t action_position = find_first_keyword_top_level(body, {"ADD", "DROP", "ALTER"});
            if (action_position == std::string::npos) {
                statement.identifier = body;
            } else {
                statement.identifier = trim_copy(body.substr(0U, action_position));
                std::string action_clause = trim_copy(body.substr(action_position));
                const auto [action, remainder] = split_first_word(action_clause);
                action_clause = trim_copy(remainder);
                if (starts_with_insensitive(action_clause, "COLUMN ")) {
                    action_clause = trim_copy(action_clause.substr(7U));
                }
                statement.expression = action_clause;
                statement.secondary_expression = lowercase_copy(action);
            }
        } else if (starts_with_insensitive(line, "CREATE TABLE ")) {
            statement.kind = StatementKind::create_table_command;
            const std::string body = trim_copy(line.substr(13U));
            const auto paren_open = body.find('(');
            if (paren_open != std::string::npos && body.back() == ')') {
                statement.identifier = trim_copy(body.substr(0U, paren_open));
                statement.expression = body.substr(paren_open + 1U, body.size() - paren_open - 2U);
            } else {
                statement.identifier = body;
            }
        } else if (starts_with_insensitive(line, "CREATE CURSOR ")) {
            // CREATE CURSOR <alias> (<field_list>)
            statement.kind = StatementKind::create_cursor_command;
            const std::string body = trim_copy(line.substr(14U));
            const auto paren_open = body.find('(');
            if (paren_open != std::string::npos && body.back() == ')') {
                statement.identifier = trim_copy(body.substr(0U, paren_open));
                statement.expression = body.substr(paren_open + 1U, body.size() - paren_open - 2U);
            } else {
                statement.identifier = body;
            }
        } else if (starts_with_insensitive(line, "COPY TO ARRAY ")) {
            // COPY TO ARRAY <array> [FIELDS <list>] [FOR <expr>]
            statement.kind = StatementKind::copy_to_command;
            statement.identifier = "array";
            const std::string body = trim_copy(line.substr(14U));
            const auto tail_start = find_first_keyword_top_level(body, {"FIELDS", "FOR", "WHILE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.tertiary_expression = extract_command_clause(body, "FIELDS", {"FOR", "WHILE"});
            statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE"});
        } else if (starts_with_insensitive(line, "COPY TO ") || starts_with_insensitive(line, "COPY STRUCTURE TO ")) {
            statement.kind = StatementKind::copy_to_command;
            const bool is_structure = starts_with_insensitive(line, "COPY STRUCTURE TO ");
            const std::string body = trim_copy(line.substr(is_structure ? 18U : 8U));
            statement.identifier = is_structure ? "structure" : std::string{};
            const auto tail_start = find_first_keyword_top_level(body, {"TYPE", "DELIMITED", "FIELDS", "FOR", "WHILE", "IN"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "TYPE", {"WITH", "FIELDS", "FOR", "WHILE", "IN"});
            if (statement.secondary_expression.empty() && has_keyword(body, "DELIMITED")) {
                statement.secondary_expression = "DELIMITED";
            }
            statement.tertiary_expression = extract_command_clause(body, "FIELDS", {"TYPE", "FOR", "WHILE", "IN"});
            statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE", "IN"});
            const std::string with_clause = extract_command_clause(body, "WITH", {"FIELDS", "FOR", "WHILE", "IN"});
            if (!with_clause.empty()) {
                statement.names.push_back(with_clause);
            }
        } else if (starts_with_insensitive(line, "APPEND FROM ARRAY ")) {
            // APPEND FROM ARRAY <array> [FIELDS <list>]
            statement.kind = StatementKind::append_from_command;
            statement.identifier = "array";
            const std::string body = trim_copy(line.substr(18U));
            const auto tail_start = find_first_keyword_top_level(body, {"FIELDS", "FOR", "WHILE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.tertiary_expression = extract_command_clause(body, "FIELDS", {"FOR", "WHILE"});
            statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE"});
        } else if (starts_with_insensitive(line, "APPEND FROM ")) {
            statement.kind = StatementKind::append_from_command;
            const std::string body = trim_copy(line.substr(12U));
            const auto tail_start = find_first_keyword_top_level(body, {"TYPE", "DELIMITED", "FIELDS", "FOR", "WHILE"});
            statement.expression = tail_start == std::string::npos ? body : trim_copy(body.substr(0U, tail_start));
            statement.secondary_expression = extract_command_clause(body, "TYPE", {"WITH", "FIELDS", "FOR", "WHILE"});
            if (statement.secondary_expression.empty() && has_keyword(body, "DELIMITED")) {
                statement.secondary_expression = "DELIMITED";
            }
            statement.tertiary_expression = extract_command_clause(body, "FIELDS", {"TYPE", "FOR", "WHILE"});
            statement.quaternary_expression = extract_command_clause(body, "FOR", {"WHILE"});
            const std::string with_clause = extract_command_clause(body, "WITH", {"FIELDS", "FOR", "WHILE"});
            if (!with_clause.empty()) {
                statement.names.push_back(with_clause);
            }
        } else if (starts_with_insensitive(line, "SCATTER ")) {
            statement.kind = StatementKind::scatter_command;
            const std::string body = trim_copy(line.substr(8U));
            // SCATTER FIELDS <list> TO <var>|MEMVAR [BLANK]
            statement.expression = extract_command_clause(body, "TO", {"FIELDS", "MEMVAR", "BLANK"});
            statement.secondary_expression = extract_command_clause(body, "FIELDS", {"TO", "MEMVAR", "BLANK"});
            statement.identifier = has_keyword(body, "MEMVAR") ? "memvar" : std::string{};
            statement.tertiary_expression = has_keyword(body, "BLANK") ? "blank" : std::string{};
        } else if (upper == "RETRY") {
            statement.kind = StatementKind::retry_statement;
        } else if (upper == "RESUME" || starts_with_insensitive(line, "RESUME ")) {
            statement.kind = StatementKind::resume_statement;
            // RESUME [NEXT | <line>] — store optional qualifier in expression
            if (starts_with_insensitive(line, "RESUME ")) {
                statement.expression = trim_copy(line.substr(7U));
            }
        } else if (starts_with_insensitive(line, "DECLARE ") &&
                   !looks_like_array_declaration_body(line.substr(8U))) {
            // DECLARE <rettype> <funcname> IN <dll> [AS <alias>] [(<params>)]
            // Fields: identifier=alias(funcname), expression=dll_path,
            //   secondary_expression=rettype, tertiary_expression=param_types
            statement.kind = StatementKind::declare_dll;
            const std::string body = trim_copy(line.substr(8U));
            // Parse: <rettype> <funcname> IN <dll> ...
            const std::size_t in_pos = find_keyword_top_level(body, "IN");
            if (in_pos != std::string::npos) {
                const std::string lhs = trim_copy(body.substr(0U, in_pos));
                const std::string rhs = trim_copy(body.substr(in_pos + 2U));
                // lhs = "<rettype> <funcname>"
                const auto space_pos = lhs.find(' ');
                if (space_pos != std::string::npos) {
                    statement.secondary_expression = trim_copy(lhs.substr(0U, space_pos)); // rettype
                    const std::string fn_part = trim_copy(lhs.substr(space_pos + 1U));
                    // Strip parameter list if present: funcname [(params)]
                    const auto paren_pos = fn_part.find('(');
                    if (paren_pos != std::string::npos) {
                        statement.identifier = trim_copy(fn_part.substr(0U, paren_pos));
                        const auto close_paren = fn_part.rfind(')');
                        if (close_paren != std::string::npos && close_paren > paren_pos) {
                            statement.tertiary_expression = fn_part.substr(paren_pos + 1U, close_paren - paren_pos - 1U);
                        }
                    } else {
                        statement.identifier = fn_part;
                    }
                } else {
                    statement.identifier = lhs; // No rettype, just funcname
                }
                // rhs may include AS <alias>
                const std::size_t as_pos = find_keyword_top_level(rhs, "AS");
                if (as_pos != std::string::npos) {
                    statement.expression = trim_copy(rhs.substr(0U, as_pos));
                    statement.quaternary_expression = trim_copy(rhs.substr(as_pos + 2U)); // alias
                } else {
                    statement.expression = rhs; // dll_path
                }
            } else {
                // Malformed — keep as no_op
                statement.kind = StatementKind::no_op;
                statement.expression = body;
            }
        } else if (starts_with_insensitive(line, "GATHER FROM ") || starts_with_insensitive(line, "GATHER MEMVAR")) {
            statement.kind = StatementKind::gather_command;
            const std::string body = trim_copy(line.substr(7U));
            statement.expression = extract_command_clause(body, "FROM", {"FIELDS", "MEMVAR", "FOR"});
            statement.secondary_expression = extract_command_clause(body, "FIELDS", {"FROM", "MEMVAR", "FOR"});
            statement.identifier = has_keyword(body, "MEMVAR") ? "memvar" : std::string{};
            statement.quaternary_expression = extract_command_clause(body, "FOR", {"FROM", "FIELDS", "MEMVAR"});
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

}  // namespace copperfin::runtime
