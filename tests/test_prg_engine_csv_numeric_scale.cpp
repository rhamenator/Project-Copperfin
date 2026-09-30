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

// Governing requirement: RQ-CF-PRG-CSV-NUMERIC-SCALE-001 (#6573; #6572 keeps the overflow policy).
//
// Installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-09-30, retained at
// /home/rich/temp/vfp9-probes/csv-numeric-round-6573/vfp9-result.txt) rounds decimal text half
// away from zero, digit by digit, to the field scale when APPEND FROM TYPE CSV loads a numeric
// field: N(5,2) takes -1.235 as -1.24, 1.005 as 1.01, 2.675 as 2.68, 0.125 as 0.13, and -0.004 as
// 0; N(3,0) takes 1.9 as 2, 0.5 as 1 and -2.5 as -3. A value that is still wider than the field
// after rounding (VFP9 silently drops decimals or stores 0) keeps Copperfin's documented rejection
// and rollback, which is the open owner decision in #6572.

struct Outcome {
    bool completed;
    std::string value;   // STR(N,12,4) of the first record
    std::string message;
};

Outcome import_one(const fs::path &root, const std::string &name, const std::string &schema, const std::string &text) {
    const fs::path dir = root / name;
    fs::create_directories(dir / "script");
    write_text(dir / "in.csv", "N\r\n" + text + "\r\n");
    write_text(dir / "script" / "import.prg",
        "CREATE CURSOR t (N N" + schema + ")\n"
        "APPEND FROM in.csv TYPE CSV\n"
        "GO TOP\n"
        "STRTOFILE(STR(N,12,4), 'result.txt')\n"
        "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "import.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const fs::path result = dir / "result.txt";
    return {state.completed, fs::exists(result) ? read_text(result) : std::string{}, state.message};
}

void test_numeric_text_rounds_to_the_field_scale() {
    const fs::path root = fs::temp_directory_path() / "copperfin_csv_numeric_scale";
    std::error_code ignored;
    fs::remove_all(root, ignored);

    struct Case {
        std::string schema;
        std::string text;
        std::string expected;   // STR(N,12,4); empty = rejected after rounding (policy: #6572)
    };
    const std::vector<Case> cases = {
        // Rounding into N(5,2): the issue's case and the rounding boundaries.
        {"(5,2)", "-1.235", "     -1.2400"},
        {"(5,2)", "1.005", "      1.0100"},
        {"(5,2)", "2.675", "      2.6800"},
        {"(5,2)", "0.125", "      0.1300"},
        {"(5,2)", "-0.004", "      0.0000"},
        {"(5,2)", "0.004", "      0.0000"},
        {"(5,2)", "-0.005", "     -0.0100"},
        {"(5,2)", "1.999", "      2.0000"},
        {"(5,2)", "12.345", "     12.3500"},
        {"(5,2)", "1.235", "      1.2400"},
        // A carry that lengthens the integer part still fits when the field is wide enough.
        {"(6,2)", "99.995", "    100.0000"},
        {"(7,2)", "-99.995", "   -100.0000"},
        {"(5,2)", "0.995", "      1.0000"},
        {"(5,2)", "9.995", "     10.0000"},
        {"(4,0)", "999.5", "   1000.0000"},
        // Controls: values with no excess decimals are untouched.
        {"(5,2)", "1.5", "      1.5000"},
        {"(5,2)", "007.5", "      7.5000"},
        {"(5,2)", ".5", "      0.5000"},
        {"(5,2)", "-.5", "     -0.5000"},
        {"(5,2)", "+1.5", "      1.5000"},
        // Integer fields round to whole numbers, half away from zero.
        {"(3,0)", "999", "    999.0000"},
        {"(3,0)", "1.9", "      2.0000"},
        {"(3,0)", "0.5", "      1.0000"},
        {"(3,0)", "-0.5", "     -1.0000"},
        {"(3,0)", "2.5", "      3.0000"},
        {"(3,0)", "-2.5", "     -3.0000"},
        {"(3,1)", "0.05", "      0.1000"},
        {"(3,1)", "1.25", "      1.3000"},
        // Still wider than the field after rounding: rejected and rolled back (#6572 policy).
        {"(5,2)", "99.995", ""},
        {"(5,2)", "999.99", ""},
        {"(5,2)", "-9.995", ""},
        {"(3,0)", "1000", ""},
        {"(3,0)", "-999", ""},
        {"(3,1)", "-1.25", ""},
        {"(3,1)", "9.95", ""},
        {"(6,2)", "999.995", ""},
    };
    int index = 0;
    for (const Case &c : cases) {
        const Outcome outcome = import_one(root, "case" + std::to_string(++index), c.schema, c.text);
        const std::string label = "#6573 N" + c.schema + " [" + c.text + "]: ";
        if (c.expected.empty()) {
            expect(!outcome.completed, label + "a value still too wide after rounding must be rejected");
            expect(outcome.value.empty(), label + "a rejected import must leave no record");
        } else {
            expect(outcome.completed, label + "import should complete: " + outcome.message);
            expect(outcome.value == c.expected, label + "expected [" + c.expected + "], got [" + outcome.value + "]");
        }
    }
    fs::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_numeric_text_rounds_to_the_field_scale();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
