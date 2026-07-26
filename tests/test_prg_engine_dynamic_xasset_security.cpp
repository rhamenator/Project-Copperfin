// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace copperfin::test_support;
namespace fs = std::filesystem;

void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_form_fixture(
    const fs::path& form_path,
    const fs::path& memo_path,
    const std::string& method_source = "PROCEDURE Init\nTHIS.SETALL('fontname', 'Tahoma')\nRETURN\nENDPROC") {
    constexpr std::size_t field_count = 3U;
    constexpr std::size_t header_length = 32U + (field_count * 32U) + 1U;
    constexpr std::size_t record_length = 1U + (field_count * 4U);
    std::vector<std::uint8_t> table(header_length + record_length + 1U, 0U);
    table[0] = 0xF5U;
    write_le_u32(table, 4U, 1U);
    write_le_u16(table, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(table, 10U, static_cast<std::uint16_t>(record_length));

    const std::vector<std::string> field_names{"OBJNAME", "BASECLASS", "METHODS"};
    for (std::size_t index = 0U; index < field_names.size(); ++index) {
        const std::size_t descriptor = 32U + (index * 32U);
        for (std::size_t character = 0U; character < field_names[index].size(); ++character) {
            table[descriptor + character] = static_cast<std::uint8_t>(field_names[index][character]);
        }
        table[descriptor + 11U] = static_cast<std::uint8_t>('M');
        write_le_u32(table, descriptor + 12U, static_cast<std::uint32_t>(index * 4U + 1U));
        table[descriptor + 16U] = 4U;
    }
    table[32U + (field_count * 32U)] = 0x0DU;
    const std::size_t record = header_length;
    table[record] = 0x20U;
    for (std::size_t index = 0U; index < field_count; ++index) {
        write_le_u32(table, record + 1U + (index * 4U), static_cast<std::uint32_t>(index + 1U));
    }
    table.back() = 0x1AU;

    std::ofstream table_output(form_path, std::ios::binary | std::ios::trunc);
    table_output.write(reinterpret_cast<const char*>(table.data()), static_cast<std::streamsize>(table.size()));

    std::vector<std::uint8_t> memo(512U * 4U, 0U);
    write_be_u16(memo, 6U, 512U);
    const std::vector<std::string> values{
        "frmDynamic",
        "form",
        method_source};
    for (std::size_t index = 0U; index < values.size(); ++index) {
        const std::size_t block = (index + 1U) * 512U;
        write_be_u32(memo, block, 1U);
        write_be_u32(memo, block + 4U, static_cast<std::uint32_t>(values[index].size()));
        for (std::size_t character = 0U; character < values[index].size(); ++character) {
            memo[block + 8U + character] = static_cast<std::uint8_t>(values[index][character]);
        }
    }
    std::ofstream memo_output(memo_path, std::ios::binary | std::ios::trunc);
    memo_output.write(reinterpret_cast<const char*>(memo.data()), static_cast<std::streamsize>(memo.size()));
}

std::string find_xasset_bootstrap_path(const copperfin::runtime::RuntimePauseState& state) {
    for (const auto& frame : state.call_stack) {
        if (frame.file_path.find("_copperfin_bootstrap_") != std::string::npos) {
            return frame.file_path;
        }
    }
    return {};
}

void test_dynamic_xasset_uses_verified_snapshot() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_xasset_verified";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path form_path = root / "dynamic.scx";
    const fs::path memo_path = root / "dynamic.sct";
    const fs::path main_path = root / "main.prg";
    write_form_fixture(form_path, memo_path);
    const std::string form_bytes = read_text(form_path);
    const std::string memo_bytes = read_text(memo_path);
    write_text(main_path, "DO FORM '" + form_path.string() + "'\n");

    auto options = make_runtime_session_options(main_path, root);
    options.verified_file_byte_overrides.emplace(form_path.string(), form_bytes);
    options.verified_file_byte_overrides.emplace(memo_path.string(), memo_bytes);
    options.require_verified_file_byte_overrides = true;

    write_text(form_path, "changed on disk");
    write_text(memo_path, "changed on disk");
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason != copperfin::runtime::DebugPauseReason::error,
           "dynamic xAsset should execute from its verified primary and memo snapshots");
    const auto setall_event_count = static_cast<std::size_t>(std::count_if(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "prg.object.setall" &&
                   event.detail == "__cf_xasset_root.fontname:0";
        }));
    expect(setall_event_count == 1U,
           "dynamic xAsset form methods should invoke SetAll with a live THIS form context");
    expect(has_runtime_event(state.events, "form.open", form_path.string()),
           "dynamic xAsset should retain the logical asset path in runtime events");
    fs::remove_all(root, ignored);
}

void test_dynamic_xasset_uses_admitted_snapshot_during_replacement() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_xasset_replacement";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path form_path = root / "dynamic.scx";
    const fs::path memo_path = root / "dynamic.sct";
    const fs::path main_path = root / "main.prg";
    write_form_fixture(form_path, memo_path, "PROCEDURE Init\nTHIS.SETALL('fontname', 'AdmittedFont')\nRETURN\nENDPROC");
    const std::string form_bytes = read_text(form_path);
    const std::string memo_bytes = read_text(memo_path);
    write_text(main_path, "DO FORM '" + form_path.string() + "'\n");

    auto options = make_runtime_session_options(main_path, root);
    options.verified_file_byte_overrides.emplace(form_path.string(), form_bytes);
    options.verified_file_byte_overrides.emplace(memo_path.string(), memo_bytes);
    options.require_verified_file_byte_overrides = true;

    std::atomic<bool> stop_writer{false};
    std::atomic<unsigned int> replacements{0U};
    std::thread replacement_writer([&]() {
        while (!stop_writer.load(std::memory_order_relaxed))
        {
            write_text(form_path, "REPLACED-FORM");
            write_text(memo_path, "REPLACED-MEMO");
            replacements.fetch_add(1U, std::memory_order_relaxed);
            std::this_thread::yield();
        }
    });
    for (unsigned int attempt = 0U;
         attempt < 10000U && replacements.load(std::memory_order_relaxed) == 0U;
         ++attempt)
    {
        std::this_thread::yield();
    }

    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    stop_writer.store(true, std::memory_order_relaxed);
    replacement_writer.join();

    expect(state.reason != copperfin::runtime::DebugPauseReason::error,
           "strict DO FORM should execute from admitted xAsset bytes during replacement: " + state.message);
    const auto setall_event_count = static_cast<std::size_t>(std::count_if(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "prg.object.setall" &&
                   event.detail == "__cf_xasset_root.fontname:0";
        }));
    expect(setall_event_count == 1U,
           "strict DO FORM replacement coverage should execute the admitted form method");
    expect(replacements.load(std::memory_order_relaxed) > 0U,
           "strict DO FORM replacement coverage should replace the physical xAsset paths");
    fs::remove_all(root, ignored);
}

void test_dynamic_xasset_uses_admitted_snapshot_when_paths_are_absent() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_xasset_absent_paths";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path form_path = root / "dynamic.scx";
    const fs::path memo_path = root / "dynamic.sct";
    const fs::path main_path = root / "main.prg";
    write_form_fixture(form_path, memo_path, "PROCEDURE Init\nTHIS.SETALL('fontname', 'AbsentPathFont')\nRETURN\nENDPROC");
    const std::string form_bytes = read_text(form_path);
    const std::string memo_bytes = read_text(memo_path);
    write_text(main_path, "DO FORM '" + form_path.string() + "'\n");
    fs::remove(form_path, ignored);
    fs::remove(memo_path, ignored);

    auto options = make_runtime_session_options(main_path, root);
    options.verified_file_byte_overrides.emplace(form_path.string(), form_bytes);
    options.verified_file_byte_overrides.emplace(memo_path.string(), memo_bytes);
    options.require_verified_file_byte_overrides = true;
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason != copperfin::runtime::DebugPauseReason::error,
           "strict DO FORM should execute admitted xAsset bytes without physical paths: " + state.message);
    expect(has_runtime_event(state.events, "form.open", form_path.string()),
           "strict absent-path DO FORM should retain its logical form event");
    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent& event) {
                   return event.category == "prg.object.setall" &&
                       event.detail == "__cf_xasset_root.fontname:0";
               }),
           "strict absent-path DO FORM should execute the admitted form method");

    auto missing_options = make_runtime_session_options(main_path, root);
    missing_options.require_verified_file_byte_overrides = true;
    auto missing_session = copperfin::runtime::PrgRuntimeSession::create(missing_options);
    const auto missing_state = missing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(missing_state.reason == copperfin::runtime::DebugPauseReason::error &&
               missing_state.message.find("Verified package bytes are unavailable") != std::string::npos,
           "strict absent-path DO FORM should fail closed without admitted bytes: " + missing_state.message);

    fs::remove_all(root, ignored);
}

void test_dynamic_xasset_requires_verified_snapshot() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_xasset_unverified";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path form_path = root / "dynamic.scx";
    const fs::path memo_path = root / "dynamic.sct";
    const fs::path main_path = root / "main.prg";
    write_form_fixture(form_path, memo_path);
    write_text(main_path, "DO FORM '" + form_path.string() + "'\n");

    auto options = make_runtime_session_options(main_path, root);
    options.require_verified_file_byte_overrides = true;
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "dynamic xAsset should fail closed without verified package bytes");
    expect(state.message.find("dynamic.scx") != std::string::npos,
           "unverified dynamic xAsset diagnostics should identify the requested asset");
    fs::remove_all(root, ignored);
}

void test_dynamic_xasset_bootstrap_paths_are_session_unique() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_xasset_session_unique";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    const fs::path first_root = root / "first";
    const fs::path second_root = root / "second";
    const fs::path shared_temp = root / "shared-runtime-temp";
    fs::create_directories(first_root);
    fs::create_directories(second_root);

    const fs::path first_form = first_root / "dynamic.scx";
    const fs::path first_memo = first_root / "dynamic.sct";
    const fs::path first_main = first_root / "main.prg";
    const fs::path second_form = second_root / "dynamic.scx";
    const fs::path second_memo = second_root / "dynamic.sct";
    const fs::path second_main = second_root / "main.prg";
    write_form_fixture(first_form, first_memo, "PROCEDURE Init\n? 'first-session'\nRETURN\nENDPROC");
    write_form_fixture(second_form, second_memo, "PROCEDURE Init\n? 'second-session'\nRETURN\nENDPROC");
    write_text(first_main, "DO FORM '" + first_form.string() + "'\n");
    write_text(second_main, "DO FORM '" + second_form.string() + "'\n");

    auto first_options = make_runtime_session_options(first_main, first_root);
    first_options.temp_directory = shared_temp.string();
    first_options.verified_file_byte_overrides.emplace(first_form.string(), read_text(first_form));
    first_options.verified_file_byte_overrides.emplace(first_memo.string(), read_text(first_memo));
    first_options.require_verified_file_byte_overrides = true;

    auto second_options = make_runtime_session_options(second_main, second_root);
    second_options.temp_directory = shared_temp.string();
    second_options.verified_file_byte_overrides.emplace(second_form.string(), read_text(second_form));
    second_options.verified_file_byte_overrides.emplace(second_memo.string(), read_text(second_memo));
    second_options.require_verified_file_byte_overrides = true;

    std::string first_bootstrap_path;
    std::string second_bootstrap_path;
    {
        auto first_session = copperfin::runtime::PrgRuntimeSession::create(first_options);
        auto second_session = copperfin::runtime::PrgRuntimeSession::create(second_options);
        const auto first_state = first_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto second_state = second_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(first_state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               "first same-stem xAsset session should enter its event loop");
        expect(second_state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               "second same-stem xAsset session should enter its event loop");

        first_bootstrap_path = find_xasset_bootstrap_path(first_state);
        second_bootstrap_path = find_xasset_bootstrap_path(second_state);
        expect(!first_bootstrap_path.empty() && !second_bootstrap_path.empty(),
               "same-stem xAsset sessions should expose generated bootstrap frames");
        expect(first_bootstrap_path != second_bootstrap_path,
               "same-stem xAsset sessions should use distinct bootstrap paths");
        if (!first_bootstrap_path.empty() && !second_bootstrap_path.empty()) {
            expect(fs::exists(first_bootstrap_path) && fs::exists(second_bootstrap_path),
                   "active same-stem xAsset sessions should retain both generated sources");
            const std::string first_source = read_text(first_bootstrap_path);
            const std::string second_source = read_text(second_bootstrap_path);
            expect(first_source.find("first-session") != std::string::npos &&
                       second_source.find("second-session") != std::string::npos &&
                       first_source != second_source,
                   "same-stem xAsset sessions should retain independent generated source contents");
        }
    }

    fs::remove_all(root, ignored);
}

void test_dynamic_xasset_bootstrap_cleanup() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_xasset_bootstrap_cleanup";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path form_path = root / "dynamic.scx";
    const fs::path memo_path = root / "dynamic.sct";
    const fs::path main_path = root / "main.prg";
    write_form_fixture(form_path, memo_path);
    write_text(main_path, "DO FORM '" + form_path.string() + "'\n");

    auto options = make_runtime_session_options(main_path, root);
    options.verified_file_byte_overrides.emplace(form_path.string(), read_text(form_path));
    options.verified_file_byte_overrides.emplace(memo_path.string(), read_text(memo_path));
    options.require_verified_file_byte_overrides = true;

    std::string bootstrap_path;
    {
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        bootstrap_path = find_xasset_bootstrap_path(state);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   !bootstrap_path.empty() && fs::exists(bootstrap_path),
               "successful xAsset bootstrap should remain available while its PRG session is active");
    }
    expect(!bootstrap_path.empty() && !fs::exists(bootstrap_path),
           "successful xAsset bootstrap should be removed when its PRG session is destroyed");
    fs::remove_all(root, ignored);
}

void test_dynamic_xasset_failed_write_cleanup() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_xasset_bootstrap_write_failure";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path form_path = root / "dynamic.scx";
    const fs::path memo_path = root / "dynamic.sct";
    const fs::path main_path = root / "main.prg";
    write_form_fixture(form_path, memo_path);
    write_text(main_path, "DO FORM '" + form_path.string() + "'\n");

    auto options = make_runtime_session_options(main_path, root);
    options.verified_file_byte_overrides.emplace(form_path.string(), read_text(form_path));
    options.verified_file_byte_overrides.emplace(memo_path.string(), read_text(memo_path));
    options.require_verified_file_byte_overrides = true;

    const copperfin::test_support::ScopedEnvironmentValue fail_path(
        "COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS",
        "_copperfin_bootstrap_");
    const copperfin::test_support::ScopedEnvironmentValue fail_stage(
        "COPPERFIN_TEST_FAIL_WRITE_STAGE",
        "prg-xasset-bootstrap");
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "failed xAsset bootstrap writes should fault the PRG session");

    bool bootstrap_leaked = false;
    if (fs::exists(options.temp_directory))
    {
        for (const auto& entry : fs::directory_iterator(options.temp_directory))
        {
            if (entry.path().filename().string().find("_copperfin_bootstrap_") != std::string::npos)
            {
                bootstrap_leaked = true;
                break;
            }
        }
    }
    expect(!bootstrap_leaked,
           "failed xAsset bootstrap writes should remove partial generated sources");
    fs::remove_all(root, ignored);
}

void test_strict_do_uses_verified_source_during_replacement() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_do_verified";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    const fs::path main_path = root / "main.prg";
    const fs::path child_path = root / "child.prg";
    const std::string child_target = copperfin::platform::path_to_utf8_string(child_path);
    const std::string startup_source = "PUBLIC cMarker\nDO " + child_target + "\nRETURN\n";
    const std::string admitted_child_source =
        "PUBLIC cMarker\ncMarker = 'admitted-child'\nRETURN\n";
    write_text(main_path, startup_source);
    write_text(child_path, "PUBLIC cMarker\ncMarker = 'disk-child'\nRETURN\n");

    auto options = make_runtime_session_options(main_path, root);
    options.startup_source_text = startup_source;
    options.source_text_overrides.emplace(child_path.string(), admitted_child_source);
    options.require_source_text_overrides = true;

    std::atomic<bool> stop_writer{false};
    std::thread replacement_writer([&]() {
        while (!stop_writer.load(std::memory_order_relaxed)) {
            write_text(child_path, "PUBLIC cMarker\ncMarker = 'replaced-child'\nRETURN\n");
            std::this_thread::yield();
        }
    });

    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    stop_writer.store(true, std::memory_order_relaxed);
    replacement_writer.join();

    expect(state.completed,
           "strict DO should complete from admitted child source bytes: " + state.message);
    const auto marker = state.globals.find("cmarker");
    expect(marker != state.globals.end() &&
               copperfin::runtime::format_value(marker->second) == "admitted-child",
           "strict DO should preserve the admitted child result during pathname replacement");

    auto missing_options = make_runtime_session_options(main_path, root);
    missing_options.startup_source_text = startup_source;
    missing_options.require_source_text_overrides = true;
    auto missing_session = copperfin::runtime::PrgRuntimeSession::create(missing_options);
    const auto missing_state = missing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(missing_state.reason == copperfin::runtime::DebugPauseReason::error &&
               missing_state.message.find("Verified package source is unavailable") != std::string::npos,
           "strict DO should fail closed when the child source admission is missing: " + missing_state.message);

    fs::remove_all(root, ignored);
}

void test_strict_do_and_call_use_admitted_source_when_paths_are_absent() {
    const fs::path root = fs::temp_directory_path() / "copperfin_dynamic_prg_admitted_without_paths";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    const fs::path main_path = root / "main.prg";
    const fs::path do_path = root / "child.prg";
    const fs::path call_path = root / "helper.prg";
    const std::string startup_source =
        "PUBLIC cDoMarker\n"
        "PUBLIC cCallMarker\n"
        "DO child\n"
        "CALL helper\n"
        "RETURN\n";
    const std::string admitted_do_source =
        "PUBLIC cDoMarker\n"
        "cDoMarker = 'admitted-do'\n"
        "RETURN\n";
    const std::string admitted_call_source =
        "PUBLIC cCallMarker\n"
        "cCallMarker = 'admitted-call'\n"
        "RETURN\n";
    write_text(main_path, startup_source);

    auto options = make_runtime_session_options(main_path, root);
    options.startup_source_text = startup_source;
    options.source_text_overrides.emplace(do_path.string(), admitted_do_source);
    options.source_text_overrides.emplace(call_path.string(), admitted_call_source);
    options.require_source_text_overrides = true;
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "strict DO and CALL should use admitted source without physical files: " + state.message);
    const auto do_marker = state.globals.find("cdomarker");
    expect(do_marker != state.globals.end() &&
               copperfin::runtime::format_value(do_marker->second) == "admitted-do",
           "strict DO should execute its admitted source");
    const auto call_marker = state.globals.find("ccallmarker");
    expect(call_marker != state.globals.end() &&
               copperfin::runtime::format_value(call_marker->second) == "admitted-call",
           "strict CALL should execute its admitted source");

    auto missing_options = make_runtime_session_options(main_path, root);
    missing_options.startup_source_text = startup_source;
    missing_options.source_text_overrides.emplace(do_path.string(), admitted_do_source);
    missing_options.require_source_text_overrides = true;
    auto missing_session = copperfin::runtime::PrgRuntimeSession::create(missing_options);
    const auto missing_state = missing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(missing_state.reason == copperfin::runtime::DebugPauseReason::error &&
               missing_state.message.find("Verified package source is unavailable") != std::string::npos,
           "strict CALL should fail closed when its source admission is missing: " + missing_state.message);

    fs::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_dynamic_xasset_uses_verified_snapshot();
    test_dynamic_xasset_uses_admitted_snapshot_during_replacement();
    test_dynamic_xasset_uses_admitted_snapshot_when_paths_are_absent();
    test_dynamic_xasset_requires_verified_snapshot();
    test_dynamic_xasset_bootstrap_paths_are_session_unique();
    test_dynamic_xasset_bootstrap_cleanup();
    test_dynamic_xasset_failed_write_cleanup();
    test_strict_do_uses_verified_source_during_replacement();
    test_strict_do_and_call_use_admitted_source_when_paths_are_absent();
    return test_failures() == 0 ? 0 : 1;
}
