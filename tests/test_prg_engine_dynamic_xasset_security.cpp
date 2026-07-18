// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
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

void write_form_fixture(const fs::path& form_path, const fs::path& memo_path) {
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
        "PROCEDURE Init\nRETURN\nENDPROC"};
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
    expect(has_runtime_event(state.events, "form.open", form_path.string()),
           "dynamic xAsset should retain the logical asset path in runtime events");
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

}  // namespace

int main() {
    test_dynamic_xasset_uses_verified_snapshot();
    test_dynamic_xasset_requires_verified_snapshot();
    return test_failures() == 0 ? 0 : 1;
}
