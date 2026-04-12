#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

int failures = 0;
    std::string uppercase_ascii(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });
        return value;
    }

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

void write_text(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary);
    output << contents;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void write_simple_dbf(const std::filesystem::path& path, const std::vector<std::string>& records) {
    constexpr std::size_t field_length = 10U;
    constexpr std::size_t header_length = 32U + 32U + 1U;
    constexpr std::size_t record_length = 1U + field_length;
    std::vector<std::uint8_t> bytes(header_length + (records.size() * record_length) + 1U, 0U);
    bytes[0] = 0x30U;
    write_le_u32(bytes, 4U, static_cast<std::uint32_t>(records.size()));
    write_le_u16(bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(bytes, 10U, static_cast<std::uint16_t>(record_length));

    const char name[] = "NAME";
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[32U + index] = static_cast<std::uint8_t>(name[index]);
    }
    bytes[32U + 11U] = static_cast<std::uint8_t>('C');
    write_le_u32(bytes, 32U + 12U, 1U);
    bytes[32U + 16U] = static_cast<std::uint8_t>(field_length);
    bytes[64U] = 0x0DU;

    for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
        const std::size_t offset = header_length + (record_index * record_length);
        bytes[offset] = 0x20U;
        const std::string value = records[record_index].substr(0U, field_length);
        for (std::size_t index = 0; index < value.size(); ++index) {
            bytes[offset + 1U + index] = static_cast<std::uint8_t>(value[index]);
        }
        for (std::size_t index = value.size(); index < field_length; ++index) {
            bytes[offset + 1U + index] = 0x20U;
        }
    }

    bytes.back() = 0x1AU;
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_people_dbf(
    const std::filesystem::path& path,
    const std::vector<std::pair<std::string, int>>& records) {
    constexpr std::size_t name_length = 10U;
    constexpr std::size_t age_length = 3U;
    constexpr std::size_t header_length = 32U + 32U + 32U + 1U;
    constexpr std::size_t record_length = 1U + name_length + age_length;
    std::vector<std::uint8_t> bytes(header_length + (records.size() * record_length) + 1U, 0U);
    bytes[0] = 0x30U;
    write_le_u32(bytes, 4U, static_cast<std::uint32_t>(records.size()));
    write_le_u16(bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(bytes, 10U, static_cast<std::uint16_t>(record_length));

    const char name_field[] = "NAME";
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[32U + index] = static_cast<std::uint8_t>(name_field[index]);
    }
    bytes[32U + 11U] = static_cast<std::uint8_t>('C');
    write_le_u32(bytes, 32U + 12U, 1U);
    bytes[32U + 16U] = static_cast<std::uint8_t>(name_length);

    const char age_field[] = "AGE";
    for (std::size_t index = 0; index < 3U; ++index) {
        bytes[64U + index] = static_cast<std::uint8_t>(age_field[index]);
    }
    bytes[64U + 11U] = static_cast<std::uint8_t>('N');
    write_le_u32(bytes, 64U + 12U, 1U + static_cast<std::uint32_t>(name_length));
    bytes[64U + 16U] = static_cast<std::uint8_t>(age_length);
    bytes[96U] = 0x0DU;

    for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
        const std::size_t offset = header_length + (record_index * record_length);
        bytes[offset] = 0x20U;

        const std::string name = records[record_index].first.substr(0U, name_length);
        for (std::size_t index = 0; index < name.size(); ++index) {
            bytes[offset + 1U + index] = static_cast<std::uint8_t>(name[index]);
        }
        for (std::size_t index = name.size(); index < name_length; ++index) {
            bytes[offset + 1U + index] = 0x20U;
        }

        const std::string age = std::to_string(records[record_index].second);
        const std::size_t padding = age.size() >= age_length ? 0U : age_length - age.size();
        for (std::size_t index = 0; index < padding; ++index) {
            bytes[offset + 1U + name_length + index] = 0x20U;
        }
        for (std::size_t index = 0; index < age.size() && index < age_length; ++index) {
            bytes[offset + 1U + name_length + padding + index] = static_cast<std::uint8_t>(age[index]);
        }
    }

    bytes.back() = 0x1AU;
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_synthetic_cdx(const std::filesystem::path& path, const std::string& tag_name, const std::string& expression) {
    std::vector<std::uint8_t> bytes(4096U, 0U);
    write_le_u16(bytes, 0U, 1024U);
    write_le_u16(bytes, 12U, 10U);
    write_le_u16(bytes, 14U, 480U);
    write_le_u16(bytes, 1024U, 0x0003U);
    write_le_u16(bytes, 1026U, 0x0001U);

    for (std::size_t index = 0; index < expression.size(); ++index) {
        bytes[2048U + index] = static_cast<std::uint8_t>(expression[index]);
    }

    const std::size_t tail_offset = (3U * 512U) - 10U;
    for (std::size_t index = 0; index < tag_name.size(); ++index) {
        bytes[tail_offset + index] = static_cast<std::uint8_t>(tag_name[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_synthetic_idx(const std::filesystem::path& path, const std::string& expression) {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u32(bytes, 0U, 512U);
    write_le_u32(bytes, 4U, 0U);
    write_le_u32(bytes, 8U, 1024U);
    write_le_u16(bytes, 12U, static_cast<std::uint16_t>(std::min<std::size_t>(expression.size(), 220U)));
    for (std::size_t index = 0; index < expression.size() && index < 220U; ++index) {
        bytes[16U + index] = static_cast<std::uint8_t>(expression[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_synthetic_idx_with_for(
    const std::filesystem::path& path,
    const std::string& expression,
    const std::string& for_expression) {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u32(bytes, 0U, 512U);
    write_le_u32(bytes, 4U, 0U);
    write_le_u32(bytes, 8U, 1024U);
    write_le_u16(bytes, 12U, static_cast<std::uint16_t>(std::min<std::size_t>(expression.size(), 220U)));
    for (std::size_t index = 0; index < expression.size() && index < 220U; ++index) {
        bytes[16U + index] = static_cast<std::uint8_t>(expression[index]);
    }
    for (std::size_t index = 0; index < for_expression.size() && index < 220U; ++index) {
        bytes[236U + index] = static_cast<std::uint8_t>(for_expression[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void mark_simple_dbf_record_deleted(const std::filesystem::path& path, std::size_t recno) {
    constexpr std::size_t header_length = 32U + 32U + 1U;
    constexpr std::size_t record_length = 1U + 10U;
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    const std::size_t deletion_offset = header_length + ((recno - 1U) * record_length);
    file.seekp(static_cast<std::streamoff>(deletion_offset), std::ios::beg);
    const char deleted = '*';
    file.write(&deleted, 1);
}

void write_synthetic_ndx(
    const std::filesystem::path& path,
    const std::string& expression,
    bool numeric_or_date_domain) {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u32(bytes, 0U, 1U);
    write_le_u32(bytes, 4U, 2U);
    write_le_u32(bytes, 8U, 0x00000034U);
    write_le_u16(bytes, 12U, static_cast<std::uint16_t>(std::min<std::size_t>(expression.size(), 100U)));
    write_le_u16(bytes, 14U, 42U);
    write_le_u16(bytes, 16U, numeric_or_date_domain ? 1U : 0U);
    write_le_u16(bytes, 18U, 12U);
    write_le_u16(bytes, 22U, 0U);
    for (std::size_t index = 0; index < expression.size() && index < 100U; ++index) {
        bytes[24U + index] = static_cast<std::uint8_t>(expression[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

bool has_runtime_event(
    const std::vector<copperfin::runtime::RuntimeEvent>& events,
    const std::string& category,
    const std::string& detail) {
    return std::any_of(events.begin(), events.end(), [&](const copperfin::runtime::RuntimeEvent& event) {
        return event.category == category && event.detail == detail;
    });
}

void test_breakpoints_and_stepping() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "main.prg";
    write_text(
        main_path,
        "x = 1\n"
        "DO localproc\n"
        "x = x + 1\n"
        "RETURN\n"
        "PROCEDURE localproc\n"
        "x = x + 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });
    session.add_breakpoint({.file_path = main_path.string(), .line = 2});

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::breakpoint, "continue should stop on the configured breakpoint");
    expect(state.location.line == 2U, "breakpoint should land on line 2");

    state = session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(state.reason == copperfin::runtime::DebugPauseReason::step, "step into should pause after entering the procedure");
    expect(state.location.line == 6U, "step into should land on the first line of the local procedure");

    state = session.run(copperfin::runtime::DebugResumeAction::step_out);
    expect(state.reason == copperfin::runtime::DebugPauseReason::step, "step out should pause after returning to the caller");
    expect(state.location.line == 3U, "step out should land on the caller line after the DO");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continue should complete the script");
    const auto x = state.globals.find("x");
    expect(x != state.globals.end(), "script should leave x in globals");
    if (x != state.globals.end()) {
        expect(copperfin::runtime::format_value(x->second) == "4", "x should equal 4 after the procedure and caller increment");
    }

    fs::remove_all(temp_root, ignored);
}

void test_read_events_pause() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_events";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "events.prg";
    write_text(
        main_path,
        "x = 1\n"
        "READ EVENTS\n"
        "x = x + 1\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "READ EVENTS should pause the runtime in event-loop mode");
    expect(state.waiting_for_events, "event-loop pause should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_activate_popup_pause() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_popup";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "popup.prg";
    write_text(
        main_path,
        "ACTIVATE POPUP Shortcut\n"
        "x = 2\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "ACTIVATE POPUP should pause the runtime in event-loop mode");
    expect(state.waiting_for_events, "popup activation should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_dispatch_event_handler() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_dispatch";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "dispatch.prg";
    write_text(
        main_path,
        "PUBLIC x\n"
        "ACTIVATE POPUP Shortcut\n"
        "RETURN\n"
        "PROCEDURE item1\n"
        "x = 7\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "startup popup activation should pause in the event loop");
    expect(session.dispatch_event_handler("item1"), "dispatch_event_handler should find the target routine while waiting for events");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "dispatching an event handler should return to the event loop");
    const auto x = state.globals.find("x");
    expect(x != state.globals.end(), "dispatched routine should be able to set global variables");
    if (x != state.globals.end()) {
        expect(copperfin::runtime::format_value(x->second) == "7", "dispatched routine should update x before returning to the event loop");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_variables_in_stack_frame() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_locals";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "locals.prg";
    write_text(
        main_path,
        "DO localproc\n"
        "RETURN\n"
        "PROCEDURE localproc\n"
        "LOCAL itemCount\n"
        "itemCount = 9\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    session.add_breakpoint({.file_path = main_path.string(), .line = 5});
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::breakpoint, "local-variable test should stop on the itemCount assignment");
    expect(!state.call_stack.empty(), "local-variable test should include a stack frame");
    if (!state.call_stack.empty()) {
        const auto local = state.call_stack.front().locals.find("itemcount");
        expect(local != state.call_stack.front().locals.end(), "stack frame should expose declared LOCAL variables");
        if (local != state.call_stack.front().locals.end()) {
            expect(copperfin::runtime::format_value(local->second) == "", "LOCAL variables should exist before assignment");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_report_form_pause() {
    namespace fs = std::filesystem;
    const fs::path report_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\invoice.frx)";
    if (!fs::exists(report_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_report";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "report.prg";
    write_text(
        main_path,
        "REPORT FORM '" + report_path.string() + "' PREVIEW\n"
        "x = 2\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "REPORT FORM PREVIEW should pause in the event loop");
    expect(state.waiting_for_events, "report preview should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_do_form_pause() {
    namespace fs = std::filesystem;
    const fs::path form_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Wizards\Template\Books\Forms\books.scx)";
    if (!fs::exists(form_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_doform";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "doform.prg";
    write_text(
        main_path,
        "DO FORM '" + form_path.string() + "'\n"
        "x = 2\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "DO FORM should now enter the event loop for runnable forms");
    expect(state.waiting_for_events, "form launch should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_export_vfp_compatibility_corpus_script() {
    namespace fs = std::filesystem;
    const fs::path script_path = R"(E:\Project-Copperfin\scripts\export-vfp-compatibility-corpus.ps1)";
    const fs::path fixture_root = R"(E:\Project-Copperfin\build\compatibility_corpus_fixture)";
    const fs::path output_root = R"(E:\Project-Copperfin\build\compatibility_corpus_output)";
    const fs::path installed_root = fixture_root / "installed";
    const fs::path vfp_source_root = fixture_root / "vfpsource";
    const fs::path legacy_root = fixture_root / "legacy";
    const fs::path regression_root = fixture_root / "regression";

    std::error_code ignored;
    fs::remove_all(fixture_root, ignored);
    fs::remove_all(output_root, ignored);

    fs::create_directories(installed_root / "Samples" / "Solution" / "Reports");
    fs::create_directories(installed_root / "Wizards" / "Template" / "Books" / "Forms");
    fs::create_directories(vfp_source_root / "ReportBuilder");
    fs::create_directories(legacy_root / "Legacy");
    fs::create_directories(regression_root / "runtime");

    write_text(installed_root / "Samples" / "Solution" / "Reports" / "invoice.frx", "report fixture");
    write_text(installed_root / "Wizards" / "Template" / "Books" / "Forms" / "books.scx", "form fixture");
    write_text(vfp_source_root / "ReportBuilder" / "builder.prg", "PROCEDURE builder\nRETURN\n");
    write_text(legacy_root / "Legacy" / "sample.pjx", "project fixture");
    write_text(regression_root / "runtime" / "macro.spr", "SCREEN fixture");
    write_text(vfp_source_root / "ReportBuilder" / "ignore.txt", "not a FoxPro asset");

    const std::string command =
        "powershell -NoProfile -ExecutionPolicy Bypass -File \"" + script_path.string() +
        "\" -OutputDirectory \"" + output_root.string() +
        "\" -InstalledVfpRoots \"" + installed_root.string() +
        "\" -VfpSourceRoots \"" + vfp_source_root.string() +
        "\" -LegacyProjectRoots \"" + legacy_root.string() +
        "\" -RegressionSampleRoots \"" + regression_root.string() + "\"";

    const int exit_code = std::system(command.c_str());
    expect(exit_code == 0, "compatibility corpus exporter should succeed for synthetic fixture roots");

    const fs::path manifest_path = output_root / "vfp-compatibility-corpus.json";
    const fs::path summary_path = output_root / "vfp-compatibility-corpus-summary.json";
    expect(fs::exists(manifest_path), "compatibility corpus exporter should write the manifest JSON");
    expect(fs::exists(summary_path), "compatibility corpus exporter should write the summary JSON");
    if (!fs::exists(manifest_path) || !fs::exists(summary_path)) {
        fs::remove_all(fixture_root, ignored);
        fs::remove_all(output_root, ignored);
        return;
    }

    const std::string manifest = read_text(manifest_path);
    const std::string summary = read_text(summary_path);

    expect(manifest.find(R"(Samples\\Solution\\Reports\\invoice.frx)") != std::string::npos,
           "manifest should include installed VFP sample report assets");
    expect(manifest.find(R"(Wizards\\Template\\Books\\Forms\\books.scx)") != std::string::npos,
           "manifest should include installed VFP wizard form assets");
    expect(manifest.find(R"(ReportBuilder\\builder.prg)") != std::string::npos,
           "manifest should include local VFP source PRGs");
    expect(manifest.find(R"(Legacy\\sample.pjx)") != std::string::npos,
           "manifest should include legacy project assets");
    expect(manifest.find(R"(runtime\\macro.spr)") != std::string::npos,
           "manifest should include regression sample assets");
    expect(manifest.find("\"assetCategory\":  \"designer\"") != std::string::npos,
           "manifest should classify designer assets");
    expect(manifest.find("\"assetCategory\":  \"code\"") != std::string::npos,
           "manifest should classify code assets");
    expect(manifest.find("\"assetCategory\":  \"application\"") != std::string::npos,
           "manifest should classify project and app assets");
    expect(manifest.find("ignore.txt") == std::string::npos,
           "manifest should ignore unsupported file extensions");

    expect(summary.find("\"totalEntries\":  5") != std::string::npos,
           "summary should report the exported entry count");
    expect(summary.find("\"installed-vfp\":  2") != std::string::npos,
           "summary should count installed VFP assets");
    expect(summary.find("\"local-vfp-source\":  1") != std::string::npos,
           "summary should count VFP source assets");
    expect(summary.find("\"legacy-project\":  1") != std::string::npos,
           "summary should count legacy project assets");
    expect(summary.find("\"regression-sample\":  1") != std::string::npos,
           "summary should count regression sample assets");

    fs::remove_all(fixture_root, ignored);
    fs::remove_all(output_root, ignored);
}

void test_work_area_and_data_session_compatibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_workareas";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "workarea.prg";
    write_text(
        main_path,
        "SELECT 0\n"
        "nArea = SELECT()\n"
        "SET DATASESSION TO 3\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "work area script should complete");
    expect(state.work_area.selected == 1, "SELECT 0 should allocate and select work area 1 in a fresh session");
    expect(state.work_area.data_session == 3, "SET DATASESSION TO should update compatibility state");
    const auto area = state.globals.find("narea");
    expect(area != state.globals.end(), "SELECT() result should be available to PRG code");
    if (area != state.globals.end()) {
        expect(copperfin::runtime::format_value(area->second) == "1", "SELECT() should return the current work area");
    }

    fs::remove_all(temp_root, ignored);
}

void test_eval_macro_and_runtime_state_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_eval_macro_state";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path new_default = temp_root / "workspace";
    fs::create_directories(new_default);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    const fs::path main_path = temp_root / "eval_macro_state.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "cAliasExpr = 'People'\n"
        "cFieldExpr = 'NAME'\n"
        "cEvalExpr = 'AGE + 5'\n"
        "cNearBefore = SET('NEAR')\n"
        "SET NEAR ON\n"
        "cNearAfter = SET('NEAR')\n"
        "cDefaultBefore = SET('DEFAULT')\n"
        "SET DEFAULT TO '" + new_default.string() + "'\n"
        "cDefaultAfter = SET('DEFAULT')\n"
        "cAliasFromEval = EVAL('ALIAS()')\n"
        "cNameFromMacro = &cFieldExpr\n"
        "nEvalAge = EVAL(cEvalExpr)\n"
        "USE IN &cAliasExpr\n"
        "lUsedAfterClose = USED('People')\n"
        "nAreaAfterClose = SELECT('People')\n"
        "SET DATASESSION TO 2\n"
        "cNearSession2 = SET('NEAR')\n"
        "cDefaultSession2 = SET('DEFAULT')\n"
        "lFileSession2 = FILE('people.dbf')\n"
        "SET DATASESSION TO 1\n"
        "cNearRestored = SET('NEAR')\n"
        "cDefaultRestored = SET('DEFAULT')\n"
        "lFileRestored = FILE('people.dbf')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "eval/macro/state script should complete");

    const auto near_before = state.globals.find("cnearbefore");
    const auto near_after = state.globals.find("cnearafter");
    const auto default_before = state.globals.find("cdefaultbefore");
    const auto default_after = state.globals.find("cdefaultafter");
    const auto alias_from_eval = state.globals.find("caliasfromeval");
    const auto name_from_macro = state.globals.find("cnamefrommacro");
    const auto eval_age = state.globals.find("nevalage");
    const auto used_after_close = state.globals.find("lusedafterclose");
    const auto area_after_close = state.globals.find("nareaafterclose");
    const auto near_session2 = state.globals.find("cnearsession2");
    const auto default_session2 = state.globals.find("cdefaultsession2");
    const auto file_session2 = state.globals.find("lfilesession2");
    const auto near_restored = state.globals.find("cnearrestored");
    const auto default_restored = state.globals.find("cdefaultrestored");
    const auto file_restored = state.globals.find("lfilerestored");

    expect(near_before != state.globals.end(), "SET('NEAR') before enabling it should be captured");
    expect(near_after != state.globals.end(), "SET('NEAR') after enabling it should be captured");
    expect(default_before != state.globals.end(), "SET('DEFAULT') before change should be captured");
    expect(default_after != state.globals.end(), "SET('DEFAULT') after change should be captured");
    expect(alias_from_eval != state.globals.end(), "EVAL() should be able to evaluate runtime-state expressions");
    expect(name_from_macro != state.globals.end(), "&macro field resolution should be captured");
    expect(eval_age != state.globals.end(), "EVAL() of a stored expression should be captured");
    expect(used_after_close != state.globals.end(), "USE IN <expr> close semantics should be captured");
    expect(area_after_close != state.globals.end(), "SELECT('alias') after USE IN <expr> should be captured");
    expect(near_session2 != state.globals.end(), "SET('NEAR') in a fresh second session should be captured");
    expect(default_session2 != state.globals.end(), "SET('DEFAULT') in a fresh second session should be captured");
    expect(file_session2 != state.globals.end(), "FILE() in a fresh second session should be captured");
    expect(near_restored != state.globals.end(), "SET('NEAR') after restoring the original session should be captured");
    expect(default_restored != state.globals.end(), "SET('DEFAULT') after restoring the original session should be captured");
    expect(file_restored != state.globals.end(), "FILE() after restoring the original session should be captured");

    if (near_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_before->second) == "OFF", "SET('NEAR') should report OFF before it is enabled");
    }
    if (near_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_after->second) == "ON", "SET('NEAR') should report ON after SET NEAR ON");
    }
    if (default_before != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(default_before->second)) == lowercase_copy(temp_root.string()),
            "SET('DEFAULT') should expose the startup working directory before changes");
    }
    if (default_after != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(default_after->second)) == lowercase_copy(new_default.string()),
            "SET('DEFAULT') should expose the updated default directory");
    }
    if (alias_from_eval != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_from_eval->second) == "People", "EVAL('ALIAS()') should evaluate in the current runtime context");
    }
    if (name_from_macro != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_from_macro->second) == "ALPHA", "&macro should substitute a stored field name inside expressions");
    }
    if (eval_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(eval_age->second) == "15", "EVAL() should evaluate stored arithmetic expressions against the current record");
    }
    if (used_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_after_close->second) == "false", "USE IN <expr> should close the targeted alias");
    }
    if (area_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_after_close->second) == "0", "closing an alias through USE IN <expr> should clear SELECT('alias') lookup");
    }
    if (near_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_session2->second) == "OFF", "SET() state should stay isolated in a fresh data session");
    }
    if (default_session2 != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(default_session2->second)) == lowercase_copy(temp_root.string()),
            "a fresh data session should start with the startup working directory as SET('DEFAULT')");
    }
    if (file_session2 != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(file_session2->second) == "true",
            "relative FILE() checks in a fresh data session should resolve against that session's default directory");
    }
    if (near_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_restored->second) == "ON", "restoring the original data session should restore its SET() state");
    }
    if (default_restored != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(default_restored->second)) == lowercase_copy(new_default.string()),
            "restoring the original data session should restore its changed SET('DEFAULT') value");
    }
    if (file_restored != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(file_restored->second) == "false",
            "relative FILE() checks after restoring the original session should use that session's default directory");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_and_ole_compatibility_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_ole";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "interop.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers')\n"
        "nDisc = SQLDISCONNECT(nConn)\n"
        "oExcel = CREATEOBJECT('Excel.Application')\n"
        "oRunning = GETOBJECT('Word.Application')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "interop script should complete");
    const auto conn = state.globals.find("nconn");
    expect(conn != state.globals.end(), "SQLCONNECT should return a handle");
    if (conn != state.globals.end()) {
        expect(copperfin::runtime::format_value(conn->second) == "1", "first SQLCONNECT handle should be 1");
    }
    const auto exec = state.globals.find("nexec");
    expect(exec != state.globals.end(), "SQLEXEC should return a result code");
    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should report success for a known handle");
    }
    const auto disc = state.globals.find("ndisc");
    expect(disc != state.globals.end(), "SQLDISCONNECT should return a result code");
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should report success for a known handle");
    }
    expect(state.sql_connections.empty(), "SQLDISCONNECT should clear tracked SQL connections");
    expect(state.ole_objects.size() == 2U, "CREATEOBJECT and GETOBJECT should register OLE compatibility objects");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "sql.connect"; }),
        "SQLCONNECT should emit a sql.connect event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.createobject"; }),
        "CREATEOBJECT should emit an ole.createobject event");

    fs::remove_all(temp_root, ignored);
}

void test_use_and_data_session_isolation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_tables";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "tables.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "cAlias1 = ALIAS()\n"
        "nCount1 = RECCOUNT()\n"
        "nRec1 = RECNO()\n"
        "lEof1 = EOF()\n"
        "lBof1 = BOF()\n"
        "SELECT People\n"
        "SET DATASESSION TO 2\n"
        "cAlias2 = ALIAS()\n"
        "nCount2 = RECCOUNT()\n"
        "USE '" + table_path.string() + "' ALIAS SessionTwo IN 0\n"
        "cAlias3 = ALIAS()\n"
        "SET DATASESSION TO 1\n"
        "cAlias4 = ALIAS()\n"
        "USE IN People\n"
        "cAlias5 = ALIAS()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "table/data-session script should complete");
    expect(state.work_area.data_session == 1, "runtime should finish back in data session 1");
    expect(state.work_area.aliases.empty(), "USE IN People should close the session-1 cursor");

    const auto alias1 = state.globals.find("calias1");
    const auto count1 = state.globals.find("ncount1");
    const auto rec1 = state.globals.find("nrec1");
    const auto eof1 = state.globals.find("leof1");
    const auto bof1 = state.globals.find("lbof1");
    const auto alias2 = state.globals.find("calias2");
    const auto count2 = state.globals.find("ncount2");
    const auto alias3 = state.globals.find("calias3");
    const auto alias4 = state.globals.find("calias4");
    const auto alias5 = state.globals.find("calias5");

    expect(alias1 != state.globals.end(), "ALIAS() after USE should be captured");
    expect(count1 != state.globals.end(), "RECCOUNT() after USE should be captured");
    expect(rec1 != state.globals.end(), "RECNO() after USE should be captured");
    expect(eof1 != state.globals.end(), "EOF() after USE should be captured");
    expect(bof1 != state.globals.end(), "BOF() after USE should be captured");
    expect(alias2 != state.globals.end(), "ALIAS() in a fresh second data session should be captured");
    expect(count2 != state.globals.end(), "RECCOUNT() in a fresh second data session should be captured");
    expect(alias3 != state.globals.end(), "ALIAS() after opening a second-session cursor should be captured");
    expect(alias4 != state.globals.end(), "ALIAS() after restoring session 1 should be captured");
    expect(alias5 != state.globals.end(), "ALIAS() after USE IN should be captured");

    if (alias1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias1->second) == "People", "USE ... ALIAS should establish the alias in the selected work area");
    }
    if (count1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(count1->second) == "2", "RECCOUNT() should reflect opened DBF rows");
    }
    if (rec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec1->second) == "1", "RECNO() should start at the first record");
    }
    if (eof1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof1->second) == "false", "EOF() should be false immediately after opening a non-empty table");
    }
    if (bof1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof1->second) == "false", "BOF() should be false immediately after opening a non-empty table");
    }
    if (alias2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias2->second).empty(), "switching to a fresh data session should isolate work-area aliases");
    }
    if (count2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(count2->second) == "0", "RECCOUNT() in a fresh data session should be zero");
    }
    if (alias3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias3->second) == "SessionTwo", "session 2 should maintain its own alias set");
    }
    if (alias4 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias4->second) == "People", "restoring data session 1 should restore its selected alias");
    }
    if (alias5 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias5->second).empty(), "USE IN alias should close the targeted work area");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_existing_alias_reuses_target_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_in_existing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});

    const fs::path main_path = temp_root / "use_in_existing_alias.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "nAreaBefore = SELECT()\n"
        "cAliasBefore = ALIAS()\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN People\n"
        "nAreaAfter = SELECT()\n"
        "cAliasAfter = ALIAS()\n"
        "cTopName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "USE ... IN existing alias script should complete");
    expect(state.work_area.selected == 1, "USE ... IN alias should keep the targeted work area selected");
    expect(state.work_area.aliases.size() == 1U, "USE ... IN alias should replace the target work area instead of allocating a new one");

    const auto area_before = state.globals.find("nareabefore");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto area_after = state.globals.find("nareaafter");
    const auto alias_after = state.globals.find("caliasafter");
    const auto top_name = state.globals.find("ctopname");

    expect(area_before != state.globals.end(), "initial SELECT() should be captured");
    expect(alias_before != state.globals.end(), "initial ALIAS() should be captured");
    expect(area_after != state.globals.end(), "SELECT() after USE ... IN alias should be captured");
    expect(alias_after != state.globals.end(), "ALIAS() after USE ... IN alias should be captured");
    expect(top_name != state.globals.end(), "replacement cursor fields should be available after USE ... IN alias");

    if (area_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_before->second) == "1", "initial USE IN 0 should allocate work area 1");
    }
    if (alias_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_before->second) == "People", "initial alias should be visible before replacement");
    }
    if (area_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_after->second) == "1", "USE ... IN alias should reuse the alias target's work area");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Cities", "USE ... IN alias should replace the target work area's alias");
    }
    if (top_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_name->second) == "OSLO", "USE ... IN alias should expose the replacement table's current record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_nonselected_alias_preserves_selected_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_in_nonselected_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path orders_path = temp_root / "orders.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});
    write_simple_dbf(orders_path, {"ORD100", "ORD200"});

    const fs::path main_path = temp_root / "use_in_nonselected_alias.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN 0\n"
        "SELECT People\n"
        "nSelectedBefore = SELECT()\n"
        "cAliasBefore = ALIAS()\n"
        "USE '" + orders_path.string() + "' ALIAS Orders IN Cities\n"
        "nSelectedAfterReplace = SELECT()\n"
        "cAliasAfterReplace = ALIAS()\n"
        "nOrdersArea = SELECT('Orders')\n"
        "cOrdersDbf = DBF('Orders')\n"
        "USE IN Orders\n"
        "nSelectedAfterClose = SELECT()\n"
        "cAliasAfterClose = ALIAS()\n"
        "lOrdersOpenAfterClose = USED('Orders')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "USE ... IN non-selected alias script should complete");
    expect(state.work_area.selected == 1, "replacing and closing a non-selected alias should preserve the selected work area");
    expect(state.work_area.aliases.size() == 1U, "closing the non-selected replacement alias should leave only the selected alias open");

    const auto selected_before = state.globals.find("nselectedbefore");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto selected_after_replace = state.globals.find("nselectedafterreplace");
    const auto alias_after_replace = state.globals.find("caliasafterreplace");
    const auto orders_area = state.globals.find("nordersarea");
    const auto orders_dbf = state.globals.find("cordersdbf");
    const auto selected_after_close = state.globals.find("nselectedafterclose");
    const auto alias_after_close = state.globals.find("caliasafterclose");
    const auto orders_open_after_close = state.globals.find("lordersopenafterclose");

    expect(selected_before != state.globals.end(), "initial selected work area should be captured");
    expect(alias_before != state.globals.end(), "initial selected alias should be captured");
    expect(selected_after_replace != state.globals.end(), "selected work area after non-selected replacement should be captured");
    expect(alias_after_replace != state.globals.end(), "selected alias after non-selected replacement should be captured");
    expect(orders_area != state.globals.end(), "replacement alias work area should be discoverable");
    expect(orders_dbf != state.globals.end(), "replacement alias DBF identity should be captured");
    expect(selected_after_close != state.globals.end(), "selected work area after closing the non-selected alias should be captured");
    expect(alias_after_close != state.globals.end(), "selected alias after closing the non-selected alias should be captured");
    expect(orders_open_after_close != state.globals.end(), "USED('Orders') after close should be captured");

    if (selected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_before->second) == "1", "People should remain selected before the non-selected replacement");
    }
    if (alias_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_before->second) == "People", "People should be the selected alias before replacement");
    }
    if (selected_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_replace->second) == "1", "USE ... IN a non-selected alias should preserve the selected work area");
    }
    if (alias_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_replace->second) == "People", "USE ... IN a non-selected alias should preserve the selected alias");
    }
    if (orders_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_area->second) == "2", "the replacement alias should still reuse the targeted non-selected work area");
    }
    if (orders_dbf != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_dbf->second) == orders_path.lexically_normal().string(), "the replacement alias should point at the new table");
    }
    if (selected_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_close->second) == "1", "closing a non-selected alias should preserve the selected work area");
    }
    if (alias_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_close->second) == "People", "closing a non-selected alias should preserve the selected alias");
    }
    if (orders_open_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_open_after_close->second) == "false", "closing the replacement alias should clear its USED() state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_plain_use_reuses_current_selected_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_plain_use_current_area";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});

    const fs::path main_path = temp_root / "plain_use_current_area.prg";
    write_text(
        main_path,
        "SELECT 0\n"
        "nAreaEmpty = SELECT()\n"
        "USE '" + people_path.string() + "' ALIAS People\n"
        "nAreaAfterFirstUse = SELECT()\n"
        "cAliasAfterFirstUse = ALIAS()\n"
        "USE '" + cities_path.string() + "' ALIAS Cities\n"
        "nAreaAfterReplace = SELECT()\n"
        "cAliasAfterReplace = ALIAS()\n"
        "cDbfAfterReplace = DBF()\n"
        "nPeopleAreaAfterReplace = SELECT('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "plain USE current-area script should complete");
    expect(state.work_area.selected == 1, "plain USE should keep using the current selected work area");
    expect(state.work_area.aliases.size() == 1U, "plain USE replacement should reuse the selected work area instead of allocating another one");

    const auto area_empty = state.globals.find("nareaempty");
    const auto area_after_first_use = state.globals.find("nareaafterfirstuse");
    const auto alias_after_first_use = state.globals.find("caliasafterfirstuse");
    const auto area_after_replace = state.globals.find("nareaafterreplace");
    const auto alias_after_replace = state.globals.find("caliasafterreplace");
    const auto dbf_after_replace = state.globals.find("cdbfafterreplace");
    const auto people_area_after_replace = state.globals.find("npeopleareaafterreplace");

    expect(area_empty != state.globals.end(), "selected empty area should be captured");
    expect(area_after_first_use != state.globals.end(), "plain USE on an empty selected area should be captured");
    expect(alias_after_first_use != state.globals.end(), "alias after the first plain USE should be captured");
    expect(area_after_replace != state.globals.end(), "plain USE replacement area should be captured");
    expect(alias_after_replace != state.globals.end(), "alias after plain USE replacement should be captured");
    expect(dbf_after_replace != state.globals.end(), "DBF() after plain USE replacement should be captured");
    expect(people_area_after_replace != state.globals.end(), "SELECT('People') after plain USE replacement should be captured");

    if (area_empty != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_empty->second) == "1", "SELECT 0 should choose work area 1 in a fresh session");
    }
    if (area_after_first_use != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_after_first_use->second) == "1", "plain USE should fill the currently selected empty work area");
    }
    if (alias_after_first_use != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_first_use->second) == "People", "plain USE should establish its alias in the current work area");
    }
    if (area_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_after_replace->second) == "1", "plain USE replacement should stay in the selected work area");
    }
    if (alias_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_replace->second) == "Cities", "plain USE replacement should swap the alias in place");
    }
    if (dbf_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(dbf_after_replace->second) == cities_path.lexically_normal().string(), "plain USE replacement should point at the replacement table");
    }
    if (people_area_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_area_after_replace->second) == "0", "plain USE replacement should remove the replaced alias from SELECT('alias') lookup");
    }

    fs::remove_all(temp_root, ignored);
}

void test_select_and_use_in_designator_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_designator_expressions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path orders_path = temp_root / "orders.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "ROME"});
    write_simple_dbf(orders_path, {"ORDER1", "ORDER2"});

    const fs::path main_path = temp_root / "designator_expressions.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN 0\n"
        "nTargetArea = 1\n"
        "SELECT nTargetArea\n"
        "nSelectedNumeric = SELECT()\n"
        "cSelectedNumericAlias = ALIAS()\n"
        "cTargetAlias = 'Cities'\n"
        "SELECT cTargetAlias\n"
        "nSelectedAlias = SELECT()\n"
        "cSelectedAlias = ALIAS()\n"
        "USE '" + orders_path.string() + "' ALIAS Orders IN cTargetAlias\n"
        "nOrdersArea = SELECT('Orders')\n"
        "nCitiesArea = SELECT('Cities')\n"
        "nSelectedAfterUse = SELECT()\n"
        "cSelectedAfterUse = ALIAS()\n"
        "GO TOP\n"
        "cTopAfterUse = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "expression-based SELECT and USE ... IN script should complete");

    const auto selected_numeric = state.globals.find("nselectednumeric");
    const auto selected_numeric_alias = state.globals.find("cselectednumericalias");
    const auto selected_alias = state.globals.find("nselectedalias");
    const auto selected_alias_name = state.globals.find("cselectedalias");
    const auto orders_area = state.globals.find("nordersarea");
    const auto cities_area = state.globals.find("ncitiesarea");
    const auto selected_after_use = state.globals.find("nselectedafteruse");
    const auto selected_after_use_name = state.globals.find("cselectedafteruse");
    const auto top_after_use = state.globals.find("ctopafteruse");

    expect(selected_numeric != state.globals.end(), "SELECT nTargetArea should expose the selected area");
    expect(selected_numeric_alias != state.globals.end(), "SELECT nTargetArea should expose the selected alias");
    expect(selected_alias != state.globals.end(), "SELECT cTargetAlias should expose the selected area");
    expect(selected_alias_name != state.globals.end(), "SELECT cTargetAlias should expose the selected alias");
    expect(orders_area != state.globals.end(), "USE ... IN cTargetAlias should expose the replacement alias area");
    expect(cities_area != state.globals.end(), "USE ... IN cTargetAlias should clear the replaced alias lookup");
    expect(selected_after_use != state.globals.end(), "USE ... IN cTargetAlias should preserve the selected work area");
    expect(selected_after_use_name != state.globals.end(), "USE ... IN cTargetAlias should select the replacement alias");
    expect(top_after_use != state.globals.end(), "USE ... IN cTargetAlias should expose the replacement table's current record");

    if (selected_numeric != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_numeric->second) == "1", "SELECT nTargetArea should resolve numeric expression targets");
    }
    if (selected_numeric_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_numeric_alias->second) == "People", "SELECT nTargetArea should select the numeric target area");
    }
    if (selected_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_alias->second) == "2", "SELECT cTargetAlias should resolve alias expression targets");
    }
    if (selected_alias_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_alias_name->second) == "Cities", "SELECT cTargetAlias should select the targeted alias");
    }
    if (orders_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_area->second) == "2", "USE ... IN cTargetAlias should reuse the targeted alias work area");
    }
    if (cities_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(cities_area->second) == "0", "USE ... IN cTargetAlias should remove the replaced alias");
    }
    if (selected_after_use != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_use->second) == "2", "USE ... IN cTargetAlias should keep the targeted work area selected");
    }
    if (selected_after_use_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_use_name->second) == "Orders", "USE ... IN cTargetAlias should select the replacement alias");
    }
    if (top_after_use != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_after_use->second) == "ORDER1", "USE ... IN cTargetAlias should expose the replacement table rows");
    }

    fs::remove_all(temp_root, ignored);
}

void test_expression_driven_in_targeting_across_local_data_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_in_target_commands";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path orders_path = temp_root / "orders.dbf";
    const fs::path people_cdx_path = temp_root / "people.cdx";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_people_dbf(cities_path, {{"OSLO", 1}, {"ROME", 2}});
    write_people_dbf(orders_path, {{"ONE", 10}, {"TWO", 20}, {"THREE", 30}});
    write_synthetic_cdx(people_cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "in_target_commands.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN 0\n"
        "USE '" + orders_path.string() + "' ALIAS Orders IN 0\n"
        "SELECT Cities\n"
        "cOrderTarget = 'People'\n"
        "cDataTarget = 'Orders'\n"
        "cResolvedTargetAlias = ALIAS(cOrderTarget)\n"
        "cPeopleTagBefore = TAG(1, 'People')\n"
        "SET ORDER TO TAG NAME IN cOrderTarget\n"
        "cPeopleOrder = ORDER('People')\n"
        "cCitiesOrder = ORDER('Cities')\n"
        "SEEK 'BRAVO' IN cOrderTarget\n"
        "nPeopleRecAfterSeek = RECNO('People')\n"
        "GO TOP IN cDataTarget\n"
        "SKIP 1 IN cDataTarget\n"
        "nOrdersRecAfterSkip = RECNO('Orders')\n"
        "LOCATE FOR AGE = 30 IN cDataTarget\n"
        "nOrdersRecAfterLocate = RECNO('Orders')\n"
        "GO TOP IN cDataTarget\n"
        "nScanHits = 0\n"
        "SCAN FOR AGE >= 20 IN cDataTarget\n"
        "  nScanHits = nScanHits + 1\n"
        "ENDSCAN\n"
        "GO BOTTOM IN cDataTarget\n"
        "REPLACE AGE WITH 99 IN cDataTarget\n"
        "DELETE FOR AGE = 99 IN cDataTarget\n"
        "RECALL FOR AGE = 99 IN cDataTarget\n"
        "nSelectedAfterCommands = SELECT()\n"
        "cSelectedAfterCommands = ALIAS()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "expression-driven IN targeting script should complete");

    const auto people_order = state.globals.find("cpeopleorder");
    const auto cities_order = state.globals.find("ccitiesorder");
    const auto people_rec_after_seek = state.globals.find("npeoplerecafterseek");
    const auto orders_rec_after_skip = state.globals.find("nordersrecafterskip");
    const auto orders_rec_after_locate = state.globals.find("nordersrecafterlocate");
    const auto scan_hits = state.globals.find("nscanhits");
    const auto selected_after_commands = state.globals.find("nselectedaftercommands");
    const auto selected_alias_after_commands = state.globals.find("cselectedaftercommands");

    expect(people_order != state.globals.end(), "SET ORDER TO ... IN cTarget should expose the targeted cursor order");
    expect(cities_order != state.globals.end(), "SET ORDER TO ... IN cTarget should leave the selected cursor order unchanged");
    expect(people_rec_after_seek != state.globals.end(), "SEEK ... IN cTarget should expose the targeted cursor position");
    expect(orders_rec_after_skip != state.globals.end(), "GO/SKIP IN cTarget should expose the targeted cursor position");
    expect(orders_rec_after_locate != state.globals.end(), "LOCATE ... IN cTarget should expose the targeted cursor position");
    expect(scan_hits != state.globals.end(), "SCAN ... IN cTarget should expose the targeted cursor iteration count");
    expect(selected_after_commands != state.globals.end(), "IN-targeted commands should preserve the current selected work area");
    expect(selected_alias_after_commands != state.globals.end(), "IN-targeted commands should preserve the current selected alias");

    if (people_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_order->second) == "NAME", "SET ORDER TO ... IN cTarget should affect the targeted non-selected cursor");
    }
    if (cities_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(cities_order->second).empty(), "SET ORDER TO ... IN cTarget should not alter the selected cursor");
    }
    if (people_rec_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec_after_seek->second) == "2", "SEEK ... IN cTarget should position the targeted cursor");
    }
    if (orders_rec_after_skip != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_rec_after_skip->second) == "2", "GO/SKIP IN cTarget should navigate the targeted cursor");
    }
    if (orders_rec_after_locate != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_rec_after_locate->second) == "3", "LOCATE ... IN cTarget should find records on the targeted cursor");
    }
    if (scan_hits != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_hits->second) == "2", "SCAN ... IN cTarget should iterate visible matches on the targeted cursor");
    }
    if (selected_after_commands != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_commands->second) == "2", "IN-targeted commands should preserve the selected work area");
    }
    if (selected_alias_after_commands != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_alias_after_commands->second) == "Cities", "IN-targeted commands should preserve the selected alias");
    }

    const auto orders_result = copperfin::vfp::parse_dbf_table_from_file(orders_path.string(), 3U);
    expect(orders_result.ok, "Orders DBF should remain readable after IN-targeted mutation commands");
    if (orders_result.ok) {
        expect(orders_result.table.records.size() == 3U, "Orders DBF should still contain three records");
        if (orders_result.table.records.size() >= 3U) {
            expect(orders_result.table.records[2].values[0].display_value == "THREE", "targeted mutation flow should keep the THREE record");
            expect(orders_result.table.records[2].values[1].display_value == "99", "REPLACE ... IN cTarget should update the targeted cursor record");
            expect(!orders_result.table.records[2].deleted, "DELETE/RECALL ... IN cTarget should leave the targeted record recalled");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_select_zero_and_use_in_zero_reuse_closed_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_select_zero_reuse";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});

    const fs::path main_path = temp_root / "select_zero_reuse.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE IN People\n"
        "nSelectedAfterClose = SELECT()\n"
        "nNextFreeBeforeReuse = SELECT(0)\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN 0\n"
        "nCitiesArea = SELECT('Cities')\n"
        "nSelectedAfterReuse = SELECT()\n"
        "nNextFreeAfterReuse = SELECT(0)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SELECT(0) reuse script should complete");
    expect(state.work_area.selected == 1, "reopening through IN 0 should keep the reused work area selected");
    expect(state.work_area.aliases.size() == 1U, "reopening through IN 0 should reuse the freed work area instead of allocating a new one");

    const auto selected_after_close = state.globals.find("nselectedafterclose");
    const auto next_free_before_reuse = state.globals.find("nnextfreebeforereuse");
    const auto cities_area = state.globals.find("ncitiesarea");
    const auto selected_after_reuse = state.globals.find("nselectedafterreuse");
    const auto next_free_after_reuse = state.globals.find("nnextfreeafterreuse");

    expect(selected_after_close != state.globals.end(), "selected work area after closing the cursor should be captured");
    expect(next_free_before_reuse != state.globals.end(), "SELECT(0) before reopening should be captured");
    expect(cities_area != state.globals.end(), "SELECT('Cities') after reopening should be captured");
    expect(selected_after_reuse != state.globals.end(), "selected work area after reopening should be captured");
    expect(next_free_after_reuse != state.globals.end(), "SELECT(0) after reopening should be captured");

    if (selected_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_close->second) == "1", "closing the selected alias should leave the same work area selected");
    }
    if (next_free_before_reuse != state.globals.end()) {
        expect(copperfin::runtime::format_value(next_free_before_reuse->second) == "1", "SELECT(0) should report the freed work area without consuming it");
    }
    if (cities_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(cities_area->second) == "1", "USE ... IN 0 should reuse the freed work area");
    }
    if (selected_after_reuse != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_reuse->second) == "1", "reopening through IN 0 should keep the reused work area selected");
    }
    if (next_free_after_reuse != state.globals.end()) {
        expect(copperfin::runtime::format_value(next_free_after_reuse->second) == "2", "SELECT(0) should advance only after the freed work area is reused");
    }

    fs::remove_all(temp_root, ignored);
}

void test_go_and_skip_cursor_navigation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_navigation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});

    const fs::path main_path = temp_root / "navigate.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO BOTTOM\n"
        "nBottom = RECNO()\n"
        "SKIP\n"
        "lEof1 = EOF()\n"
        "nAfterEof = RECNO()\n"
        "SKIP -1\n"
        "nBackOne = RECNO()\n"
        "GO TOP\n"
        "nTop = RECNO()\n"
        "SKIP -1\n"
        "lBof1 = BOF()\n"
        "nAfterBof = RECNO()\n"
        "GO 2\n"
        "nMiddle = RECNO()\n"
        "GO 99\n"
        "lEof2 = EOF()\n"
        "nAfterGoPastEnd = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "navigation script should complete");

    const auto bottom = state.globals.find("nbottom");
    const auto eof1 = state.globals.find("leof1");
    const auto after_eof = state.globals.find("naftereof");
    const auto back_one = state.globals.find("nbackone");
    const auto top = state.globals.find("ntop");
    const auto bof1 = state.globals.find("lbof1");
    const auto after_bof = state.globals.find("nafterbof");
    const auto middle = state.globals.find("nmiddle");
    const auto eof2 = state.globals.find("leof2");
    const auto after_go_past_end = state.globals.find("naftergopastend");

    expect(bottom != state.globals.end(), "GO BOTTOM should affect RECNO()");
    expect(eof1 != state.globals.end(), "SKIP past the end should affect EOF()");
    expect(after_eof != state.globals.end(), "RECNO() after EOF should be captured");
    expect(back_one != state.globals.end(), "SKIP -1 should recover from EOF");
    expect(top != state.globals.end(), "GO TOP should affect RECNO()");
    expect(bof1 != state.globals.end(), "SKIP -1 from the top should affect BOF()");
    expect(after_bof != state.globals.end(), "RECNO() after BOF should be captured");
    expect(middle != state.globals.end(), "GO 2 should move to the requested record");
    expect(eof2 != state.globals.end(), "GO past the end should affect EOF()");
    expect(after_go_past_end != state.globals.end(), "RECNO() after GO past the end should be captured");

    if (bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom->second) == "3", "GO BOTTOM should move to the last record");
    }
    if (eof1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof1->second) == "true", "SKIP from the last record should move to EOF");
    }
    if (after_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_eof->second) == "4", "RECNO() at EOF should be record_count + 1");
    }
    if (back_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(back_one->second) == "3", "SKIP -1 from EOF should move back to the last record");
    }
    if (top != state.globals.end()) {
        expect(copperfin::runtime::format_value(top->second) == "1", "GO TOP should move to the first record");
    }
    if (bof1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof1->second) == "true", "SKIP -1 from the first record should move to BOF");
    }
    if (after_bof != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_bof->second) == "0", "RECNO() at BOF should be zero");
    }
    if (middle != state.globals.end()) {
        expect(copperfin::runtime::format_value(middle->second) == "2", "GO 2 should move to the requested record number");
    }
    if (eof2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof2->second) == "true", "GO past the end should move to EOF");
    }
    if (after_go_past_end != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_go_past_end->second) == "4", "GO past the end should place RECNO() at record_count + 1");
    }

    expect(
        has_runtime_event(state.events, "runtime.go", "BOTTOM") &&
        has_runtime_event(state.events, "runtime.go", "TOP") &&
        has_runtime_event(state.events, "runtime.skip", "1") &&
        has_runtime_event(state.events, "runtime.skip", "-1"),
        "navigation commands should emit runtime.go and runtime.skip events");

    fs::remove_all(temp_root, ignored);
}

void test_cursor_identity_functions_for_local_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cursor_identity_local";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "identity_local.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "lUsed1 = USED()\n"
        "lUsedPeople = USED('People')\n"
        "cDbf1 = DBF()\n"
        "cDbfPeople = DBF('People')\n"
        "nFields1 = FCOUNT()\n"
        "nFieldsPeople = FCOUNT('People')\n"
        "SET DATASESSION TO 2\n"
        "lUsed2 = USED('People')\n"
        "SET DATASESSION TO 1\n"
        "USE IN People\n"
        "lUsedAfterClose = USED('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local cursor identity script should complete");

    const auto used1 = state.globals.find("lused1");
    const auto used_people = state.globals.find("lusedpeople");
    const auto dbf1 = state.globals.find("cdbf1");
    const auto dbf_people = state.globals.find("cdbfpeople");
    const auto fields1 = state.globals.find("nfields1");
    const auto fields_people = state.globals.find("nfieldspeople");
    const auto used2 = state.globals.find("lused2");
    const auto used_after_close = state.globals.find("lusedafterclose");

    expect(used1 != state.globals.end(), "USED() should be captured for the current local cursor");
    expect(used_people != state.globals.end(), "USED('People') should be captured for the named local cursor");
    expect(dbf1 != state.globals.end(), "DBF() should be captured for the current local cursor");
    expect(dbf_people != state.globals.end(), "DBF('People') should be captured for the named local cursor");
    expect(fields1 != state.globals.end(), "FCOUNT() should be captured for the current local cursor");
    expect(fields_people != state.globals.end(), "FCOUNT('People') should be captured for the named local cursor");
    expect(used2 != state.globals.end(), "USED('People') in a different data session should be captured");
    expect(used_after_close != state.globals.end(), "USED('People') after USE IN should be captured");

    if (used1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(used1->second) == "true", "USED() should report true for an open local cursor");
    }
    if (used_people != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_people->second) == "true", "USED('People') should report true for an open alias");
    }
    if (dbf1 != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(dbf1->second)).find("people.dbf") != std::string::npos,
            "DBF() should expose the opened local table path");
    }
    if (dbf_people != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(dbf_people->second)).find("people.dbf") != std::string::npos,
            "DBF('People') should expose the opened local table path");
    }
    if (fields1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields1->second) == "1", "FCOUNT() should reflect the local DBF schema");
    }
    if (fields_people != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields_people->second) == "1", "FCOUNT('People') should reflect the local DBF schema");
    }
    if (used2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(used2->second) == "false", "data sessions should isolate USED('alias') state");
    }
    if (used_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_after_close->second) == "false", "USE IN should clear USED('alias') state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_cursor_identity_functions_for_sql_result_cursors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cursor_identity_sql";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "identity_sql.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "lUsed = USED('sqlcust')\n"
        "cDbf = DBF('sqlcust')\n"
        "nFields = FCOUNT('sqlcust')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL cursor identity script should complete");

    const auto used = state.globals.find("lused");
    const auto dbf = state.globals.find("cdbf");
    const auto fields = state.globals.find("nfields");

    expect(used != state.globals.end(), "USED('sqlcust') should be captured for the SQL cursor");
    expect(dbf != state.globals.end(), "DBF('sqlcust') should be captured for the SQL cursor");
    expect(fields != state.globals.end(), "FCOUNT('sqlcust') should be captured for the SQL cursor");

    if (used != state.globals.end()) {
        expect(copperfin::runtime::format_value(used->second) == "true", "USED('sqlcust') should report true for a materialized SQL cursor");
    }
    if (dbf != state.globals.end()) {
        expect(copperfin::runtime::format_value(dbf->second) == "sqlcust", "DBF('sqlcust') should expose the runtime identity for a SQL cursor");
    }
    if (fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields->second) == "3", "FCOUNT('sqlcust') should expose the synthetic SQL cursor schema");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursors_are_isolated_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_datasession.prg";
    write_text(
        main_path,
        "nConn1 = SQLCONNECT('dsn=Northwind')\n"
        "nExec1 = SQLEXEC(nConn1, 'select * from customers', 'sqlcust')\n"
        "nArea1 = SELECT('sqlcust')\n"
        "SET DATASESSION TO 2\n"
        "lUsedSession2Before = USED('sqlcust')\n"
        "nAreaSession2Before = SELECT('sqlcust')\n"
        "nExecCrossSession = SQLEXEC(nConn1, 'select * from orders', 'sqlcust2')\n"
        "lDisconnectSession2BeforeConnect = SQLDISCONNECT(nConn1)\n"
        "nConn2 = SQLCONNECT('dsn=SessionTwo')\n"
        "nExec2 = SQLEXEC(nConn2, 'select * from orders', 'sqlother')\n"
        "lUsedSession2After = USED('sqlother')\n"
        "nAreaSession2After = SELECT('sqlother')\n"
        "lDisconnectSession2Own = SQLDISCONNECT(nConn2)\n"
        "SET DATASESSION TO 1\n"
        "lUsedSession1Back = USED('sqlcust')\n"
        "nAreaSession1Back = SELECT('sqlcust')\n"
        "lUsedSession1Other = USED('sqlother')\n"
        "nConn1Again = SQLCONNECT('dsn=NorthwindAgain')\n"
        "lDisconnectSession1Again = SQLDISCONNECT(nConn1Again)\n"
        "lDisconnectSession1Own = SQLDISCONNECT(nConn1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL data-session isolation script should complete");
    expect(state.work_area.data_session == 1, "SQL data-session isolation script should restore data session 1");
    expect(state.sql_connections.empty(), "all session-local SQL connections should be disconnected by the end of the script");

    const auto area1 = state.globals.find("narea1");
    const auto used_session2_before = state.globals.find("lusedsession2before");
    const auto area_session2_before = state.globals.find("nareasession2before");
    const auto exec_cross_session = state.globals.find("nexeccrosssession");
    const auto disconnect_session2_before_connect = state.globals.find("ldisconnectsession2beforeconnect");
    const auto conn2 = state.globals.find("nconn2");
    const auto exec2 = state.globals.find("nexec2");
    const auto used_session2_after = state.globals.find("lusedsession2after");
    const auto area_session2_after = state.globals.find("nareasession2after");
    const auto disconnect_session2_own = state.globals.find("ldisconnectsession2own");
    const auto used_session1_back = state.globals.find("lusedsession1back");
    const auto area_session1_back = state.globals.find("nareasession1back");
    const auto used_session1_other = state.globals.find("lusedsession1other");
    const auto conn1_again = state.globals.find("nconn1again");
    const auto disconnect_session1_again = state.globals.find("ldisconnectsession1again");
    const auto disconnect_session1_own = state.globals.find("ldisconnectsession1own");

    expect(area1 != state.globals.end(), "session-1 SQL cursor area should be captured");
    expect(used_session2_before != state.globals.end(), "session-2 preexisting SQL cursor visibility should be captured");
    expect(area_session2_before != state.globals.end(), "session-2 preexisting SQL cursor area should be captured");
    expect(exec_cross_session != state.globals.end(), "cross-session SQLEXEC result should be captured");
    expect(disconnect_session2_before_connect != state.globals.end(), "cross-session SQLDISCONNECT before a local connect should be captured");
    expect(conn2 != state.globals.end(), "session-2 SQLCONNECT handle should be captured");
    expect(exec2 != state.globals.end(), "session-2 SQLEXEC result should be captured");
    expect(used_session2_after != state.globals.end(), "session-2 SQL cursor visibility should be captured");
    expect(area_session2_after != state.globals.end(), "session-2 SQL cursor area should be captured");
    expect(disconnect_session2_own != state.globals.end(), "session-2 SQLDISCONNECT result should be captured");
    expect(used_session1_back != state.globals.end(), "restored session-1 SQL cursor visibility should be captured");
    expect(area_session1_back != state.globals.end(), "restored session-1 SQL cursor area should be captured");
    expect(used_session1_other != state.globals.end(), "restored session-1 visibility for session-2 alias should be captured");
    expect(conn1_again != state.globals.end(), "restored session-1 SQLCONNECT handle should be captured");
    expect(disconnect_session1_again != state.globals.end(), "restored session-1 second SQLDISCONNECT result should be captured");
    expect(disconnect_session1_own != state.globals.end(), "session-1 SQLDISCONNECT result should be captured");

    if (area1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(area1->second) == "1", "session 1 should materialize its SQL cursor in work area 1");
    }
    if (used_session2_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_session2_before->second) == "false", "switching to a fresh data session should hide session-1 SQL cursors");
    }
    if (area_session2_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_session2_before->second) == "0", "SELECT('alias') should not resolve a SQL cursor from another data session");
    }
    if (exec_cross_session != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cross_session->second) == "-1", "SQLEXEC should reject a SQL handle from another data session");
    }
    if (disconnect_session2_before_connect != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session2_before_connect->second) == "-1", "SQLDISCONNECT should reject a SQL handle from another data session before the session creates its own handle");
    }
    if (conn2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(conn2->second) == "1", "the first SQLCONNECT handle in a fresh data session should restart at 1");
    }
    if (exec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec2->second) == "1", "session 2 should still be able to create its own SQL cursor");
    }
    if (used_session2_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_session2_after->second) == "true", "session 2 should see its own SQL cursor");
    }
    if (area_session2_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_session2_after->second) == "1", "session 2 should resolve its own SQL cursor area");
    }
    if (disconnect_session2_own != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session2_own->second) == "1", "session 2 should disconnect its own SQL handle");
    }
    if (used_session1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_session1_back->second) == "true", "restoring session 1 should restore its SQL cursor visibility");
    }
    if (area_session1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_session1_back->second) == "1", "restoring session 1 should restore its SQL cursor work area");
    }
    if (used_session1_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_session1_other->second) == "false", "session-2 SQL aliases should stay hidden after restoring session 1");
    }
    if (conn1_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(conn1_again->second) == "2", "restoring session 1 should resume that session's SQLCONNECT handle numbering");
    }
    if (disconnect_session1_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session1_again->second) == "1", "session 1 should disconnect its later SQL handle after restoring the session");
    }
    if (disconnect_session1_own != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session1_own->second) == "1", "session 1 should disconnect its own SQL handle");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_auto_allocation_tracks_session_selection_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_selection_flow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_selection_flow.prg";
    write_text(
        main_path,
        "nConn1 = SQLCONNECT('dsn=Northwind')\n"
        "SELECT 0\n"
        "nSession1SelectedBefore = SELECT()\n"
        "nExec1 = SQLEXEC(nConn1, 'select * from customers', 'sqlcust1')\n"
        "nSession1Area = SELECT('sqlcust1')\n"
        "nSession1SelectedAfter = SELECT()\n"
        "SET DATASESSION TO 2\n"
        "nConn2 = SQLCONNECT('dsn=SessionTwo')\n"
        "SELECT 0\n"
        "SELECT 0\n"
        "nSession2SelectedBefore = SELECT()\n"
        "nExec2 = SQLEXEC(nConn2, 'select * from orders', 'sqlcust2')\n"
        "nSession2Area = SELECT('sqlcust2')\n"
        "nSession2SelectedAfter = SELECT()\n"
        "lDisc2 = SQLDISCONNECT(nConn2)\n"
        "SET DATASESSION TO 1\n"
        "nSession1AreaBack = SELECT('sqlcust1')\n"
        "lDisc1 = SQLDISCONNECT(nConn1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL selection-flow script should complete");
    expect(state.work_area.data_session == 1, "SQL selection-flow script should restore data session 1");
    expect(state.sql_connections.empty(), "SQL selection-flow script should disconnect all session-local handles");

    const auto session1_selected_before = state.globals.find("nsession1selectedbefore");
    const auto exec1 = state.globals.find("nexec1");
    const auto session1_area = state.globals.find("nsession1area");
    const auto session1_selected_after = state.globals.find("nsession1selectedafter");
    const auto session2_selected_before = state.globals.find("nsession2selectedbefore");
    const auto exec2 = state.globals.find("nexec2");
    const auto session2_area = state.globals.find("nsession2area");
    const auto session2_selected_after = state.globals.find("nsession2selectedafter");
    const auto session1_area_back = state.globals.find("nsession1areaback");
    const auto disc2 = state.globals.find("ldisc2");
    const auto disc1 = state.globals.find("ldisc1");

    expect(session1_selected_before != state.globals.end(), "session-1 selected area before SQLEXEC should be captured");
    expect(exec1 != state.globals.end(), "session-1 SQLEXEC result should be captured");
    expect(session1_area != state.globals.end(), "session-1 SQL cursor area should be captured");
    expect(session1_selected_after != state.globals.end(), "session-1 selected area after SQLEXEC should be captured");
    expect(session2_selected_before != state.globals.end(), "session-2 selected area before SQLEXEC should be captured");
    expect(exec2 != state.globals.end(), "session-2 SQLEXEC result should be captured");
    expect(session2_area != state.globals.end(), "session-2 SQL cursor area should be captured");
    expect(session2_selected_after != state.globals.end(), "session-2 selected area after SQLEXEC should be captured");
    expect(session1_area_back != state.globals.end(), "session-1 SQL cursor area after restoring the session should be captured");
    expect(disc2 != state.globals.end(), "session-2 SQLDISCONNECT result should be captured");
    expect(disc1 != state.globals.end(), "session-1 SQLDISCONNECT result should be captured");

    if (session1_selected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_before->second) == "1", "session 1 should auto-select work area 1 before its first SQLEXEC");
    }
    if (exec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec1->second) == "1", "session 1 SQLEXEC should succeed");
    }
    if (session1_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_area->second) == "1", "session 1 SQLEXEC should reuse the selected empty work area");
    }
    if (session1_selected_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_after->second) == "1", "session 1 should keep the SQL cursor on its selected work area");
    }
    if (session2_selected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_selected_before->second) == "2", "session 2 should preserve its own current SELECT 0 flow before SQLEXEC");
    }
    if (exec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec2->second) == "1", "session 2 SQLEXEC should succeed");
    }
    if (session2_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_area->second) == "2", "session 2 SQLEXEC should reuse that session's selected empty work area");
    }
    if (session2_selected_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_selected_after->second) == "2", "session 2 should keep its SQL cursor on the selected work area");
    }
    if (session1_area_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_area_back->second) == "1", "restoring session 1 should keep its SQL cursor bound to session 1's selection flow");
    }
    if (disc2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc2->second) == "1", "session 2 should disconnect its own SQL handle");
    }
    if (disc1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc1->second) == "1", "session 1 should disconnect its own SQL handle");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_order_and_seek_for_local_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SEEK 'BRAVO'\n"
        "lFound1 = FOUND()\n"
        "nRec1 = RECNO()\n"
        "SEEK 'ZZZZ'\n"
        "lFound2 = FOUND()\n"
        "lEof2 = EOF()\n"
        "nRec2 = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "seek script should complete");

    const auto found1 = state.globals.find("lfound1");
    const auto rec1 = state.globals.find("nrec1");
    const auto found2 = state.globals.find("lfound2");
    const auto eof2 = state.globals.find("leof2");
    const auto rec2 = state.globals.find("nrec2");

    expect(found1 != state.globals.end(), "FOUND() after a successful SEEK should be captured");
    expect(rec1 != state.globals.end(), "RECNO() after a successful SEEK should be captured");
    expect(found2 != state.globals.end(), "FOUND() after a failed SEEK should be captured");
    expect(eof2 != state.globals.end(), "EOF() after a failed SEEK should be captured");
    expect(rec2 != state.globals.end(), "RECNO() after a failed SEEK should be captured");

    if (found1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(found1->second) == "true", "SEEK should set FOUND() when it locates a matching record");
    }
    if (rec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec1->second) == "2", "SEEK should move the record pointer to the matched row");
    }
    if (found2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(found2->second) == "false", "failed SEEK should clear FOUND()");
    }
    if (eof2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof2->second) == "true", "failed SEEK should move the cursor to EOF");
    }
    if (rec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec2->second) == "4", "failed SEEK should place RECNO() at record_count + 1");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]: BRAVO -> found") != std::string::npos;
        }),
        "SET ORDER and SEEK should emit runtime.order and runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_uses_grounded_order_normalization_hints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_normalization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_normalization.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "lSeekCmd = SEEK('bravo')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('charlie', 'People', 'NAME')\n"
        "nSeekFnRec = RECNO()\n"
        "SET ORDER TO TAG NAME DESCENDING\n"
        "lSeekDesc = SEEK('alpha')\n"
        "nSeekDescRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "normalization-aware seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");
    const auto seek_desc = state.globals.find("lseekdesc");
    const auto seek_desc_rec = state.globals.find("nseekdescrec");

    expect(seek_cmd != state.globals.end(), "command SEEK result should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() result should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() RECNO() should be captured");
    expect(seek_desc != state.globals.end(), "descending normalized SEEK result should be captured");
    expect(seek_desc_rec != state.globals.end(), "descending normalized SEEK RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should honor grounded upper normalization hints");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "2", "command SEEK should land on the case-folded match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should honor grounded upper normalization hints");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "3", "SEEK() should move to the normalized match");
    }
    if (seek_desc != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_desc->second) == "true", "descending SEEK should also honor grounded normalization hints");
    }
    if (seek_desc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_desc_rec->second) == "1", "descending SEEK should land on the normalized exact match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_near_changes_seek_failure_position() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_near";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_near.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "lNearEof = EOF()\n"
        "nNearRec = RECNO()\n"
        "SET NEAR OFF\n"
        "SEEK 'BRAVO'\n"
        "lFarFound = FOUND()\n"
        "lFarEof = EOF()\n"
        "nFarRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET NEAR seek script should complete");

    const auto near_found = state.globals.find("lnearfound");
    const auto near_eof = state.globals.find("lneareof");
    const auto near_rec = state.globals.find("nnearrec");
    const auto far_found = state.globals.find("lfarfound");
    const auto far_eof = state.globals.find("lfareof");
    const auto far_rec = state.globals.find("nfarrec");

    expect(near_found != state.globals.end(), "FOUND() after SET NEAR ON should be captured");
    expect(near_eof != state.globals.end(), "EOF() after SET NEAR ON should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after SET NEAR ON should be captured");
    expect(far_found != state.globals.end(), "FOUND() after SET NEAR OFF should be captured");
    expect(far_eof != state.globals.end(), "EOF() after SET NEAR OFF should be captured");
    expect(far_rec != state.globals.end(), "RECNO() after SET NEAR OFF should be captured");

    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR ON should keep FOUND() false when SEEK misses");
    }
    if (near_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_eof->second) == "false", "SET NEAR ON should position to the nearest record instead of EOF");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "2", "SET NEAR ON should position to the next ordered record");
    }
    if (far_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_found->second) == "false", "SET NEAR OFF should still report a failed seek");
    }
    if (far_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_eof->second) == "true", "SET NEAR OFF should leave the cursor at EOF after a missed seek");
    }
    if (far_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_rec->second) == "4", "SET NEAR OFF should place RECNO() at record_count + 1 after a missed seek");
    }

    expect(
        has_runtime_event(state.events, "runtime.set", "NEAR ON") &&
        has_runtime_event(state.events, "runtime.set", "NEAR OFF"),
        "SET NEAR changes should emit runtime.set events");

    fs::remove_all(temp_root, ignored);
}

void test_set_order_descending_changes_seek_ordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_order_descending";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_order_descending.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME DESCENDING\n"
        "cOrder = ORDER()\n"
        "SEEK 'CHARLIE'\n"
        "lExactFound = FOUND()\n"
        "nExactRec = RECNO()\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "lNearEof = EOF()\n"
        "nNearRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "descending SET ORDER script should complete");

    const auto order = state.globals.find("corder");
    const auto exact_found = state.globals.find("lexactfound");
    const auto exact_rec = state.globals.find("nexactrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_eof = state.globals.find("lneareof");
    const auto near_rec = state.globals.find("nnearrec");

    expect(order != state.globals.end(), "ORDER() after descending SET ORDER should be captured");
    expect(exact_found != state.globals.end(), "FOUND() after exact descending SEEK should be captured");
    expect(exact_rec != state.globals.end(), "RECNO() after exact descending SEEK should be captured");
    expect(near_found != state.globals.end(), "FOUND() after descending SET NEAR seek should be captured");
    expect(near_eof != state.globals.end(), "EOF() after descending SET NEAR seek should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after descending SET NEAR seek should be captured");

    if (order != state.globals.end()) {
        expect(copperfin::runtime::format_value(order->second) == "NAME", "descending SET ORDER should still expose the controlling tag name");
    }
    if (exact_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_found->second) == "true", "descending exact SEEK should still report FOUND()");
    }
    if (exact_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_rec->second) == "2", "descending exact SEEK should still land on the matching row");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "descending SET NEAR SEEK should still report a miss");
    }
    if (near_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_eof->second) == "false", "descending SET NEAR SEEK should stay off EOF when a nearby key exists");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "1", "descending SET NEAR SEEK should move to the next row in descending order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]: BRAVO -> not found") != std::string::npos;
        }),
        "descending SET ORDER and SEEK should emit order-direction metadata");

    fs::remove_all(temp_root, ignored);
}

void test_seek_command_accepts_tag_override_without_set_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_tag_override";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_tag_override.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SEEK 'BRAVO' TAG NAME\n"
        "lFound = FOUND()\n"
        "nRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK ... TAG script should complete");

    const auto found = state.globals.find("lfound");
    const auto rec = state.globals.find("nrec");
    const auto order_after = state.globals.find("corderafter");

    expect(found != state.globals.end(), "SEEK ... TAG should expose FOUND()");
    expect(rec != state.globals.end(), "SEEK ... TAG should expose RECNO()");
    expect(order_after != state.globals.end(), "SEEK ... TAG should leave ORDER() observable");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true", "SEEK ... TAG should find matches using the named tag");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "2", "SEEK ... TAG should position the cursor on the matching row");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "SEEK ... TAG should not permanently change the controlling order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]: BRAVO -> found") != std::string::npos;
        }),
        "SEEK ... TAG should expose the temporary order metadata in runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_command_accepts_descending_tag_override_without_set_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_descending_tag_override";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_descending_tag_override.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO' TAG NAME DESCENDING\n"
        "lFound = FOUND()\n"
        "lEof = EOF()\n"
        "nRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK ... TAG DESCENDING script should complete");

    const auto found = state.globals.find("lfound");
    const auto eof = state.globals.find("leof");
    const auto rec = state.globals.find("nrec");
    const auto order_after = state.globals.find("corderafter");

    expect(found != state.globals.end(), "SEEK ... TAG DESCENDING should expose FOUND()");
    expect(eof != state.globals.end(), "SEEK ... TAG DESCENDING should expose EOF()");
    expect(rec != state.globals.end(), "SEEK ... TAG DESCENDING should expose RECNO()");
    expect(order_after != state.globals.end(), "SEEK ... TAG DESCENDING should leave ORDER() observable");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "false", "descending tag override should still report a miss for an in-between key");
    }
    if (eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof->second) == "false", "descending tag override should honor SET NEAR and stay off EOF");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "1", "descending tag override should position to the next row in descending order");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "descending tag override should not permanently change the controlling order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]: BRAVO -> not found") != std::string::npos;
        }),
        "SEEK ... TAG DESCENDING should expose the temporary descending metadata in runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_related_index_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_functions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_functions.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "cOrder1 = ORDER()\n"
        "cOrder2 = ORDER('People', 1)\n"
        "cTag1 = TAG(1, 'People')\n"
        "lSeekFn = SEEK('BRAVO', 'People', 'NAME')\n"
        "nSeekRec = RECNO()\n"
        "GO TOP\n"
        "lIndexNoMove = INDEXSEEK('CHARLIE', .F., 'People', 'NAME')\n"
        "nAfterNoMove = RECNO()\n"
        "lIndexMove = INDEXSEEK('CHARLIE', .T., 'People', 'NAME')\n"
        "nAfterMove = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "seek/index helper script should complete");

    const auto order1 = state.globals.find("corder1");
    const auto order2 = state.globals.find("corder2");
    const auto tag1 = state.globals.find("ctag1");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_rec = state.globals.find("nseekrec");
    const auto index_no_move = state.globals.find("lindexnomove");
    const auto after_no_move = state.globals.find("nafternomove");
    const auto index_move = state.globals.find("lindexmove");
    const auto after_move = state.globals.find("naftermove");

    expect(order1 != state.globals.end(), "ORDER() should be captured");
    expect(order2 != state.globals.end(), "ORDER(alias, pathFlag) should be captured");
    expect(tag1 != state.globals.end(), "TAG() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() should be captured");
    expect(seek_rec != state.globals.end(), "RECNO() after SEEK() should be captured");
    expect(index_no_move != state.globals.end(), "INDEXSEEK(.F.) should be captured");
    expect(after_no_move != state.globals.end(), "RECNO() after INDEXSEEK(.F.) should be captured");
    expect(index_move != state.globals.end(), "INDEXSEEK(.T.) should be captured");
    expect(after_move != state.globals.end(), "RECNO() after INDEXSEEK(.T.) should be captured");

    if (order1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(order1->second) == "NAME", "ORDER() should expose the controlling tag");
    }
    if (order2 != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(order2->second).find("PEOPLE.CDX") != std::string::npos,
            "ORDER(alias, pathFlag) should expose the controlling index path");
    }
    if (tag1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(tag1->second) == "NAME", "TAG() should expose the first open tag");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should return true for a match");
    }
    if (seek_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_rec->second) == "2", "SEEK() should move the record pointer to the matching row");
    }
    if (index_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_no_move->second) == "true", "INDEXSEEK(.F.) should report matches");
    }
    if (after_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_no_move->second) == "1", "INDEXSEEK(.F.) should not move the record pointer");
    }
    if (index_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_move->second) == "true", "INDEXSEEK(.T.) should report matches");
    }
    if (after_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_move->second) == "3", "INDEXSEEK(.T.) should move the record pointer to the match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_function_accepts_direction_suffix_in_order_designator() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_direction_suffix";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_direction_suffix.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "lAscFound = SEEK('BRAVO', 'People', 'NAME')\n"
        "nAscRec = RECNO()\n"
        "GO TOP\n"
        "lDescFound = SEEK('BRAVO', 'People', 'NAME DESCENDING')\n"
        "nDescRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK() direction-suffix script should complete");

    const auto asc_found = state.globals.find("lascfound");
    const auto asc_rec = state.globals.find("nascrec");
    const auto desc_found = state.globals.find("ldescfound");
    const auto desc_rec = state.globals.find("ndescrec");
    const auto order_after = state.globals.find("corderafter");

    expect(asc_found != state.globals.end(), "ascending SEEK() result should be captured");
    expect(asc_rec != state.globals.end(), "ascending SEEK() RECNO() should be captured");
    expect(desc_found != state.globals.end(), "descending SEEK() result should be captured");
    expect(desc_rec != state.globals.end(), "descending SEEK() RECNO() should be captured");
    expect(order_after != state.globals.end(), "ORDER() after SEEK() probes should be captured");

    if (asc_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(asc_found->second) == "false", "ascending SEEK() should report a miss for an in-between key");
    }
    if (asc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(asc_rec->second) == "2", "ascending SEEK() should move to the next row in ascending order");
    }
    if (desc_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(desc_found->second) == "false", "descending SEEK() should report a miss for an in-between key");
    }
    if (desc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(desc_rec->second) == "1", "descending SEEK() should move to the next row in descending order");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "SEEK() order-designator override should not permanently change ORDER()");
    }

    fs::remove_all(temp_root, ignored);
}

void test_order_and_tag_preserve_index_file_identity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_idx_identity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path idx_path = temp_root / "people.idx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_idx(idx_path, "UPPER(NAME)");

    const fs::path main_path = temp_root / "idx_identity.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "cOrderPath = ORDER('People', 1)\n"
        "cTagFromIdx = TAG('" + idx_path.string() + "', 1, 'People')\n"
        "lSeek = SEEK('CHARLIE', 'People')\n"
        "nSeekRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "idx identity script should complete");

    const auto order_path = state.globals.find("corderpath");
    const auto tag_from_idx = state.globals.find("ctagfromidx");
    const auto seek_value = state.globals.find("lseek");
    const auto seek_rec = state.globals.find("nseekrec");

    expect(order_path != state.globals.end(), "ORDER(alias, pathFlag) should be captured for IDX-backed orders");
    expect(tag_from_idx != state.globals.end(), "TAG(indexFile, tagNumber, alias) should be captured for IDX-backed orders");
    expect(seek_value != state.globals.end(), "SEEK() should be captured for IDX-backed orders");
    expect(seek_rec != state.globals.end(), "RECNO() after IDX-backed SEEK() should be captured");

    if (order_path != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(order_path->second).find("PEOPLE.IDX") != std::string::npos,
            "ORDER(alias, pathFlag) should preserve the actual IDX file identity");
    }
    if (tag_from_idx != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(tag_from_idx->second) == "PEOPLE",
            "TAG(indexFile, tagNumber, alias) should resolve the order from the actual IDX file");
    }
    if (seek_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_value->second) == "true", "SEEK() should work with the loaded IDX order");
    }
    if (seek_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_rec->second) == "3", "SEEK() should move to the matching record when using the IDX order");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_respects_grounded_order_for_expression_hints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_order_for_expression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path idx_path = temp_root / "people.idx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    mark_simple_dbf_record_deleted(table_path, 2U);
    write_synthetic_idx_with_for(idx_path, "UPPER(NAME)", "DELETED() = .F.");

    const fs::path main_path = temp_root / "order_for_expression.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "SEEK 'BRAVO'\n"
        "lDeletedFound = FOUND()\n"
        "lDeletedEof = EOF()\n"
        "nDeletedRec = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "SEEK 'CHARLIE'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR-filtered order SEEK script should complete");

    const auto deleted_found = state.globals.find("ldeletedfound");
    const auto deleted_eof = state.globals.find("ldeletedeof");
    const auto deleted_rec = state.globals.find("ndeletedrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto visible_found = state.globals.find("lvisiblefound");
    const auto visible_rec = state.globals.find("nvisiblerec");

    expect(deleted_found != state.globals.end(), "SEEK on a filtered-out key should expose FOUND()");
    expect(deleted_eof != state.globals.end(), "SEEK on a filtered-out key should expose EOF()");
    expect(deleted_rec != state.globals.end(), "SEEK on a filtered-out key should expose RECNO()");
    expect(near_found != state.globals.end(), "SET NEAR SEEK on a filtered-out key should expose FOUND()");
    expect(near_rec != state.globals.end(), "SET NEAR SEEK on a filtered-out key should expose RECNO()");
    expect(visible_found != state.globals.end(), "SEEK on a visible key should expose FOUND()");
    expect(visible_rec != state.globals.end(), "SEEK on a visible key should expose RECNO()");

    if (deleted_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_found->second) == "false", "SEEK should ignore keys filtered out by the grounded order FOR expression");
    }
    if (deleted_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_eof->second) == "true", "SEEK without SET NEAR should move to EOF when only a filtered-out key matches");
    }
    if (deleted_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_rec->second) == "4", "SEEK without SET NEAR should position after the visible rows when the filtered-out key is skipped");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR should still report a miss for a filtered-out key");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "3", "SET NEAR should move to the next visible indexed key after a filtered-out match");
    }
    if (visible_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_found->second) == "true", "SEEK should still find keys allowed by the grounded order FOR expression");
    }
    if (visible_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_rec->second) == "3", "SEEK should position on the visible row that survives the order FOR expression");
    }

    fs::remove_all(temp_root, ignored);
}


void test_ndx_numeric_domain_guides_seek_near_ordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ndx_numeric_domain";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path ndx_path = temp_root / "people.ndx";
    write_people_dbf(table_path, {{"ALPHA", 2}, {"BRAVO", 10}, {"CHARLIE", 20}});
    write_synthetic_ndx(ndx_path, "AGE", true);

    const fs::path main_path = temp_root / "ndx_numeric_domain.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "SET NEAR ON\n"
        "SEEK '9'\n"
        "lFound = FOUND()\n"
        "lEof = EOF()\n"
        "nRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "NDX numeric-domain seek script should complete");

    const auto found = state.globals.find("lfound");
    const auto eof = state.globals.find("leof");
    const auto rec = state.globals.find("nrec");

    expect(found != state.globals.end(), "NDX numeric-domain seek should expose FOUND()");
    expect(eof != state.globals.end(), "NDX numeric-domain seek should expose EOF()");
    expect(rec != state.globals.end(), "NDX numeric-domain seek should expose RECNO()");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "false", "NDX numeric-domain seek should still report a miss for a non-existent key");
    }
    if (eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof->second) == "false", "NDX numeric-domain SET NEAR should position to the next numeric key instead of EOF");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "2", "NDX numeric-domain seek should treat AGE keys numerically when choosing the nearest record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_near_is_scoped_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_near_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_near_datasession.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNear1Found = FOUND()\n"
        "lNear1Eof = EOF()\n"
        "nNear1Rec = RECNO()\n"
        "SET DATASESSION TO 2\n"
        "USE '" + table_path.string() + "' ALIAS PeopleTwo IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SEEK 'BRAVO'\n"
        "lNear2Found = FOUND()\n"
        "lNear2Eof = EOF()\n"
        "nNear2Rec = RECNO()\n"
        "SET DATASESSION TO 1\n"
        "GO TOP\n"
        "SEEK 'BRAVO'\n"
        "lNear1BackFound = FOUND()\n"
        "lNear1BackEof = EOF()\n"
        "nNear1BackRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET NEAR data-session script should complete");

    const auto near1_found = state.globals.find("lnear1found");
    const auto near1_eof = state.globals.find("lnear1eof");
    const auto near1_rec = state.globals.find("nnear1rec");
    const auto near2_found = state.globals.find("lnear2found");
    const auto near2_eof = state.globals.find("lnear2eof");
    const auto near2_rec = state.globals.find("nnear2rec");
    const auto near1_back_found = state.globals.find("lnear1backfound");
    const auto near1_back_eof = state.globals.find("lnear1backeof");
    const auto near1_back_rec = state.globals.find("nnear1backrec");

    expect(near1_found != state.globals.end(), "session-1 SET NEAR FOUND() should be captured");
    expect(near1_eof != state.globals.end(), "session-1 SET NEAR EOF() should be captured");
    expect(near1_rec != state.globals.end(), "session-1 SET NEAR RECNO() should be captured");
    expect(near2_found != state.globals.end(), "session-2 SEEK FOUND() should be captured");
    expect(near2_eof != state.globals.end(), "session-2 SEEK EOF() should be captured");
    expect(near2_rec != state.globals.end(), "session-2 SEEK RECNO() should be captured");
    expect(near1_back_found != state.globals.end(), "restored session-1 SEEK FOUND() should be captured");
    expect(near1_back_eof != state.globals.end(), "restored session-1 SEEK EOF() should be captured");
    expect(near1_back_rec != state.globals.end(), "restored session-1 SEEK RECNO() should be captured");

    if (near1_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_found->second) == "false", "SET NEAR ON should still leave FOUND() false on a missed seek");
    }
    if (near1_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_eof->second) == "false", "SET NEAR ON in session 1 should keep the cursor off EOF");
    }
    if (near1_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_rec->second) == "2", "SET NEAR ON in session 1 should move to the nearest ordered row");
    }
    if (near2_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_found->second) == "false", "a fresh second data session should still report a missed seek");
    }
    if (near2_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_eof->second) == "true", "SET NEAR should not bleed into a fresh second data session");
    }
    if (near2_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_rec->second) == "4", "a fresh second data session should keep the default SET NEAR OFF seek position");
    }
    if (near1_back_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_found->second) == "false", "restoring session 1 should preserve missed-seek FOUND() behavior");
    }
    if (near1_back_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_eof->second) == "false", "restoring session 1 should restore its SET NEAR ON behavior");
    }
    if (near1_back_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_rec->second) == "2", "restoring session 1 should restore its nearest-record seek position");
    }

    fs::remove_all(temp_root, ignored);
}

void test_foxtools_registration_and_call_bridge() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_foxtools";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "foxtools.prg";
    write_text(
        main_path,
        "SET LIBRARY TO 'Foxtools'\n"
        "cFoxTools = FoxToolVer()\n"
        "nMain = MainHwnd()\n"
        "hPid = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "nPid = CallFn(hPid)\n"
        "hLen = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen = CallFn(hLen, 'Copperfin')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools bridge script should complete");

    const auto foxtools = state.globals.find("cfoxtools");
    const auto main = state.globals.find("nmain");
    const auto hpid = state.globals.find("hpid");
    const auto pid = state.globals.find("npid");
    const auto hlen = state.globals.find("hlen");
    const auto length = state.globals.find("nlen");

    expect(foxtools != state.globals.end(), "FoxToolVer() should be captured");
    expect(main != state.globals.end(), "MainHwnd() should be captured");
    expect(hpid != state.globals.end(), "RegFn32 handle should be captured");
    expect(pid != state.globals.end(), "CallFn(handle) should be captured");
    expect(hlen != state.globals.end(), "second RegFn32 handle should be captured");
    expect(length != state.globals.end(), "CallFn(string) should be captured");

    if (foxtools != state.globals.end()) {
        expect(!copperfin::runtime::format_value(foxtools->second).empty(), "FoxToolVer() should return a non-empty version string");
    }
    if (main != state.globals.end()) {
        expect(copperfin::runtime::format_value(main->second) == "1001", "MainHwnd() should expose the placeholder host window handle");
    }
    if (hpid != state.globals.end()) {
        expect(copperfin::runtime::format_value(hpid->second) == "1", "first RegFn32 call should allocate handle 1");
    }
    if (pid != state.globals.end()) {
        expect(copperfin::runtime::format_value(pid->second) != "0", "CallFn(GetCurrentProcessId) should return a non-zero process id");
    }
    if (hlen != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen->second) == "2", "second RegFn32 call should allocate handle 2");
    }
    if (length != state.globals.end()) {
        expect(copperfin::runtime::format_value(length->second) == "9", "CallFn(lstrlenA, 'Copperfin') should return the string length");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.library"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "interop.regfn"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "interop.callfn"; }),
        "Foxtools bridge should emit library, registration, and call events");

    fs::remove_all(temp_root, ignored);
}

void test_foxtools_registration_is_scoped_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_foxtools_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "foxtools_datasession.prg";
    write_text(
        main_path,
        "SET LIBRARY TO 'Foxtools'\n"
        "hPid1 = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "SET DATASESSION TO 2\n"
        "SET LIBRARY TO 'Foxtools'\n"
        "nCrossCall = CallFn(hPid1)\n"
        "hLen2 = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen2 = CallFn(hLen2, 'AB')\n"
        "SET DATASESSION TO 1\n"
        "nPid1Back = CallFn(hPid1)\n"
        "hLen1Back = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools data-session script should complete");

    const auto hpid1 = state.globals.find("hpid1");
    const auto cross_call = state.globals.find("ncrosscall");
    const auto hlen2 = state.globals.find("hlen2");
    const auto len2 = state.globals.find("nlen2");
    const auto pid1_back = state.globals.find("npid1back");
    const auto hlen1_back = state.globals.find("hlen1back");

    expect(hpid1 != state.globals.end(), "session-1 RegFn32 handle should be captured");
    expect(cross_call != state.globals.end(), "cross-session CallFn result should be captured");
    expect(hlen2 != state.globals.end(), "session-2 RegFn32 handle should be captured");
    expect(len2 != state.globals.end(), "session-2 CallFn result should be captured");
    expect(pid1_back != state.globals.end(), "restored session-1 CallFn result should be captured");
    expect(hlen1_back != state.globals.end(), "restored session-1 RegFn32 handle should be captured");

    if (hpid1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(hpid1->second) == "1", "the first RegFn32 handle in session 1 should be 1");
    }
    if (cross_call != state.globals.end()) {
        expect(copperfin::runtime::format_value(cross_call->second) == "-1", "CallFn should reject a RegFn32 handle from another data session");
    }
    if (hlen2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen2->second) == "1", "the first RegFn32 handle in a fresh second data session should restart at 1");
    }
    if (len2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(len2->second) == "2", "session-2 CallFn should use its own registered handle");
    }
    if (pid1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(pid1_back->second) != "0", "restoring session 1 should restore its RegFn32 handle lookup");
    }
    if (hlen1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen1_back->second) == "2", "restoring session 1 should restore its next RegFn32 handle allocation");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_exact_affects_comparisons_and_seek() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_exact";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_exact.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "lEqOff = 'CHARLIE' = 'CHAR'\n"
        "lSeekOff = SEEK('BR')\n"
        "nRecOff = RECNO()\n"
        "SET EXACT ON\n"
        "lEqOn = 'CHARLIE' = 'CHAR'\n"
        "lSeekOn = SEEK('BR')\n"
        "lEofOn = EOF()\n"
        "SET DATASESSION TO 2\n"
        "lEqSession2 = 'CHARLIE' = 'CHAR'\n"
        "SET DATASESSION TO 1\n"
        "lEqBack = 'CHARLIE' = 'CHAR'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET EXACT script should complete");

    const auto eq_off = state.globals.find("leqoff");
    const auto seek_off = state.globals.find("lseekoff");
    const auto rec_off = state.globals.find("nrecoff");
    const auto eq_on = state.globals.find("leqon");
    const auto seek_on = state.globals.find("lseekon");
    const auto eof_on = state.globals.find("leofon");
    const auto eq_session2 = state.globals.find("leqsession2");
    const auto eq_back = state.globals.find("leqback");

    expect(eq_off != state.globals.end(), "SET EXACT OFF comparison result should be captured");
    expect(seek_off != state.globals.end(), "SET EXACT OFF seek result should be captured");
    expect(rec_off != state.globals.end(), "SET EXACT OFF RECNO() should be captured");
    expect(eq_on != state.globals.end(), "SET EXACT ON comparison result should be captured");
    expect(seek_on != state.globals.end(), "SET EXACT ON seek result should be captured");
    expect(eof_on != state.globals.end(), "SET EXACT ON EOF() should be captured");
    expect(eq_session2 != state.globals.end(), "session 2 comparison result should be captured");
    expect(eq_back != state.globals.end(), "session 1 restored comparison result should be captured");

    if (eq_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_off->second) == "true", "SET EXACT OFF should allow right-side prefix string comparison");
    }
    if (seek_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_off->second) == "true", "SET EXACT OFF should allow prefix seeks");
    }
    if (rec_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_off->second) == "2", "SET EXACT OFF seek should land on the matching prefix row");
    }
    if (eq_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_on->second) == "false", "SET EXACT ON should require full string equality");
    }
    if (seek_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_on->second) == "false", "SET EXACT ON should reject prefix seeks");
    }
    if (eof_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_on->second) == "true", "SET EXACT ON failed seek should leave the cursor at EOF");
    }
    if (eq_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_session2->second) == "true", "SET EXACT should be scoped to the current data session");
    }
    if (eq_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_back->second) == "false", "restoring the original data session should restore its SET EXACT state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_again_and_alias_collision_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_again";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_again.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS PeopleAgain AGAIN IN 0\n"
        "nAreaAgain = SELECT()\n"
        "cAliasAgain = ALIAS()\n"
        "USE '" + table_path.string() + "' ALIAS PeopleThird IN 0\n"
        "xAfterError = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "duplicate table opens without AGAIN should pause with an error");
    expect(state.location.line == 5U, "duplicate table opens without AGAIN should highlight the failing USE statement");
    expect(
        state.message.find("USE AGAIN is required") != std::string::npos,
        "duplicate table opens without AGAIN should report a USE AGAIN message");

    const auto area_again = state.globals.find("nareaagain");
    const auto alias_again = state.globals.find("caliasagain");
    expect(area_again != state.globals.end(), "USE AGAIN should let execution reach the second-area checks");
    expect(alias_again != state.globals.end(), "USE AGAIN should let execution expose the second alias");
    if (area_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_again->second) == "2", "USE AGAIN IN 0 should allocate a second work area");
    }
    if (alias_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_again->second) == "PeopleAgain", "USE AGAIN should keep the requested second alias");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a USE AGAIN error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after continuing");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "7", "post-error statements should still update globals");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_selected_alias_replacement_clears_old_alias_and_order_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_selected_alias_replacement";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path people_idx_path = temp_root / "people.idx";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "ROME"});
    write_synthetic_idx(people_idx_path, "UPPER(NAME)");

    const fs::path main_path = temp_root / "selected_alias_replacement.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "GO BOTTOM\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN People\n"
        "nOldAliasArea = SELECT('People')\n"
        "nNewAliasArea = SELECT('Cities')\n"
        "cAliasAfter = ALIAS()\n"
        "cOrderAfter = ORDER()\n"
        "nRecAfter = RECNO()\n"
        "cTopAfter = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "selected alias replacement script should complete");

    const auto old_alias_area = state.globals.find("noldaliasarea");
    const auto new_alias_area = state.globals.find("nnewaliasarea");
    const auto alias_after = state.globals.find("caliasafter");
    const auto order_after = state.globals.find("corderafter");
    const auto rec_after = state.globals.find("nrecafter");
    const auto top_after = state.globals.find("ctopafter");

    expect(old_alias_area != state.globals.end(), "SELECT('People') after selected alias replacement should be captured");
    expect(new_alias_area != state.globals.end(), "SELECT('Cities') after selected alias replacement should be captured");
    expect(alias_after != state.globals.end(), "ALIAS() after selected alias replacement should be captured");
    expect(order_after != state.globals.end(), "ORDER() after selected alias replacement should be captured");
    expect(rec_after != state.globals.end(), "RECNO() after selected alias replacement should be captured");
    expect(top_after != state.globals.end(), "field access after selected alias replacement should be captured");

    if (old_alias_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(old_alias_area->second) == "0", "selected alias replacement should clear the old alias lookup");
    }
    if (new_alias_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(new_alias_area->second) == "1", "selected alias replacement should reuse the selected work area in place");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Cities", "selected alias replacement should expose the new alias immediately");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "selected alias replacement should clear the old active order state when the replacement has no orders");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "1", "selected alias replacement should reset the cursor position for the new table");
    }
    if (top_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_after->second) == "OSLO", "selected alias replacement should expose the new table's first record");
    }

    fs::remove_all(temp_root, ignored);
}


void test_select_missing_alias_is_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_select_missing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "select_missing.prg";
    write_text(
        main_path,
        "SELECT MissingAlias\n"
        "xAfterError = 9\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "SELECT MissingAlias should pause with an error");
    expect(state.location.line == 1U, "SELECT MissingAlias should highlight the failing line");
    expect(
        state.message.find("SELECT target work area not found") != std::string::npos,
        "SELECT MissingAlias should report a missing-target message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a SELECT error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after SELECT errors");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "9", "post-error statements should still update globals after SELECT errors");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_missing_alias_is_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_in_missing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_in_missing_alias.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN MissingAlias\n"
        "xAfterError = 11\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "USE ... IN MissingAlias should pause with an error");
    expect(state.location.line == 1U, "USE ... IN MissingAlias should highlight the failing line");
    expect(
        state.message.find("USE target work area not found") != std::string::npos,
        "USE ... IN MissingAlias should report a missing-target message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a USE ... IN alias error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after USE ... IN alias errors");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "11", "post-error statements should still update globals after USE ... IN alias errors");
    }
    expect(state.work_area.aliases.empty(), "USE ... IN MissingAlias should not open a fallback work area");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursors_and_ole_actions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sqlcursor";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sqlcursor.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "cAlias = ALIAS()\n"
        "nCount = RECCOUNT()\n"
        "nRec = RECNO()\n"
        "oExcel = CREATEOBJECT('Excel.Application')\n"
        "oExcel.Visible = .T.\n"
        "cVisible = oExcel.Visible\n"
        "oBook = oExcel.Workbooks.Add()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL cursor/OLE script should complete");
    expect(!state.cursors.empty(), "SQLEXEC should materialize a result cursor");
    expect(state.ole_objects.size() == 1U, "CREATEOBJECT and follow-on automation should track one OLE object");

    const auto alias = state.globals.find("calias");
    const auto count = state.globals.find("ncount");
    const auto rec = state.globals.find("nrec");
    const auto visible = state.globals.find("cvisible");
    const auto book = state.globals.find("obook");

    expect(alias != state.globals.end(), "ALIAS() for SQL cursor should be captured");
    expect(count != state.globals.end(), "RECCOUNT() for SQL cursor should be captured");
    expect(rec != state.globals.end(), "RECNO() for SQL cursor should be captured");
    expect(visible != state.globals.end(), "OLE property reads should flow back into VFP code");
    expect(book != state.globals.end(), "OLE method calls should return a placeholder value");

    if (alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias->second) == "sqlcust", "SQLEXEC cursor alias should be selectable like a normal work area");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "3", "synthetic SQL result cursors should expose row counts");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "1", "synthetic SQL result cursors should begin on record 1");
    }
    if (visible != state.globals.end()) {
        expect(!copperfin::runtime::format_value(visible->second).empty(), "OLE property access should produce a debuggable value");
    }
    if (book != state.globals.end()) {
        expect(!copperfin::runtime::format_value(book->second).empty(), "OLE method invocation should return a placeholder object/value");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "sql.cursor"; }),
        "SQLEXEC with a cursor alias should emit a sql.cursor event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.set"; }),
        "OLE property assignments should emit ole.set events");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.invoke"; }),
        "OLE method calls should emit ole.invoke events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_read_only_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_read_only_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_read_only_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET FILTER TO ID >= 2\n"
        "GO TOP\n"
        "cTopName = NAME\n"
        "LOCATE FOR AMOUNT = 20\n"
        "lFound = FOUND()\n"
        "nLocateRec = RECNO()\n"
        "cLocateName = NAME\n"
        "nCountVisible = COUNT()\n"
        "nSumVisible = SUM(AMOUNT)\n"
        "CALCULATE COUNT() TO nCalcCount, SUM(AMOUNT) TO nCalcSum\n"
        "nCountAlias = COUNT(ID >= 2, 'sqlcust')\n"
        "nSumAlias = SUM(AMOUNT, ID >= 2, 'sqlcust')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL read-only parity script should complete");
    expect(state.sql_connections.empty(), "SQL read-only parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto top_name = state.globals.find("ctopname");
    const auto found = state.globals.find("lfound");
    const auto locate_rec = state.globals.find("nlocaterec");
    const auto locate_name = state.globals.find("clocatename");
    const auto count_visible = state.globals.find("ncountvisible");
    const auto sum_visible = state.globals.find("nsumvisible");
    const auto calc_count = state.globals.find("ncalccount");
    const auto calc_sum = state.globals.find("ncalcsum");
    const auto count_alias = state.globals.find("ncountalias");
    const auto sum_alias = state.globals.find("nsumalias");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for the SQL read-only parity script");
    expect(top_name != state.globals.end(), "filtered GO TOP should expose the current SQL cursor row");
    expect(found != state.globals.end(), "LOCATE on a SQL cursor should expose FOUND()");
    expect(locate_rec != state.globals.end(), "LOCATE on a SQL cursor should expose RECNO()");
    expect(locate_name != state.globals.end(), "LOCATE on a SQL cursor should expose field values");
    expect(count_visible != state.globals.end(), "COUNT() should work against a filtered SQL cursor");
    expect(sum_visible != state.globals.end(), "SUM() should work against a filtered SQL cursor");
    expect(calc_count != state.globals.end(), "CALCULATE COUNT() should work against a SQL cursor");
    expect(calc_sum != state.globals.end(), "CALCULATE SUM() should work against a SQL cursor");
    expect(count_alias != state.globals.end(), "COUNT(..., alias) should target a SQL cursor by alias");
    expect(sum_alias != state.globals.end(), "SUM(..., alias) should target a SQL cursor by alias");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for the SQL read-only parity script");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before read-only SQL cursor checks");
    }
    if (top_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_name->second) == "BRAVO", "SET FILTER plus GO TOP should position a SQL cursor on the first visible synthetic row");
    }
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true", "LOCATE should succeed on a SQL cursor when the synthetic row matches");
    }
    if (locate_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_rec->second) == "2", "LOCATE should leave the SQL cursor on the matching synthetic row");
    }
    if (locate_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_name->second) == "BRAVO", "SQL cursor field lookup should flow through the located synthetic row");
    }
    if (count_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_visible->second) == "2", "COUNT() should respect active SQL cursor filters");
    }
    if (sum_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_visible->second) == "50", "SUM() should aggregate visible synthetic SQL rows");
    }
    if (calc_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(calc_count->second) == "2", "CALCULATE COUNT() should respect active SQL cursor filters");
    }
    if (calc_sum != state.globals.end()) {
        expect(copperfin::runtime::format_value(calc_sum->second) == "50", "CALCULATE SUM() should aggregate visible synthetic SQL rows");
    }
    if (count_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_alias->second) == "2", "COUNT(condition, alias) should resolve the SQL cursor by alias");
    }
    if (sum_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_alias->second) == "50", "SUM(value, condition, alias) should resolve the SQL cursor by alias");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after read-only SQL cursor operations");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter"; }),
        "SQL cursor filter changes should emit runtime.filter events");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.locate"; }),
        "SQL cursor LOCATE should emit runtime.locate events");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.calculate"; }),
        "SQL cursor CALCULATE should emit runtime.calculate events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_seek_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "lSeekName = SEEK('BRAVO', 'sqlcust', 'NAME')\n"
        "nSeekRec = RECNO()\n"
        "GO TOP\n"
        "lIndexNoMove = INDEXSEEK('CHARLIE', .F., 'sqlcust', 'NAME')\n"
        "nAfterNoMove = RECNO()\n"
        "lIndexMove = INDEXSEEK('CHARLIE', .T., 'sqlcust', 'NAME')\n"
        "nAfterMove = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "lSeekNear = SEEK('BETA', 'sqlcust', 'NAME')\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL seek parity script should complete");
    expect(state.sql_connections.empty(), "SQL seek parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_name = state.globals.find("lseekname");
    const auto seek_rec = state.globals.find("nseekrec");
    const auto index_no_move = state.globals.find("lindexnomove");
    const auto after_no_move = state.globals.find("nafternomove");
    const auto index_move = state.globals.find("lindexmove");
    const auto after_move = state.globals.find("naftermove");
    const auto seek_near = state.globals.find("lseeknear");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto order_after = state.globals.find("corderafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL seek parity");
    expect(seek_name != state.globals.end(), "SEEK() on a SQL cursor should be captured");
    expect(seek_rec != state.globals.end(), "RECNO() after SQL SEEK() should be captured");
    expect(index_no_move != state.globals.end(), "INDEXSEEK(.F.) on a SQL cursor should be captured");
    expect(after_no_move != state.globals.end(), "RECNO() after SQL INDEXSEEK(.F.) should be captured");
    expect(index_move != state.globals.end(), "INDEXSEEK(.T.) on a SQL cursor should be captured");
    expect(after_move != state.globals.end(), "RECNO() after SQL INDEXSEEK(.T.) should be captured");
    expect(seek_near != state.globals.end(), "SEEK() miss with SET NEAR on a SQL cursor should be captured");
    expect(near_found != state.globals.end(), "FOUND() after SQL SEEK() miss should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after SQL SEEK() miss should be captured");
    expect(order_after != state.globals.end(), "ORDER() after SQL SEEK()/INDEXSEEK() probes should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL seek parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL SEEK checks");
    }
    if (seek_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_name->second) == "true", "SEEK() should find a matching synthetic SQL row by one-off order expression");
    }
    if (seek_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_rec->second) == "2", "SEEK() should position the SQL cursor on the matching synthetic row");
    }
    if (index_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_no_move->second) == "true", "INDEXSEEK(.F.) should report SQL cursor matches");
    }
    if (after_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_no_move->second) == "1", "INDEXSEEK(.F.) should not move the SQL cursor pointer");
    }
    if (index_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_move->second) == "true", "INDEXSEEK(.T.) should report SQL cursor matches");
    }
    if (after_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_move->second) == "3", "INDEXSEEK(.T.) should move the SQL cursor pointer to the matching row");
    }
    if (seek_near != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_near->second) == "false", "SEEK() should report a miss for an in-between SQL key");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "FOUND() should stay false after a SQL SEEK() miss");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "2", "SET NEAR should position a SQL cursor to the next matching synthetic row");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "one-off SQL SEEK()/INDEXSEEK() order expressions should not permanently change ORDER()");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after SQL SEEK() checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_command_seek_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_command_seek_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_command_seek_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET ORDER TO NAME\n"
        "cOrder = ORDER()\n"
        "SEEK 'CHARLIE'\n"
        "lFoundExact = FOUND()\n"
        "nRecExact = RECNO()\n"
        "SET NEAR ON\n"
        "SEEK 'BETA'\n"
        "lFoundNear = FOUND()\n"
        "nRecNear = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL command-path seek parity script should complete");
    expect(state.sql_connections.empty(), "SQL command-path seek parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto order = state.globals.find("corder");
    const auto found_exact = state.globals.find("lfoundexact");
    const auto rec_exact = state.globals.find("nrecexact");
    const auto found_near = state.globals.find("lfoundnear");
    const auto rec_near = state.globals.find("nrecnear");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL command-path seek parity");
    expect(order != state.globals.end(), "ORDER() after SQL SET ORDER should be captured");
    expect(found_exact != state.globals.end(), "FOUND() after SQL command SEEK exact match should be captured");
    expect(rec_exact != state.globals.end(), "RECNO() after SQL command SEEK exact match should be captured");
    expect(found_near != state.globals.end(), "FOUND() after SQL command SEEK miss should be captured");
    expect(rec_near != state.globals.end(), "RECNO() after SQL command SEEK miss should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL command-path seek parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL command-path seek checks");
    }
    if (order != state.globals.end()) {
        expect(copperfin::runtime::format_value(order->second) == "NAME", "SET ORDER TO NAME should establish a synthetic SQL order expression");
    }
    if (found_exact != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_exact->second) == "true", "command SEEK should find the synthetic SQL row");
    }
    if (rec_exact != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_exact->second) == "3", "command SEEK should position the SQL cursor on the matching row");
    }
    if (found_near != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_near->second) == "false", "command SEEK should report a miss for an in-between SQL key");
    }
    if (rec_near != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_near->second) == "2", "SET NEAR plus command SEEK should position the SQL cursor to the next synthetic row");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL command-path SEEK checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" && event.detail == "NAME";
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" && event.detail.find("NAME: BETA -> not found") != std::string::npos;
        }),
        "SQL command-path SET ORDER and SEEK should emit runtime.order and runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_command_seek_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_command_seek_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_command_seek_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "nOtherRecBefore = RECNO()\n"
        "SET ORDER TO NAME IN sqlcust\n"
        "cCustOrder = ORDER('sqlcust')\n"
        "cOtherOrder = ORDER('sqlother')\n"
        "SEEK 'CHARLIE' IN sqlcust\n"
        "cAliasAfterSeek = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "SELECT sqlcust\n"
        "nCustRecAfterSeek = RECNO()\n"
        "cCustNameAfterSeek = NAME\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL command-path SEEK IN target parity script should complete");
    expect(state.sql_connections.empty(), "SQL command-path SEEK IN target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_order = state.globals.find("ccustorder");
    const auto other_order = state.globals.find("cotherorder");
    const auto alias_after_seek = state.globals.find("caliasafterseek");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto cust_rec_after_seek = state.globals.find("ncustrecafterseek");
    const auto cust_name_after_seek = state.globals.find("ccustnameafterseek");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL command-path SEEK IN target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL command-path SEEK IN target parity");
    expect(other_rec_before != state.globals.end(), "RECNO() before targeted SQL SEEK should be captured");
    expect(cust_order != state.globals.end(), "ORDER('sqlcust') after targeted SQL SET ORDER should be captured");
    expect(other_order != state.globals.end(), "ORDER('sqlother') after targeted SQL SET ORDER should be captured");
    expect(alias_after_seek != state.globals.end(), "ALIAS() after targeted SQL SEEK should be captured");
    expect(other_rec_after != state.globals.end(), "RECNO() on selected SQL cursor after targeted SEEK should be captured");
    expect(cust_rec_after_seek != state.globals.end(), "RECNO() on targeted SQL cursor after targeted SEEK should be captured");
    expect(cust_name_after_seek != state.globals.end(), "NAME on targeted SQL cursor after targeted SEEK should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL command-path SEEK IN target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL SEEK checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL SEEK checks");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "1", "selected SQL cursor should begin at first record");
    }
    if (cust_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_order->second) == "NAME", "SET ORDER TO ... IN sqlcust should affect the targeted SQL cursor");
    }
    if (other_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_order->second).empty(), "SET ORDER TO ... IN sqlcust should not alter the selected non-target SQL cursor");
    }
    if (alias_after_seek != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_seek->second)) == "SQLOTHER", "SEEK ... IN sqlcust should preserve the current selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "1", "SEEK ... IN sqlcust should not move the selected non-target SQL cursor pointer");
    }
    if (cust_rec_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_seek->second) == "3", "SEEK ... IN sqlcust should move the targeted SQL cursor pointer to the match");
    }
    if (cust_name_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_name_after_seek->second) == "CHARLIE", "SEEK ... IN sqlcust should expose the targeted SQL row values");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL SEEK checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" && event.detail == "NAME";
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" && event.detail.find("NAME: CHARLIE -> found") != std::string::npos;
        }),
        "SQL command-path SET ORDER ... IN and SEEK ... IN should emit runtime.order and runtime.seek events for targeted SQL cursors");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_scan_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_scan_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_scan_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "nOtherRecBefore = RECNO()\n"
        "nCustRecBefore = RECNO('sqlcust')\n"
        "nScanHits = 0\n"
        "SCAN FOR AMOUNT >= 20 IN sqlcust\n"
        "    nScanHits = nScanHits + 1\n"
        "ENDSCAN\n"
        "cAliasAfterScan = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "nCustRecAfter = RECNO('sqlcust')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL SCAN IN target parity script should complete");
    expect(state.sql_connections.empty(), "SQL SCAN IN target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_before = state.globals.find("ncustrecbefore");
    const auto scan_hits = state.globals.find("nscanhits");
    const auto alias_after_scan = state.globals.find("caliasafterscan");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto cust_rec_after = state.globals.find("ncustrecafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL SCAN IN target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL SCAN IN target parity");
    expect(other_rec_before != state.globals.end(), "selected SQL cursor RECNO() before targeted SCAN should be captured");
    expect(cust_rec_before != state.globals.end(), "target SQL cursor RECNO() before targeted SCAN should be captured");
    expect(scan_hits != state.globals.end(), "SCAN FOR ... IN sqlcust hit count should be captured");
    expect(alias_after_scan != state.globals.end(), "ALIAS() after targeted SQL SCAN should be captured");
    expect(other_rec_after != state.globals.end(), "selected SQL cursor RECNO() after targeted SCAN should be captured");
    expect(cust_rec_after != state.globals.end(), "target SQL cursor RECNO() after targeted SCAN should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL SCAN IN target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL SCAN checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL SCAN checks");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should be on the bottom record before targeted SCAN");
    }
    if (cust_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_before->second) == "1", "target SQL cursor should start on its first record before targeted SCAN");
    }
    if (scan_hits != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_hits->second) == "2", "SCAN FOR ... IN sqlcust should iterate matching rows on the targeted SQL cursor");
    }
    if (alias_after_scan != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_scan->second)) == "SQLOTHER", "SCAN ... IN sqlcust should preserve the currently selected SQL alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "SCAN ... IN sqlcust should preserve the selected SQL cursor pointer");
    }
    if (cust_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after->second) == "4", "targeted SQL SCAN should leave the targeted cursor just past the last record");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL SCAN checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.scan" && event.detail == "AMOUNT >= 20";
        }),
        "SCAN ... IN sqlcust should emit a runtime.scan event with the targeted filter expression");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_mutation_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_mutation_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_mutation_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cFound = NAME\n"
        "nFoundAmount = AMOUNT\n"
        "REPLACE AMOUNT WITH 21, NAME WITH 'BRAVOX'\n"
        "cAfterReplace = NAME\n"
        "nAfterReplace = AMOUNT\n"
        "nBeforeAppend = RECCOUNT('sqlcust')\n"
        "APPEND BLANK\n"
        "nAfterAppend = RECCOUNT('sqlcust')\n"
        "nRecAfterAppend = RECNO()\n"
        "lAppendDeleted = DELETED()\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "nBeforeTargetedAppend = RECCOUNT('sqlcust')\n"
        "APPEND BLANK IN sqlcust\n"
        "nAfterTargetedAppend = RECCOUNT('sqlcust')\n"
        "cAliasAfterTargetedAppend = ALIAS()\n"
        "SELECT sqlcust\n"
        "nTargetedRecAfterAppend = RECNO()\n"
        "lTargetedAppendDeleted = DELETED()\n"
        "REPLACE NAME WITH 'DELTA', AMOUNT WITH 40\n"
        "cAppendedName = NAME\n"
        "nAppendedAmount = AMOUNT\n"
        "SET ORDER TO NAME\n"
        "lSeekDelta = SEEK('DELTA')\n"
        "nSeekRec = RECNO()\n"
        "cSeekName = NAME\n"
        "DELETE\n"
        "lDeleted = DELETED()\n"
        "RECALL\n"
        "lRecalled = DELETED()\n"
        "DELETE FOR AMOUNT = 30\n"
        "LOCATE FOR DELETED()\n"
        "cDeletedName = NAME\n"
        "nDeletedAmount = AMOUNT\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL mutation parity script should complete");
    expect(state.sql_connections.empty(), "SQL mutation parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto found = state.globals.find("cfound");
    const auto found_amount = state.globals.find("nfoundamount");
    const auto after_replace = state.globals.find("cafterreplace");
    const auto after_replace_amount = state.globals.find("nafterreplace");
    const auto before_append = state.globals.find("nbeforeappend");
    const auto after_append = state.globals.find("nafterappend");
    const auto rec_after_append = state.globals.find("nrecafterappend");
    const auto append_deleted = state.globals.find("lappenddeleted");
    const auto exec_other = state.globals.find("nexecother");
    const auto before_targeted_append = state.globals.find("nbeforetargetedappend");
    const auto after_targeted_append = state.globals.find("naftertargetedappend");
    const auto alias_after_targeted_append = state.globals.find("caliasaftertargetedappend");
    const auto targeted_rec_after_append = state.globals.find("ntargetedrecafterappend");
    const auto targeted_append_deleted = state.globals.find("ltargetedappenddeleted");
    const auto appended_name = state.globals.find("cappendedname");
    const auto appended_amount = state.globals.find("nappendedamount");
    const auto seek_delta = state.globals.find("lseekdelta");
    const auto seek_rec = state.globals.find("nseekrec");
    const auto seek_name = state.globals.find("cseekname");
    const auto deleted = state.globals.find("ldeleted");
    const auto recalled = state.globals.find("lrecalled");
    const auto deleted_name = state.globals.find("cdeletedname");
    const auto deleted_amount = state.globals.find("ndeletedamount");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL mutation parity");
    expect(found != state.globals.end(), "LOCATE on a SQL cursor should expose the matching NAME");
    expect(found_amount != state.globals.end(), "LOCATE on a SQL cursor should expose the matching AMOUNT");
    expect(after_replace != state.globals.end(), "REPLACE on a SQL cursor should expose the updated NAME");
    expect(after_replace_amount != state.globals.end(), "REPLACE on a SQL cursor should expose the updated AMOUNT");
    expect(before_append != state.globals.end(), "RECCOUNT() before SQL APPEND BLANK should be captured");
    expect(after_append != state.globals.end(), "RECCOUNT() after SQL APPEND BLANK should be captured");
    expect(rec_after_append != state.globals.end(), "RECNO() after SQL APPEND BLANK should be captured");
    expect(append_deleted != state.globals.end(), "DELETED() after SQL APPEND BLANK should be captured");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for targeted SQL APPEND BLANK");
    expect(before_targeted_append != state.globals.end(), "RECCOUNT() before targeted SQL APPEND BLANK should be captured");
    expect(after_targeted_append != state.globals.end(), "RECCOUNT() after targeted SQL APPEND BLANK should be captured");
    expect(alias_after_targeted_append != state.globals.end(), "ALIAS() after targeted SQL APPEND BLANK should be captured");
    expect(targeted_rec_after_append != state.globals.end(), "RECNO() after targeted SQL APPEND BLANK should be captured");
    expect(targeted_append_deleted != state.globals.end(), "DELETED() after targeted SQL APPEND BLANK should be captured");
    expect(appended_name != state.globals.end(), "REPLACE after SQL APPEND BLANK should expose the appended NAME");
    expect(appended_amount != state.globals.end(), "REPLACE after SQL APPEND BLANK should expose the appended AMOUNT");
    expect(seek_delta != state.globals.end(), "SEEK after SQL APPEND BLANK should expose whether the appended row is indexed");
    expect(seek_rec != state.globals.end(), "RECNO() after SQL SEEK should be captured");
    expect(seek_name != state.globals.end(), "SEEK after SQL APPEND BLANK should expose the matching NAME");
    expect(deleted != state.globals.end(), "DELETE on a SQL cursor should expose DELETED()");
    expect(recalled != state.globals.end(), "RECALL on a SQL cursor should expose DELETED()");
    expect(deleted_name != state.globals.end(), "DELETE FOR on a SQL cursor should expose the tombstoned NAME");
    expect(deleted_amount != state.globals.end(), "DELETE FOR on a SQL cursor should expose the tombstoned AMOUNT");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL mutation parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL mutation checks");
    }
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "BRAVO", "LOCATE should position the matching synthetic SQL row before mutation");
    }
    if (found_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_amount->second) == "20", "field resolution should expose the original SQL row values before mutation");
    }
    if (after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_replace->second) == "BRAVOX", "REPLACE should update synthetic SQL character fields in place");
    }
    if (after_replace_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_replace_amount->second) == "21", "REPLACE should update synthetic SQL numeric fields in place");
    }
    if (before_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(before_append->second) == "3", "synthetic SQL result cursors should start with three rows in this fixture");
    }
    if (after_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_append->second) == "4", "APPEND BLANK should add a new synthetic SQL row");
    }
    if (rec_after_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after_append->second) == "4", "APPEND BLANK should move the SQL cursor pointer to the appended row");
    }
    if (append_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(append_deleted->second) == "false", "APPEND BLANK should create a non-deleted synthetic SQL row");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL APPEND BLANK checks");
    }
    if (before_targeted_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(before_targeted_append->second) == "4", "targeted SQL APPEND BLANK should start from the prior appended row count");
    }
    if (after_targeted_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_targeted_append->second) == "5", "targeted SQL APPEND BLANK should append to the requested non-selected SQL cursor");
    }
    if (alias_after_targeted_append != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_targeted_append->second)) == "SQLOTHER", "targeted SQL APPEND BLANK should preserve the current selected alias");
    }
    if (targeted_rec_after_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(targeted_rec_after_append->second) == "5", "targeted SQL APPEND BLANK should move the targeted SQL cursor pointer to the appended row");
    }
    if (targeted_append_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(targeted_append_deleted->second) == "false", "targeted SQL APPEND BLANK should create a non-deleted row on the targeted cursor");
    }
    if (appended_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(appended_name->second) == "DELTA", "REPLACE after APPEND BLANK should update the appended SQL row");
    }
    if (appended_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(appended_amount->second) == "40", "REPLACE after APPEND BLANK should update numeric fields on the appended SQL row");
    }
    if (seek_delta != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_delta->second) == "true", "SEEK should find SQL rows appended and mutated in memory");
    }
    if (seek_rec != state.globals.end()) {
        expect(
            targeted_rec_after_append != state.globals.end() &&
                copperfin::runtime::format_value(seek_rec->second) == copperfin::runtime::format_value(targeted_rec_after_append->second),
            "SEEK should position to the SQL row appended by the targeted APPEND BLANK");
    }
    if (seek_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_name->second) == "DELTA", "SEEK should expose the appended SQL row values after in-memory mutation");
    }
    if (deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted->second) == "true", "DELETE should tombstone the current synthetic SQL row");
    }
    if (recalled != state.globals.end()) {
        expect(copperfin::runtime::format_value(recalled->second) == "false", "RECALL should clear the synthetic SQL tombstone flag");
    }
    if (deleted_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_name->second) == "CHARLIE", "DELETE FOR should tombstone the matching synthetic SQL row");
    }
    if (deleted_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_amount->second) == "30", "DELETE FOR should preserve field lookup on the tombstoned synthetic SQL row");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL mutation checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.locate"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.replace"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.append_blank"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.delete"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.recall"; }),
        "SQL mutation commands should emit the same runtime events as local mutation commands");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_mutation_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_mutation_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_mutation_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "cAliasBefore = ALIAS()\n"
        "nOtherRecBefore = RECNO()\n"
        "GO BOTTOM IN sqlcust\n"
        "nCustRecBefore = RECNO('sqlcust')\n"
        "REPLACE NAME WITH 'CHARLIEX' IN sqlcust\n"
        "cAliasAfterReplace = ALIAS()\n"
        "nOtherRecAfterReplace = RECNO()\n"
        "nCustRecAfterReplace = RECNO('sqlcust')\n"
        "DELETE FOR NAME = 'BRAVO' IN sqlcust\n"
        "cAliasAfterDelete = ALIAS()\n"
        "nOtherRecAfterDelete = RECNO()\n"
        "nCustRecAfterDelete = RECNO('sqlcust')\n"
        "RECALL FOR NAME = 'BRAVO' IN sqlcust\n"
        "cAliasAfterRecall = ALIAS()\n"
        "nOtherRecAfterRecall = RECNO()\n"
        "nCustRecAfterRecall = RECNO('sqlcust')\n"
        "SELECT sqlcust\n"
        "LOCATE FOR NAME = 'CHARLIEX'\n"
        "cTargetReplacedName = NAME\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "lTargetBravoDeleted = DELETED()\n"
        "SELECT sqlother\n"
        "cAliasFinal = ALIAS()\n"
        "nOtherRecFinal = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL mutation IN-target parity script should complete");
    expect(state.sql_connections.empty(), "SQL mutation IN-target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_before = state.globals.find("ncustrecbefore");
    const auto alias_after_replace = state.globals.find("caliasafterreplace");
    const auto other_rec_after_replace = state.globals.find("notherrecafterreplace");
    const auto cust_rec_after_replace = state.globals.find("ncustrecafterreplace");
    const auto alias_after_delete = state.globals.find("caliasafterdelete");
    const auto other_rec_after_delete = state.globals.find("notherrecafterdelete");
    const auto cust_rec_after_delete = state.globals.find("ncustrecafterdelete");
    const auto alias_after_recall = state.globals.find("caliasafterrecall");
    const auto other_rec_after_recall = state.globals.find("notherrecafterrecall");
    const auto cust_rec_after_recall = state.globals.find("ncustrecafterrecall");
    const auto target_replaced_name = state.globals.find("ctargetreplacedname");
    const auto target_bravo_deleted = state.globals.find("ltargetbravodeleted");
    const auto alias_final = state.globals.find("caliasfinal");
    const auto other_rec_final = state.globals.find("notherrecfinal");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL mutation IN-target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL mutation IN-target parity");
    expect(alias_before != state.globals.end(), "Selected alias before targeted SQL mutation commands should be captured");
    expect(other_rec_before != state.globals.end(), "Selected SQL cursor RECNO() before targeted mutations should be captured");
    expect(cust_rec_before != state.globals.end(), "Target SQL cursor RECNO() before targeted mutations should be captured");
    expect(alias_after_replace != state.globals.end(), "Selected alias after REPLACE IN should be captured");
    expect(other_rec_after_replace != state.globals.end(), "Selected SQL cursor RECNO() after REPLACE IN should be captured");
    expect(cust_rec_after_replace != state.globals.end(), "Target SQL cursor RECNO() after REPLACE IN should be captured");
    expect(alias_after_delete != state.globals.end(), "Selected alias after DELETE FOR ... IN should be captured");
    expect(other_rec_after_delete != state.globals.end(), "Selected SQL cursor RECNO() after DELETE FOR ... IN should be captured");
    expect(cust_rec_after_delete != state.globals.end(), "Target SQL cursor RECNO() after DELETE FOR ... IN should be captured");
    expect(alias_after_recall != state.globals.end(), "Selected alias after RECALL FOR ... IN should be captured");
    expect(other_rec_after_recall != state.globals.end(), "Selected SQL cursor RECNO() after RECALL FOR ... IN should be captured");
    expect(cust_rec_after_recall != state.globals.end(), "Target SQL cursor RECNO() after RECALL FOR ... IN should be captured");
    expect(target_replaced_name != state.globals.end(), "Target SQL cursor REPLACE IN field update should be captured");
    expect(target_bravo_deleted != state.globals.end(), "Target SQL cursor DELETE/RECALL IN state should be captured");
    expect(alias_final != state.globals.end(), "Selected alias after targeted SQL mutation verification should be captured");
    expect(other_rec_final != state.globals.end(), "Selected SQL cursor RECNO() final position should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL mutation IN-target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL mutation checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL mutation checks");
    }
    if (alias_before != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_before->second)) == "SQLOTHER", "selected SQL alias should start on sqlother before targeted mutations");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before targeted mutations");
    }
    if (cust_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_before->second) == "3", "target SQL cursor should be positioned at bottom before REPLACE IN");
    }
    if (alias_after_replace != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_replace->second)) == "SQLOTHER", "REPLACE IN should preserve the selected SQL alias");
    }
    if (other_rec_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after_replace->second) == "3", "REPLACE IN should preserve the selected SQL cursor pointer");
    }
    if (cust_rec_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_replace->second) == "3", "REPLACE IN should keep the targeted SQL cursor pointer on the current record");
    }
    if (alias_after_delete != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_delete->second)) == "SQLOTHER", "DELETE FOR ... IN should preserve the selected SQL alias");
    }
    if (other_rec_after_delete != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after_delete->second) == "3", "DELETE FOR ... IN should preserve the selected SQL cursor pointer");
    }
    if (cust_rec_after_delete != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_delete->second) == "3", "DELETE FOR ... IN should restore the targeted SQL cursor pointer");
    }
    if (alias_after_recall != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_recall->second)) == "SQLOTHER", "RECALL FOR ... IN should preserve the selected SQL alias");
    }
    if (other_rec_after_recall != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after_recall->second) == "3", "RECALL FOR ... IN should preserve the selected SQL cursor pointer");
    }
    if (cust_rec_after_recall != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_recall->second) == "3", "RECALL FOR ... IN should restore the targeted SQL cursor pointer");
    }
    if (target_replaced_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_replaced_name->second) == "CHARLIEX", "REPLACE IN should update the targeted SQL row fields");
    }
    if (target_bravo_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_bravo_deleted->second) == "false", "DELETE FOR ... IN followed by RECALL FOR ... IN should leave the targeted SQL row recalled");
    }
    if (alias_final != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_final->second)) == "SQLOTHER", "selected SQL alias should remain on sqlother at the end of targeted mutation checks");
    }
    if (other_rec_final != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_final->second) == "3", "selected SQL cursor pointer should remain unchanged at the end of targeted mutation checks");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL mutation checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.replace"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.delete"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.recall"; }),
        "targeted SQL mutation commands should emit runtime.replace, runtime.delete, and runtime.recall events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_navigation_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_navigation_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_navigation_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "cAliasBefore = ALIAS()\n"
        "nOtherRecBefore = RECNO()\n"
        "nCustRecBefore = RECNO('sqlcust')\n"
        "GO TOP IN sqlcust\n"
        "nCustRecAfterGoTop = RECNO('sqlcust')\n"
        "SKIP 1 IN sqlcust\n"
        "nCustRecAfterSkip = RECNO('sqlcust')\n"
        "LOCATE FOR AMOUNT = 30 IN sqlcust\n"
        "nCustRecAfterLocate = RECNO('sqlcust')\n"
        "GO 99 IN sqlcust\n"
        "nCustRecAfterGoEdge = RECNO('sqlcust')\n"
        "cAliasAfterCommands = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL navigation IN-target parity script should complete");
    expect(state.sql_connections.empty(), "SQL navigation IN-target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_before = state.globals.find("ncustrecbefore");
    const auto cust_rec_after_go_top = state.globals.find("ncustrecaftergotop");
    const auto cust_rec_after_skip = state.globals.find("ncustrecafterskip");
    const auto cust_rec_after_locate = state.globals.find("ncustrecafterlocate");
    const auto cust_rec_after_go_edge = state.globals.find("ncustrecaftergoedge");
    const auto alias_after_commands = state.globals.find("caliasaftercommands");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL navigation IN-target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL navigation IN-target parity");
    expect(alias_before != state.globals.end(), "Selected alias before targeted SQL navigation commands should be captured");
    expect(other_rec_before != state.globals.end(), "Selected SQL cursor RECNO() before targeted navigation should be captured");
    expect(cust_rec_before != state.globals.end(), "Target SQL cursor RECNO() before targeted navigation should be captured");
    expect(cust_rec_after_go_top != state.globals.end(), "Target SQL cursor RECNO() after GO TOP IN should be captured");
    expect(cust_rec_after_skip != state.globals.end(), "Target SQL cursor RECNO() after SKIP IN should be captured");
    expect(cust_rec_after_locate != state.globals.end(), "Target SQL cursor RECNO() after LOCATE IN should be captured");
    expect(cust_rec_after_go_edge != state.globals.end(), "Target SQL cursor RECNO() after GO edge IN should be captured");
    expect(alias_after_commands != state.globals.end(), "Selected alias after targeted SQL navigation commands should be captured");
    expect(other_rec_after != state.globals.end(), "Selected SQL cursor RECNO() after targeted navigation should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL navigation IN-target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL navigation checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL navigation checks");
    }
    if (alias_before != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_before->second)) == "SQLOTHER", "selected SQL alias should start on sqlother before targeted navigation");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before targeted navigation");
    }
    if (cust_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_before->second) == "1", "target SQL cursor should start at first record before targeted navigation");
    }
    if (cust_rec_after_go_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_go_top->second) == "1", "GO TOP IN should reposition the targeted SQL cursor to first record");
    }
    if (cust_rec_after_skip != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_skip->second) == "2", "SKIP IN should move the targeted SQL cursor pointer");
    }
    if (cust_rec_after_locate != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_locate->second) == "3", "LOCATE ... IN should position the targeted SQL cursor on the match");
    }
    if (cust_rec_after_go_edge != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_go_edge->second) == "4", "GO 99 IN should move targeted SQL cursor to EOF position");
    }
    if (alias_after_commands != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_commands->second)) == "SQLOTHER", "targeted SQL navigation commands should preserve the selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "targeted SQL navigation commands should preserve the selected SQL cursor pointer");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL navigation checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.go"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.skip"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.locate"; }),
        "targeted SQL GO/SKIP/LOCATE commands should emit runtime navigation events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_filter_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_filter_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_filter_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "cAliasBefore = ALIAS()\n"
        "nOtherRecBefore = RECNO()\n"
        "SET FILTER TO AMOUNT >= 20 IN sqlcust\n"
        "GO TOP IN sqlcust\n"
        "nCustRecAfterGoTop = RECNO('sqlcust')\n"
        "SKIP 1 IN sqlcust\n"
        "nCustRecAfterSkip = RECNO('sqlcust')\n"
        "SKIP 1 IN sqlcust\n"
        "nCustRecAfterSkipEdge = RECNO('sqlcust')\n"
        "SET FILTER TO IN sqlcust\n"
        "GO TOP IN sqlcust\n"
        "nCustRecAfterFilterOff = RECNO('sqlcust')\n"
        "cAliasAfter = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL filter IN-target parity script should complete");
    expect(state.sql_connections.empty(), "SQL filter IN-target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_after_go_top = state.globals.find("ncustrecaftergotop");
    const auto cust_rec_after_skip = state.globals.find("ncustrecafterskip");
    const auto cust_rec_after_skip_edge = state.globals.find("ncustrecafterskipedge");
    const auto cust_rec_after_filter_off = state.globals.find("ncustrecafterfilteroff");
    const auto alias_after = state.globals.find("caliasafter");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL filter IN-target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL filter IN-target parity");
    expect(alias_before != state.globals.end(), "Selected alias before targeted SQL filter flow should be captured");
    expect(other_rec_before != state.globals.end(), "Selected SQL cursor RECNO() before targeted SQL filter flow should be captured");
    expect(cust_rec_after_go_top != state.globals.end(), "Target SQL cursor RECNO() after GO TOP IN with filter should be captured");
    expect(cust_rec_after_skip != state.globals.end(), "Target SQL cursor RECNO() after SKIP IN with filter should be captured");
    expect(cust_rec_after_skip_edge != state.globals.end(), "Target SQL cursor RECNO() after filtered SKIP edge should be captured");
    expect(cust_rec_after_filter_off != state.globals.end(), "Target SQL cursor RECNO() after clearing filter should be captured");
    expect(alias_after != state.globals.end(), "Selected alias after targeted SQL filter flow should be captured");
    expect(other_rec_after != state.globals.end(), "Selected SQL cursor RECNO() after targeted SQL filter flow should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL filter IN-target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL filter checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL filter checks");
    }
    if (alias_before != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_before->second)) == "SQLOTHER", "selected SQL alias should remain on sqlother before targeted filter flow");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before targeted filter flow");
    }
    if (cust_rec_after_go_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_go_top->second) == "2", "GO TOP IN should honor targeted SQL filter visibility");
    }
    if (cust_rec_after_skip != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_skip->second) == "3", "SKIP IN should move across filtered visible SQL rows");
    }
    if (cust_rec_after_skip_edge != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_skip_edge->second) == "4", "filtered SKIP IN edge should move targeted SQL cursor to EOF position");
    }
    if (cust_rec_after_filter_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_filter_off->second) == "1", "clearing targeted SQL filter should restore full GO TOP visibility");
    }
    if (alias_after != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after->second)) == "SQLOTHER", "targeted SQL filter flow should preserve selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "targeted SQL filter flow should preserve selected SQL cursor pointer");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL filter checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.go"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.skip"; }),
        "targeted SQL SET FILTER/GO/SKIP flow should emit runtime.filter and navigation events");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_fault_containment() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "faults.prg";
    write_text(
        main_path,
        "x = 'abc' - 1\n"
        "y = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "broken code should pause with an error instead of killing the host");
    expect(state.location.line == 1U, "runtime faults should highlight the faulting line");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.error"; }),
        "runtime faults should emit a runtime.error event");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a trapped runtime error should keep the session alive");
    const auto y = state.globals.find("y");
    expect(y != state.globals.end(), "post-fault statements should still be able to run");
    if (y != state.globals.end()) {
        expect(copperfin::runtime::format_value(y->second) == "7", "post-fault statements should update globals");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_table_mutation_and_scan_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_mutation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "mutation.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cFound = NAME\n"
        "nFoundAge = AGE\n"
        "REPLACE AGE WITH 21, NAME WITH 'BRAVOX'\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'DELTA', AGE WITH 40\n"
        "GO TOP\n"
        "nTotal = 0\n"
        "cNames = ''\n"
        "nScanCount = 0\n"
        "SCAN FOR AGE >= 21\n"
        "    nTotal = nTotal + AGE\n"
        "    nScanCount = nScanCount + 1\n"
        "    IF nScanCount = 1\n"
        "        cNames = NAME\n"
        "    ELSE\n"
        "        cNames = cNames + ',' + NAME\n"
        "    ENDIF\n"
        "ENDSCAN\n"
        "GO 2\n"
        "DELETE\n"
        "lDeleted = DELETED()\n"
        "RECALL\n"
        "lRecalled = DELETED()\n"
        "DELETE FOR AGE = 40\n"
        "LOCATE FOR DELETED()\n"
        "cDeletedName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "mutation/scan script should complete");

    const auto found = state.globals.find("cfound");
    const auto found_age = state.globals.find("nfoundage");
    const auto total = state.globals.find("ntotal");
    const auto names = state.globals.find("cnames");
    const auto deleted = state.globals.find("ldeleted");
    const auto recalled = state.globals.find("lrecalled");
    const auto deleted_name = state.globals.find("cdeletedname");

    expect(found != state.globals.end(), "LOCATE should expose the found NAME field");
    expect(found_age != state.globals.end(), "LOCATE should expose the found AGE field");
    expect(total != state.globals.end(), "SCAN aggregate should be captured");
    expect(names != state.globals.end(), "SCAN field concatenation should be captured");
    expect(deleted != state.globals.end(), "DELETE state should be captured");
    expect(recalled != state.globals.end(), "RECALL state should be captured");
    expect(deleted_name != state.globals.end(), "LOCATE FOR DELETED() should identify the tombstoned record");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "BRAVO", "LOCATE should position the matching record before REPLACE");
    }
    if (found_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_age->second) == "20", "field resolution should expose numeric record values before mutation");
    }
    if (total != state.globals.end()) {
        expect(copperfin::runtime::format_value(total->second) == "91", "SCAN should iterate the mutated matching records and sum AGE");
    }
    if (names != state.globals.end()) {
        expect(copperfin::runtime::format_value(names->second) == "BRAVOX,CHARLIE,DELTA", "SCAN FOR should iterate the matching records in table order");
    }
    if (deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted->second) == "true", "DELETE should tombstone the current record");
    }
    if (recalled != state.globals.end()) {
        expect(copperfin::runtime::format_value(recalled->second) == "false", "RECALL should clear the tombstone flag");
    }
    if (deleted_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_name->second) == "DELTA", "DELETE FOR should tombstone the matching appended record");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.locate"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.scan"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.replace"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.append_blank"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.delete"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.recall"; }),
        "mutation/query commands should emit runtime events");

    fs::remove_all(temp_root, ignored);
}

void test_indexed_table_mutation_surfaces_runtime_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_indexed_mutation_guard";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const auto original_table_bytes = std::filesystem::file_size(table_path);

    const fs::path main_path = temp_root / "indexed_mutation_guard.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "APPEND BLANK\n"
        "xAfterError = 11\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "indexed-table APPEND BLANK should pause with an error");
    expect(state.location.line == 2U, "indexed-table APPEND BLANK should highlight the mutation statement");
    expect(
        state.message.find("Indexed DBF mutation is not yet supported") != std::string::npos,
        "indexed-table APPEND BLANK should explain that structural index writes are not supported");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after indexed-table mutation failure should keep the session alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still execute after indexed-table mutation failure");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "11", "post-error execution should preserve later statements");
    }

    expect(!std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.append_blank";
    }), "failed indexed-table APPEND BLANK should not emit a success append event");
    expect(std::filesystem::file_size(table_path) == original_table_bytes, "failed indexed-table APPEND BLANK should not change the DBF size");

    fs::remove_all(temp_root, ignored);
}

void test_append_blank_for_unsupported_field_layout_surfaces_runtime_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_blank_unsupported_layout";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "unsupported.dbf";
    std::vector<std::uint8_t> table_bytes(65U + 9U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 65U);
    write_le_u16(table_bytes, 10U, 9U);
    const char value_field[] = "VALUE";
    for (std::size_t index = 0; index < 5U; ++index) {
        table_bytes[32U + index] = static_cast<std::uint8_t>(value_field[index]);
    }
    table_bytes[32U + 11U] = static_cast<std::uint8_t>('B');
    write_le_u32(table_bytes, 32U + 12U, 1U);
    table_bytes[32U + 16U] = 8U;
    table_bytes[64U] = 0x0DU;
    table_bytes[65U] = 0x20U;
    for (std::size_t index = 0; index < 8U; ++index) {
        table_bytes[66U + index] = static_cast<std::uint8_t>('0' + index);
    }
    table_bytes.back() = 0x1AU;
    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto original_size = std::filesystem::file_size(table_path);

    const fs::path main_path = temp_root / "append_blank_unsupported.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Weird IN 0\n"
        "APPEND BLANK\n"
        "xAfterError = 17\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "unsupported-layout APPEND BLANK should pause with an error");
    expect(state.location.line == 2U, "unsupported-layout APPEND BLANK should highlight the mutation statement");
    expect(
        state.message.find("APPEND BLANK is not yet supported") != std::string::npos,
        "unsupported-layout APPEND BLANK should explain the unsupported field layout");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after unsupported-layout APPEND BLANK failure should keep the session alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still execute after unsupported-layout APPEND BLANK failure");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "17", "post-error execution should preserve later statements after unsupported-layout APPEND BLANK failure");
    }

    expect(std::filesystem::file_size(table_path) == original_size, "unsupported-layout APPEND BLANK should not change the DBF size");

    fs::remove_all(temp_root, ignored);
}

void test_set_filter_scopes_local_cursor_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "SET FILTER TO AGE >= 30\n"
        "GO TOP\n"
        "cTopFiltered = NAME\n"
        "SKIP\n"
        "cNextFiltered = NAME\n"
        "SKIP\n"
        "lFilteredEof = EOF()\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "lFilteredFound = FOUND()\n"
        "lFilteredLocateEof = EOF()\n"
        "SELECT Other\n"
        "GO TOP\n"
        "cOtherTop = NAME\n"
        "SELECT People\n"
        "SET FILTER OFF\n"
        "GO TOP\n"
        "cTopUnfiltered = NAME\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cLocateUnfiltered = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FILTER script should complete");

    const auto top_filtered = state.globals.find("ctopfiltered");
    const auto next_filtered = state.globals.find("cnextfiltered");
    const auto filtered_eof = state.globals.find("lfilteredeof");
    const auto filtered_found = state.globals.find("lfilteredfound");
    const auto filtered_locate_eof = state.globals.find("lfilteredlocateeof");
    const auto other_top = state.globals.find("cothertop");
    const auto top_unfiltered = state.globals.find("ctopunfiltered");
    const auto locate_unfiltered = state.globals.find("clocateunfiltered");

    expect(top_filtered != state.globals.end(), "filtered GO TOP should expose the first visible record");
    expect(next_filtered != state.globals.end(), "filtered SKIP should expose the next visible record");
    expect(filtered_eof != state.globals.end(), "filtered SKIP past the last visible row should update EOF()");
    expect(filtered_found != state.globals.end(), "filtered LOCATE should expose FOUND()");
    expect(filtered_locate_eof != state.globals.end(), "filtered LOCATE miss should expose EOF()");
    expect(other_top != state.globals.end(), "filters should not bleed into a second alias/work area");
    expect(top_unfiltered != state.globals.end(), "SET FILTER OFF should restore unfiltered navigation");
    expect(locate_unfiltered != state.globals.end(), "SET FILTER OFF should restore unfiltered LOCATE behavior");

    if (top_filtered != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_filtered->second) == "CHARLIE", "GO TOP should land on the first filtered-visible row");
    }
    if (next_filtered != state.globals.end()) {
        expect(copperfin::runtime::format_value(next_filtered->second) == "DELTA", "SKIP should move among filtered-visible rows");
    }
    if (filtered_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_eof->second) == "true", "SKIP past the filtered-visible tail should reach EOF");
    }
    if (filtered_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_found->second) == "false", "LOCATE should not find rows excluded by the active filter");
    }
    if (filtered_locate_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_locate_eof->second) == "true", "LOCATE misses within a filtered set should leave the cursor at EOF");
    }
    if (other_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top->second) == "ALPHA", "SET FILTER should remain scoped to the targeted cursor/work area");
    }
    if (top_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_unfiltered->second) == "ALPHA", "SET FILTER OFF should restore full-table GO TOP semantics");
    }
    if (locate_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_unfiltered->second) == "BRAVO", "SET FILTER OFF should restore full-table LOCATE behavior");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail.find("AGE >= 30") != std::string::npos; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail == "OFF"; }),
        "SET FILTER changes should emit runtime.filter events");

    fs::remove_all(temp_root, ignored);
}

void test_set_filter_in_targets_nonselected_alias() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter_in";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter_in.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "cTargetAlias = 'Other'\n"
        "SET FILTER TO AGE >= 30 IN cTargetAlias\n"
        "GO TOP\n"
        "cPeopleTop = NAME\n"
        "SELECT Other\n"
        "GO TOP\n"
        "cOtherTop = NAME\n"
        "SET FILTER OFF IN cTargetAlias\n"
        "GO TOP\n"
        "cOtherTopUnfiltered = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FILTER ... IN script should complete");

    const auto people_top = state.globals.find("cpeopletop");
    const auto other_top = state.globals.find("cothertop");
    const auto other_top_unfiltered = state.globals.find("cothertopunfiltered");

    expect(people_top != state.globals.end(), "selected alias top row should be captured");
    expect(other_top != state.globals.end(), "targeted alias filtered row should be captured");
    expect(other_top_unfiltered != state.globals.end(), "targeted alias unfiltered row should be captured");

    if (people_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_top->second) == "ALPHA", "SET FILTER ... IN cTargetAlias should not affect People");
    }
    if (other_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top->second) == "CHARLIE", "SET FILTER ... IN cTargetAlias should affect the targeted alias");
    }
    if (other_top_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top_unfiltered->second) == "ALPHA", "SET FILTER OFF IN cTargetAlias should restore unfiltered navigation for the targeted alias");
    }

    expect(
        std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.filter";
        }) >= 2,
        "SET FILTER ... IN and SET FILTER OFF IN should emit runtime.filter events");

    fs::remove_all(temp_root, ignored);
}

void test_do_while_and_loop_control_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_do_while";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 21}, {"BRAVO", 28}, {"CHARLIE", 33}, {"DELTA", 44}});

    const fs::path main_path = temp_root / "control.prg";
    write_text(
        main_path,
        "nWhile = 0\n"
        "i = 0\n"
        "DO WHILE i < 5\n"
        "    i = i + 1\n"
        "    IF i = 2\n"
        "        CONTINUE\n"
        "    ENDIF\n"
        "    nWhile = nWhile + i\n"
        "    IF i = 4\n"
        "        EXIT\n"
        "    ENDIF\n"
        "ENDDO\n"
        "nNested = 0\n"
        "outer = 0\n"
        "DO WHILE outer < 2\n"
        "    outer = outer + 1\n"
        "    inner = 0\n"
        "    DO WHILE inner < 3\n"
        "        inner = inner + 1\n"
        "        IF inner = 2\n"
        "            CONTINUE\n"
        "        ENDIF\n"
        "        nNested = nNested + 1\n"
        "    ENDDO\n"
        "ENDDO\n"
        "nFor = 0\n"
        "FOR j = 1 TO 5\n"
        "    IF j = 2\n"
        "        LOOP\n"
        "    ENDIF\n"
        "    nFor = nFor + j\n"
        "    IF j = 4\n"
        "        EXIT\n"
        "    ENDIF\n"
        "ENDFOR\n"
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT People\n"
        "nScan = 0\n"
        "SCAN\n"
        "    IF NAME = 'BRAVO'\n"
        "        LOOP\n"
        "    ENDIF\n"
        "    nScan = nScan + 1\n"
        "    IF NAME = 'CHARLIE'\n"
        "        EXIT\n"
        "    ENDIF\n"
        "ENDSCAN\n"
        "cAfterScan = NAME\n"
        "nAfterWhileIndex = i\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO WHILE/loop control script should complete");

    const auto while_total = state.globals.find("nwhile");
    const auto nested_total = state.globals.find("nnested");
    const auto for_total = state.globals.find("nfor");
    const auto scan_total = state.globals.find("nscan");
    const auto after_scan = state.globals.find("cafterscan");
    const auto after_while_index = state.globals.find("nafterwhileindex");

    expect(while_total != state.globals.end(), "DO WHILE should leave its accumulator in globals");
    expect(nested_total != state.globals.end(), "nested DO WHILE loops should leave their accumulator in globals");
    expect(for_total != state.globals.end(), "FOR with LOOP/EXIT should leave its accumulator in globals");
    expect(scan_total != state.globals.end(), "SCAN with LOOP/EXIT should leave its accumulator in globals");
    expect(after_scan != state.globals.end(), "SCAN EXIT should leave the current record available");
    expect(after_while_index != state.globals.end(), "DO WHILE EXIT should preserve the exiting iteration state");

    if (while_total != state.globals.end()) {
        expect(copperfin::runtime::format_value(while_total->second) == "8", "DO WHILE should honor CONTINUE and EXIT");
    }
    if (nested_total != state.globals.end()) {
        expect(copperfin::runtime::format_value(nested_total->second) == "4", "nested DO WHILE loops should reevaluate each loop independently");
    }
    if (for_total != state.globals.end()) {
        expect(copperfin::runtime::format_value(for_total->second) == "8", "LOOP and EXIT should apply to FOR loops");
    }
    if (scan_total != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_total->second) == "2", "LOOP and EXIT should apply to SCAN loops");
    }
    if (after_scan != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_scan->second) == "CHARLIE", "EXIT inside SCAN should leave the cursor on the exiting record");
    }
    if (after_while_index != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_while_index->second) == "4", "EXIT inside DO WHILE should leave the current iteration state intact");
    }

    fs::remove_all(temp_root, ignored);
}

void test_do_case_control_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_do_case";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "do_case.prg";
    write_text(
        main_path,
        "nValue = 2\n"
        "cBranch = ''\n"
        "DO CASE\n"
        "    CASE nValue = 1\n"
        "        cBranch = 'ONE'\n"
        "    CASE nValue = 2\n"
        "        cBranch = 'TWO'\n"
        "    OTHERWISE\n"
        "        cBranch = 'OTHER'\n"
        "ENDCASE\n"
        "nNoMatch = 0\n"
        "DO CASE\n"
        "    CASE .F.\n"
        "        nNoMatch = 1\n"
        "ENDCASE\n"
        "cNested = ''\n"
        "DO CASE\n"
        "    CASE .T.\n"
        "        DO CASE\n"
        "            CASE 1 = 2\n"
        "                cNested = 'BAD'\n"
        "            CASE 2 = 2\n"
        "                cNested = 'INNER'\n"
        "            OTHERWISE\n"
        "                cNested = 'MISS'\n"
        "        ENDCASE\n"
        "    OTHERWISE\n"
        "        cNested = 'OUTER'\n"
        "ENDCASE\n"
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT People\n"
        "nTagged = 0\n"
        "SCAN\n"
        "    DO CASE\n"
        "        CASE AGE < 20\n"
        "            cTag = 'YOUNG'\n"
        "        CASE AGE < 35\n"
        "            cTag = 'MID'\n"
        "        OTHERWISE\n"
        "            cTag = 'SENIOR'\n"
        "    ENDCASE\n"
        "    IF cTag = 'MID'\n"
        "        nTagged = nTagged + 1\n"
        "    ENDIF\n"
        "ENDSCAN\n"
        "cAfterScan = cTag\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO CASE script should complete");

    const auto branch = state.globals.find("cbranch");
    const auto no_match = state.globals.find("nnomatch");
    const auto nested = state.globals.find("cnested");
    const auto tagged = state.globals.find("ntagged");
    const auto after_scan = state.globals.find("cafterscan");

    expect(branch != state.globals.end(), "DO CASE should expose the selected branch result");
    expect(no_match != state.globals.end(), "DO CASE without OTHERWISE should complete without mutating unmatched state");
    expect(nested != state.globals.end(), "nested DO CASE blocks should execute correctly");
    expect(tagged != state.globals.end(), "DO CASE inside SCAN should participate in cursor-backed logic");
    expect(after_scan != state.globals.end(), "DO CASE inside SCAN should leave the last computed branch value");

    if (branch != state.globals.end()) {
        expect(copperfin::runtime::format_value(branch->second) == "TWO", "DO CASE should execute the first matching CASE branch only");
    }
    if (no_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(no_match->second) == "0", "DO CASE with no match and no OTHERWISE should fall through cleanly");
    }
    if (nested != state.globals.end()) {
        expect(copperfin::runtime::format_value(nested->second) == "INNER", "nested DO CASE blocks should honor inner matching semantics");
    }
    if (tagged != state.globals.end()) {
        expect(copperfin::runtime::format_value(tagged->second) == "2", "DO CASE inside SCAN should classify matching rows without fall-through");
    }
    if (after_scan != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_scan->second) == "SENIOR", "DO CASE should preserve the last branch result inside loop-driven execution");
    }

    fs::remove_all(temp_root, ignored);
}

void test_text_endtext_literal_blocks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_text_blocks";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "text_blocks.prg";
    write_text(
        main_path,
        "TEXT TO cBody NOSHOW\n"
        "Alpha\n"
        "\n"
        "* literal star line\n"
        "&& literal ampersand line\n"
        "ENDTEXT\n"
        "TEXT TO cBody ADDITIVE NOSHOW\n"
        "Bravo\n"
        "ENDTEXT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TEXT/ENDTEXT script should complete");

    const auto body = state.globals.find("cbody");
    expect(body != state.globals.end(), "TEXT TO should assign the captured block into the target variable");
    if (body != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(body->second) == "Alpha\n\n* literal star line\n&& literal ampersand line\nBravo\n",
            "TEXT/ENDTEXT should preserve literal lines and ADDITIVE should append the next block");
    }

    const auto text_events = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.text";
    }));
    expect(text_events == 2, "each TEXT block should emit a runtime.text event");

    fs::remove_all(temp_root, ignored);
}

void test_aggregate_functions_respect_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aggregates";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "aggregates.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nCountAll = COUNT()\n"
        "nSumAll = SUM(AGE)\n"
        "nAvgAll = AVG(AGE)\n"
        "nMinAll = MIN(AGE)\n"
        "nMaxAll = MAX(AGE)\n"
        "SET FILTER TO AGE >= 20\n"
        "DELETE FOR AGE = 40\n"
        "SET DELETED ON\n"
        "nCountVisible = COUNT()\n"
        "nCountConditional = COUNT(AGE >= 30)\n"
        "nSumVisible = SUM(AGE)\n"
        "nSumConditional = SUM(AGE, AGE >= 30)\n"
        "nAverageVisible = AVERAGE(AGE)\n"
        "nMinVisible = MIN(AGE)\n"
        "nMaxVisible = MAX(AGE)\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "nCountPeopleAlias = COUNT(AGE >= 20, 'People')\n"
        "nSumPeopleAlias = SUM(AGE, AGE >= 20, 'People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "aggregate script should complete");

    const auto count_all = state.globals.find("ncountall");
    const auto sum_all = state.globals.find("nsumall");
    const auto avg_all = state.globals.find("navgall");
    const auto min_all = state.globals.find("nminall");
    const auto max_all = state.globals.find("nmaxall");
    const auto count_visible = state.globals.find("ncountvisible");
    const auto count_conditional = state.globals.find("ncountconditional");
    const auto sum_visible = state.globals.find("nsumvisible");
    const auto sum_conditional = state.globals.find("nsumconditional");
    const auto avg_visible = state.globals.find("naveragevisible");
    const auto min_visible = state.globals.find("nminvisible");
    const auto max_visible = state.globals.find("nmaxvisible");
    const auto count_alias = state.globals.find("ncountpeoplealias");
    const auto sum_alias = state.globals.find("nsumpeoplealias");

    expect(count_all != state.globals.end(), "COUNT() should be captured");
    expect(sum_all != state.globals.end(), "SUM() should be captured");
    expect(avg_all != state.globals.end(), "AVG() should be captured");
    expect(min_all != state.globals.end(), "MIN() should be captured");
    expect(max_all != state.globals.end(), "MAX() should be captured");
    expect(count_visible != state.globals.end(), "COUNT() should respect active visibility rules");
    expect(count_conditional != state.globals.end(), "COUNT(condition) should be captured");
    expect(sum_visible != state.globals.end(), "SUM() should respect active visibility rules");
    expect(sum_conditional != state.globals.end(), "SUM(value, condition) should be captured");
    expect(avg_visible != state.globals.end(), "AVERAGE() should be captured");
    expect(min_visible != state.globals.end(), "MIN() under filter should be captured");
    expect(max_visible != state.globals.end(), "MAX() under filter should be captured");
    expect(count_alias != state.globals.end(), "COUNT(condition, alias) should be captured");
    expect(sum_alias != state.globals.end(), "SUM(value, condition, alias) should be captured");

    if (count_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_all->second) == "4", "COUNT() should count all visible rows before filters");
    }
    if (sum_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_all->second) == "100", "SUM() should total numeric field values across the current cursor");
    }
    if (avg_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_all->second) == "25", "AVG() should compute the mean across visible rows");
    }
    if (min_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(min_all->second) == "10", "MIN() should capture the smallest visible value");
    }
    if (max_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(max_all->second) == "40", "MAX() should capture the largest visible value");
    }
    if (count_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_visible->second) == "2", "COUNT() should respect SET FILTER TO and SET DELETED ON");
    }
    if (count_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_conditional->second) == "1", "COUNT(condition) should evaluate an additional aggregate condition");
    }
    if (sum_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_visible->second) == "50", "SUM() should total only currently visible rows");
    }
    if (sum_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_conditional->second) == "30", "SUM(value, condition) should apply the extra condition after visibility filtering");
    }
    if (avg_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_visible->second) == "25", "AVERAGE() should compute the mean over visible rows");
    }
    if (min_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(min_visible->second) == "20", "MIN() should respect active visibility rules");
    }
    if (max_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(max_visible->second) == "30", "MAX() should respect active visibility rules");
    }
    if (count_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_alias->second) == "2", "COUNT(condition, alias) should target a non-selected cursor");
    }
    if (sum_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_alias->second) == "50", "SUM(value, condition, alias) should target a non-selected cursor");
    }

    fs::remove_all(temp_root, ignored);
}

void test_calculate_command_aggregates() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_calculate";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "calculate.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "CALCULATE COUNT() TO nCountAll, SUM(AGE) TO nSumAll, AVG(AGE) TO nAvgAll\n"
        "SET FILTER TO AGE >= 20\n"
        "DELETE FOR AGE = 40\n"
        "SET DELETED ON\n"
        "CALCULATE COUNT() TO nCountVisible, SUM(AGE) TO nSumVisible, MIN(AGE) TO nMinVisible, MAX(AGE) TO nMaxVisible\n"
        "CALCULATE COUNT() TO nCountConditional, SUM(AGE) TO nSumConditional FOR AGE >= 30\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "CALCULATE COUNT() TO nCountPeopleAlias, SUM(AGE) TO nSumPeopleAlias FOR AGE >= 20 IN 'People'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CALCULATE script should complete");

    const auto count_all = state.globals.find("ncountall");
    const auto sum_all = state.globals.find("nsumall");
    const auto avg_all = state.globals.find("navgall");
    const auto count_visible = state.globals.find("ncountvisible");
    const auto sum_visible = state.globals.find("nsumvisible");
    const auto min_visible = state.globals.find("nminvisible");
    const auto max_visible = state.globals.find("nmaxvisible");
    const auto count_conditional = state.globals.find("ncountconditional");
    const auto sum_conditional = state.globals.find("nsumconditional");
    const auto count_alias = state.globals.find("ncountpeoplealias");
    const auto sum_alias = state.globals.find("nsumpeoplealias");

    expect(count_all != state.globals.end(), "CALCULATE COUNT() should assign into a variable");
    expect(sum_all != state.globals.end(), "CALCULATE SUM() should assign into a variable");
    expect(avg_all != state.globals.end(), "CALCULATE AVG() should assign into a variable");
    expect(count_visible != state.globals.end(), "CALCULATE should respect current visibility rules");
    expect(sum_visible != state.globals.end(), "CALCULATE SUM() should respect current visibility rules");
    expect(min_visible != state.globals.end(), "CALCULATE MIN() should assign into a variable");
    expect(max_visible != state.globals.end(), "CALCULATE MAX() should assign into a variable");
    expect(count_conditional != state.globals.end(), "CALCULATE FOR should assign into a variable");
    expect(sum_conditional != state.globals.end(), "CALCULATE FOR should constrain SUM() results");
    expect(count_alias != state.globals.end(), "CALCULATE IN alias should target a non-selected cursor");
    expect(sum_alias != state.globals.end(), "CALCULATE IN alias should sum against a non-selected cursor");

    if (count_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_all->second) == "4", "CALCULATE COUNT() should count all rows before filters");
    }
    if (sum_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_all->second) == "100", "CALCULATE SUM() should total numeric field values");
    }
    if (avg_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_all->second) == "25", "CALCULATE AVG() should compute a mean");
    }
    if (count_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_visible->second) == "2", "CALCULATE should respect SET FILTER TO and SET DELETED ON");
    }
    if (sum_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_visible->second) == "50", "CALCULATE SUM() should total only visible rows");
    }
    if (min_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(min_visible->second) == "20", "CALCULATE MIN() should respect visibility rules");
    }
    if (max_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(max_visible->second) == "30", "CALCULATE MAX() should respect visibility rules");
    }
    if (count_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_conditional->second) == "1", "CALCULATE FOR should apply an additional condition to COUNT()");
    }
    if (sum_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_conditional->second) == "30", "CALCULATE FOR should apply an additional condition to SUM()");
    }
    if (count_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_alias->second) == "2", "CALCULATE IN alias should use the targeted cursor context");
    }
    if (sum_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_alias->second) == "50", "CALCULATE IN alias should sum visible rows from the targeted cursor");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.calculate"; }),
        "CALCULATE should emit runtime.calculate events");

    fs::remove_all(temp_root, ignored);
}

void test_command_level_aggregate_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_command_aggregates";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "command_aggregates.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET FILTER TO AGE >= 20\n"
        "DELETE FOR AGE = 40\n"
        "SET DELETED ON\n"
        "GO TOP\n"
        "COUNT TO nCountVisible\n"
        "COUNT FOR AGE >= 30 TO nCountConditional\n"
        "SUM AGE, AGE * 2 TO nSumAge, nSumDouble\n"
        "AVERAGE AGE TO nAvgVisible\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "SET FILTER TO AGE >= 30\n"
        "GO TOP\n"
        "SELECT People\n"
        "COUNT TO nOtherCount IN 'Other'\n"
        "SUM AGE TO nOtherSum IN 'Other'\n"
        "AVERAGE AGE TO nOtherAvg IN 'Other'\n"
        "nOtherRec = RECNO('Other')\n"
        "nOtherScanHits = 0\n"
        "LOCATE FOR AGE = 30 IN 'Other'\n"
        "nOtherLocate = RECNO('Other')\n"
        "SCAN FOR AGE >= 30 IN 'Other'\n"
        "    nOtherScanHits = nOtherScanHits + 1\n"
        "ENDSCAN\n"
        "nPeopleRec = RECNO('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "command-level aggregate script should complete");

    const auto count_visible = state.globals.find("ncountvisible");
    const auto count_conditional = state.globals.find("ncountconditional");
    const auto sum_age = state.globals.find("nsumage");
    const auto sum_double = state.globals.find("nsumdouble");
    const auto avg_visible = state.globals.find("navgvisible");
    const auto other_count = state.globals.find("nothercount");
    const auto other_sum = state.globals.find("nothersum");
    const auto other_avg = state.globals.find("notheravg");
    const auto other_rec = state.globals.find("notherrec");
    const auto other_locate = state.globals.find("notherlocate");
    const auto other_scan_hits = state.globals.find("notherscanhits");
    const auto people_rec = state.globals.find("npeoplerec");

    expect(count_visible != state.globals.end(), "COUNT TO should assign into a variable");
    expect(count_conditional != state.globals.end(), "COUNT FOR TO should assign into a variable");
    expect(sum_age != state.globals.end(), "SUM TO should assign into a variable");
    expect(sum_double != state.globals.end(), "SUM with multiple expressions should assign into variables");
    expect(avg_visible != state.globals.end(), "AVERAGE TO should assign into a variable");
    expect(other_count != state.globals.end(), "COUNT IN alias should assign into a variable");
    expect(other_sum != state.globals.end(), "SUM IN alias should assign into a variable");
    expect(other_avg != state.globals.end(), "AVERAGE IN alias should assign into a variable");
    expect(other_rec != state.globals.end(), "aggregate IN alias commands should preserve the targeted cursor position");
    expect(other_locate != state.globals.end(), "LOCATE FOR ... IN alias should update the targeted cursor");
    expect(other_scan_hits != state.globals.end(), "SCAN FOR ... IN alias should execute against the targeted cursor");
    expect(people_rec != state.globals.end(), "targeted alias scans should preserve the selected cursor position");

    if (count_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_visible->second) == "2", "COUNT should respect SET FILTER TO and SET DELETED ON");
    }
    if (count_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_conditional->second) == "1", "COUNT FOR should apply an additional condition");
    }
    if (sum_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_age->second) == "50", "SUM should total visible numeric values");
    }
    if (sum_double != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_double->second) == "100", "SUM should evaluate numeric expressions per visible row");
    }
    if (avg_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_visible->second) == "25", "AVERAGE should compute the mean across visible rows");
    }
    if (other_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_count->second) == "1", "COUNT IN alias should use the targeted cursor visibility rules");
    }
    if (other_sum != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_sum->second) == "30", "SUM IN alias should total values from the targeted cursor");
    }
    if (other_avg != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_avg->second) == "30", "AVERAGE IN alias should evaluate against the targeted cursor");
    }
    if (other_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec->second) == "3", "aggregate IN alias commands should restore the targeted cursor record");
    }
    if (other_locate != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_locate->second) == "3", "LOCATE FOR ... IN alias should position the targeted cursor");
    }
    if (other_scan_hits != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_scan_hits->second) == "1", "SCAN FOR ... IN alias should honor the targeted cursor visibility rules");
    }
    if (people_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec->second) == "2", "aggregate commands and IN-targeted scans should preserve the selected cursor state");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.count"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.sum"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.average"; }),
        "command-level aggregate commands should emit runtime aggregate events");

    fs::remove_all(temp_root, ignored);
}

void test_command_level_aggregate_scope_and_while_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_command_aggregate_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "command_aggregate_scope.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET FILTER TO AGE >= 20\n"
        "DELETE FOR AGE = 40\n"
        "SET DELETED ON\n"
        "GO TOP\n"
        "COUNT REST TO nCountRest\n"
        "COUNT NEXT 2 TO nCountNextTwo\n"
        "COUNT RECORD 4 TO nCountRecordFour\n"
        "SUM AGE REST TO nSumRest\n"
        "SUM AGE, AGE * 2 NEXT 3 TO nSumNextThree, nSumDoubleNextThree\n"
        "AVERAGE AGE WHILE AGE < 40 TO nAvgWhile\n"
        "nPeopleRec = RECNO('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "aggregate scope/WHILE script should complete");

    const auto count_rest = state.globals.find("ncountrest");
    const auto count_next_two = state.globals.find("ncountnexttwo");
    const auto count_record_four = state.globals.find("ncountrecordfour");
    const auto sum_rest = state.globals.find("nsumrest");
    const auto sum_next_three = state.globals.find("nsumnextthree");
    const auto sum_double_next_three = state.globals.find("nsumdoublenextthree");
    const auto avg_while = state.globals.find("navgwhile");
    const auto people_rec = state.globals.find("npeoplerec");

    expect(count_rest != state.globals.end(), "COUNT REST should assign into a variable");
    expect(count_next_two != state.globals.end(), "COUNT NEXT should assign into a variable");
    expect(count_record_four != state.globals.end(), "COUNT RECORD should assign into a variable");
    expect(sum_rest != state.globals.end(), "SUM REST should assign into a variable");
    expect(sum_next_three != state.globals.end(), "SUM NEXT should assign into a variable");
    expect(sum_double_next_three != state.globals.end(), "SUM NEXT should support multiple expressions");
    expect(avg_while != state.globals.end(), "AVERAGE WHILE should assign into a variable");
    expect(people_rec != state.globals.end(), "aggregate scope commands should preserve the selected cursor position");

    if (count_rest != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_rest->second) == "2", "COUNT REST should respect the current record and visibility rules");
    }
    if (count_next_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_next_two->second) == "2", "COUNT NEXT should apply scope before visibility filtering");
    }
    if (count_record_four != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_record_four->second) == "0", "COUNT RECORD should still respect deleted/filter visibility");
    }
    if (sum_rest != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_rest->second) == "50", "SUM REST should total visible rows from the current record forward");
    }
    if (sum_next_three != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_next_three->second) == "50", "SUM NEXT should total only visible rows within the raw scope");
    }
    if (sum_double_next_three != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_double_next_three->second) == "100", "SUM NEXT should evaluate each numeric expression within scope");
    }
    if (avg_while != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_while->second) == "25", "AVERAGE WHILE should stop when the WHILE condition becomes false");
    }
    if (people_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec->second) == "2", "aggregate scope commands should restore the current selected record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_total_command_for_local_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_total_command";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "sales.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "REGION", .type = 'C', .length = 10U},
        {.name = "AMOUNT", .type = 'N', .length = 6U},
        {.name = "QTY", .type = 'N', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"EAST", "10", "1"},
        {"EAST", "15", "2"},
        {"WEST", "8", "4"},
        {"WEST", "12", "5"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "sales DBF fixture should be created");

    const fs::path output_path = temp_root / "totals.dbf";
    const fs::path main_path = temp_root / "total.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Sales IN 0\n"
        "SET FILTER TO QTY >= 2\n"
        "GO TOP\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "SET FILTER TO QTY >= 4\n"
        "GO TOP\n"
        "SELECT Sales\n"
        "TOTAL TO '" + output_path.string() + "' ON REGION FIELDS AMOUNT, QTY REST FOR AMOUNT >= 8 IN 'Other'\n"
        "nSalesRec = RECNO('Sales')\n"
        "nOtherRec = RECNO('Other')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TOTAL script should complete");
    const auto sales_rec = state.globals.find("nsalesrec");
    const auto other_rec = state.globals.find("notherrec");
    expect(sales_rec != state.globals.end(), "TOTAL script should capture the current selected record");
    expect(other_rec != state.globals.end(), "TOTAL IN alias should preserve the targeted cursor position");
    if (sales_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(sales_rec->second) == "2", "TOTAL should preserve the selected cursor position");
    }
    if (other_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec->second) == "3", "TOTAL IN alias should restore the targeted cursor record");
    }
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.total"; }),
        "TOTAL should emit runtime.total events");

    const auto totals_result = copperfin::vfp::parse_dbf_table_from_file(output_path.string(), 10U);
    expect(totals_result.ok, "TOTAL should write a readable output DBF");
    expect(totals_result.table.fields.size() == 3U, "TOTAL output should include the group field plus requested numeric fields");
    expect(totals_result.table.records.size() == 1U, "TOTAL IN alias should aggregate only the targeted cursor's visible rows");
    if (totals_result.table.records.size() == 1U) {
        expect(totals_result.table.records[0].values[0].display_value == "WEST", "TOTAL IN alias should keep the targeted cursor's group key");
        expect(totals_result.table.records[0].values[1].display_value == "20", "TOTAL IN alias should sum numeric fields for the targeted cursor");
        expect(totals_result.table.records[0].values[2].display_value == "9", "TOTAL IN alias should sum each requested numeric field");
    }

    fs::remove_all(temp_root, ignored);
}

void test_total_command_for_sql_result_cursors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_total_sql_cursor";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path output_path = temp_root / "sql_totals.dbf";
    const fs::path main_path = temp_root / "total_sql.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "REPLACE NAME WITH 'ALPHA', AMOUNT WITH 5\n"
        "GO 2\n"
        "REPLACE NAME WITH 'ALPHA', AMOUNT WITH 7\n"
        "GO 3\n"
        "REPLACE NAME WITH 'WEST', AMOUNT WITH 9\n"
        "GO TOP\n"
        "TOTAL TO '" + output_path.string() + "' ON NAME FIELDS AMOUNT REST FOR AMOUNT >= 7 IN 'sqlcust'\n"
        "nSqlRec = RECNO('sqlcust')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TOTAL SQL script should complete");
    expect(state.sql_connections.empty(), "TOTAL SQL script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto sql_rec = state.globals.find("nsqlrec");
    const auto disc = state.globals.find("ldisc");
    expect(exec != state.globals.end(), "SQLEXEC result should be captured for TOTAL SQL parity");
    expect(sql_rec != state.globals.end(), "TOTAL IN alias should preserve SQL cursor position");
    expect(disc != state.globals.end(), "SQLDISCONNECT should be captured for TOTAL SQL parity");
    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before TOTAL SQL checks");
    }
    if (sql_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_rec->second) == "1", "TOTAL should preserve targeted SQL cursor record position");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after TOTAL SQL checks");
    }
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.total"; }),
        "TOTAL for SQL cursors should emit runtime.total events");

    const auto totals_result = copperfin::vfp::parse_dbf_table_from_file(output_path.string(), 10U);
    expect(totals_result.ok, "TOTAL should write a readable output DBF for SQL cursors");
    expect(totals_result.table.fields.size() == 2U, "TOTAL SQL output should include ON plus requested numeric fields");
    expect(totals_result.table.records.size() == 2U, "TOTAL SQL output should include one group per contiguous ON value");
    if (totals_result.table.records.size() == 2U) {
        expect(totals_result.table.records[0].values[0].display_value == "ALPHA", "TOTAL SQL output should preserve first grouped ON value");
        expect(totals_result.table.records[0].values[1].display_value == "7", "TOTAL SQL output should sum grouped numeric fields");
        expect(totals_result.table.records[1].values[0].display_value == "WEST", "TOTAL SQL output should include trailing group values");
        expect(totals_result.table.records[1].values[1].display_value == "9", "TOTAL SQL output should sum trailing grouped numeric values");
    }

    fs::remove_all(temp_root, ignored);
}

void test_private_declaration_masks_caller_variable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_private_mask";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "private_mask.prg";
    write_text(
        main_path,
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "sub_x = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PRIVATE mask script should complete");

    const auto sub_x = state.globals.find("sub_x");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x != state.globals.end(), "sub_x should be in globals");
    expect(caller_x != state.globals.end(), "caller_x should be in globals");

    if (sub_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x->second) == "99", "sub should see its own PRIVATE x = 99");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42", "caller x should be restored to 42 after sub returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_private_variable_visible_to_called_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_private_visible";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "private_visible.prg";
    write_text(
        main_path,
        "DO caller\n"
        "RETURN\n"
        "PROCEDURE caller\n"
        "PRIVATE shared_val\n"
        "shared_val = 77\n"
        "DO inner\n"
        "RETURN\n"
        "PROCEDURE inner\n"
        "inner_saw = shared_val\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PRIVATE visibility script should complete");

    const auto inner_saw = state.globals.find("inner_saw");
    expect(inner_saw != state.globals.end(), "inner_saw should be in globals");
    if (inner_saw != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_saw->second) == "77", "PRIVATE variable should be visible to called routines");
    }

    fs::remove_all(temp_root, ignored);
}

void test_store_command_assigns_multiple_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_store";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "store_test.prg";
    write_text(
        main_path,
        "STORE 7 TO a, b, c\n"
        "STORE 'hello' TO s1, s2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STORE script should complete");

    const auto a = state.globals.find("a");
    const auto b = state.globals.find("b");
    const auto c = state.globals.find("c");
    const auto s1 = state.globals.find("s1");
    const auto s2 = state.globals.find("s2");

    expect(a != state.globals.end(), "STORE should assign a");
    expect(b != state.globals.end(), "STORE should assign b");
    expect(c != state.globals.end(), "STORE should assign c");
    expect(s1 != state.globals.end(), "STORE should assign s1");
    expect(s2 != state.globals.end(), "STORE should assign s2");

    if (a != state.globals.end()) {
        expect(copperfin::runtime::format_value(a->second) == "7", "a should equal 7");
    }
    if (b != state.globals.end()) {
        expect(copperfin::runtime::format_value(b->second) == "7", "b should equal 7");
    }
    if (c != state.globals.end()) {
        expect(copperfin::runtime::format_value(c->second) == "7", "c should equal 7");
    }
    if (s1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(s1->second) == "hello", "s1 should equal 'hello'");
    }
    if (s2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2->second) == "hello", "s2 should equal 'hello'");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_breakpoints_and_stepping();
    test_read_events_pause();
    test_activate_popup_pause();
    test_dispatch_event_handler();
    test_local_variables_in_stack_frame();
    test_report_form_pause();
    test_do_form_pause();
    test_export_vfp_compatibility_corpus_script();
    test_work_area_and_data_session_compatibility();
    test_eval_macro_and_runtime_state_semantics();
    test_sql_and_ole_compatibility_functions();
    test_use_and_data_session_isolation();
    test_use_in_existing_alias_reuses_target_work_area();
    test_use_in_nonselected_alias_preserves_selected_work_area();
    test_plain_use_reuses_current_selected_work_area();
    test_select_and_use_in_designator_expressions();
    test_expression_driven_in_targeting_across_local_data_commands();
    test_select_zero_and_use_in_zero_reuse_closed_work_area();
    test_go_and_skip_cursor_navigation();
    test_cursor_identity_functions_for_local_tables();
    test_cursor_identity_functions_for_sql_result_cursors();
    test_sql_result_cursors_are_isolated_by_data_session();
    test_sql_result_cursor_auto_allocation_tracks_session_selection_flow();
    test_set_order_and_seek_for_local_tables();
    test_seek_uses_grounded_order_normalization_hints();
    test_set_near_changes_seek_failure_position();
    test_set_order_descending_changes_seek_ordering();
    test_seek_command_accepts_tag_override_without_set_order();
    test_seek_command_accepts_descending_tag_override_without_set_order();
    test_set_near_is_scoped_by_data_session();
    test_seek_related_index_functions();
    test_seek_function_accepts_direction_suffix_in_order_designator();
    test_order_and_tag_preserve_index_file_identity();
    test_seek_respects_grounded_order_for_expression_hints();
    test_ndx_numeric_domain_guides_seek_near_ordering();
    test_foxtools_registration_and_call_bridge();
    test_foxtools_registration_is_scoped_by_data_session();
    test_set_exact_affects_comparisons_and_seek();
    test_use_again_and_alias_collision_semantics();
    test_use_in_selected_alias_replacement_clears_old_alias_and_order_state();
    test_select_missing_alias_is_an_error();
    test_use_in_missing_alias_is_an_error();
    test_sql_result_cursors_and_ole_actions();
    test_sql_result_cursor_read_only_parity();
    test_sql_result_cursor_seek_parity();
    test_sql_result_cursor_command_seek_parity();
    test_sql_result_cursor_command_seek_in_target_parity();
    test_sql_result_cursor_scan_in_target_parity();
    test_sql_result_cursor_mutation_parity();
    test_sql_result_cursor_mutation_in_target_parity();
    test_sql_result_cursor_navigation_in_target_parity();
    test_sql_result_cursor_filter_in_target_parity();
    test_local_table_mutation_and_scan_flow();
    test_set_filter_scopes_local_cursor_visibility();
    test_set_filter_in_targets_nonselected_alias();
    test_do_while_and_loop_control_flow();
    test_do_case_control_flow();
    test_text_endtext_literal_blocks();
    test_aggregate_functions_respect_visibility();
    test_calculate_command_aggregates();
    test_command_level_aggregate_commands();
    test_command_level_aggregate_scope_and_while_semantics();
    test_total_command_for_local_tables();
    test_total_command_for_sql_result_cursors();
    test_runtime_fault_containment();
    test_indexed_table_mutation_surfaces_runtime_error();
    test_append_blank_for_unsupported_field_layout_surfaces_runtime_error();
    test_private_declaration_masks_caller_variable();
    test_private_variable_visible_to_called_routines();
    test_store_command_assigns_multiple_variables();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
