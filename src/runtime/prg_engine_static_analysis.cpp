#include "copperfin/runtime/prg_engine.h"

#include "prg_engine_helpers.h"

#include <fstream>
#include <vector>

namespace copperfin::runtime {

namespace {

enum class StaticLineKind {
    do_while_true,
    do_while_other,
    enddo,
    exit_statement,
    return_statement,
    other
};

struct StaticLine {
    StaticLineKind kind = StaticLineKind::other;
    std::size_t line = 0;
};

std::string strip_comment(const std::string& line) {
    bool in_string = false;
    for (std::size_t index = 0U; index < line.size(); ++index) {
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

bool is_literal_true_expression(const std::string& expression) {
    const std::string normalized = normalize_identifier(trim_copy(expression));
    return normalized == ".t." || normalized == "true" || normalized == "1";
}

std::vector<StaticLine> read_static_lines(const std::string& path) {
    std::vector<StaticLine> lines;
    std::ifstream input(path, std::ios::binary);
    std::string raw;
    std::size_t line_number = 0U;
    while (std::getline(input, raw)) {
        ++line_number;
        std::string line = trim_copy(strip_comment(raw));
        if (line.empty()) {
            continue;
        }
        if (line[0] == '*' || starts_with_insensitive(line, "* ") || starts_with_insensitive(line, "#")) {
            continue;
        }

        StaticLine current;
        current.line = line_number;
        if (starts_with_insensitive(line, "DO WHILE ")) {
            current.kind = is_literal_true_expression(trim_copy(line.substr(9U)))
                ? StaticLineKind::do_while_true
                : StaticLineKind::do_while_other;
        } else if (uppercase_copy(line) == "ENDDO") {
            current.kind = StaticLineKind::enddo;
        } else if (uppercase_copy(line) == "EXIT") {
            current.kind = StaticLineKind::exit_statement;
        } else if (uppercase_copy(line) == "RETURN") {
            current.kind = StaticLineKind::return_statement;
        } else {
            current.kind = StaticLineKind::other;
        }
        lines.push_back(current);
    }

    return lines;
}

std::optional<std::size_t> find_matching_enddo(const std::vector<StaticLine>& lines, std::size_t index) {
    int depth = 0;
    for (std::size_t cursor = index + 1U; cursor < lines.size(); ++cursor) {
        if (lines[cursor].kind == StaticLineKind::do_while_true || lines[cursor].kind == StaticLineKind::do_while_other) {
            ++depth;
        } else if (lines[cursor].kind == StaticLineKind::enddo) {
            if (depth == 0) {
                return cursor;
            }
            --depth;
        }
    }
    return std::nullopt;
}

bool body_has_obvious_exit(const std::vector<StaticLine>& lines, std::size_t start_index, std::size_t end_index) {
    for (std::size_t cursor = start_index; cursor < end_index; ++cursor) {
        if (lines[cursor].kind == StaticLineKind::exit_statement || lines[cursor].kind == StaticLineKind::return_statement) {
            return true;
        }
    }
    return false;
}

}  // namespace

std::vector<PrgStaticDiagnostic> analyze_prg_file(const std::string& path) {
    std::vector<PrgStaticDiagnostic> diagnostics;

    std::ifstream probe(path, std::ios::binary);
    if (!probe.good()) {
        diagnostics.push_back({
            .code = "PRG0001",
            .severity = DiagnosticSeverity::error,
            .message = "Failed to analyze PRG source: file could not be opened.",
            .location = {.file_path = normalize_path(path), .line = 1}
        });
        return diagnostics;
    }

    const std::vector<StaticLine> lines = read_static_lines(path);
    for (std::size_t index = 0U; index < lines.size(); ++index) {
        if (lines[index].kind != StaticLineKind::do_while_true) {
            continue;
        }

        const auto enddo = find_matching_enddo(lines, index);
        if (!enddo.has_value()) {
            diagnostics.push_back({
                .code = "PRG1002",
                .severity = DiagnosticSeverity::error,
                .message = "DO WHILE block is missing ENDDO.",
                .location = {.file_path = normalize_path(path), .line = lines[index].line}
            });
            continue;
        }

        if (!body_has_obvious_exit(lines, index + 1U, *enddo)) {
            diagnostics.push_back({
                .code = "PRG1001",
                .severity = DiagnosticSeverity::warning,
                .message = "Likely infinite loop: DO WHILE condition is always true and no EXIT/RETURN path was found.",
                .location = {.file_path = normalize_path(path), .line = lines[index].line}
            });
        }
    }

    return diagnostics;
}

}  // namespace copperfin::runtime
