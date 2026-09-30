// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <chrono>
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
    for (const auto &entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            names.insert(fs::relative(entry.path(), dir).generic_string());
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
        // VFP-style backslash separators in a relative operand (review of #6702): on POSIX hosts the
        // backslash is not a path separator, so the operand must be normalized before it is split.
        {"backslash_subdirectory_wildcard", {"sub/one.tmp", "sub/two.tmp", "sub/keep.txt", "top.tmp"},
            "ERASE sub\\*.tmp\n", "sub/keep.txt,top.tmp", 2U},
        {"backslash_subdirectory_literal", {"sub/gone.txt", "sub/keep.txt"},
            "ERASE sub\\gone.txt\n", "sub/keep.txt", 1U},
        // ? matches one character, not one byte, and non-ASCII letters compare case-insensitively.
        {"question_mark_matches_one_multibyte_character", {"\xC3\xA9.tmp", "ab.tmp", "keep.txt"},
            "ERASE ?.tmp\n", "ab.tmp,keep.txt", 1U},
        {"non_ascii_case_insensitive", {"\xC3\x89" "COLE.tmp", "keep.txt"}, "ERASE \xC3\xA9" "col*.tmp\n", "keep.txt", 1U},
        // A hostile mask must not cost exponential time (a backtracking matcher needs ~2M calls here).
        {"pathological_mask_is_linear", {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.tmp", "keep.txt"},
            "ERASE *a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*b.tmp\n",
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.tmp,keep.txt", 0U},
    };

    for (const EraseCase &c : cases) {
        const fs::path dir = temp_root / c.name;
        fs::remove_all(dir, ignored);
        fs::create_directories(dir);
        for (const std::string &file : c.files) {
            fs::create_directories((dir / file).parent_path());
            write_text(dir / file, "x");
        }
        const fs::path script_dir = temp_root / (c.name + "_script");
        fs::create_directories(script_dir);
        const fs::path script_path = script_dir / "erase_case.prg";
        write_text(script_path, c.script_body + "lAfter = .T.\nRETURN\n" + udf);
        const auto started = std::chrono::steady_clock::now();
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), dir.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed, "#6582/#6589 " + c.name + ": script should complete: " + state.message);
        expect(std::chrono::steady_clock::now() - started < std::chrono::seconds(20),
            "#6582/#6589 " + c.name + ": the command should finish quickly");
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


#if !defined(_WIN32)
// Wildcard expansion removes genuine regular files only: a matching symlink is not erased
// (review of #6702).
void test_erase_wildcard_does_not_remove_symlinks() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_file_command_operands_symlink";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    const fs::path dir = temp_root / "dir";
    const fs::path script_dir = temp_root / "script";
    fs::create_directories(dir);
    fs::create_directories(script_dir);
    write_text(dir / "target.txt", "x");
    write_text(dir / "real.tmp", "x");
    std::error_code link_error;
    fs::create_symlink(dir / "target.txt", dir / "link.tmp", link_error);
    if (link_error) {
        fs::remove_all(temp_root, ignored);
        return;  // the host cannot create symlinks; nothing to verify
    }
    const fs::path script_path = script_dir / "erase_symlink.prg";
    write_text(script_path, "ERASE *.tmp\nlAfter = .T.\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6589 symlink: script should complete: " + state.message);
    expect(!fs::exists(dir / "real.tmp"), "#6589 symlink: the regular file should be erased");
    expect(fs::is_symlink(dir / "link.tmp"), "#6589 symlink: a matching symlink must not be erased");
    expect(fs::exists(dir / "target.txt"), "#6589 symlink: the symlink target must survive");
    fs::remove_all(temp_root, ignored);
}
#endif

// name=content listing (recursive, sorted, '/'-separated) used by the COPY FILE / RENAME cases.
std::string content_listing(const fs::path &dir) {
    std::set<std::string> entries;
    for (const auto &entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            entries.insert(fs::relative(entry.path(), dir).generic_string() + "=" + read_text(entry.path()));
        }
    }
    std::string joined;
    for (const auto &entry : entries) {
        joined += (joined.empty() ? "" : "|") + entry;
    }
    return joined;
}

struct TransferCase {
    std::string name;
    std::vector<std::string> files;   // trailing '/' makes a directory; files contain their own name
    std::string script_body;
    std::string expected;             // content_listing afterwards
    bool expect_completed;
    std::string event_category;       // runtime.copy_file or runtime.rename
    std::size_t expected_events;
};

// Governing requirement: RQ-CF-PRG-FILE-COMMAND-OPERANDS-001 (#6583, #6585, #6586, #6587).
//
// Installed VFP9 (Windows VM COM probes 2026-09-30): a bare token is a literal
// filename and (expr) evaluates; * and ? in the source expand and map into the
// destination (`*.bin TO *.bak` copies one.bin to one.bak, `?ne.bin TO ?ne.bak`
// maps the ?), a directory destination receives the copies, a fixed destination
// with several sources is overwritten by the last COPY FILE and hit by the second
// RENAME (error 7), and, unlike ERASE, a missing source or a pattern that matches
// nothing raises error 1.
void test_copy_file_and_rename_operand_forms() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_file_command_operands_transfer";
    std::error_code ignored;

    const std::vector<TransferCase> cases = {
        // ---- COPY FILE
        {"cf_bare", {"a.bin"}, "COPY FILE a.bin TO b.bin\n", "a.bin=a.bin|b.bin=a.bin", true, "runtime.copy_file", 1U},
        {"cf_bare_tokens_are_literal", {"xsrc", "a.bin"},
            "xsrc = 'a.bin'\nxdst = 'z.bin'\nCOPY FILE xsrc TO xdst\n", "a.bin=a.bin|xdst=xsrc|xsrc=xsrc", true, "runtime.copy_file", 1U},
        {"cf_parenthesized_expressions", {"a.bin"},
            "xsrc = 'a.bin'\nxdst = 'z.bin'\nCOPY FILE (xsrc) TO (xdst)\n", "a.bin=a.bin|z.bin=a.bin", true, "runtime.copy_file", 1U},
        {"cf_wildcard_maps_star", {"one.bin", "two.bin", "keep.txt"}, "COPY FILE *.bin TO *.bak\n",
            "keep.txt=keep.txt|one.bak=one.bin|one.bin=one.bin|two.bak=two.bin|two.bin=two.bin", true, "runtime.copy_file", 2U},
        {"cf_wildcard_into_directory", {"one.bin", "two.bin", "d/"}, "COPY FILE *.bin TO d\n",
            "d/one.bin=one.bin|d/two.bin=two.bin|one.bin=one.bin|two.bin=two.bin", true, "runtime.copy_file", 2U},
        {"cf_quoted_wildcards", {"one.bin", "two.bin", "keep.txt"}, "COPY FILE '*.bin' TO '*.bak'\n",
            "keep.txt=keep.txt|one.bak=one.bin|one.bin=one.bin|two.bak=two.bin|two.bin=two.bin", true, "runtime.copy_file", 2U},
        {"cf_overwrites_existing_destination", {"a.bin", "b.bin"}, "COPY FILE a.bin TO b.bin\n", "a.bin=a.bin|b.bin=a.bin", true, "runtime.copy_file", 1U},
        {"cf_question_mark_maps_character", {"one.bin", "xne.bin", "two.bin"}, "COPY FILE ?ne.bin TO ?ne.bak\n",
            "one.bak=one.bin|one.bin=one.bin|two.bin=two.bin|xne.bak=xne.bin|xne.bin=xne.bin", true, "runtime.copy_file", 2U},
        {"cf_fixed_destination_last_copy_wins", {"one.bin", "two.bin"}, "COPY FILE *.bin TO fixed.bak\n",
            "fixed.bak=two.bin|one.bin=one.bin|two.bin=two.bin", true, "runtime.copy_file", 2U},
        // Review of #6702: VFP-style backslash separators in a relative operand, and a multibyte
        // character captured by ? and carried into the destination.
        {"cf_backslash_subdirectory_wildcard", {"sub/one.bin"}, "COPY FILE sub\\*.bin TO sub\\*.bak\n",
            "sub/one.bak=sub/one.bin|sub/one.bin=sub/one.bin", true, "runtime.copy_file", 1U},
        {"cf_multibyte_question_mark_capture", {"\xC3\xA9" "a.bin"}, "COPY FILE ?a.bin TO ?a.bak\n",
            "\xC3\xA9" "a.bak=\xC3\xA9" "a.bin|\xC3\xA9" "a.bin=\xC3\xA9" "a.bin", true, "runtime.copy_file", 1U},
        {"cf_missing_source_is_an_error", {"keep.txt"}, "COPY FILE nosuch.bin TO b.bin\n", "keep.txt=keep.txt", false, "runtime.copy_file", 0U},
        {"cf_wildcard_without_match_is_an_error", {"keep.txt"}, "COPY FILE *.bin TO *.bak\n", "keep.txt=keep.txt", false, "runtime.copy_file", 0U},
        // ---- RENAME
        {"rn_bare", {"old.txt"}, "RENAME old.txt TO new.txt\n", "new.txt=old.txt", true, "runtime.rename", 1U},
        {"rn_parenthesized_expressions", {"old.txt"},
            "xsrc = 'old.txt'\nxdst = 'n.txt'\nRENAME (xsrc) TO (xdst)\n", "n.txt=old.txt", true, "runtime.rename", 1U},
        {"rn_wildcard_maps_star", {"a.prg", "b.prg", "keep.txt"}, "RENAME *.prg TO *.bak\n",
            "a.bak=a.prg|b.bak=b.prg|keep.txt=keep.txt", true, "runtime.rename", 2U},
        {"rn_destination_exists_is_an_error", {"old.txt", "new.txt"}, "RENAME old.txt TO new.txt\n",
            "new.txt=new.txt|old.txt=old.txt", false, "runtime.rename", 0U},
        {"rn_backslash_subdirectory", {"sub/old.txt"}, "RENAME sub\\old.txt TO sub\\new.txt\n",
            "sub/new.txt=sub/old.txt", true, "runtime.rename", 1U},
        {"rn_missing_source_is_an_error", {"keep.txt"}, "RENAME nosuch.txt TO new.txt\n", "keep.txt=keep.txt", false, "runtime.rename", 0U},
        {"rn_wildcard_without_match_is_an_error", {"keep.txt"}, "RENAME *.prg TO *.bak\n", "keep.txt=keep.txt", false, "runtime.rename", 0U},
        {"rn_question_mark_maps_character", {"a.prg", "bb.prg"}, "RENAME ?.prg TO ?.bak\n", "a.bak=a.prg|bb.prg=bb.prg", true, "runtime.rename", 1U},
        {"rn_quoted_wildcards", {"a.prg", "b.prg"}, "RENAME '*.prg' TO '*.bak'\n", "a.bak=a.prg|b.bak=b.prg", true, "runtime.rename", 2U},
        {"rn_fixed_destination_second_rename_fails", {"a.prg", "b.prg"}, "RENAME *.prg TO fixed.bak\n",
            "b.prg=b.prg|fixed.bak=a.prg", false, "runtime.rename", 1U},
    };

    for (const TransferCase &c : cases) {
        const fs::path dir = temp_root / c.name;
        const fs::path script_dir = temp_root / (c.name + "_script");
        fs::remove_all(dir, ignored);
        fs::remove_all(script_dir, ignored);
        fs::create_directories(dir);
        fs::create_directories(script_dir);
        for (const std::string &file : c.files) {
            if (!file.empty() && file.back() == '/') {
                fs::create_directories(dir / file.substr(0U, file.size() - 1U));
            } else {
                fs::create_directories((dir / file).parent_path());
                write_text(dir / file, file);
            }
        }
        const fs::path script_path = script_dir / "transfer_case.prg";
        write_text(script_path, c.script_body + "lAfter = .T.\nRETURN\n");
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), dir.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed == c.expect_completed,
            "#6583-#6587 " + c.name + ": expected completed=" + std::string(c.expect_completed ? "true" : "false") +
                ", got message: " + state.message);
        expect(content_listing(dir) == c.expected,
            "#6583-#6587 " + c.name + ": expected [" + c.expected + "], got [" + content_listing(dir) + "]");
        const auto events = std::count_if(
            state.events.begin(), state.events.end(),
            [&](const copperfin::runtime::RuntimeEvent &event) { return event.category == c.event_category; });
        expect(static_cast<std::size_t>(events) == c.expected_events,
            "#6583-#6587 " + c.name + ": expected " + std::to_string(c.expected_events) + " " + c.event_category +
                " event(s), got " + std::to_string(events));
        fs::remove_all(dir, ignored);
        fs::remove_all(script_dir, ignored);
    }
    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_erase_and_delete_file_operand_forms();
    test_copy_file_and_rename_operand_forms();
#if !defined(_WIN32)
    test_erase_wildcard_does_not_remove_symlinks();
#endif
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
