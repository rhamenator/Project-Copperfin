// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

// Governing requirement: RQ-CF-PRG-SOURCE-RESOURCE-LIMITS-001 (#5731).
//
// PRG source loading used to be unbounded: a large or sparse source file was read in full and then
// copied into several in-memory forms before anything refused it. Each limit is now checked from a
// size or a running count before the memory it protects is allocated, and exceeding one stops the
// program with a localized diagnostic that names the source.

struct Result {
    bool completed;
    std::string message;
    bool marker;
};

using Files = std::vector<std::pair<std::string, std::string>>;

Result run(const fs::path &dir, const Files &files, const std::function<void(copperfin::runtime::RuntimeSessionOptions &)> &configure) {
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    for (const auto &[name, text] : files) {
        write_text(dir / name, text);
    }
    auto options = make_runtime_session_options((dir / "main.prg").string(), dir.string(), false);
    configure(options);
    // A source-size or structure error surfaces when the startup program is parsed, i.e. from create().
    try {
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        return {state.completed, state.message, fs::exists(dir / "marker.txt")};
    } catch (const std::exception &error) {
        return {false, error.what(), fs::exists(dir / "marker.txt")};
    }
}

bool mentions(const std::string &message, const std::vector<std::string> &parts) {
    for (const std::string &part : parts) {
        if (message.find(part) == std::string::npos) {
            return false;
        }
    }
    return true;
}

const std::string kMarkerProgram = "STRTOFILE('ran', 'marker.txt')\nRETURN\n";

void test_oversized_source_is_refused_from_its_size() {
    const fs::path root = fs::temp_directory_path() / "copperfin_source_limits_size";
    std::error_code ignored;

    // A program just over the limit is refused, naming the file, its size and the limit.
    std::string padded = kMarkerProgram;
    padded += "* " + std::string(2000U, 'x') + "\n";
    Result r = run(root / "over", {{"main.prg", padded}}, [](auto &options) { options.max_source_bytes = 1000U; });
    expect(!r.completed && !r.marker, "#5731: a source over the limit must not run: " + r.message);
    expect(mentions(r.message, {"main.prg", std::to_string(padded.size()), "1000"}),
        "#5731: the diagnostic should name the file, its size and the limit, got: " + r.message);

    // Exactly at the limit is accepted.
    std::string exact = kMarkerProgram;
    exact += "* " + std::string(1000U - exact.size() - 3U, 'y') + "\n";
    expect(exact.size() == 1000U, "fixture: exact-size program");
    r = run(root / "exact", {{"main.prg", exact}}, [](auto &options) { options.max_source_bytes = 1000U; });
    expect(r.completed && r.marker, "#5731: a source exactly at the limit must run: " + r.message);

    // An include is bounded by the same per-file limit.
    r = run(root / "include_over",
        {{"main.prg", "#INCLUDE \"big.h\"\n" + kMarkerProgram}, {"big.h", "* " + std::string(3000U, 'z') + "\n"}},
        [](auto &options) { options.max_source_bytes = 1000U; });
    expect(!r.completed && !r.marker && mentions(r.message, {"big.h", "1000"}),
        "#5731: an oversized include is refused with its name: " + r.message);
    fs::remove_all(root, ignored);
}

// The issue's reproduction: a sparse file far larger than the default limit. It is refused from its
// size, so nothing the size of the file is ever allocated.
void test_a_sparse_file_beyond_the_default_limit_is_refused_without_reading_it() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_source_limits_sparse";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "main.prg", kMarkerProgram);
    {
        // 4 GiB of apparent size, no data: a read-everything implementation would try to allocate it.
        const fs::path sparse = dir / "sparse.prg";
        write_text(sparse, "RETURN\n");
        std::error_code resize_error;
        fs::resize_file(sparse, 4ULL * 1024ULL * 1024ULL * 1024ULL, resize_error);
        if (resize_error) {
            std::cout << "sparse files are not supported here; skipping the sparse-file case\n";
            fs::remove_all(dir, ignored);
            return;
        }
        write_text(dir / "main.prg", "DO sparse.prg\n" + kMarkerProgram);
    }
    auto options = make_runtime_session_options((dir / "main.prg").string(), dir.string(), false);
    try {
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(!state.completed, "#5731: DO of a 4 GiB sparse source must not complete");
        // The size in the message proves it was refused by its size, not after being read in full.
        expect(mentions(state.message, {"sparse.prg", "4294967296"}),
            "#5731: the diagnostic names the sparse source and its size, got: " + state.message);
        expect(!fs::exists(dir / "marker.txt"), "#5731: nothing after the refused DO may run");
    } catch (const std::exception &error) {
        expect(mentions(error.what(), {"sparse.prg", "4294967296"}),
            std::string("#5731: the diagnostic names the sparse source and its size, got: ") + error.what());
    }
    fs::remove_all(dir, ignored);
}

void test_line_length_and_line_count_limits() {
    const fs::path root = fs::temp_directory_path() / "copperfin_source_limits_lines";
    std::error_code ignored;

    Result r = run(root / "long_line",
        {{"main.prg", "x = 1\n* " + std::string(2000U, 'a') + "\n" + kMarkerProgram}},
        [](auto &options) { options.max_logical_line_bytes = 1024U; });
    expect(!r.completed && !r.marker && mentions(r.message, {"main.prg", "2", "1024"}),
        "#5731: a line over the logical-line limit is refused with its line number: " + r.message);

    // A continued statement is one logical line, bounded as a whole.
    std::string continued = "x = 'a' + ;\n";
    for (int index = 0; index < 40; ++index) {
        continued += "'" + std::string(60U, 'b') + "' + ;\n";
    }
    continued += "'end'\n" + kMarkerProgram;
    r = run(root / "continued", {{"main.prg", continued}}, [](auto &options) { options.max_logical_line_bytes = 1024U; });
    expect(!r.completed && !r.marker && mentions(r.message, {"main.prg", "1024"}),
        "#5731: a long continued statement is bounded as one logical line: " + r.message);

    std::string many_lines;
    for (int index = 0; index < 100; ++index) {
        many_lines += "x" + std::to_string(index) + " = " + std::to_string(index) + "\n";
    }
    many_lines += kMarkerProgram;
    r = run(root / "many_lines", {{"main.prg", many_lines}}, [](auto &options) { options.max_source_lines = 50U; });
    expect(!r.completed && !r.marker && mentions(r.message, {"main.prg", "50"}),
        "#5731: a program with too many logical lines is refused: " + r.message);
    r = run(root / "lines_ok", {{"main.prg", many_lines}}, [](auto &options) { options.max_source_lines = 200U; });
    expect(r.completed && r.marker, "#5731: a program within the line limit runs: " + r.message);
    fs::remove_all(root, ignored);
}

void test_include_count_and_aggregate_limits() {
    const fs::path root = fs::temp_directory_path() / "copperfin_source_limits_includes";
    std::error_code ignored;

    // Many small includes: each is tiny, so only the count can stop this.
    Files files;
    std::string main_source;
    for (int index = 0; index < 20; ++index) {
        files.push_back({"inc" + std::to_string(index) + ".h", "#DEFINE C" + std::to_string(index) + " " + std::to_string(index) + "\n"});
        main_source += "#INCLUDE \"inc" + std::to_string(index) + ".h\"\n";
    }
    main_source += kMarkerProgram;
    files.push_back({"main.prg", main_source});
    Result r = run(root / "count_over", files, [](auto &options) { options.max_include_files = 10U; });
    expect(!r.completed && !r.marker && mentions(r.message, {"main.prg", "10"}),
        "#5731: more includes than the limit are refused: " + r.message);
    r = run(root / "count_ok", files, [](auto &options) { options.max_include_files = 20U; });
    expect(r.completed && r.marker, "#5731: exactly the allowed number of includes runs: " + r.message);

    // Aggregate: the program plus its includes together exceed the total, though each file is small.
    const std::string filler = "* " + std::string(500U, 'f') + "\n";
    r = run(root / "aggregate",
        {{"main.prg", "#INCLUDE \"a.h\"\n#INCLUDE \"b.h\"\n" + kMarkerProgram + filler}, {"a.h", filler}, {"b.h", filler}},
        [](auto &options) { options.max_source_bytes = 2000U; options.max_aggregate_source_bytes = 1200U; });
    expect(!r.completed && !r.marker && mentions(r.message, {"1200"}),
        "#5731: the program and its includes are bounded in total: " + r.message);
    fs::remove_all(root, ignored);
}

// Every program a session loads stays cached, so loaded sources are bounded in total too.
void test_session_aggregate_limit_applies_across_loaded_programs() {
    const fs::path root = fs::temp_directory_path() / "copperfin_source_limits_session";
    std::error_code ignored;
    const std::string filler = "* " + std::string(600U, 'g') + "\nRETURN\n";
    const std::string main_source = "DO other.prg\n" + kMarkerProgram;
    Result r = run(root, {{"main.prg", main_source}, {"other.prg", filler}},
        [&](auto &options) { options.max_aggregate_source_bytes = main_source.size() + 100U; });
    expect(!r.completed && !r.marker && mentions(r.message, {"other.prg"}),
        "#5731: loading another program past the session total is refused: " + r.message);
    r = run(root, {{"main.prg", main_source}, {"other.prg", filler}},
        [&](auto &options) { options.max_aggregate_source_bytes = main_source.size() + filler.size() + 100U; });
    expect(r.completed && r.marker, "#5731: loading within the session total works: " + r.message);
    fs::remove_all(root, ignored);
}

// The limits can be raised or lowered from CONFIG.FPW, like the other runtime ceilings.
void test_limits_are_configurable_from_config_fpw() {
    const fs::path root = fs::temp_directory_path() / "copperfin_source_limits_config";
    std::error_code ignored;
    std::string padded = kMarkerProgram + "* " + std::string(2000U, 'h') + "\n";
    Result r = run(root / "lowered", {{"main.prg", padded}, {"config.fpw", "MAXSOURCEBYTES = 1500\n"}}, [](auto &) {});
    expect(!r.completed && !r.marker && mentions(r.message, {"main.prg", "1500"}),
        "#5731: MAXSOURCEBYTES in CONFIG.FPW lowers the limit: " + r.message);
    r = run(root / "default", {{"main.prg", padded}}, [](auto &) {});
    expect(r.completed && r.marker, "#5731: the same program runs under the default limit: " + r.message);
    fs::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_oversized_source_is_refused_from_its_size();
    test_a_sparse_file_beyond_the_default_limit_is_refused_without_reading_it();
    test_line_length_and_line_count_limits();
    test_include_count_and_aggregate_limits();
    test_session_aggregate_limit_applies_across_loaded_programs();
    test_limits_are_configurable_from_config_fpw();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
