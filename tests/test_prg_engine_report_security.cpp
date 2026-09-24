// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace {

using namespace copperfin::test_support;
namespace fs = std::filesystem;

void write_report_fixture(const fs::path& asset_path, const std::string& detail_expression = "NAME") {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "200", "detail-header-guid"},
        {"8", "", detail_expression, "100", "20", "700", "100", "name-field-guid"}
    };
    const auto result = copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
    expect(result.ok, "synthetic report/label asset fixture should be created");
}

void test_strict_report_and_label_use_admitted_bytes_without_physical_paths() {
    const fs::path root = fs::temp_directory_path() / "copperfin_report_security_absent_paths";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    for (const auto& [command, extension, memo_extension, event_category] : {
             std::tuple<std::string, std::string, std::string, std::string>{"REPORT FORM", ".frx", ".frt", "report"},
             {"LABEL FORM", ".lbx", ".lbt", "label"}}) {
        const fs::path asset_path = root / ("admitted" + extension);
        const fs::path memo_path = root / ("admitted" + memo_extension);
        const fs::path main_path = root / ("main" + extension + ".prg");
        const fs::path output_path = root / ("rendered" + extension + ".txt");
        write_report_fixture(asset_path);
        const std::string asset_bytes = read_text(asset_path);
        const std::string memo_bytes = read_text(memo_path);
        write_text(main_path,
                   command + " '" + asset_path.string() + "' TO FILE '" + output_path.string() + "'\n"
                   "RETURN\n");
        fs::remove(asset_path, ignored);
        fs::remove(memo_path, ignored);

        auto options = make_runtime_session_options(main_path, root);
        options.verified_file_byte_overrides.emplace(asset_path.string(), asset_bytes);
        options.verified_file_byte_overrides.emplace(memo_path.string(), memo_bytes);
        options.require_verified_file_byte_overrides = true;
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               command + " should use admitted primary and memo bytes without physical files: " + state.message);
        expect(fs::exists(output_path), command + " should render from the verified snapshot");
        expect(has_runtime_event(state.events, event_category + ".render", output_path.string() + " rows=0"),
               command + " should retain its normal render event");

        auto missing_options = make_runtime_session_options(main_path, root);
        missing_options.require_verified_file_byte_overrides = true;
        auto missing_session = copperfin::runtime::PrgRuntimeSession::create(missing_options);
        const auto missing_state = missing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(missing_state.reason == copperfin::runtime::DebugPauseReason::error &&
                   missing_state.message.find("Verified package bytes are unavailable") != std::string::npos,
               command + " should fail closed when admitted bytes are missing: " + missing_state.message);
        fs::remove(output_path, ignored);
    }

    fs::remove_all(root, ignored);
}

// #6240: report/label row rendering kept moving, reading, and restoring the
// active cursor after a WHILE, FOR/filter, or FRX/LBX object expression had
// closed it. The command must fail catchably, stop evaluating, leave any
// existing destination untouched, and emit no render event.
void test_report_expression_closing_cursor_fails_catchably() {
    const fs::path root = fs::temp_directory_path() / "copperfin_report_6240";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    const auto active_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    const auto global_text = [](const auto& state, const std::string& name) -> std::string {
        const auto found = state.globals.find(name);
        return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
    };

    struct Scenario {
        std::string label;
        std::string detail_expression;
        std::string clauses;  // FOR/WHILE clauses placed before TO FILE
        int drop_on_call = 1;
        bool reopen = false;
        bool filter = false;
    };
    const std::vector<Scenario> scenarios = {
        {"object_first", "DropCursor()", "", 1},
        {"object_last_reopen", "DropCursor()", "", 2, true},
        {"for_first", "NAME", " FOR DropCursor()", 1},
        {"while_last", "NAME", " WHILE DropCursor()", 2},
        {"filter_first_reopen", "NAME", "", 1, true, true},
    };

    for (const auto& [command, extension, category] : {
             std::tuple<std::string, std::string, std::string>{"REPORT FORM", ".frx", "report"},
             {"LABEL FORM", ".lbx", "label"}}) {
        for (const auto& scenario : scenarios) {
            const std::string name = category + "_" + scenario.label;
            const fs::path asset_path = root / (name + extension);
            const fs::path output_path = root / (name + ".txt");
            const fs::path main_path = root / (name + ".prg");
            write_report_fixture(asset_path, scenario.detail_expression);
            write_text(output_path, "previous output");
            write_text(
                main_path,
                "nCalls = 0\n"
                "nDropOnCall = " + std::to_string(scenario.drop_on_call) + "\n"
                "lReopen = " + (scenario.reopen ? ".T." : ".F.") + "\n"
                "lErrorCaught = .F.\n"
                "cErrMsg = ''\n"
                "CREATE CURSOR Source (NAME C(10))\n"
                "INSERT INTO Source VALUES ('ALPHA')\n"
                "INSERT INTO Source VALUES ('BRAVO')\n"
                "SELECT Source\n"
                "GO TOP\n" +
                std::string(scenario.filter ? "SET FILTER TO DropCursor()\n" : "") +
                "TRY\n"
                "    " + command + " '" + asset_path.string() + "'" + scenario.clauses +
                " TO FILE '" + output_path.string() + "'\n"
                "CATCH TO oErr\n"
                "    lErrorCaught = .T.\n"
                "    cErrMsg = oErr.Message\n"
                "ENDTRY\n"
                "lSourceOpen = USED('Source')\n"
                "nReplacementRecno = IIF(USED('Replacement'), RECNO('Replacement'), -1)\n"
                "lAfter = .T.\n"
                "RETURN\n"
                "FUNCTION DropCursor\n"
                "    nCalls = nCalls + 1\n"
                "    IF nCalls = nDropOnCall\n"
                "        USE IN Source\n"
                "        IF lReopen\n"
                "            CREATE CURSOR Replacement (NAME C(10))\n"
                "            INSERT INTO Replacement VALUES ('R1')\n"
                "            INSERT INTO Replacement VALUES ('R2')\n"
                "            GO TOP IN Replacement\n"
                "        ENDIF\n"
                "    ENDIF\n"
                "    RETURN .T.\n"
                "ENDFUNC\n");

            auto session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, root));
            const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            const std::string prefix = "#6240 " + name + ": ";
            expect(state.completed, prefix + "script should complete: " + state.message);
            expect(global_text(state, "lafter") == "true", prefix + "execution should continue after the command");
            const std::string expected_message = active_catalog.translate(
                "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", {{"command", command}});
            expect(global_text(state, "lerrorcaught") == "true" &&
                       global_text(state, "cerrmsg").find(expected_message) != std::string::npos,
                   prefix + "closing the report cursor should raise the catchable " + command + " error, got: " +
                       global_text(state, "cerrmsg"));
            expect(global_text(state, "ncalls") == std::to_string(scenario.drop_on_call),
                   prefix + "evaluation should stop at the closing call, got calls: " + global_text(state, "ncalls"));
            expect(global_text(state, "lsourceopen") == "false", prefix + "the closed cursor should stay closed");
            expect(read_text(output_path) == "previous output",
                   prefix + "the existing destination must not be truncated or partially written");
            if (scenario.reopen) {
                expect(global_text(state, "nreplacementrecno") == "1",
                       prefix + "the replacement cursor must keep its own position, got RECNO " +
                           global_text(state, "nreplacementrecno"));
            }
            const auto render_events = std::count_if(state.events.begin(), state.events.end(), [&](const auto& event) {
                return event.category == category + ".render";
            });
            expect(render_events == 0, prefix + "the failed render must not emit " + category + ".render");
        }
    }

    fs::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_strict_report_and_label_use_admitted_bytes_without_physical_paths();
    test_report_expression_closing_cursor_fails_catchably();
    return test_failures() == 0 ? 0 : 1;
}
