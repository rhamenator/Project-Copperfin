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
    std::string count;   // RECCOUNT() after the import, caught so a rejection can be observed
    std::string rejected;   // "1" when APPEND FROM raised (and was caught), "0" when it succeeded
    std::string value;   // STR(N,12,4) of the first record when one exists
    std::string message;
};

// `type` is the APPEND FROM source kind: CSV (header line first), DELIMITED, or TAB (DELIMITED WITH TAB).
Outcome import_one(const fs::path &root, const std::string &name, const std::string &type,
                   const std::string &field_type, const std::string &schema, const std::string &text) {
    const fs::path dir = root / name;
    fs::create_directories(dir / "script");
    write_text(dir / "in.txt", (type == "CSV" ? std::string("N\r\n") : std::string()) + text + "\r\n");
    const std::string clause = type == "CSV" ? "TYPE CSV" : (type == "TAB" ? "TYPE DELIMITED WITH TAB" : "TYPE DELIMITED");
    write_text(dir / "script" / "import.prg",
        "CREATE CURSOR t (N " + field_type + schema + ")\n"
        "lRejected = .F.\n"
        "TRY\n"
        "APPEND FROM in.txt " + clause + "\n"
        "CATCH\n"
        "lRejected = .T.\n"
        "ENDTRY\n"
        "STRTOFILE(TRANSFORM(RECCOUNT()), 'count.txt')\n"
        "IF RECCOUNT() > 0\n"
        "GO TOP\n"
        "STRTOFILE(STR(N,12,4), 'result.txt')\n"
        "ENDIF\n"
        "STRTOFILE(IIF(lRejected,'1','0'), 'rejected.txt')\n"
        "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "import.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto slurp = [&](const char *file) {
        return fs::exists(dir / file) ? read_text(dir / file) : std::string{};
    };
    return {state.completed, slurp("count.txt"), slurp("rejected.txt"), slurp("result.txt"), state.message};
}

struct Case {
    std::string type;        // CSV, DELIMITED or TAB
    std::string field_type;  // N or F
    std::string schema;
    std::string text;
    std::string expected;    // STR(N,12,4); empty = rejected after rounding (policy: #6572)
};

void run_cases(const std::vector<Case> &cases) {
    const fs::path root = fs::temp_directory_path() / "copperfin_csv_numeric_scale";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    int index = 0;
    for (const Case &c : cases) {
        const Outcome outcome = import_one(root, "case" + std::to_string(++index), c.type, c.field_type, c.schema, c.text);
        const std::string label = "#6573 " + c.type + " " + c.field_type + c.schema + " [" + c.text + "]: ";
        expect(outcome.completed, label + "the script should finish (a rejection is caught): " + outcome.message);
        if (c.expected.empty()) {
            expect(outcome.rejected == "1",
                label + "a value still too wide after rounding must be rejected with an error, not silently skipped, got [" + outcome.rejected + "]");
            expect(outcome.count == "0", label + "a value still too wide after rounding must roll back, leaving 0 records, got [" + outcome.count + "]");
        } else {
            expect(outcome.rejected == "0", label + "the import must succeed without raising, got [" + outcome.rejected + "]");
            expect(outcome.count == "1", label + "the import should append one record, got [" + outcome.count + "]");
            expect(outcome.value == c.expected, label + "expected [" + c.expected + "], got [" + outcome.value + "]");
        }
    }
    fs::remove_all(root, ignored);
}

void test_numeric_text_rounds_to_the_field_scale() {
    std::vector<Case> cases;
    const auto both = [&](const std::string &schema, const std::string &text, const std::string &expected) {
        cases.push_back({"CSV", "N", schema, text, expected});
    };
    // Rounding into N(5,2): the issue's case and the rounding boundaries.
    both("(5,2)", "-1.235", "     -1.2400");
    both("(5,2)", "1.005", "      1.0100");
    both("(5,2)", "2.675", "      2.6800");
    both("(5,2)", "0.125", "      0.1300");
    both("(5,2)", "-0.004", "      0.0000");
    both("(5,2)", "0.004", "      0.0000");
    both("(5,2)", "-0.005", "     -0.0100");
    both("(5,2)", "1.999", "      2.0000");
    both("(5,2)", "12.345", "     12.3500");
    both("(5,2)", "1.235", "      1.2400");
    // A carry that lengthens the integer part still fits when the field is wide enough.
    both("(6,2)", "99.995", "    100.0000");
    both("(7,2)", "-99.995", "   -100.0000");
    both("(5,2)", "0.995", "      1.0000");
    both("(5,2)", "9.995", "     10.0000");
    both("(4,0)", "999.5", "   1000.0000");
    // Controls: values with no excess decimals are untouched.
    both("(5,2)", "1.5", "      1.5000");
    both("(5,2)", "007.5", "      7.5000");
    both("(5,2)", ".5", "      0.5000");
    both("(5,2)", "-.5", "     -0.5000");
    both("(5,2)", "+1.5", "      1.5000");
    // Integer fields round to whole numbers, half away from zero.
    both("(3,0)", "999", "    999.0000");
    both("(3,0)", "1.9", "      2.0000");
    both("(3,0)", "0.5", "      1.0000");
    both("(3,0)", "-0.5", "     -1.0000");
    both("(3,0)", "2.5", "      3.0000");
    both("(3,0)", "-2.5", "     -3.0000");
    both("(3,1)", "0.05", "      0.1000");
    both("(3,1)", "1.25", "      1.3000");
    // The leading zero of a fraction is dropped when that is what makes the value fit (VFP9 stores
    // N(3,2) 0.125 as .13, N(4,3) 0.125 as .125); a value with no excess decimals needs the same.
    both("(3,2)", "0.125", "      0.1300");
    both("(3,2)", "0.05", "      0.0500");
    both("(3,2)", "0.5", "      0.5000");
    both("(4,3)", "0.125", "      0.1250");
    both("(4,3)", "0.5", "      0.5000");
    both("(20,19)", "0.12345678901234567895", "      0.1235");
    // Still wider than the field after rounding: rejected and rolled back (#6572 policy).
    both("(5,2)", "99.995", "");
    both("(5,2)", "999.99", "");
    both("(5,2)", "-9.995", "");
    both("(3,0)", "1000", "");
    both("(3,0)", "-999", "");
    both("(3,1)", "-1.25", "");
    both("(3,1)", "9.95", "");
    both("(6,2)", "999.995", "");
    both("(3,2)", "-0.125", "");
    both("(3,2)", "0.995", "");

    // The same conversion applies to TYPE DELIMITED and TYPE TAB (VFP9 probe retained in
    // vfp9-result.txt) and to F (float) fields.
    for (const std::string type : {"DELIMITED", "TAB"}) {
        cases.push_back({type, "N", "(5,2)", "-1.235", "     -1.2400"});
        cases.push_back({type, "N", "(5,2)", "1.005", "      1.0100"});
        cases.push_back({type, "N", "(3,0)", "2.5", "      3.0000"});
        cases.push_back({type, "N", "(3,2)", "0.125", "      0.1300"});
        cases.push_back({type, "N", "(3,0)", "1000", ""});
        cases.push_back({type, "N", "(5,2)", "99.995", ""});
    }
    for (const std::string type : {"CSV", "DELIMITED"}) {
        cases.push_back({type, "F", "(8,2)", "-1.235", "     -1.2400"});
        cases.push_back({type, "F", "(8,2)", "1.005", "      1.0100"});
        cases.push_back({type, "F", "(5,2)", "0.125", "      0.1300"});
        cases.push_back({type, "F", "(3,0)", "1000", ""});
        cases.push_back({type, "F", "(5,2)", "999.99", ""});
    }
    // #6572, the issue's exact fixture: one over-wide row in the middle rejects the WHOLE import and
    // rolls it back, so not even the valid first row (999) is appended. VFP9 would silently store
    // 999, 0, 0, 0, 0, 2; the owner decision (2026-09-30) is to keep rejecting instead.
    cases.push_back({"CSV", "N", "(3,0)", "999\r\n1000\r\n-999\r\n-1000\r\n0\r\n1.9", ""});
    cases.push_back({"DELIMITED", "N", "(3,0)", "999\r\n1000\r\n-999\r\n-1000\r\n0\r\n1.9", ""});
    run_cases(cases);
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
