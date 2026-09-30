// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-TEXT-INTERCHANGE-WHITESPACE-001 (#6512, #6515).
//
// Every expectation below is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-09-30,
// retained at /home/rich/temp/vfp9-probes/csv-whitespace-6512/vfp9-result.txt).
//
// Import (APPEND FROM TYPE CSV and TYPE DELIMITED into a C(20) field): leading whitespace is
// data, whether the field is enclosed or not (an unquoted " Alice " and a tab-led field both keep
// their leading character); whitespace outside the enclosure of a quoted field is padding and is
// dropped. Export (COPY TO TYPE CSV / DELIMITED): a Character value is enclosed verbatim with its
// leading whitespace, trailing DBF padding is trimmed, and a blank value is an empty enclosure.

std::string rtrim_spaces(std::string text) {
    while (!text.empty() && text.back() == ' ') {
        text.pop_back();
    }
    return text;
}

struct ImportCase {
    std::string name;
    std::string line;              // the data line (a CSV header line is prepended for TYPE CSV)
    std::string expected_csv;      // NAME after the import, right-trimmed of DBF padding
    std::string expected_delimited;
};

std::string run_import(const fs::path &root, const std::string &name, const std::string &type, const std::string &content) {
    const fs::path dir = root / (name + "_" + type);
    fs::create_directories(dir / "script");
    write_text(dir / "in.txt", content);
    write_text(dir / "script" / "import.prg",
        "CREATE CURSOR t (NAME C(20))\n"
        "APPEND FROM in.txt TYPE " + type + "\n"
        "nCount = RECCOUNT()\n"
        "GO TOP\n"
        "STRTOFILE(NAME, 'result.txt')\n"
        "STRTOFILE(TRANSFORM(nCount), 'count.txt')\n"
        "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "import.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    if (!state.completed) {
        return "<incomplete: " + state.message + ">";
    }
    const std::string count = read_text(dir / "count.txt");
    if (count != "1") {
        return "<count " + count + ">";
    }
    return rtrim_spaces(read_text(dir / "result.txt"));
}

void test_import_whitespace_matches_vfp9() {
    const fs::path root = fs::temp_directory_path() / "copperfin_csv_whitespace_import";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    const std::vector<ImportCase> cases = {
        {"quoted_leading", "\" Alice \"", " Alice", " Alice"},
        {"unquoted_leading", " Alice ", " Alice", " Alice"},
        {"spaces_around_quoted", "  \"Alice\"  ", "Alice", "Alice"},
        {"empty_quoted", "\"\"", "", ""},
        {"quoted_single_space", "\" \"", "", ""},
        {"doubled_quote", "\"a\"\"b\"", "a\"\"b", "a"},
        {"quoted_lead_comma", "\"  lead,comma\"", "  lead,comma", "  lead,comma"},
        {"quoted_trailing", "\"trail  \"", "trail", "trail"},
        {"unquoted_tab_lead", "\tTab", "\tTab", "\tTab"},
    };
    for (const ImportCase &c : cases) {
        const std::string csv = run_import(root, c.name, "CSV", "NAME\r\n" + c.line + "\r\n");
        expect(csv == c.expected_csv, "#6512 CSV " + c.name + ": expected [" + c.expected_csv + "], got [" + csv + "]");
        const std::string delimited = run_import(root, c.name, "DELIMITED", c.line + "\r\n");
        expect(delimited == c.expected_delimited,
            "#6512 DELIMITED " + c.name + ": expected [" + c.expected_delimited + "], got [" + delimited + "]");
    }
    fs::remove_all(root, ignored);
}

std::string run_export(const fs::path &root, const std::string &name, const std::string &type,
                       const std::vector<std::string> &values) {
    const fs::path dir = root / (name + "_" + type);
    fs::create_directories(dir / "script");
    std::string body =
        "CREATE CURSOR t (NAME C(20), CODE N(3))\n";
    int code = 0;
    for (const std::string &value : values) {
        body += "APPEND BLANK\nREPLACE NAME WITH '" + value + "', CODE WITH " + std::to_string(++code) + "\n";
    }
    body += "COPY TO out.txt TYPE " + type + "\nRETURN\n";
    write_text(dir / "script" / "export.prg", body);
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "export.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    if (!state.completed) {
        return "<incomplete: " + state.message + ">";
    }
    return read_text(dir / "out.txt");
}

void test_export_whitespace_matches_vfp9() {
    const fs::path root = fs::temp_directory_path() / "copperfin_csv_whitespace_export";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    struct ExportCase {
        std::string name;
        std::vector<std::string> values;
        std::string rows;   // body rows shared by CSV and DELIMITED
    };
    const std::vector<ExportCase> cases = {
        {"leading_space", {" Alice "}, "\" Alice\",1\r\n"},
        {"blank_and_space", {"", " "}, "\"\",1\r\n\"\",2\r\n"},
        {"embedded_comma", {"a,b"}, "\"a,b\",1\r\n"},
        {"two_leading", {"  two"}, "\"  two\",1\r\n"},
        {"trailing_spaces", {"trail  "}, "\"trail\",1\r\n"},
        {"multi_rows", {" a", "b ", " c "}, "\" a\",1\r\n\"b\",2\r\n\" c\",3\r\n"},
    };
    for (const ExportCase &c : cases) {
        const std::string csv = run_export(root, c.name, "CSV", c.values);
        expect(csv == "name,code\r\n" + c.rows, "#6515 CSV " + c.name + ": got [" + csv + "]");
        const std::string delimited = run_export(root, c.name, "DELIMITED", c.values);
        expect(delimited == c.rows, "#6515 DELIMITED " + c.name + ": got [" + delimited + "]");
    }
    fs::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_import_whitespace_matches_vfp9();
    test_export_whitespace_matches_vfp9();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
