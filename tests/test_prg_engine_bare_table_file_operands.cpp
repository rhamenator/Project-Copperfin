// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-BARE-TABLE-FILE-OPERANDS-001 (#6562, #6563, #6565, #6567, #6568).
//
// Installed VFP9 accepts an unquoted, unparenthesized filename operand for APPEND FROM,
// COPY TO, COPY STRUCTURE TO, SAVE TO and RESTORE FROM: the token is the literal file name
// (resolved against the default directory, with the command's default extension when it has
// none), never a variable or expression. Parenthesized names, quoted strings and function
// calls are still evaluated.

struct RunResult {
    bool completed;
    std::string message;
};

RunResult run_script(const fs::path &dir, const std::string &body) {
    const fs::path script_dir = dir / "script";
    fs::create_directories(script_dir);
    const fs::path script_path = script_dir / "bare_operands.prg";
    write_text(script_path, body + "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    return {state.completed, state.message};
}

fs::path fresh_dir(const std::string &name) {
    std::error_code ignored;
    const fs::path dir = fs::temp_directory_path() / ("copperfin_bare_table_operands_" + name);
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    return dir;
}

std::string trimmed(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r' || text.back() == ' ')) {
        text.pop_back();
    }
    return text;
}

void test_append_from_bare_filename() {
    const fs::path dir = fresh_dir("append_from");
    write_simple_dbf(dir / "target.dbf", {"ONE"});
    write_text(dir / "duplicate.csv", "NAME\nDUP\n");
    const auto result = run_script(
        dir,
        "USE target.dbf\n"
        "APPEND FROM duplicate.csv TYPE CSV\n"
        "STRTOFILE(TRANSFORM(RECCOUNT()), 'bare_count.txt')\n"
        "GO BOTTOM\n"
        "STRTOFILE(ALLTRIM(NAME), 'bare_last.txt')\n"
        "USE\n"
        "USE target.dbf\n"
        "APPEND FROM 'duplicate.csv' TYPE CSV\n"
        "STRTOFILE(TRANSFORM(RECCOUNT()), 'quoted_count.txt')\n");
    expect(result.completed, "#6562: script should complete: " + result.message);
    expect(trimmed(read_text(dir / "bare_count.txt")) == "2", "#6562: bare APPEND FROM should add the CSV row");
    expect(trimmed(read_text(dir / "bare_last.txt")) == "DUP", "#6562: the appended row should be DUP");
    expect(trimmed(read_text(dir / "quoted_count.txt")) == "3", "#6562: quoted control should also append");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_copy_to_bare_filename() {
    const fs::path dir = fresh_dir("copy_to");
    write_simple_dbf(dir / "source.dbf", {"row1", "row2"});
    const auto result = run_script(
        dir,
        "USE source.dbf\n"
        "xname = 'variable.csv'\n"
        "COPY TO bare.csv TYPE CSV\n"
        "COPY TO xname TYPE CSV\n"
        "COPY TO (xname) TYPE CSV\n");
    expect(result.completed, "#6563: script should complete: " + result.message);
    expect(fs::exists(dir / "bare.csv"), "#6563: bare COPY TO should create bare.csv");
    expect(fs::exists(dir / "xname.csv"), "#6563: a bare variable-looking token is a literal file name");
    expect(fs::exists(dir / "variable.csv"), "#6563: a parenthesized expression must still be evaluated");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_copy_structure_to_bare_table_name() {
    const fs::path dir = fresh_dir("copy_structure");
    write_simple_dbf(dir / "source.dbf", {"row1"});
    const auto result = run_script(
        dir,
        "USE source.dbf\n"
        "COPY STRUCTURE TO bare\n");
    expect(result.completed, "#6565: script should complete: " + result.message);
    expect(fs::exists(dir / "bare.dbf"), "#6565: bare COPY STRUCTURE TO should create bare.dbf");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_save_to_and_restore_from_bare_filename() {
    const fs::path dir = fresh_dir("save_restore");
    const auto result = run_script(
        dir,
        "PUBLIC probeValue\n"
        "probeValue = 42\n"
        "SAVE TO bare.mem\n"
        "SAVE TO bareext\n"
        "RELEASE probeValue\n"
        "RESTORE FROM bare.mem\n"
        "STRTOFILE(TRANSFORM(probeValue), 'restored_bare.txt')\n"
        "RELEASE probeValue\n"
        "RESTORE FROM bareext\n"
        "STRTOFILE(TRANSFORM(probeValue), 'restored_default_ext.txt')\n"
        "SAVE TO 'quoted.mem'\n"
        "RELEASE probeValue\n"
        "RESTORE FROM 'quoted.mem'\n"
        "STRTOFILE(TRANSFORM(probeValue), 'restored_quoted.txt')\n");
    expect(result.completed, "#6567/#6568: script should complete: " + result.message);
    expect(fs::exists(dir / "bare.mem"), "#6567: bare SAVE TO should create bare.mem");
    expect(fs::exists(dir / "bareext.mem"), "#6567: SAVE TO without an extension should default to .mem");
    expect(trimmed(read_text(dir / "restored_bare.txt")) == "42", "#6568: bare RESTORE FROM should restore 42");
    expect(trimmed(read_text(dir / "restored_default_ext.txt")) == "42", "#6568: extensionless RESTORE FROM should restore 42");
    expect(trimmed(read_text(dir / "restored_quoted.txt")) == "42", "#6568: quoted control should restore 42");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// VFP-style backslash separators in a bare relative operand reach the right subdirectory on
// every host (POSIX does not treat a backslash as a separator).
void test_bare_operands_accept_backslash_subdirectories() {
    const fs::path dir = fresh_dir("backslash");
    fs::create_directories(dir / "sub");
    write_simple_dbf(dir / "sub" / "source.dbf", {"ONE"});
    write_text(dir / "sub" / "rows.csv", "NAME\nTWO\n");
    const auto result = run_script(
        dir,
        "USE 'sub/source.dbf'\n"
        "APPEND FROM sub\\rows.csv TYPE CSV\n"
        "STRTOFILE(TRANSFORM(RECCOUNT()), 'sub/count.txt')\n"
        "COPY TO sub\\out.csv TYPE CSV\n"
        "COPY STRUCTURE TO sub\\structure\n"
        "PUBLIC probeValue\n"
        "probeValue = 7\n"
        "SAVE TO sub\\state.mem\n"
        "RELEASE probeValue\n"
        "RESTORE FROM sub\\state.mem\n"
        "STRTOFILE(TRANSFORM(probeValue), 'sub/restored.txt')\n");
    expect(result.completed, "#6562-#6568 backslash: script should complete: " + result.message);
    expect(trimmed(read_text(dir / "sub" / "count.txt")) == "2", "backslash: APPEND FROM sub\\rows.csv should append");
    expect(fs::exists(dir / "sub" / "out.csv"), "backslash: COPY TO sub\\out.csv should write into sub");
    expect(fs::exists(dir / "sub" / "structure.dbf"), "backslash: COPY STRUCTURE TO sub\\structure should write into sub");
    expect(fs::exists(dir / "sub" / "state.mem"), "backslash: SAVE TO sub\\state.mem should write into sub");
    expect(trimmed(read_text(dir / "sub" / "restored.txt")) == "7", "backslash: RESTORE FROM sub\\state.mem should restore 7");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// COPY STRUCTURE EXTENDED TO keeps evaluating its operand: this slice does not change it.
void test_copy_structure_extended_still_evaluates_operand() {
    const fs::path dir = fresh_dir("structure_extended");
    write_simple_dbf(dir / "source.dbf", {"row1"});
    const auto result = run_script(
        dir,
        "USE source.dbf\n"
        "cOut = 'metadata.dbf'\n"
        "COPY STRUCTURE EXTENDED TO cOut\n");
    expect(result.completed, "COPY STRUCTURE EXTENDED: script should complete: " + result.message);
    expect(fs::exists(dir / "metadata.dbf"), "COPY STRUCTURE EXTENDED TO cOut must still evaluate the variable");
    expect(!fs::exists(dir / "cOut.dbf"), "COPY STRUCTURE EXTENDED TO cOut must not treat cOut as a literal name");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_append_from_bare_filename();
    test_copy_to_bare_filename();
    test_copy_structure_to_bare_table_name();
    test_save_to_and_restore_from_bare_filename();
    test_bare_operands_accept_backslash_subdirectories();
    test_copy_structure_extended_still_evaluates_operand();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
