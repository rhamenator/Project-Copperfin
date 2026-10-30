// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001:
// Actual native complete-expression expectations, frozen before production changes.
#include "copperfin/runtime/prg_engine.h"
#include "fixtures/vfp9-numeric-division-context/nonwhole-public-expected.h"
#include "fixtures/vfp9-numeric-division-context/nonwhole-timing-expected.h"

#include <bit>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
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
                ("numeric-nonwhole-division-" + std::to_string(stamp) + "-" + std::to_string(i));
            if (std::filesystem::create_directory(candidate)) {
                path = std::move(candidate);
                return;
            }
        }
        throw std::runtime_error("Cannot create exclusive non-whole division test directory");
    }
    ~OwnedDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
};
std::string hex(std::uint64_t bits) {
    std::ostringstream out;
    out << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << bits;
    return out.str();
}
}

int main() {
    using namespace copperfin::runtime;
    unsigned failures = 0, descriptorFailures = 0, bitFailures = 0;
    unsigned literalBitFailures = 0, evaluationFailures = 0, controlFailures = 0;
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
                    options.startup_path = (directory.path / "nonwhole-division.prg").string();
                    options.working_directory = directory.path.string();
                    options.temp_directory = (directory.path / "runtime-temp").string();
                    options.require_source_text_overrides = true;
                    options.startup_source_text = std::string("SET NUMERICBEHAVIOR TO ") + mode +
                        "\nSET DECIMALS TO " + std::to_string(settings[d]) + "\nSET FIXED " + fixed +
                        "\nLOCAL iTwo,nTwo,nPause\niTwo=2\nnTwo=2.00\nnPause=0\nnPause=1\nRETURN\n";
                    const auto context = std::string(mode) + "/D" + std::to_string(settings[d]) +
                        "/FIXED" + fixed;
                    auto session = PrgRuntimeSession::create(options);
                    session.add_breakpoint({options.startup_path, 8});
                    const auto paused = session.run(DebugResumeAction::continue_run);
                    expect(paused.reason == DebugPauseReason::breakpoint,
                           context + " paused " + paused.message, controlFailures);
                    if (paused.reason != DebugPauseReason::breakpoint) continue;
                    unsigned selected = 0, strengthened = 0, retainedGaps = 0;
                    for (const auto& row : copperfin::test::native_nonwhole_division_rows) {
                        const std::string rowLabel = row.label;
                        const bool smallDivisorPrerequisite =
                            rowLabel == "retained-outside-scope:575" ||
                            rowLabel == "retained-outside-scope:576";
                        if (row.admitted) ++selected;
                        else if (smallDivisorPrerequisite) ++strengthened;
                        else ++retainedGaps;
                        const auto result = session.evaluate_watch_expression(row.expression);
                        const auto label = context + " " + row.label + " " + row.expression;
                        expect(result.ok, label + " evaluates " + result.message, evaluationFailures);
                        if (!result.ok) continue;
                        const auto& value = result.value;
                        expect(value.kind == PrgValueKind::number && !value.is_null,
                               label + " ordinary Number", evaluationFailures);
                        if (value.kind != PrgValueKind::number || value.is_null) continue;
                        if (row.admitted || smallDivisorPrerequisite) {
                            // The two former small-divisor controls are now inside
                            // the separately frozen/admitted prerequisite; strengthen
                            // their existing native expectations instead of deleting
                            // or weakening the assertions.
                            expect(value.numeric_metadata == row.metadata[d],
                                   label + " native descriptor", descriptorFailures);
                            const auto actual = std::bit_cast<std::uint64_t>(value.number_value);
                            if (actual != row.bits && std::string(row.label).starts_with("literal-control:"))
                                ++literalBitFailures;
                            expect(actual == row.bits, label + " bits actual=" + hex(actual) +
                                   " native=" + hex(row.bits), bitFailures);
                        } else {
                            expect(!value.numeric_metadata,
                                   label + " retained unknown producer", controlFailures);
                        }
                    }
                    expect(selected == 574 && strengthened == 2 && retainedGaps == 2,
                           context + " exact frozen plus strengthened frontier inventory", controlFailures);
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

                    unsigned callbackCases = 0;
                    for (const auto& row : copperfin::test::native_nonwhole_timing_rows) {
                        if (!row.callback_admitted) continue;
                        ++callbackCases;
                        RuntimeSessionOptions callbackOptions;
                        callbackOptions.startup_path = (directory.path / "nonwhole-callback.prg").string();
                        callbackOptions.working_directory = directory.path.string();
                        callbackOptions.temp_directory = (directory.path / "callback-temp").string();
                        callbackOptions.require_source_text_overrides = true;
                        callbackOptions.startup_source_text = std::string("SET NUMERICBEHAVIOR TO ") + mode +
                            "\nSET DECIMALS TO " + std::to_string(settings[d]) + "\nSET FIXED " + fixed +
                            "\nPUBLIC gCalls\ngCalls=0\nanswer=" + row.expression +
                            "\nfinalDecimals=VAL(TRANSFORM(SET('DECIMALS')))\nRETURN\nFUNCTION CFMark\nLPARAMETERS nextSetting"
                            "\ngCalls=gCalls+1\nSET DECIMALS TO nextSetting\nRETURN 2.00\nENDFUNC\n";
                        const auto label = context + " callback " + row.expression;
                        auto callback = PrgRuntimeSession::create(callbackOptions);
                        // Pause after the increment but before settings change; resuming
                        // must restore completed constant-prefix precision, not replay it.
                        callback.add_breakpoint({callbackOptions.startup_path, 12});
                        const auto pause = callback.run(DebugResumeAction::continue_run);
                        expect(pause.reason == DebugPauseReason::breakpoint,
                               label + " paused " + pause.message, controlFailures);
                        const auto pausedCalls = pause.globals.find("gcalls");
                        expect(pausedCalls != pause.globals.end() && pausedCalls->second.number_value == 1,
                               label + " callback increment occurs once before pause", controlFailures);
                        const auto done = callback.run(DebugResumeAction::continue_run);
                        expect(done.completed, label + " resumed " + done.message, controlFailures);
                        const auto calls = done.globals.find("gcalls");
                        const auto answer = done.globals.find("answer");
                        const auto finalSetting = done.globals.find("finaldecimals");
                        expect(calls != done.globals.end() && calls->second.number_value == 1,
                               label + " exactly one callback after continuation", controlFailures);
                        // #7208 tracks SET query result-type parity separately. Observe
                        // exact setting state through the independently verified adapter.
                        expect(finalSetting != done.globals.end() && finalSetting->second.kind == PrgValueKind::number &&
                                   finalSetting->second.number_value == row.changes_to,
                               label + " native final setting", controlFailures);
                        expect(answer != done.globals.end() && answer->second.kind == PrgValueKind::number &&
                                   !answer->second.is_null, label + " ordinary assigned Number", evaluationFailures);
                        if (answer == done.globals.end() || answer->second.kind != PrgValueKind::number ||
                            answer->second.is_null) continue;
                        expect(std::bit_cast<std::uint64_t>(answer->second.number_value) == row.bits,
                               label + " native quotient bits", bitFailures);
                        const auto scale = row.metadata[d].decimal_places;
                        // Separate native-backed RQ-CF-PRG-NUMERIC-REAL-ASSIGNMENT-001:
                        // these finite nonzero real results all have scale2..18.
                        const auto nativePayload = std::bit_cast<double>(row.bits);
                        expect(scale >= 2U && scale <= 18U && std::isfinite(nativePayload) && nativePayload != 0.0,
                               label + " frozen assignment frontier", controlFailures);
                        const PrgNumericMetadata assigned{11U + scale + (nativePayload < 0.0 ? 1U : 0U),
                                                         scale, PrgNumericStorage::real};
                        expect(answer->second.numeric_metadata == assigned,
                               label + " native precision survives pause and scalar assignment", descriptorFailures);
                    }
                    expect(callbackCases == 72, context + " complete frozen callback inventory", controlFailures);
                }
            }
        }
    } catch (const std::exception& error) {
        expect(false, error.what(), controlFailures);
    }
    std::cout << "non-whole division: descriptor=" << descriptorFailures << " bits=" << bitFailures
              << " literalBits=" << literalBitFailures << " evaluation=" << evaluationFailures
              << " controls=" << controlFailures << " total=" << failures << '\n';
    return failures ? 1 : 0;
}
