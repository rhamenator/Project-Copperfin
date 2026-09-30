// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

// Governing requirement: RQ-CF-PRG-FILE-COMMAND-OPERANDS-001 (#6582, #6589).
//
// Installed VFP9 (09.00.0000.7423, Windows VM COM probes 2026-09-30) treats an
// unquoted, unparenthesized ERASE operand as a literal filename (`ERASE cF`
// erases the file named "cF", not the file named by variable cF), evaluates a
// parenthesized name expression or a function call, and expands * and ? in the
// filename, case-insensitively, whether the wildcard is bare, quoted, or
// returned by a UDF. A pattern or file that matches nothing is not an error.

namespace fs = std::filesystem;

std::string listing(const fs::path &dir) {
    std::set<std::string> names;
    for (const auto &entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            names.insert(entry.path().filename().string());
        }
    }
    std::string joined;
    for (const auto &name : names) {
        joined += (joined.empty() ? "" : ",") + name;
    }
    return joined;
}

struct EraseCase {
    std::string name;
    std::vector<std::string> files;
    std::string script_body;      // commands run with the temp dir as default directory
    std::string expected_remaining;
    std::size_t expected_erase_events;
};

void test_erase_and_delete_file_operand_forms() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_file_command_operands_erase";
    std::error_code ignored;

    const std::string udf =
        "FUNCTION erase_mask\n"
        "LPARAMETERS cMask\n"
        "RETURN cMask\n"
        "ENDFUNC\n";

    const std::vector<EraseCase> cases = {
        {"bare_filename", {"bare.txt", "keep.txt"}, "ERASE bare.txt\n", "keep.txt", 1U},
        {"bare_token_is_literal_not_variable", {"v.txt", "cF", "keep.txt"},
            "cF = 'v.txt'\nERASE cF\n", "keep.txt,v.txt", 1U},
        {"parenthesized_name_expression", {"v.txt", "keep.txt"},
            "cF = 'v.txt'\nERASE (cF)\n", "keep.txt", 1U},
        {"wildcard_star", {"one.tmp", "two.tmp", "keep.txt"}, "ERASE *.tmp\n", "keep.txt", 2U},
        {"wildcard_question_mark", {"one.tmp", "xne.tmp", "two.tmp", "keep.txt"},
            "ERASE ?ne.tmp\n", "keep.txt,two.tmp", 2U},
        {"quoted_wildcard", {"one.tmp", "two.tmp", "keep.txt"}, "ERASE '*.tmp'\n", "keep.txt", 2U},
        {"udf_returned_wildcard", {"one.tmp", "two.tmp", "keep.txt"},
            "ERASE erase_mask('*.tmp')\n", "keep.txt", 2U},
        {"wildcard_is_case_insensitive", {"one.tmp", "two.tmp", "keep.txt"}, "ERASE *.TMP\n", "keep.txt", 2U},
        {"star_dot_star", {"a.txt", "b.log"}, "ERASE *.*\n", "", 2U},
        {"wildcard_without_match_is_silent", {"keep.txt"}, "ERASE nomatch*.zzz\n", "keep.txt", 0U},
        {"absent_literal_file_is_silent", {"keep.txt"}, "ERASE absent.txt\n", "keep.txt", 0U},
        {"delete_file_bare_filename", {"bare.txt", "keep.txt"}, "DELETE FILE bare.txt\n", "keep.txt", 1U},
        {"delete_file_wildcard", {"one.tmp", "two.tmp", "keep.txt"}, "DELETE FILE *.tmp\n", "keep.txt", 2U},
    };

    for (const EraseCase &c : cases) {
        const fs::path dir = temp_root / c.name;
        fs::remove_all(dir, ignored);
        fs::create_directories(dir);
        for (const std::string &file : c.files) {
            write_text(dir / file, "x");
        }
        const fs::path script_dir = temp_root / (c.name + "_script");
        fs::create_directories(script_dir);
        const fs::path script_path = script_dir / "erase_case.prg";
        write_text(script_path, c.script_body + "lAfter = .T.\nRETURN\n" + udf);
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), dir.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed, "#6582/#6589 " + c.name + ": script should complete: " + state.message);
        expect(listing(dir) == c.expected_remaining,
            "#6582/#6589 " + c.name + ": expected remaining [" + c.expected_remaining + "], got [" + listing(dir) + "]");
        const auto erase_events = std::count_if(
            state.events.begin(), state.events.end(),
            [](const copperfin::runtime::RuntimeEvent &event) { return event.category == "runtime.erase"; });
        expect(static_cast<std::size_t>(erase_events) == c.expected_erase_events,
            "#6582/#6589 " + c.name + ": expected " + std::to_string(c.expected_erase_events) +
                " runtime.erase event(s), got " + std::to_string(erase_events));
        fs::remove_all(dir, ignored);
        fs::remove_all(script_dir, ignored);
    }
    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_erase_and_delete_file_operand_forms();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
