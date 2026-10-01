// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-PREPROCESSOR-STRUCTURE-001 (#5729, #5730).
//
// Installed VFP9 (Windows VM COM probes 2026-09-30, retained at
// /home/rich/temp/vfp9-probes/preprocessor-5729-5730/vfp9-result-err.txt) logs compile errors for a
// missing #INCLUDE ("File does not exist.") and for a stray #ENDIF or #ELSE ("Mismatched
// #IF/#ELSIF/#ELSE/#ENDIF."). It is silent about an unterminated #IFDEF, a duplicate #ELSE and a
// conditional closed across an include boundary. Copperfin rejects all of them with a file-and-line
// diagnostic (an intentional, documented hardening beyond VFP9 for the silent cases: each one used to
// drop part or all of the program while the runtime reported a normal completion).

struct Result {
    bool completed;
    std::string message;
    bool marker;
};

// `setup` runs after the directory is recreated and the files are written, for fixtures a plain file
// list cannot express (a directory where an include should be).
Result run(
    const fs::path &dir,
    const std::vector<std::pair<std::string, std::string>> &files,
    const std::function<void(const fs::path &)> &setup = nullptr) {
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    for (const auto &[name, text] : files) {
        write_text(dir / name, text);
    }
    if (setup) {
        setup(dir);
    }
    // A source-structure error surfaces when the startup program is parsed, i.e. from create(); the
    // runtime host turns that exception into `status: error` and a non-zero exit code.
    try {
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options((dir / "main.prg").string(), dir.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        return {state.completed, state.message, fs::exists(dir / "marker.txt")};
    } catch (const std::exception &error) {
        return {false, error.what(), fs::exists(dir / "marker.txt")};
    }
}

bool mentions(const std::string &message, const std::string &file, const std::string &line) {
    return message.find(file) != std::string::npos && message.find(line) != std::string::npos;
}

void test_missing_and_unreadable_includes_stop_the_program() {
    const fs::path root = fs::temp_directory_path() / "copperfin_preprocessor_includes";
    std::error_code ignored;

    // The issue's reproduction: nothing after the missing include may run.
    Result r = run(root / "missing", {
        {"main.prg", "#INCLUDE \"absent-header.h\"\nSTRTOFILE('continued', 'marker.txt')\nRETURN\n"}});
    expect(!r.completed, "#5729: a missing active #INCLUDE must not report completion");
    expect(!r.marker, "#5729: statements after a missing #INCLUDE must not run");
    expect(mentions(r.message, "absent-header.h", "1") && mentions(r.message, "main.prg", "1"),
        "#5729: the diagnostic should name the include, the including file and the line, got: " + r.message);

    // Line number of the directive, not of the first line.
    r = run(root / "line3", {
        {"main.prg", "x = 1\n* comment\n#INCLUDE \"nope.h\"\nRETURN\n"}});
    expect(!r.completed && mentions(r.message, "nope.h", "3"), "#5729: the line of the #INCLUDE is reported, got: " + r.message);

    // An include that exists but is not a readable file (a directory). The directory is created by the
    // setup callback, after run() has recreated the folder, so the include really is a directory.
    const auto catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    const auto message_prefix = [&](const std::string &key) {
        const std::string text = catalog.translate(key);
        return text.substr(0U, text.find('{'));
    };
    r = run(root / "dirinclude",
        {{"main.prg", "#INCLUDE \"header.h\"\nSTRTOFILE('x', 'marker.txt')\nRETURN\n"}},
        [](const fs::path &dir) { fs::create_directories(dir / "header.h"); });
    expect(!r.completed && !r.marker, "#5729: an include that is a directory is rejected, got: " + r.message);
    expect(r.message.rfind(message_prefix("Runtime.Prg.Parser.Error.IncludeFileUnreadable"), 0U) == 0U &&
               mentions(r.message, "header.h", "1"),
        "#5729: a directory include reports the unreadable diagnostic, not 'not found', got: " + r.message);
    expect(r.message.rfind(message_prefix("Runtime.Prg.Parser.Error.IncludeFileNotFound"), 0U) != 0U,
        "#5729: an existing-but-unreadable include must not be reported as missing, got: " + r.message);

    // A verified package fails closed on a missing include too, with the same file-and-line location.
    {
        const fs::path dir = root / "verified";
        fs::remove_all(dir, ignored);
        fs::create_directories(dir);
        auto options = make_runtime_session_options((dir / "main.prg").string(), dir.string(), false);
        options.startup_source_text = "x = 1\n#INCLUDE \"pkg-header.h\"\nRETURN\n";
        options.require_source_text_overrides = true;
        bool typed_diagnostic = false;
        std::string verified_message;
        try {
            auto session = copperfin::runtime::PrgRuntimeSession::create(options);
            (void)session;
        } catch (const copperfin::runtime::PrgSourceDiagnostic &diagnostic) {
            typed_diagnostic = true;
            verified_message = diagnostic.what();
        } catch (const std::exception &error) {
            verified_message = error.what();
        }
        expect(typed_diagnostic, "#5729: a verified-package include failure is a PrgSourceDiagnostic, got: " + verified_message);
        expect(mentions(verified_message, "pkg-header.h", "2"),
            "#5729: the verified-package diagnostic names the include and its line, got: " + verified_message);
    }

    // An include inside an inactive branch is never resolved.
    r = run(root / "inactive", {
        {"main.prg", "#IFDEF NEVER_DEFINED\n#INCLUDE \"absent-header.h\"\n#ENDIF\nSTRTOFILE('ran', 'marker.txt')\nRETURN\n"}});
    expect(r.completed && r.marker, "#5729: an include in an inactive #IFDEF branch stays ignored: " + r.message);
    r = run(root / "inactive_else", {
        {"main.prg", "#DEFINE HAVE_IT 1\n#IFDEF HAVE_IT\nSTRTOFILE('ran', 'marker.txt')\n#ELSE\n#INCLUDE \"absent-header.h\"\n#ENDIF\nRETURN\n"}});
    expect(r.completed && r.marker, "#5729: an include in the untaken #ELSE branch stays ignored: " + r.message);

    // A present include still works, including from a subdirectory spelled with a backslash.
    r = run(root / "present", {
        {"main.prg", "#INCLUDE \"ok.h\"\nSTRTOFILE(TRANSFORM(ANSWER), 'marker.txt')\nRETURN\n"},
        {"ok.h", "#DEFINE ANSWER 42\n"}});
    expect(r.completed && r.marker, "#5729: a present include must still load: " + r.message);
    fs::remove_all(root, ignored);
}

void test_malformed_conditionals_are_rejected_with_locations() {
    const fs::path root = fs::temp_directory_path() / "copperfin_preprocessor_conditionals";
    std::error_code ignored;
    struct Case {
        std::string name;
        std::vector<std::pair<std::string, std::string>> files;
        std::string expected_file;
        std::string expected_line;
    };
    const std::vector<Case> cases = {
        {"unterminated_ifdef",
         {{"main.prg", "x = 1\n#IFDEF NEVER_DEFINED\nSTRTOFILE('no', 'marker.txt')\nRETURN\n"}}, "main.prg", "2"},
        {"unterminated_if",
         {{"main.prg", "#IF .T.\nSTRTOFILE('no', 'marker.txt')\nRETURN\n"}}, "main.prg", "1"},
        {"unterminated_ifndef",
         {{"main.prg", "#IFNDEF SOMETHING\nSTRTOFILE('no', 'marker.txt')\nRETURN\n"}}, "main.prg", "1"},
        {"nested_one_missing",
         {{"main.prg", "#IF .T.\n#IF .F.\nx = 1\n#ENDIF\nSTRTOFILE('no', 'marker.txt')\nRETURN\n"}}, "main.prg", "1"},
        // The issue's cross-file reproduction: an unterminated conditional in a header.
        {"cross_file_unterminated",
         {{"main.prg", "#INCLUDE \"bad.h\"\nSTRTOFILE('should-run', 'marker.txt')\nRETURN\n"},
          {"bad.h", "#IFDEF NEVER_DEFINED\n"}}, "bad.h", "1"},
        {"stray_endif", {{"main.prg", "x = 1\n#ENDIF\nSTRTOFILE('no', 'marker.txt')\nRETURN\n"}}, "main.prg", "2"},
        {"stray_else", {{"main.prg", "#ELSE\nSTRTOFILE('no', 'marker.txt')\nRETURN\n"}}, "main.prg", "1"},
        {"duplicate_else",
         {{"main.prg", "#IFDEF ABC\n#ELSE\n#ELSE\n#ENDIF\nSTRTOFILE('no', 'marker.txt')\nRETURN\n"}}, "main.prg", "3"},
        // A header may not close its caller's conditional.
        {"header_closes_caller",
         {{"main.prg", "#IF .T.\n#INCLUDE \"inner.h\"\nSTRTOFILE('no', 'marker.txt')\nRETURN\n"},
          {"inner.h", "#ENDIF\n"}}, "inner.h", "1"},
        // Structure is validated even inside an inactive region.
        {"stray_endif_in_inactive",
         {{"main.prg", "#IFDEF NEVER\n#ENDIF\n#ENDIF\nRETURN\n"}}, "main.prg", "3"},
    };
    for (const Case &c : cases) {
        const Result r = run(root / c.name, c.files);
        expect(!r.completed, "#5730 " + c.name + ": a malformed conditional must not report completion");
        expect(!r.marker, "#5730 " + c.name + ": no statement may run after the structure error");
        expect(mentions(r.message, c.expected_file, c.expected_line),
            "#5730 " + c.name + ": expected a diagnostic naming " + c.expected_file + " line " + c.expected_line + ", got: " + r.message);
    }
    fs::remove_all(root, ignored);
}

void test_well_formed_conditionals_still_work() {
    const fs::path root = fs::temp_directory_path() / "copperfin_preprocessor_valid";
    std::error_code ignored;
    Result r = run(root / "nested", {
        {"main.prg",
         "#DEFINE FLAG 1\n"
         "#IF FLAG = 1\n"
         "#IFDEF OTHER\n"
         "STRTOFILE('wrong-a', 'marker.txt')\n"
         "#ELSE\n"
         "#IFNDEF MISSING\n"
         "STRTOFILE('right', 'marker.txt')\n"
         "#ENDIF\n"
         "#ENDIF\n"
         "#ELSE\n"
         "STRTOFILE('wrong-b', 'marker.txt')\n"
         "#ENDIF\n"
         "RETURN\n"}});
    expect(r.completed && r.marker, "valid nesting must still run: " + r.message);
    expect(read_text(root / "nested" / "marker.txt") == "right", "valid nesting must pick the right branch");

    // A header with its own balanced conditionals, included from inside the caller's conditional.
    r = run(root / "balanced_include", {
        {"main.prg", "#IF .T.\n#INCLUDE \"balanced.h\"\n#ENDIF\nSTRTOFILE(TRANSFORM(VALUE_FROM_HEADER), 'marker.txt')\nRETURN\n"},
        {"balanced.h", "#IFDEF NEVER\n#DEFINE VALUE_FROM_HEADER 0\n#ELSE\n#DEFINE VALUE_FROM_HEADER 7\n#ENDIF\n"}});
    expect(r.completed && r.marker, "a header with balanced conditionals must still work: " + r.message);
    expect(read_text(root / "balanced_include" / "marker.txt") == "7", "the header's taken branch defines the value");
    fs::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_missing_and_unreadable_includes_stop_the_program();
    test_malformed_conditionals_are_rejected_with_locations();
    test_well_formed_conditionals_still_work();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
