// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

#include <algorithm>

namespace cf_test_studio_host_json {

void test_studio_host_json_exposes_rushmore_explain_plan(const std::string& studio_host_path) {
    const std::vector<std::string> arguments{
        "--rushmore-explain",
        "--json",
        "--rushmore-cpu-units", "4",
        "--rushmore-memory-units", "1",
        "--rushmore-total-units", "5",
        "--rushmore-cursor", "people",
        "--rushmore-index-signature", "NAME:UPPER(NAME)",
        "--rushmore-row-count", "100",
        "--rushmore-stats-state", "fresh",
        "--rushmore-stats-version", "7",
        "--rushmore-stats-record-count", "100",
        "--rushmore-stats-record-length", "64",
        "--rushmore-stats-distinct-count", "80",
        "--rushmore-stats-density-ppm", "250000",
        "--rushmore-expression", "NAME = 'BRAVO'",
        "--rushmore-plan-kind", "index_seek",
        "--rushmore-index-name", "NAME",
        "--rushmore-indexable-predicate", "NAME = 'BRAVO'",
        "--rushmore-residual-predicate", "DELETED()",
        "--rushmore-selected", "true",
        "--rushmore-fallback-reason", "none",
        "--rushmore-options-version", "2"};
    const auto process = run_process_capture(studio_host_path, arguments, std::filesystem::current_path());
    if (process.exit_code != 0) {
        std::cerr << "rushmore explain stdout:\n" << process.stdout_text << "\n";
        std::cerr << "rushmore explain stderr:\n" << process.stderr_text << "\n";
    }
    expect(process.exit_code == 0, "#3237: Studio host Rushmore explain JSON should exit successfully");
    expect_contains(process.stdout_text, "\"rushmoreExplain\": {",
                    "#3237: host JSON should expose the Rushmore explain object");
    expect_contains(process.stdout_text, "\"cursorIdentity\": \"people\"",
                    "#3237: explain JSON should preserve cursor identity");
    expect_contains(process.stdout_text, "\"kind\": \"index_seek\"",
                    "#3237: explain JSON should preserve machine plan kind");
    expect_contains(process.stdout_text, "\"kindDisplay\": \"Index seek\"",
                    "#3237: explain JSON should expose localized plan-kind display text");
    expect_contains(process.stdout_text, "\"estimatedRows\": 0",
                    "#3237: explain JSON should preserve the runtime cost contract even when estimated rows are absent");
    expect_contains(process.stdout_text, "\"indexablePredicates\": [{\"normalizedExpression\": \"NAME = 'BRAVO'\"",
                    "#3237: explain JSON should expose indexable predicates");
    expect_contains(process.stdout_text, "\"residualPredicates\": [{\"normalizedExpression\": \"DELETED()\"",
                    "#3237: explain JSON should expose residual predicates");
    expect_contains(process.stdout_text, "\"selectedCandidate\": {",
                    "#3237: explain JSON should expose the selected candidate");
    expect_contains(process.stdout_text, "\"fallbackReason\": \"none\"",
                    "#3237: explain JSON should preserve fallback reason identity");
    expect_contains(process.stdout_text, "\"optionsVersion\": 2",
                    "#3237: explain JSON should preserve options version");

    auto text_arguments = arguments;
    text_arguments.erase(
        std::remove(text_arguments.begin(), text_arguments.end(), "--json"),
        text_arguments.end());
    const auto text_process = run_process_capture(
        studio_host_path,
        text_arguments,
        std::filesystem::current_path());
    expect(text_process.exit_code == 0,
           "#3237: human-readable Rushmore explain output should exit successfully");
    expect_contains(text_process.stdout_text, "status: ok",
                    "#3237: human-readable explain status should remain localized through en-US");
    expect_contains(text_process.stdout_text, "Rushmore explain plan",
                    "#3237: human-readable explain title should come from the catalog");

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_locale_process = run_process_capture(
        studio_host_path,
        arguments,
        std::filesystem::current_path());
    expect(pseudo_locale_process.exit_code == 0,
           "#3237: pseudo-locale Rushmore explain JSON should exit successfully");
    expect_contains(pseudo_locale_process.stdout_text, "\"kind\": \"index_seek\"",
                    "#3237: pseudo-locale must preserve machine plan identities");
    expect_contains(pseudo_locale_process.stdout_text, "\"kindDisplay\": \"[!! Îñðëx šëëƙ !!]\"",
        "#3237: pseudo-locale must route plan-kind display through the catalog");

    const auto pseudo_text_process = run_process_capture(
        studio_host_path,
        text_arguments,
        std::filesystem::current_path());
    expect(pseudo_text_process.exit_code == 0,
           "#3237: pseudo-locale human-readable explain output should exit successfully");
    expect_contains(pseudo_text_process.stdout_text, "[!! šţåţµš: øƙ !!]",
                    "#4378: pseudo-locale must route human-readable explain labels through the catalog");
    expect(
        pseudo_text_process.stdout_text.find("[!! [!!") == std::string::npos,
        "#4378: pseudo-locale human-readable explain output should not nest pseudo markers");
}

}  // namespace cf_test_studio_host_json
