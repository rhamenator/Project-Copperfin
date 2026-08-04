// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <limits>
#include <system_error>

void test_rushmore_planning_contracts();

namespace {

using namespace copperfin::test_support;

bool has_rushmore_event_with_detail_fragment(
    const std::vector<copperfin::runtime::RuntimeEvent>& events,
    const std::string& fragment) {
    return std::any_of(events.begin(), events.end(), [&](const copperfin::runtime::RuntimeEvent& event) {
        return event.category == "runtime.rushmore" && event.detail.find(fragment) != std::string::npos;
    });
}

void test_locate_uses_rushmore_seek_and_restores_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rushmore_locate";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "rushmore_locate.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "cOrderBefore = ORDER()\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cOrderAfter = ORDER()\n"
        "lFound = FOUND()\n"
        "nRecno = RECNO()\n"
        "RETURN\n");

    auto runtime_options = make_runtime_session_options(main_path.string(), temp_root.string());
    runtime_options.rushmore_planning.enabled = true;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(runtime_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Rushmore LOCATE script should complete");

    const auto order_before = state.globals.find("corderbefore");
    const auto order_after = state.globals.find("corderafter");
    const auto found = state.globals.find("lfound");
    const auto recno = state.globals.find("nrecno");

    expect(order_before != state.globals.end(), "ORDER() before LOCATE should be captured");
    expect(order_after != state.globals.end(), "ORDER() after LOCATE should be captured");
    expect(found != state.globals.end(), "FOUND() after LOCATE should be captured");
    expect(recno != state.globals.end(), "RECNO() after LOCATE should be captured");

    if (order_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_before->second).empty(),
            "LOCATE should start without a permanent active order");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(),
            "Rushmore LOCATE should restore the original active order after optimization");
    }
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true",
            "Rushmore LOCATE should still find the matching record");
    }
    if (recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno->second) == "2",
            "Rushmore LOCATE should position the cursor on the matching row");
    }

    expect(
        has_rushmore_event_with_detail_fragment(state.events, "NAME = 'BRAVO'") &&
        has_rushmore_event_with_detail_fragment(state.events, "index_seek via NAME"),
        "Rushmore LOCATE should emit a diagnostics event showing the optimized seek path");

    fs::remove_all(temp_root, ignored);
}

void test_scan_uses_rushmore_seek_and_restores_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rushmore_scan";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "rushmore_scan.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "cOrderBefore = ORDER()\n"
        "SCAN FOR NAME = 'BRAVO'\n"
        "cScanName = NAME\n"
        "ENDSCAN\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Rushmore SCAN script should complete");

    const auto order_before = state.globals.find("corderbefore");
    const auto order_after = state.globals.find("corderafter");
    const auto scan_name = state.globals.find("cscanname");

    expect(order_before != state.globals.end(), "ORDER() before SCAN should be captured");
    expect(order_after != state.globals.end(), "ORDER() after SCAN should be captured");
    expect(scan_name != state.globals.end(), "SCAN body should capture the matching NAME");

    if (order_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_before->second).empty(),
            "SCAN should start without a permanent active order");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(),
            "Rushmore SCAN should restore the original active order after optimization");
    }
    if (scan_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_name->second) == "BRAVO",
            "Rushmore SCAN should execute the matching body on the located row");
    }

    expect(
        has_rushmore_event_with_detail_fragment(state.events, "NAME = 'BRAVO'") &&
        has_rushmore_event_with_detail_fragment(state.events, "index_seek via NAME"),
        "Rushmore SCAN should emit a diagnostics event showing the optimized seek path");

    fs::remove_all(temp_root, ignored);
}

void test_locate_with_double_equals_operator_uses_rushmore_seek() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rushmore_double_equals";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "rushmore_double_equals.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "LOCATE FOR NAME == 'BRAVO'\n"
        "lFound = FOUND()\n"
        "nRecno = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Rushmore LOCATE with == should complete: " + state.message);

    const auto found = state.globals.find("lfound");
    const auto recno = state.globals.find("nrecno");
    expect(found != state.globals.end() && recno != state.globals.end(),
        "LOCATE FOR NAME == 'BRAVO' should expose FOUND()/RECNO()");
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true",
            "LOCATE FOR NAME == 'BRAVO' should still find the matching record (== must not be split into a bare = plus a malformed operand)");
    }
    if (recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno->second) == "2",
            "LOCATE FOR NAME == 'BRAVO' should position the cursor on the matching row");
    }
    expect(
        has_rushmore_event_with_detail_fragment(state.events, "-> index_seek via NAME") &&
        !has_rushmore_event_with_detail_fragment(state.events, "linear_scan after index_seek"),
        "LOCATE FOR NAME == 'BRAVO' should use the index seek optimization, not silently fall back to a linear scan "
        "(the == operator must be recognized by the index-seek pattern matcher, not garbled into a bare = plus a malformed operand)");

    fs::remove_all(temp_root, ignored);
}

void test_locate_with_greater_than_operator_does_not_match_equal_record() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rushmore_greater_than";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "rushmore_greater_than.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "LOCATE FOR NAME > 'BRAVO'\n"
        "lFound = FOUND()\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Rushmore LOCATE with > should complete: " + state.message);

    const auto found = state.globals.find("lfound");
    const auto name = state.globals.find("cname");
    expect(found != state.globals.end(), "LOCATE FOR NAME > 'BRAVO' should expose FOUND()");
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true",
            "LOCATE FOR NAME > 'BRAVO' should find the strictly-greater record");
    }
    if (found != state.globals.end() && copperfin::runtime::format_value(found->second) == "true" &&
        name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "CHARLIE",
            "LOCATE FOR NAME > 'BRAVO' must not land on a record whose NAME is merely equal to 'BRAVO' (got NAME=" +
                copperfin::runtime::format_value(name->second) + ")");
    }

    fs::remove_all(temp_root, ignored);
}

void test_opt_in_cost_model_rejects_expensive_seek_and_preserves_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rushmore_cost_fallback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "rushmore_cost_fallback.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cOrderAfter = ORDER()\n"
        "lFound = FOUND()\n"
        "nRecno = RECNO()\n"
        "SET ORDER TO TAG NAME\n"
        "lSeekFound = SEEK('BRAVO')\n"
        "nSeekRecno = RECNO()\n"
        "RETURN\n");

    auto runtime_options = make_runtime_session_options(main_path.string(), temp_root.string());
    runtime_options.rushmore_planning.enabled = true;
    runtime_options.rushmore_planning.cost_model.index_seek_base_cpu_units =
        std::numeric_limits<std::uint64_t>::max();
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(runtime_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Cost-model fallback LOCATE should complete: " + state.message);

    const auto order_after = state.globals.find("corderafter");
    const auto found = state.globals.find("lfound");
    const auto recno = state.globals.find("nrecno");
    const auto seek_found = state.globals.find("lseekfound");
    const auto seek_recno = state.globals.find("nseekrecno");
    expect(order_after != state.globals.end() && found != state.globals.end() && recno != state.globals.end() &&
            seek_found != state.globals.end() && seek_recno != state.globals.end(),
        "Cost-model fallback LOCATE should preserve observable cursor globals");
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(),
            "Cost-model fallback LOCATE should restore the original active order");
    }
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true",
            "Cost-model fallback LOCATE should still find the matching record");
    }
    if (recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno->second) == "2",
            "Cost-model fallback LOCATE should preserve RECNO() semantics");
    }
    if (seek_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_found->second) == "true",
            "Cost-model fallback SEEK should still find the matching record");
    }
    if (seek_recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_recno->second) == "2",
            "Cost-model fallback SEEK should preserve RECNO() semantics");
    }
    expect(has_rushmore_event_with_detail_fragment(state.events, "Cost model selected the fallback scan") &&
            has_rushmore_event_with_detail_fragment(state.events, "SEEK -> fallback") &&
            !has_rushmore_event_with_detail_fragment(state.events, "index_seek via NAME"),
        "Cost-model fallback LOCATE and SEEK should report localized fallbacks rather than selecting indexed plans");

    fs::remove_all(temp_root, ignored);
}

void test_opt_in_cost_model_preserves_seek_exact_near_and_order_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rushmore_seek";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "NAME", "NAME");

    const fs::path main_path = temp_root / "rushmore_seek.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET EXACT ON\n"
        "SET NEAR OFF\n"
        "lExactHit = SEEK('BRAVO')\n"
        "nExactHitRecno = RECNO()\n"
        "lExactMiss = SEEK('BRAV')\n"
        "nExactMissRecno = RECNO()\n"
        "SET NEAR ON\n"
        "lNearMiss = SEEK('BRAV')\n"
        "nNearMissRecno = RECNO()\n"
        "RETURN\n");

    auto runtime_options = make_runtime_session_options(main_path.string(), temp_root.string());
    runtime_options.rushmore_planning.enabled = true;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(runtime_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Cost-model SEEK semantics script should complete: " + state.message);

    const auto exact_hit = state.globals.find("lexacthit");
    const auto exact_hit_recno = state.globals.find("nexacthitrecno");
    const auto exact_miss = state.globals.find("lexactmiss");
    const auto exact_miss_recno = state.globals.find("nexactmissrecno");
    const auto near_miss = state.globals.find("lnearmiss");
    const auto near_miss_recno = state.globals.find("nnearmissrecno");

    expect(exact_hit != state.globals.end() && exact_hit_recno != state.globals.end(),
        "Cost-model SEEK should expose the exact-hit result and position");
    expect(exact_miss != state.globals.end() && exact_miss_recno != state.globals.end(),
        "Cost-model SEEK should expose the exact-miss result and position");
    expect(near_miss != state.globals.end() && near_miss_recno != state.globals.end(),
        "Cost-model SEEK should expose the near-miss result and position");
    if (exact_hit != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_hit->second) == "true",
            "SEEK should find the exact key with SET EXACT ON");
    }
    if (exact_hit_recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_hit_recno->second) == "2",
            "SEEK should position on the exact key with SET EXACT ON");
    }
    if (exact_miss != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_miss->second) == "false",
            "SEEK should reject the prefix with SET EXACT ON and SET NEAR OFF");
    }
    if (exact_miss_recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_miss_recno->second) == "4",
            "SEEK should move to EOF for an exact miss with SET NEAR OFF");
    }
    if (near_miss != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_miss->second) == "false",
            "SEEK should report a near match as not found");
    }
    if (near_miss_recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_miss_recno->second) == "2",
            "SEEK should position on the next visible key with SET NEAR ON");
    }
    expect(has_rushmore_event_with_detail_fragment(state.events, "SEEK -> index_seek via NAME") &&
            !has_rushmore_event_with_detail_fragment(state.events, "SEEK -> fallback"),
        "Cost-model SEEK should select the indexed plan without changing SEEK diagnostics");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_rushmore_planning_contracts();
    test_locate_uses_rushmore_seek_and_restores_order();
    test_scan_uses_rushmore_seek_and_restores_order();
    test_locate_with_double_equals_operator_uses_rushmore_seek();
    test_locate_with_greater_than_operator_does_not_match_equal_record();
    test_opt_in_cost_model_rejects_expensive_seek_and_preserves_state();
    test_opt_in_cost_model_preserves_seek_exact_near_and_order_semantics();
    const int failures = copperfin::test_support::test_failures();
    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
