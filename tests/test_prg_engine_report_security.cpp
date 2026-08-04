// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <string>
#include <vector>

namespace {

using namespace copperfin::test_support;
namespace fs = std::filesystem;

void write_report_fixture(const fs::path& asset_path) {
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
        {"8", "", "NAME", "100", "20", "700", "100", "name-field-guid"}
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

}  // namespace

int main() {
    test_strict_report_and_label_use_admitted_bytes_without_physical_paths();
    return test_failures() == 0 ? 0 : 1;
}
