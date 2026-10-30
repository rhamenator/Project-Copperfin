// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-MATERIALIZATION-001:
// independently captured native descriptors/bits, frozen BEFORE parser hooks.
#include "copperfin/runtime/prg_engine.h"
#include "fixtures/vfp9-numeric-division-context/constant-public-expected.h"
#include "fixtures/vfp9-numeric-division-context/nonwhole-timing-expected.h"
#include "fixtures/vfp9-numeric-division-context/small-divisor-public-expected.h"

#include <bit>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
struct OwnedDirectory {
    std::filesystem::path path;
    OwnedDirectory() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (unsigned i = 0; i < 100; ++i) {
            auto candidate = std::filesystem::current_path() /
                ("numeric-constant-division-" + std::to_string(stamp) + "-" + std::to_string(i));
            if (std::filesystem::create_directory(candidate)) {
                path = std::move(candidate);
                return;
            }
        }
        throw std::runtime_error("Cannot create exclusive constant division test directory");
    }
    ~OwnedDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
};
}

int main() {
    using namespace copperfin::runtime;
    unsigned failures = 0, descriptorFailures = 0, bitFailures = 0;
    unsigned evaluationFailures = 0, controlFailures = 0;
    const auto expect = [&](bool ok, const std::string& label, unsigned& category) {
        if (!ok) {
            ++failures;
            ++category;
            std::cerr << "FAIL: " << label << '\n';
        }
    };
    try {
        OwnedDirectory directory;
        const int settings[] = {0, 2, 6, 14, 18};
        for (const auto mode : {"COPPERFIN", "VFP9"}) {
            for (std::size_t d = 0; d < 5; ++d) {
                for (const auto fixed : {"OFF", "ON"}) {
                    RuntimeSessionOptions options;
                    options.startup_path = (directory.path / "constant-division.prg").string();
                    options.working_directory = directory.path.string();
                    options.temp_directory = (directory.path / "runtime-temp").string();
                    options.require_source_text_overrides = true;
                    options.startup_source_text = std::string("SET NUMERICBEHAVIOR TO ") + mode +
                        "\nSET DECIMALS TO " + std::to_string(settings[d]) + "\nSET FIXED " + fixed +
                        "\nLOCAL iFive,iTwo,nPause\niFive=5\niTwo=2\nnPause=0\nnPause=1\nRETURN\n";
                    const auto context = std::string(mode) + "/D" + std::to_string(settings[d]) +
                        "/FIXED" + fixed;
                    auto session = PrgRuntimeSession::create(options);
                    session.add_breakpoint({options.startup_path, 8});
                    const auto paused = session.run(DebugResumeAction::continue_run);
                    expect(paused.reason == DebugPauseReason::breakpoint,
                           context + " paused " + paused.message, controlFailures);
                    if (paused.reason != DebugPauseReason::breakpoint) continue;
                    unsigned selected = 0, retainedGaps = 0;
                    for (const auto& row : copperfin::test::native_constant_public_rows) {
                        const auto result = session.evaluate_watch_expression(row.expression);
                        const auto label = context + " " + row.label + " " + row.expression;
                        expect(result.ok, label + " evaluates " + result.message, evaluationFailures);
                        if (!result.ok) continue;
                        const auto& value = result.value;
                        expect(value.kind == PrgValueKind::number && !value.is_null,
                               label + " ordinary Number", evaluationFailures);
                        if (row.known_literal_producers) {
                            ++selected;
                            expect(value.numeric_metadata == row.metadata[d],
                                   label + " native descriptor", descriptorFailures);
                            expect(std::bit_cast<std::uint64_t>(value.number_value) == row.bits,
                                   label + " native bits", bitFailures);
                        } else {
                            ++retainedGaps;
                            // Native expectations remain in the header; this is not parity.
                            expect(!value.numeric_metadata,
                                   label + " retained unknown producer", controlFailures);
                        }
                    }
                    expect(selected == 798 && retainedGaps == 48,
                           context + " exact frozen frontier inventory", controlFailures);
                    unsigned newlyAdmitted = 0;
                    for (const auto& row : copperfin::test::native_nonwhole_timing_rows) {
                        if (!row.old_unknown_control) continue;
                        ++newlyAdmitted;
                        const auto result = session.evaluate_watch_expression(row.expression);
                        const auto label = context + " newly admitted " + row.expression;
                        expect(result.ok && result.value.kind == PrgValueKind::number && !result.value.is_null,
                               label + " evaluates ordinary Number", evaluationFailures);
                        if (!result.ok || result.value.kind != PrgValueKind::number || result.value.is_null) continue;
                        expect(result.value.numeric_metadata == row.metadata[d],
                               label + " native descriptor", descriptorFailures);
                        expect(std::bit_cast<std::uint64_t>(result.value.number_value) == row.bits,
                               label + " native bits", bitFailures);
                    }
                    expect(newlyAdmitted == 3, context + " all three old controls strengthened", controlFailures);
                    const auto smallDivisor = session.evaluate_watch_expression("5/0.5");
                    expect(smallDivisor.ok && smallDivisor.value.kind == PrgValueKind::number &&
                               !smallDivisor.value.is_null &&
                               smallDivisor.value.numeric_metadata ==
                                   copperfin::test::native_small_divisor_rows[0].metadata[d] &&
                               std::bit_cast<std::uint64_t>(smallDivisor.value.number_value) ==
                                   copperfin::test::native_small_divisor_rows[0].bits,
                           context + " superseded small-divisor control strengthened", controlFailures);
                    for (const auto expression : {
                             "1E300/iTwo", "iFive/(1E300)", "(iFive*2)/iTwo",
                             "((5/2)*1)/iTwo", "((5/2)+0)/iTwo", "2^(5/2)/iTwo"}) {
                        const auto result = session.evaluate_watch_expression(expression);
                        expect(result.ok && !result.value.numeric_metadata,
                               context + " unsupported stays unknown: " + expression, controlFailures);
                    }
                    expect(session.evaluate_watch_expression(".NULL./(5/2)").value.is_null,
                           context + " NULL unchanged", controlFailures);
                    expect(!session.evaluate_watch_expression("(5/2)/.T.").ok,
                           context + " invalid type unchanged", controlFailures);
                    expect(!session.evaluate_watch_expression("5/(2-2)").ok,
                           context + " zero divisor unchanged", controlFailures);
                    const auto currency = session.evaluate_watch_expression("$5/(10/2)");
                    expect(currency.ok && currency.value.kind == PrgValueKind::currency &&
                               currency.value.currency_value == 10000,
                           context + " Currency unchanged", controlFailures);
                    const auto suppressed = session.evaluate_watch_expression("IIF(.T.,5,5/0)");
                    expect(suppressed.ok && suppressed.value.number_value == 5,
                           context + " suppressed zero divisor unchanged", controlFailures);
                    const auto& after = session.state();
                    expect(after.reason == paused.reason &&
                               after.executed_statement_count == paused.executed_statement_count &&
                               after.statement_text == paused.statement_text &&
                               after.globals.size() == paused.globals.size(),
                           context + " watch state unchanged", controlFailures);
                    expect(session.run(DebugResumeAction::continue_run).completed,
                           context + " resumed", controlFailures);
                }
            }
            // Native constant-effects probe: evaluate left at D14, then mutate to D0
            // in the right callback. Normal scalar assignment can normalize width;
            // require the native retained scale, payload and exactly-once effect.
            for (const auto expression : {"(5/2)/LaterReal()", "5/2/LaterReal()",
                                          "(10/2)/2/LaterReal()"}) {
                const auto callbackContext = std::string(mode) + " " + expression;
                RuntimeSessionOptions options;
                options.startup_path = (directory.path / "constant-division-continuation.prg").string();
                options.working_directory = directory.path.string();
                options.temp_directory = (directory.path / "runtime-temp").string();
                options.require_source_text_overrides = true;
                options.startup_source_text = std::string("SET NUMERICBEHAVIOR TO ") + mode +
                    "\nSET DECIMALS TO 14\nPUBLIC callCount\ncallCount=0\nanswer=" + expression +
                    "\nfinalDecimals=VAL(TRANSFORM(SET('DECIMALS')))\nRETURN\nFUNCTION LaterReal\ncallCount=callCount+1"
                    "\nSET DECIMALS TO 0\nRETURN 2.00\nENDFUNC\n";
                auto replay = PrgRuntimeSession::create(options);
                // A real pause inside the callback forces continuation/checkpoint recovery.
                replay.add_breakpoint({options.startup_path, 10});
                const auto callbackPause = replay.run(DebugResumeAction::continue_run);
                expect(callbackPause.reason == DebugPauseReason::breakpoint,
                       callbackContext + " callback paused " + callbackPause.message, controlFailures);
                const auto done = replay.run(DebugResumeAction::continue_run);
                expect(done.completed, callbackContext + " continuation completed " + done.message,
                       controlFailures);
                const auto calls = done.globals.find("callcount");
                const auto answer = done.globals.find("answer");
                const auto finalSetting = done.globals.find("finaldecimals");
                expect(calls != done.globals.end() && calls->second.number_value == 1,
                       callbackContext + " callback exactly once", controlFailures);
                expect(answer != done.globals.end() && answer->second.kind == PrgValueKind::number &&
                           answer->second.number_value == 1.25,
                       callbackContext + " continuation quotient", controlFailures);
                expect(answer != done.globals.end() && answer->second.numeric_metadata &&
                           answer->second.numeric_metadata->decimal_places == 14 &&
                           answer->second.numeric_metadata->storage == PrgNumericStorage::real,
                       callbackContext + " prepared precision survives callback", descriptorFailures);
                expect(finalSetting != done.globals.end() && finalSetting->second.kind == PrgValueKind::number &&
                           finalSetting->second.number_value == 0,
                       callbackContext + " callback mutation persists", controlFailures);
            }
        }
    } catch (const std::exception& error) {
        expect(false, error.what(), controlFailures);
    }
    std::cout << "constant division: descriptor=" << descriptorFailures << " bits=" << bitFailures
              << " evaluation=" << evaluationFailures << " controls=" << controlFailures
              << " total=" << failures << '\n';
    return failures ? 1 : 0;
}
