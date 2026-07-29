// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_prg_engine_data_io_support.h"

#include <locale>

namespace cf_test_prg_engine_data_io {
namespace {
class grouped_numpunct final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

class global_locale_guard final {
public:
    explicit global_locale_guard(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}

    ~global_locale_guard() { std::locale::global(previous_); }

    global_locale_guard(const global_locale_guard&) = delete;
    global_locale_guard& operator=(const global_locale_guard&) = delete;

private:
    std::locale previous_;
};
}  // namespace

void test_copy_to_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path no_area_path = temp_root / "copy_to_no_area.prg";
    write_text(
        no_area_path,
        "COPY TO 'missing.dbf'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession no_area_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(no_area_path.string(), temp_root.string(), false));
    const auto no_area_state = no_area_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!no_area_state.completed, "#2707: qps-ploc COPY TO without a work area should fail");
    expect(
        no_area_state.message == copperfin::localization::pseudo_localize("COPY TO: no current work area"),
        "#2707: qps-ploc COPY TO precondition error should route through the pseudo-localization transform");

    write_simple_dbf(temp_root / "source.dbf", {"Alice"});
    fs::create_directories(temp_root / "busy.json");

    const fs::path open_fail_path = temp_root / "copy_to_open_fail.prg";
    write_text(
        open_fail_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO '" + (temp_root / "busy.json").string() + "' TYPE JSON\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession open_fail_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(open_fail_path.string(), temp_root.string(), false));
    const auto open_fail_state = open_fail_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!open_fail_state.completed, "#2707: qps-ploc COPY TO TYPE JSON directory-target script should fail");
    expect(
        open_fail_state.message.find("[!! ") == 0U &&
            open_fail_state.message.find("JSON") != std::string::npos &&
            open_fail_state.message.find("unable to open output file") == std::string::npos,
        "#2707: qps-ploc COPY TO TYPE open-failure should pseudo-localize prose while preserving the type");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    write_simple_dbf(temp_root / "target.dbf", {"Alice"});

    const fs::path fields_fail_path = temp_root / "append_from_array_fields_fail.prg";
    write_text(
        fields_fail_path,
        "USE '" + (temp_root / "target.dbf").string() + "'\n"
        "DIMENSION aRows[1,1]\n"
        "aRows[1,1] = 'Bob'\n"
        "APPEND FROM ARRAY aRows FIELDS MissingField\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession fields_fail_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(fields_fail_path.string(), temp_root.string(), false));
    const auto fields_fail_state = fields_fail_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!fields_fail_state.completed, "#2708: qps-ploc APPEND FROM ARRAY with no matching fields should fail");
    expect(
        fields_fail_state.message ==
            copperfin::localization::pseudo_localize("APPEND FROM ARRAY: no fields match the FIELDS clause"),
        "#2708: qps-ploc APPEND FROM ARRAY empty-fields error should route through the pseudo-localization transform");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path no_area_path = temp_root / "append_from_no_area.prg";
    write_text(
        no_area_path,
        "APPEND FROM 'missing.dbf'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession no_area_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(no_area_path.string(), temp_root.string(), false));
    const auto no_area_state = no_area_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!no_area_state.completed, "#2709: qps-ploc APPEND FROM without a work area should fail");
    expect(
        no_area_state.message == copperfin::localization::pseudo_localize("APPEND FROM: no current work area"),
        "#2709: qps-ploc APPEND FROM no-work-area error should route through the pseudo-localization transform");

    write_simple_dbf(temp_root / "target.dbf", {"Alice"});

    const fs::path wrapper_fail_path = temp_root / "append_from_wrapper_fail.prg";
    write_text(
        wrapper_fail_path,
        "USE '" + (temp_root / "target.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "missing.dbf").string() + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession wrapper_fail_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(wrapper_fail_path.string(), temp_root.string(), false));
    const auto wrapper_fail_state = wrapper_fail_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!wrapper_fail_state.completed, "#2709: qps-ploc APPEND FROM missing-source script should fail");
    expect(
        wrapper_fail_state.message.find("[!! ") == 0U &&
            wrapper_fail_state.message.find("APPEND FROM:") == std::string::npos,
        "#2709: qps-ploc APPEND FROM wrapper error should pseudo-localize the command prefix");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_type_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_type_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    write_simple_dbf(temp_root / "target.dbf", {"Alice"});

    const fs::path open_fail_path = temp_root / "append_from_type_sdf_open_fail.prg";
    write_text(
        open_fail_path,
        "USE '" + (temp_root / "target.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "missing.txt").string() + "' TYPE SDF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession open_fail_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(open_fail_path.string(), temp_root.string(), false));
    const auto open_fail_state = open_fail_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!open_fail_state.completed, "#2710: qps-ploc APPEND FROM TYPE SDF missing-source script should fail");
    expect(
        open_fail_state.message.find("[!! ") == 0U &&
            open_fail_state.message.find("SDF") != std::string::npos &&
            open_fail_state.message.find("unable to open source file") == std::string::npos,
        "#2710: qps-ploc APPEND FROM TYPE open-source error should pseudo-localize prose while preserving the type");

    const fs::path json_path = temp_root / "rows.json";
    write_text(json_path, "[{\"NAME\":\"Bob\"}]\n");

    const fs::path fields_fail_path = temp_root / "append_from_type_json_fields_fail.prg";
    write_text(
        fields_fail_path,
        "USE '" + (temp_root / "target.dbf").string() + "'\n"
        "APPEND FROM '" + json_path.string() + "' TYPE JSON FIELDS MissingField\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession fields_fail_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(fields_fail_path.string(), temp_root.string(), false));
    const auto fields_fail_state = fields_fail_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!fields_fail_state.completed, "#2710: qps-ploc APPEND FROM TYPE JSON with no matching fields should fail");
    expect(
        fields_fail_state.message.find("[!! ") == 0U &&
            fields_fail_state.message.find("JSON") != std::string::npos &&
            fields_fail_state.message.find("no fields match the FIELDS clause") == std::string::npos,
        "#2710: qps-ploc APPEND FROM TYPE empty-fields error should pseudo-localize prose while preserving the type");

    const fs::path strict_target_path = temp_root / "strict_target.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> strict_fields{
        {.name = "NAME", .type = 'C', .length = 1U},
    };
    const auto strict_create =
        copperfin::vfp::create_dbf_table_file(strict_target_path.string(), strict_fields, {});
    expect(strict_create.ok, "#2711: strict APPEND FROM TYPE destination fixture should be created");

    const fs::path wrapper_fail_path = temp_root / "append_from_type_json_wrapper_fail.prg";
    write_text(
        wrapper_fail_path,
        "USE '" + strict_target_path.string() + "'\n"
        "APPEND FROM '" + json_path.string() + "' TYPE JSON\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession wrapper_fail_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(wrapper_fail_path.string(), temp_root.string(), false));
    const auto wrapper_fail_state = wrapper_fail_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!wrapper_fail_state.completed, "#2711: qps-ploc APPEND FROM TYPE JSON oversized field script should fail");
    expect(
        wrapper_fail_state.message.find("[!! ") == 0U &&
            wrapper_fail_state.message.find("JSON") != std::string::npos &&
            wrapper_fail_state.message.find("APPEND FROM TYPE JSON:") == std::string::npos,
        "#2711: qps-ploc APPEND FROM TYPE wrapper error should pseudo-localize prose while preserving the type");

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_emits_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"row1", "row2"});

    const fs::path main_path = temp_root / "copy_to_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "nCopyToDestinationCalls = 0\n"
        "COPY TO copy_to_destination('dest')\n"
        "RETURN\n"
        "FUNCTION copy_to_destination\n"
        "LPARAMETERS value\n"
        "nCopyToDestinationCalls = nCopyToDestinationCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO script should complete");

    const bool has_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.copy_to";
        });
    expect(has_event, "COPY TO should emit a runtime.copy_to event");
    const auto destination_calls = state.globals.find("ncopytodestinationcalls");
    expect(destination_calls != state.globals.end(), "COPY TO should preserve the destination resolver call counter");
    if (destination_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(destination_calls->second) == "1",
               "COPY TO should evaluate the destination UDF exactly once");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_creates_destination_dbf() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_full";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob", "Carol"});

    const fs::path main_path = temp_root / "copy_to_full.prg";
    const std::string dest_path = (temp_root / "dest.dbf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO '" + dest_path + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO full script should complete");
    expect(fs::exists(dest_path), "COPY TO should create destination DBF file");

    if (fs::exists(dest_path)) {
        const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path, 100U);
        expect(result.ok, "COPY TO destination DBF should be readable");
        expect(result.table.records.size() == 3U,
            "COPY TO should copy all 3 records (got " + std::to_string(result.table.records.size()) + ")");
        if (result.table.records.size() >= 1U) {
            const auto& first = result.table.records[0U].values;
            const bool has_alice = !first.empty() && first[0U].display_value.find("Alice") != std::string::npos;
            expect(has_alice, "COPY TO first record should contain Alice");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_structure_to_creates_empty_schema() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_struct";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "copy_struct.prg";
    const std::string dest_path = (temp_root / "schema.dbf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY STRUCTURE TO '" + dest_path + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY STRUCTURE TO script should complete");
    expect(fs::exists(dest_path), "COPY STRUCTURE TO should create destination DBF");

    if (fs::exists(dest_path)) {
        const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path, 100U);
        expect(result.ok, "COPY STRUCTURE TO destination DBF should be readable");
        expect(result.table.records.empty(), "COPY STRUCTURE TO should produce zero rows");
        expect(!result.table.fields.empty(), "COPY STRUCTURE TO should preserve field descriptors");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_structure_extended_emits_vfp_metadata_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_struct_extended";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> source_fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(
        source_path.string(), source_fields, {{"Alice", "42", "T"}});
    expect(source_create.ok, "COPY STRUCTURE EXTENDED source fixture should be created");

    const fs::path all_fields_path = temp_root / "structure_extended.dbf";
    const fs::path selected_fields_path = temp_root / "structure_extended_selected.dbf";
    const fs::path main_path = temp_root / "copy_struct_extended.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "COPY STRUCTURE EXTENDED TO '" + all_fields_path.string() + "'\n"
        "COPY STRUCTURE EXTENDED TO '" + selected_fields_path.string() + "' FIELDS AGE, ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY STRUCTURE EXTENDED script should complete: " + state.message);
    expect(fs::exists(all_fields_path), "COPY STRUCTURE EXTENDED should create the metadata DBF");
    expect(fs::exists(selected_fields_path), "COPY STRUCTURE EXTENDED FIELDS should create the metadata DBF");

    const auto all_fields = copperfin::vfp::parse_dbf_table_from_file(all_fields_path.string(), 100U);
    expect(all_fields.ok, "COPY STRUCTURE EXTENDED output should be readable");
    expect(all_fields.ok && all_fields.table.fields.size() == 16U,
           "COPY STRUCTURE EXTENDED should emit the fixed 16-column VFP metadata schema");
    expect(all_fields.ok && all_fields.table.records.size() == 3U,
           "COPY STRUCTURE EXTENDED should emit one metadata row per source field");
    if (all_fields.ok && all_fields.table.fields.size() == 16U && all_fields.table.records.size() == 3U) {
        expect(all_fields.table.fields[0].name == "FIELD_NAME" && all_fields.table.fields[0].type == 'C' &&
                   all_fields.table.fields[0].length == 128U,
               "COPY STRUCTURE EXTENDED should preserve the VFP FIELD_NAME descriptor");
        expect(all_fields.table.fields[6].name == "FIELD_DEFA" && all_fields.table.fields[6].type == 'M',
               "COPY STRUCTURE EXTENDED should preserve memo metadata columns");
        const auto value = [&](std::size_t row, const std::string& field_name) {
            const auto& values = all_fields.table.records[row].values;
            const auto found = std::find_if(values.begin(), values.end(), [&](const auto& candidate) {
                return candidate.field_name == field_name;
            });
            return found == values.end() ? std::string{} : found->display_value;
        };
        expect(value(0U, "FIELD_NAME") == "NAME" && value(0U, "FIELD_TYPE") == "C" &&
                   value(0U, "FIELD_LEN") == "12" && value(0U, "FIELD_DEC") == "0",
               "COPY STRUCTURE EXTENDED should emit character field metadata");
        expect(value(1U, "FIELD_NAME") == "AGE" && value(1U, "FIELD_TYPE") == "N" &&
                   value(1U, "FIELD_LEN") == "3" && value(1U, "FIELD_DEC") == "0",
               "COPY STRUCTURE EXTENDED should emit numeric field metadata");
        expect(value(2U, "FIELD_NAME") == "ACTIVE" && value(2U, "FIELD_TYPE") == "L" &&
                   value(2U, "FIELD_NULL") == "false" && value(2U, "FIELD_NOCP") == "false",
               "COPY STRUCTURE EXTENDED should emit logical and unsupported-attribute defaults");
    }

    const auto selected_fields = copperfin::vfp::parse_dbf_table_from_file(selected_fields_path.string(), 100U);
    expect(selected_fields.ok && selected_fields.table.records.size() == 2U,
           "COPY STRUCTURE EXTENDED FIELDS should emit only selected metadata rows");
    if (selected_fields.ok && selected_fields.table.records.size() == 2U) {
        expect(selected_fields.table.records[0U].values[0U].display_value == "AGE" &&
                   selected_fields.table.records[1U].values[0U].display_value == "ACTIVE",
               "COPY STRUCTURE EXTENDED FIELDS should preserve source field order");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_from_empty_table_produces_valid_empty_dbf() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_copy_empty_table";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Source is an empty table (zero records, but has a field definition)
    write_simple_dbf(temp_root / "empty_src.dbf", {});

    const fs::path main_path = temp_root / "copy_empty.prg";
    const std::string dest_path = (temp_root / "empty_dest.dbf").string();
    const std::string recv_path = (temp_root / "recv.dbf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "empty_src.dbf").string() + "' ALIAS EmptySrc IN 0\n"
        "COPY TO '" + dest_path + "'\n"
        "USE '" + recv_path + "' ALIAS Recv IN 0\n"
        "APPEND FROM '" + dest_path + "'\n"
        "nRecvCount = RECCOUNT()\n"
        "RETURN\n");

    // Pre-create the receiver as an empty table so USE opens it
    write_simple_dbf(temp_root / "recv.dbf", {});

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/FROM empty table script should complete");
    expect(fs::exists(dest_path), "COPY TO of empty table should create destination DBF file");

    if (fs::exists(dest_path)) {
        const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path, 100U);
        expect(result.ok, "COPY TO empty source should produce a readable DBF");
        expect(result.table.records.empty(), "COPY TO empty source should produce zero records");
    }

    const auto recv_count = state.globals.find("nrecvcount");
    expect(recv_count != state.globals.end(), "APPEND FROM empty DBF should expose RECCOUNT()");
    if (recv_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(recv_count->second) == "0",
               "APPEND FROM empty DBF should leave receiver with zero records");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_copies_records_into_current_table() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Create destination with one row; source with two rows
    write_simple_dbf(temp_root / "dest.dbf", {"Alice"});
    write_simple_dbf(temp_root / "source.dbf", {"Bob", "Carol"});

    const fs::path main_path = temp_root / "append_from.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "nAppendFromSourceCalls = 0\n"
        "APPEND FROM append_from_source('" + (temp_root / "source.dbf").string() + "')\n"
        "RETURN\n"
        "FUNCTION append_from_source\n"
        "LPARAMETERS value\n"
        "nAppendFromSourceCalls = nAppendFromSourceCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM script should complete");

    const bool has_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.append_from";
        });
    expect(has_event, "APPEND FROM should emit a runtime.append_from event");
    const auto source_calls = state.globals.find("nappendfromsourcecalls");
    expect(source_calls != state.globals.end(), "APPEND FROM should preserve the source resolver call counter");
    if (source_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(source_calls->second) == "1",
               "APPEND FROM should evaluate the source UDF exactly once");
    }

    // Verify destination now has 3 records (1 original + 2 from source)
    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM destination DBF should be readable after append");
    expect(result.table.records.size() == 3U,
        "APPEND FROM should produce 3 records total (got " + std::to_string(result.table.records.size()) + ")");
    if (result.table.records.size() >= 3U) {
        const bool has_bob = result.table.records[1U].values[0U].display_value.find("Bob") != std::string::npos;
        expect(has_bob, "APPEND FROM second record should be Bob");
        const bool has_carol = result.table.records[2U].values[0U].display_value.find("Carol") != std::string::npos;
        expect(has_carol, "APPEND FROM third record should be Carol");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_honors_open_source_cursor_filter() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_source_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const fs::path dest_path = temp_root / "dest.dbf";
    write_simple_dbf(source_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_simple_dbf(dest_path, {"DESTINATION"});

    const fs::path main_path = temp_root / "append_from_source_filter.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "GO 2 IN Source\n"
        "SET FILTER TO NAME == 'BRAVO' IN Source\n"
        "nSourceRecBefore = RECNO('Source')\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "APPEND FROM '" + source_path.string() + "'\n"
        "nDestCount = RECCOUNT()\n"
        "GO 2\n"
        "cAppendedName = NAME\n"
        "nSourceRecAfter = RECNO('Source')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM should honor an already-open source filter: " + state.message);

    const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_value("ndestcount", "2", "APPEND FROM should add only source rows visible through the active filter");
    expect_value("cappendedname", "BRAVO", "APPEND FROM should copy the source filter-visible row");
    expect_value("nsourcerecbefore", "2", "APPEND FROM source fixture should begin on the preserved source record");
    expect_value("nsourcerecafter", "2", "APPEND FROM should preserve the source cursor position while evaluating its filter");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 100U);
    expect(result.ok, "filter-aware APPEND FROM destination DBF should remain readable");
    expect(result.ok && result.table.records.size() == 2U,
           "filter-aware APPEND FROM should persist exactly one appended row");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_skips_extra_source_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_extra_source_field";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> source_fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "EXTRA", .type = 'C', .length = 8U},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(
        source_path.string(), source_fields, {{"BOB", "IGNORED"}});
    expect(source_create.ok, "APPEND FROM extra-field source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dest_fields{
        {.name = "NAME", .type = 'C', .length = 12U},
    };
    const auto dest_create = copperfin::vfp::create_dbf_table_file(
        dest_path.string(), dest_fields, {{"ALICE"}});
    expect(dest_create.ok, "APPEND FROM extra-field destination fixture should be created");

    const fs::path main_path = temp_root / "append_from_extra_source_field.prg";
    write_text(
        main_path,
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "APPEND FROM '" + source_path.string() + "'\n"
        "nCount = RECCOUNT()\n"
        "GO 2\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM should safely skip source fields absent from the destination: " + state.message);
    const auto count = state.globals.find("ncount");
    const auto name = state.globals.find("cname");
    expect(count != state.globals.end(), "APPEND FROM extra-field script should expose record count");
    expect(name != state.globals.end(), "APPEND FROM extra-field script should expose the matched field");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2",
               "APPEND FROM should append the row even when the source has an extra field");
    }
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "BOB",
               "APPEND FROM should still copy matched fields when skipping an extra source field");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_rolls_back_matched_field_write_failure() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_write_failure";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> source_fields{
        {.name = "NAME", .type = 'C', .length = 12U},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(
        source_path.string(), source_fields, {{"TWO"}, {"TOO-LONG"}});
    expect(source_create.ok, "APPEND FROM failure source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dest_fields{
        {.name = "NAME", .type = 'C', .length = 3U},
    };
    const auto dest_create = copperfin::vfp::create_dbf_table_file(
        dest_path.string(), dest_fields, {{"ONE"}});
    expect(dest_create.ok, "APPEND FROM failure destination fixture should be created");
    const auto original_size = fs::file_size(dest_path);

    const fs::path main_path = temp_root / "append_from_write_failure.prg";
    write_text(
        main_path,
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "GO 1\n"
        "APPEND FROM '" + source_path.string() + "'\n"
        "nAfterError = RECCOUNT()\n"
        "nRecAfterError = RECNO()\n"
        "cNameAfterError = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));
    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "matched APPEND FROM field-write failure should pause with an error");
    expect(state.location.line == 3U,
           "matched APPEND FROM field-write failure should highlight the command");
    const auto qps_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "qps-ploc");
    const std::string expected_writer_error = qps_catalog.translate(
        "Runtime.Prg.Dispatch.Error.AppendFromFailed",
        {{"errorMessage", copperfin::localization::pseudo_localize(
            "Character value is too large for the target field.")}});
    expect(state.message == expected_writer_error,
           "failed APPEND FROM should preserve the localized matched-field writer diagnostic (got '" +
               state.message + "')");
    expect(!session.can_undo_command(),
           "failed APPEND FROM should roll back instead of committing an undo entry");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after failed APPEND FROM should keep the session alive");
    const auto count = state.globals.find("naftererror");
    const auto recno = state.globals.find("nrecaftererror");
    const auto name = state.globals.find("cnameaftererror");
    expect(count != state.globals.end(), "failed APPEND FROM should expose restored record count");
    expect(recno != state.globals.end(), "failed APPEND FROM should expose restored record pointer");
    expect(name != state.globals.end(), "failed APPEND FROM should expose the original current row");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "1",
               "failed APPEND FROM should restore the in-memory record count");
    }
    if (recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno->second) == "1",
               "failed APPEND FROM should restore the pre-command record pointer");
    }
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "ONE",
               "failed APPEND FROM should restore the original current row");
    }

    expect(fs::file_size(dest_path) == original_size,
           "failed APPEND FROM should restore the original DBF size");
    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 5U);
    expect(parse_result.ok, "failed APPEND FROM rollback should leave the DBF readable");
    expect(parse_result.table.records.size() == 1U,
           "failed APPEND FROM should remove rows appended before the matched-field failure");
    if (parse_result.table.records.size() == 1U) {
        expect(parse_result.table.records[0].values[0].display_value == "ONE",
               "failed APPEND FROM should preserve the original destination row");
    }
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.append_from";
    }), "failed APPEND FROM should not emit a success event");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_is_reverted_by_undo() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_undo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "dest.dbf", {"Alice"});
    write_simple_dbf(temp_root / "source.dbf", {"Bob", "Carol"});

    const fs::path main_path = temp_root / "append_from_undo.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "source.dbf").string() + "'\n"
        "UNDO\n"
        "nCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM + UNDO script should complete: " + state.message);

    const bool has_undo_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.command_undo";
        });
    expect(has_undo_event, "UNDO after APPEND FROM should emit a runtime.command_undo event");

    const auto count = state.globals.find("ncount");
    expect(count != state.globals.end(), "nCount variable should exist after UNDO");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "1",
            "UNDO should revert APPEND FROM back to the pre-command record count (got '" +
                copperfin::runtime::format_value(count->second) + "')");
    }

    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM + UNDO destination DBF should be readable");
    expect(result.table.records.size() == 1U,
        "UNDO should leave only the original destination record on disk (got " +
            std::to_string(result.table.records.size()) + ")");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_transaction_rollback_restores_destination() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_transaction_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path dest_path = temp_root / "dest.dbf";
    const fs::path source_path = temp_root / "source.dbf";
    write_simple_dbf(dest_path, {"ALICE"});
    write_simple_dbf(source_path, {"BOB", "CAROL"});

    const fs::path main_path = temp_root / "append_from_transaction_rollback.prg";
    write_text(
        main_path,
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "BEGIN TRANSACTION\n"
        "APPEND FROM '" + source_path.string() + "'\n"
        "nDuring = RECCOUNT()\n"
        "ROLLBACK\n"
        "nAfter = RECCOUNT()\n"
        "GO 1\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM transaction rollback script should complete: " + state.message);
    const auto during = state.globals.find("nduring");
    const auto after = state.globals.find("nafter");
    const auto name = state.globals.find("cname");
    expect(during != state.globals.end(), "APPEND FROM transaction should expose the pre-rollback count");
    expect(after != state.globals.end(), "APPEND FROM transaction should expose the restored count");
    expect(name != state.globals.end(), "APPEND FROM transaction should expose the original row");
    if (during != state.globals.end()) {
        expect(copperfin::runtime::format_value(during->second) == "3",
               "APPEND FROM rows should be visible before transaction rollback");
    }
    if (after != state.globals.end()) {
        expect(copperfin::runtime::format_value(after->second) == "1",
               "ROLLBACK should remove native DBF APPEND FROM rows from cursor state");
    }
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "ALICE",
               "ROLLBACK should preserve the original native DBF destination row");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 5U);
    expect(parse_result.ok, "APPEND FROM transaction rollback should leave the DBF readable");
    expect(parse_result.table.records.size() == 1U,
           "APPEND FROM transaction rollback should restore the original disk record count");

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_sdf_writes_fixed_width_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_sdf";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "copy_to_sdf.prg";
    const std::string dest_path = (temp_root / "people.sdf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO '" + dest_path + "' TYPE SDF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO TYPE SDF script should complete");
    expect(fs::exists(dest_path), "COPY TO TYPE SDF should create the destination text file");
    if (fs::exists(dest_path)) {
        const std::string contents = read_text(dest_path);
        expect(contents == "Alice     \r\nBob       \r\n",
            "COPY TO TYPE SDF should write fixed-width rows using DBF field lengths");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_type_sdf_imports_fixed_width_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_sdf";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "dest.dbf", {});
    write_text(temp_root / "people.sdf", "Dora      \r\nEvan      \r\n");

    const fs::path main_path = temp_root / "append_from_sdf.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "people.sdf").string() + "' TYPE SDF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE SDF script should complete");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM TYPE SDF destination DBF should be readable");
    expect(result.table.records.size() == 2U,
        "APPEND FROM TYPE SDF should append two rows");
    if (result.table.records.size() >= 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Dora",
            "first SDF row should import into the first DBF record");
        expect(result.table.records[1U].values[0U].display_value == "Evan",
            "second SDF row should import into the second DBF record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_csv_and_delimited_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_csv";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Ann,Lee", 7}, {"Bob", 42}});

    const fs::path main_path = temp_root / "copy_to_csv.prg";
    const std::string csv_path = (temp_root / "people.csv").string();
    const std::string pipe_path = (temp_root / "people_pipe.txt").string();
    const std::string custom_path = (temp_root / "people_custom.txt").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "'\n"
        "COPY TO '" + csv_path + "' TYPE CSV FIELDS NAME, AGE\n"
        "COPY TO '" + pipe_path + "' DELIMITED WITH CHARACTER '|' FIELDS NAME, AGE FOR AGE > 10\n"
        "COPY TO '" + custom_path + "' DELIMITED WITH '_' WITH CHARACTER ';' FIELDS NAME, AGE FOR AGE > 10\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO TYPE CSV/DELIMITED script should complete");
    expect(fs::exists(csv_path), "COPY TO TYPE CSV should create destination text file");
    expect(fs::exists(pipe_path), "COPY TO DELIMITED should create destination text file");
    expect(fs::exists(custom_path), "COPY TO DELIMITED custom enclosure should create destination text file");

    if (fs::exists(csv_path)) {
        const std::string contents = read_text(csv_path);
        expect(contents == "NAME,AGE\r\n\"Ann,Lee\",7\r\n\"Bob\",42\r\n",
            "COPY TO TYPE CSV should write a field-name header and quote character fields");
    }
    if (fs::exists(pipe_path)) {
        const std::string contents = read_text(pipe_path);
        expect(contents == "\"Bob\"|42\r\n",
            "COPY TO DELIMITED WITH CHARACTER should honor the delimiter and FOR clause");
    }
    if (fs::exists(custom_path)) {
        const std::string contents = read_text(custom_path);
        expect(contents == "_Bob_;42\r\n",
            "COPY TO DELIMITED WITH enclosure plus WITH CHARACTER should honor both VFP options");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_type_csv_imports_delimited_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_csv";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "dest.dbf", {});
    write_text(temp_root / "people.csv", "NAME,AGE\r\n\"Ivy, Jr\",9\r\n\"Max\",44\r\n");
    write_text(temp_root / "people_pipe.txt", "\"Nia\"|12\r\n");
    write_text(temp_root / "people_custom.txt", "_Ora_;15\r\n");

    const fs::path main_path = temp_root / "append_from_csv.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "people.csv").string() + "' TYPE CSV FIELDS NAME, AGE\n"
        "APPEND FROM '" + (temp_root / "people_pipe.txt").string() + "' DELIMITED WITH CHARACTER '|' FIELDS NAME, AGE\n"
        "APPEND FROM '" + (temp_root / "people_custom.txt").string() + "' DELIMITED WITH '_' WITH CHARACTER ';' FIELDS NAME, AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE CSV/DELIMITED script should complete");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM TYPE CSV destination DBF should be readable");
    expect(result.table.records.size() == 4U,
        "APPEND FROM TYPE CSV/DELIMITED should append four rows");
    if (result.table.records.size() >= 4U) {
        expect(result.table.records[0U].values[0U].display_value == "Ivy, Jr",
            "first CSV row should preserve comma inside quoted character field");
        expect(result.table.records[0U].values[1U].display_value == "9",
            "first CSV numeric field should import into AGE");
        expect(result.table.records[1U].values[0U].display_value == "Max",
            "second CSV row should import NAME");
        expect(result.table.records[1U].values[1U].display_value == "44",
            "second CSV row should import AGE");
        expect(result.table.records[2U].values[0U].display_value == "Nia",
            "DELIMITED WITH CHARACTER row should import NAME");
        expect(result.table.records[2U].values[1U].display_value == "12",
            "DELIMITED WITH CHARACTER row should import AGE");
        expect(result.table.records[3U].values[0U].display_value == "Ora",
            "DELIMITED custom enclosure row should import NAME");
        expect(result.table.records[3U].values[1U].display_value == "15",
            "DELIMITED custom enclosure row should import AGE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_type_sdf_and_delimited_preserve_explicit_fields_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_engine_append_from_explicit_fields_order";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "ADDRESS", .type = 'C', .length = 12U},
        {.name = "CITY", .type = 'C', .length = 12U},
    };

    const fs::path sdf_dest_path = temp_root / "sdf_dest.dbf";
    const auto sdf_dest_create = copperfin::vfp::create_dbf_table_file(sdf_dest_path.string(), fields, {});
    expect(sdf_dest_create.ok, "#3692: SDF destination fixture should be created");
    write_text(temp_root / "people.sdf", "CityOne     NameOne     AddrOne     \r\n");

    const fs::path sdf_main_path = temp_root / "append_from_sdf_fields_order.prg";
    write_text(
        sdf_main_path,
        "USE '" + sdf_dest_path.string() + "'\n"
        "APPEND FROM '" + (temp_root / "people.sdf").string() +
            "' TYPE SDF FIELDS CITY, NAME, ADDRESS\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession sdf_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(sdf_main_path.string(), temp_root.string(), false));
    const auto sdf_state = sdf_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(sdf_state.completed,
           "#3692: APPEND FROM TYPE SDF with reordered explicit fields should complete: " + sdf_state.message);

    const auto sdf_result = copperfin::vfp::parse_dbf_table_from_file(sdf_dest_path.string(), 10U);
    expect(sdf_result.ok, "#3692: SDF reordered-fields destination DBF should remain readable");
    expect(sdf_result.table.records.size() == 1U,
           "#3692: APPEND FROM TYPE SDF reordered-fields script should append one row");
    if (sdf_result.ok && sdf_result.table.records.size() == 1U) {
        expect(sdf_result.table.records[0U].values[0U].display_value == "NameOne",
               "#3692: reordered SDF import should map explicit field column 2 into NAME");
        expect(sdf_result.table.records[0U].values[1U].display_value == "AddrOne",
               "#3692: reordered SDF import should map explicit field column 3 into ADDRESS");
        expect(sdf_result.table.records[0U].values[2U].display_value == "CityOne",
               "#3692: reordered SDF import should map explicit field column 1 into CITY");
    }

    const fs::path delimited_dest_path = temp_root / "delimited_dest.dbf";
    const auto delimited_dest_create =
        copperfin::vfp::create_dbf_table_file(delimited_dest_path.string(), fields, {});
    expect(delimited_dest_create.ok, "#3692: DELIMITED destination fixture should be created");
    write_text(temp_root / "people.txt", "\"CityTwo\",\"NameTwo\",\"AddrTwo\"\r\n");

    const fs::path delimited_main_path = temp_root / "append_from_delimited_fields_order.prg";
    write_text(
        delimited_main_path,
        "USE '" + delimited_dest_path.string() + "'\n"
        "APPEND FROM '" + (temp_root / "people.txt").string() +
            "' DELIMITED WITH CHARACTER ',' FIELDS CITY, NAME, ADDRESS\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession delimited_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(delimited_main_path.string(), temp_root.string(), false));
    const auto delimited_state =
        delimited_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(delimited_state.completed,
           "#3692: APPEND FROM DELIMITED with reordered explicit fields should complete: " +
               delimited_state.message);

    const auto delimited_result =
        copperfin::vfp::parse_dbf_table_from_file(delimited_dest_path.string(), 10U);
    expect(delimited_result.ok, "#3692: DELIMITED reordered-fields destination DBF should remain readable");
    expect(delimited_result.table.records.size() == 1U,
           "#3692: APPEND FROM DELIMITED reordered-fields script should append one row");
    if (delimited_result.ok && delimited_result.table.records.size() == 1U) {
        expect(delimited_result.table.records[0U].values[0U].display_value == "NameTwo",
               "#3692: reordered DELIMITED import should map explicit field column 2 into NAME (got '" +
                   delimited_result.table.records[0U].values[0U].display_value + "')");
        expect(delimited_result.table.records[0U].values[1U].display_value == "AddrTwo",
               "#3692: reordered DELIMITED import should map explicit field column 3 into ADDRESS (got '" +
                   delimited_result.table.records[0U].values[1U].display_value + "')");
        expect(delimited_result.table.records[0U].values[2U].display_value == "CityTwo",
               "#3692: reordered DELIMITED import should map explicit field column 1 into CITY (got '" +
                   delimited_result.table.records[0U].values[2U].display_value + "')");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_tab_and_append_from_type_tab_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_tab_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7"},
        {"Ben", "42"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE TAB source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE TAB destination fixture should be created");

    const std::string tab_path = (temp_root / "people.txt").string();
    const fs::path main_path = temp_root / "tab_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + tab_path + "' TYPE TAB FIELDS NAME, AGE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + tab_path + "' TYPE TAB FIELDS NAME, AGE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE TAB script should complete: " + state.message);
    expect(fs::exists(tab_path), "COPY TO TYPE TAB should create the text file");

    if (fs::exists(tab_path)) {
        const std::string contents = read_text(tab_path);
        expect(contents == "\"Ava\"\t7\r\n\"Ben\"\t42\r\n",
            "COPY TO TYPE TAB should emit tab-delimited rows");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE TAB round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("cname2", "Ben");
    check("nage2", "42");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE TAB destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE TAB should append both tab-delimited rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE TAB row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE TAB row 1 should preserve AGE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE TAB row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE TAB row 2 should preserve AGE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_xls_and_append_from_type_xls_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_xls_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7", "true"},
        {"Ben", "42", "false"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE XLS source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE XLS destination fixture should be created");

    const std::string xls_path = (temp_root / "people.xls").string();
    const fs::path main_path = temp_root / "xls_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + xls_path + "' TYPE XLS FIELDS NAME, AGE, ACTIVE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + xls_path + "' TYPE XLS FIELDS NAME, AGE, ACTIVE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "lActive1 = ACTIVE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "lActive2 = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE XLS script should complete: " + state.message);
    expect(fs::exists(xls_path), "COPY TO TYPE XLS should create the workbook file");

    if (fs::exists(xls_path)) {
        const std::string xml_text = read_text(xls_path);
        expect(xml_text.find("<Workbook") != std::string::npos && xml_text.find("<Worksheet") != std::string::npos,
            "COPY TO TYPE XLS should emit SpreadsheetML workbook content");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE XLS round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("lactive1", "true");
    check("cname2", "Ben");
    check("nage2", "42");
    check("lactive2", "false");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE XLS destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE XLS should append both workbook rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE XLS row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE XLS row 1 should preserve AGE");
        expect(result.table.records[0U].values[2U].display_value == "true",
            "TYPE XLS row 1 should preserve ACTIVE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE XLS row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE XLS row 2 should preserve AGE");
        expect(result.table.records[1U].values[2U].display_value == "false",
            "TYPE XLS row 2 should preserve ACTIVE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_dif_and_append_from_type_dif_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_dif_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7", "true"},
        {"Ben", "42", "false"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE DIF source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE DIF destination fixture should be created");

    const std::string dif_path = (temp_root / "people.dif").string();
    const fs::path main_path = temp_root / "dif_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + dif_path + "' TYPE DIF FIELDS NAME, AGE, ACTIVE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + dif_path + "' TYPE DIF FIELDS NAME, AGE, ACTIVE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "lActive1 = ACTIVE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "lActive2 = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE DIF script should complete: " + state.message);
    expect(fs::exists(dif_path), "COPY TO TYPE DIF should create the interchange file");

    if (fs::exists(dif_path)) {
        const std::string dif_text = read_text(dif_path);
        expect(dif_text.find("TABLE") != std::string::npos &&
               dif_text.find("DATA") != std::string::npos &&
               dif_text.find("EOD") != std::string::npos,
            "COPY TO TYPE DIF should emit DIF-style table markers");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE DIF round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("lactive1", "true");
    check("cname2", "Ben");
    check("nage2", "42");
    check("lactive2", "false");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE DIF destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE DIF should append both interchange rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE DIF row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE DIF row 1 should preserve AGE");
        expect(result.table.records[0U].values[2U].display_value == "true",
            "TYPE DIF row 1 should preserve ACTIVE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE DIF row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE DIF row 2 should preserve AGE");
        expect(result.table.records[1U].values[2U].display_value == "false",
            "TYPE DIF row 2 should preserve ACTIVE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_sylk_and_append_from_type_sylk_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sylk_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    std::vector<std::vector<std::string>> source_records{
        {"Ava", "7", "true"},
        {"Ben", "42", "false"},
    };
    for (std::size_t row = 3U; row <= 1000U; ++row) {
        source_records.push_back({
            "Row" + std::to_string(row),
            std::to_string(row % 100U),
            row % 2U == 0U ? "true" : "false",
        });
    }
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE SYLK source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE SYLK destination fixture should be created");

    const std::locale grouping_locale(std::locale::classic(), new grouped_numpunct());
    global_locale_guard locale_guard(grouping_locale);

    const std::string sylk_path = (temp_root / "people.slk").string();
    const fs::path main_path = temp_root / "sylk_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + sylk_path + "' TYPE SYLK FIELDS NAME, AGE, ACTIVE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + sylk_path + "' TYPE SYLK FIELDS NAME, AGE, ACTIVE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "lActive1 = ACTIVE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "lActive2 = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE SYLK script should complete: " + state.message);
    expect(fs::exists(sylk_path), "COPY TO TYPE SYLK should create the interchange file");

    if (fs::exists(sylk_path)) {
        const std::string sylk_text = read_text(sylk_path);
        expect(sylk_text.find("ID;P") != std::string::npos &&
               sylk_text.find("B;Y1001;X3") != std::string::npos &&
               sylk_text.find("C;Y1000;X1;K\"Row999\"") != std::string::npos &&
               sylk_text.find("C;Y1001;X1;K\"Row1000\"") != std::string::npos &&
               sylk_text.find("\nE\n") != std::string::npos,
            "#4843: COPY TO TYPE SYLK should emit invariant dimensions and boundary coordinates (prefix: '" +
                sylk_text.substr(0U, std::min<std::size_t>(sylk_text.size(), 80U)) + "')");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE SYLK round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("lactive1", "true");
    check("cname2", "Ben");
    check("nage2", "42");
    check("lactive2", "false");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 1005U);
    expect(result.ok, "APPEND FROM TYPE SYLK destination DBF should remain readable");
    expect(result.table.records.size() == 1000U,
        "#4843: APPEND FROM TYPE SYLK should preserve all rows across grouped-coordinate thresholds (got " +
            std::to_string(result.table.records.size()) + ")");
    if (result.ok && result.table.records.size() == 1000U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE SYLK row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE SYLK row 1 should preserve AGE");
        expect(result.table.records[0U].values[2U].display_value == "true",
            "TYPE SYLK row 1 should preserve ACTIVE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE SYLK row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE SYLK row 2 should preserve AGE");
        expect(result.table.records[1U].values[2U].display_value == "false",
            "TYPE SYLK row 2 should preserve ACTIVE");
        expect(result.table.records[998U].values[0U].display_value == "Row999" &&
               result.table.records[998U].values[1U].display_value == "99" &&
               result.table.records[998U].values[2U].display_value == "false",
            "#4843: TYPE SYLK row 999 should survive the first grouped-coordinate boundary");
        expect(result.table.records[999U].values[0U].display_value == "Row1000" &&
               result.table.records[999U].values[1U].display_value == "0" &&
               result.table.records[999U].values[2U].display_value == "true",
            "#4843: TYPE SYLK row 1000 should remain distinct after round trip");
    }

    const fs::path malformed_dest_path = temp_root / "malformed_dest.dbf";
    const auto malformed_dest_create =
        copperfin::vfp::create_dbf_table_file(malformed_dest_path.string(), fields, {});
    expect(malformed_dest_create.ok, "#4843: malformed-coordinate destination fixture should be created");
    const fs::path malformed_sylk_path = temp_root / "malformed.slk";
    write_text(
        malformed_sylk_path,
        "ID;PCopperfin\n"
        "B;Y2;X3\n"
        "C;Y1.000;X1;K\"Grouped coordinate\"\n"
        "E\n");
    const fs::path malformed_main_path = temp_root / "malformed_sylk.prg";
    write_text(
        malformed_main_path,
        "USE '" + malformed_dest_path.string() + "'\n"
        "APPEND FROM '" + malformed_sylk_path.string() + "' TYPE SYLK\n"
        "nRows = RECCOUNT()\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession malformed_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(malformed_main_path.string(), temp_root.string(), false));
    const auto malformed_state = malformed_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(malformed_state.completed,
        "#4843: malformed grouped SYLK coordinate should not abort APPEND FROM: " + malformed_state.message);
    const auto malformed_rows = malformed_state.globals.find("nrows");
    expect(malformed_rows != malformed_state.globals.end(),
        "#4843: malformed grouped SYLK coordinate test should capture the row count");
    if (malformed_rows != malformed_state.globals.end())
    {
        expect(copperfin::runtime::format_value(malformed_rows->second) == "0",
            "#4843: malformed grouped SYLK coordinate must not truncate into row 1");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_json_and_append_from_type_json_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_json_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7", "true"},
        {"Ben", "42", "false"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE JSON source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE JSON destination fixture should be created");

    const std::string json_path = (temp_root / "people.json").string();
    const fs::path main_path = temp_root / "json_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + json_path + "' TYPE JSON FIELDS NAME, AGE, ACTIVE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + json_path + "' TYPE JSON FIELDS NAME, AGE, ACTIVE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "lActive1 = ACTIVE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "lActive2 = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE JSON script should complete: " + state.message);
    expect(fs::exists(json_path), "COPY TO TYPE JSON should create the interchange file");

    if (fs::exists(json_path)) {
        const std::string contents = read_text(json_path);
        expect(contents.find("[") != std::string::npos &&
               contents.find("\"NAME\": \"Ava\"") != std::string::npos &&
               contents.find("\"ACTIVE\": true") != std::string::npos,
            "COPY TO TYPE JSON should emit object-array JSON with typed logical values");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE JSON round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("lactive1", "true");
    check("cname2", "Ben");
    check("nage2", "42");
    check("lactive2", "false");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE JSON destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE JSON should append both JSON rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE JSON row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE JSON row 1 should preserve AGE");
        expect(result.table.records[0U].values[2U].display_value == "true",
            "TYPE JSON row 1 should preserve ACTIVE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE JSON row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE JSON row 2 should preserve AGE");
        expect(result.table.records[1U].values[2U].display_value == "false",
            "TYPE JSON row 2 should preserve ACTIVE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_array_fills_2d_runtime_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}, {"Bob", 25}});

    const fs::path main_path = temp_root / "copy_to_array.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "'\n"
        "COPY TO ARRAY myarr\n"
        "cMacroArray = 'macroarr'\n"
        "cMacroArrayHolder = 'cMacroArray'\n"
        "cMacroArrayDeepHolder = 'cMacroArrayHolder'\n"
        "cMacroArraySecondHop = 'macroarr2'\n"
        "cMacroArraySecondHopHolder = 'cMacroArraySecondHop'\n"
        "cMacroArraySecondDeepHolder = 'cMacroArraySecondHopHolder'\n"
        "COPY TO ARRAY &cMacroArrayDeepHolder\n"
        "COPY TO ARRAY &cMacroArraySecondDeepHolder\n"
        "row1_name = myarr[1, 1]\n"
        "row1_age = myarr[1, 2]\n"
        "row2_name = myarr[2, 1]\n"
        "row2_age = myarr[2, 2]\n"
        "arr_rows = ALEN(myarr, 1)\n"
        "arr_cols = ALEN(myarr, 2)\n"
        "macro_row1_name = &cMacroArrayDeepHolder[1, 1]\n"
        "macro_rows = ALEN(&cMacroArrayDeepHolder, 1)\n"
        "macro_row1_name_second_hop = &cMacroArraySecondHop[1, 1]\n"
        "macro_rows_second_hop = ALEN(&cMacroArraySecondHop, 1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO ARRAY script should complete");

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };
    chk("arr_rows",  "2",     "COPY TO ARRAY 2 records should give 2 rows");
    chk("arr_cols",  "2",     "COPY TO ARRAY 2 fields should give 2 columns");
    chk("row1_name", "Alice", "COPY TO ARRAY row 1 col 1 should be NAME");
    chk("row1_age",  "30",    "COPY TO ARRAY row 1 col 2 should be AGE");
    chk("row2_name", "Bob",   "COPY TO ARRAY row 2 col 1 should be NAME");
    chk("row2_age",  "25",    "COPY TO ARRAY row 2 col 2 should be AGE");
    chk("macro_row1_name", "Alice", "COPY TO ARRAY macro-expanded target row 1 col 1 should be NAME");
    chk("macro_rows", "2", "COPY TO ARRAY macro-expanded target should give 2 rows");
    chk("macro_row1_name_second_hop", "Alice", "COPY TO ARRAY second-hop macro-expanded target row 1 col 1 should be NAME");
    chk("macro_rows_second_hop", "2", "COPY TO ARRAY second-hop macro-expanded target should give 2 rows");

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_array_fields_clause_allows_keyword_named_field() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_array_keyword_field";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Primary", "30"},
        {"Backup", "25"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "COPY TO ARRAY keyword-field fixture should be created");

    const fs::path main_path = temp_root / "copy_to_array_keyword_field.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "COPY TO ARRAY aTypeOnly FIELDS TYPE\n"
        "nRows = ALEN(aTypeOnly, 1)\n"
        "nCols = ALEN(aTypeOnly, 2)\n"
        "cRow1Type = aTypeOnly[1, 1]\n"
        "cRow2Type = aTypeOnly[2, 1]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO ARRAY FIELDS TYPE script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("nrows", "2", "COPY TO ARRAY FIELDS TYPE should preserve both qualifying records");
    chk("ncols", "1", "COPY TO ARRAY FIELDS TYPE should preserve only the selected keyword-named field");
    chk("crow1type", "Primary", "COPY TO ARRAY FIELDS TYPE should copy row 1 TYPE");
    chk("crow2type", "Backup", "COPY TO ARRAY FIELDS TYPE should copy row 2 TYPE");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_writes_records_from_2d_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Source table has two records; dest table starts empty.
    write_people_dbf(temp_root / "source.dbf", {{"Carol", 55}, {"Dave", 19}});
    write_people_dbf(temp_root / "dest.dbf", {});

    const fs::path main_path = temp_root / "append_from_array.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "cTempArray = 'tmparr'\n"
        "cTempArrayHolder = 'cTempArray'\n"
        "cTempArrayDeepHolder = 'cTempArrayHolder'\n"
        "COPY TO ARRAY &cTempArrayDeepHolder\n"
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM ARRAY &cTempArrayDeepHolder\n"
        "GO 1\n"
        "dest_name1 = NAME\n"
        "dest_age1 = AGE\n"
        "GO 2\n"
        "dest_name2 = NAME\n"
        "dest_age2 = AGE\n"
        "dest_rc = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM ARRAY script should complete");

    const auto chk2 = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };
    chk2("dest_rc",    "2",     "APPEND FROM ARRAY should append 2 records");
    chk2("dest_name1", "Carol", "APPEND FROM ARRAY record 1 NAME should match");
    chk2("dest_age1",  "55",    "APPEND FROM ARRAY record 1 AGE should match");
    chk2("dest_name2", "Dave",  "APPEND FROM ARRAY record 2 NAME should match");
    chk2("dest_age2",  "19",    "APPEND FROM ARRAY record 2 AGE should match");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_fields_clause_allows_keyword_named_field() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array_keyword_field";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Primary", "30"},
        {"Backup", "25"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "APPEND FROM ARRAY keyword-field source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "APPEND FROM ARRAY keyword-field destination fixture should be created");

    const fs::path main_path = temp_root / "append_from_array_keyword_field.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "COPY TO ARRAY aTypeOnly FIELDS TYPE\n"
        "USE '" + dest_path.string() + "'\n"
        "APPEND FROM ARRAY aTypeOnly FIELDS TYPE\n"
        "GO 1\n"
        "cType1 = TYPE\n"
        "nAge1 = AGE\n"
        "GO 2\n"
        "cType2 = TYPE\n"
        "nAge2 = AGE\n"
        "nRows = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM ARRAY FIELDS TYPE script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("nrows", "2", "APPEND FROM ARRAY FIELDS TYPE should append both rows");
    chk("ctype1", "Primary", "APPEND FROM ARRAY FIELDS TYPE should restore row 1 TYPE");
    chk("nage1", "0", "APPEND FROM ARRAY FIELDS TYPE should leave omitted AGE at numeric blank");
    chk("ctype2", "Backup", "APPEND FROM ARRAY FIELDS TYPE should restore row 2 TYPE");
    chk("nage2", "0", "APPEND FROM ARRAY FIELDS TYPE should leave omitted AGE at numeric blank");

    fs::remove_all(temp_root, ignored);
}

void test_copy_append_array_preserves_explicit_fields_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_append_array_field_order";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "NOTE", .type = 'C', .length = 12U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Alpha", "30", "One"},
        {"Bravo", "25", "Two"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "COPY/APPEND ARRAY reordered-fields source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "COPY/APPEND ARRAY reordered-fields destination fixture should be created");

    const fs::path exported_path = temp_root / "exported.dbf";
    const fs::path main_path = temp_root / "copy_append_array_field_order.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "COPY TO ARRAY aSelected FIELDS NOTE, NAME\n"
        "nArrayRows = ALEN(aSelected, 1)\n"
        "nArrayCols = ALEN(aSelected, 2)\n"
        "cArray11 = aSelected[1, 1]\n"
        "cArray12 = aSelected[1, 2]\n"
        "COPY TO '" + exported_path.string() + "' FIELDS NOTE, NAME\n"
        "USE '" + dest_path.string() + "'\n"
        "APPEND FROM ARRAY aSelected FIELDS NOTE, NAME\n"
        "GO 1\n"
        "cDestName1 = NAME\n"
        "nDestAge1 = AGE\n"
        "cDestNote1 = NOTE\n"
        "GO 2\n"
        "cDestName2 = NAME\n"
        "nDestAge2 = AGE\n"
        "cDestNote2 = NOTE\n"
        "nDestRows = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3845: reordered-field COPY/APPEND ARRAY script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("narrayrows", "2", "#3845: COPY TO ARRAY should preserve both rows with reordered fields");
    chk("narraycols", "2", "#3845: COPY TO ARRAY should preserve only the reordered selected fields");
    chk("carray11", "One", "#3845: COPY TO ARRAY should put NOTE in column 1 when FIELDS NOTE, NAME is requested");
    chk("carray12", "Alpha", "#3845: COPY TO ARRAY should put NAME in column 2 when FIELDS NOTE, NAME is requested");
    chk("ndestrows", "2", "#3845: APPEND FROM ARRAY should append both reordered rows");
    chk("cdestname1", "Alpha", "#3845: APPEND FROM ARRAY should restore NAME from reordered column 2");
    chk("ndestage1", "0", "#3845: APPEND FROM ARRAY should leave omitted AGE blank for row 1");
    chk("cdestnote1", "One", "#3845: APPEND FROM ARRAY should restore NOTE from reordered column 1");
    chk("cdestname2", "Bravo", "#3845: APPEND FROM ARRAY should restore NAME from reordered column 2 for row 2");
    chk("ndestage2", "0", "#3845: APPEND FROM ARRAY should leave omitted AGE blank for row 2");
    chk("cdestnote2", "Two", "#3845: APPEND FROM ARRAY should restore NOTE from reordered column 1 for row 2");

    const auto exported = copperfin::vfp::parse_dbf_table_from_file(exported_path.string(), 10U);
    expect(exported.ok, "#3845: reordered-field COPY TO DBF export should remain readable");
    expect(exported.ok && exported.table.fields.size() == 2U,
        "#3845: reordered-field COPY TO DBF export should contain exactly the requested two fields");
    if (exported.ok && exported.table.fields.size() >= 2U) {
        expect(exported.table.fields[0].name == "NOTE",
            "#3845: COPY TO DBF should preserve explicit FIELDS order for field 1");
        expect(exported.table.fields[1].name == "NAME",
            "#3845: COPY TO DBF should preserve explicit FIELDS order for field 2");
    }
    if (exported.ok && exported.table.records.size() >= 1U && exported.table.records[0].values.size() >= 2U) {
        expect(exported.table.records[0].values[0].display_value == "One",
            "#3845: exported DBF row 1 column 1 should carry NOTE when NOTE is listed first");
        expect(exported.table.records[0].values[1].display_value == "Alpha",
            "#3845: exported DBF row 1 column 2 should carry NAME when NAME is listed second");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_append_array_like_and_except_field_filters() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_append_array_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Alpha", "One", "30"},
        {"Bravo", "Two", "25"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "COPY/APPEND ARRAY LIKE/EXCEPT source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "COPY/APPEND ARRAY LIKE/EXCEPT destination fixture should be created");

    const fs::path main_path = temp_root / "copy_append_array_like_except.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "COPY TO ARRAY aSelected FIELDS LIKE N*\n"
        "USE '" + dest_path.string() + "'\n"
        "APPEND FROM ARRAY aSelected FIELDS EXCEPT AGE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "cNote1 = NOTE\n"
        "nAge1 = AGE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "cNote2 = NOTE\n"
        "nAge2 = AGE\n"
        "nRows = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO ARRAY FIELDS LIKE / APPEND FROM ARRAY FIELDS EXCEPT script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("nrows", "2", "APPEND FROM ARRAY FIELDS EXCEPT AGE should append both rows");
    chk("cname1", "Alpha", "COPY TO ARRAY FIELDS LIKE N* should preserve keyword-heavy NAME for row 1");
    chk("cnote1", "One", "COPY TO ARRAY FIELDS LIKE N* should preserve NOTE for row 1");
    chk("nage1", "0", "APPEND FROM ARRAY FIELDS EXCEPT AGE should leave AGE blank for row 1");
    chk("cname2", "Bravo", "COPY TO ARRAY FIELDS LIKE N* should preserve NAME for row 2");
    chk("cnote2", "Two", "COPY TO ARRAY FIELDS LIKE N* should preserve NOTE for row 2");
    chk("nage2", "0", "APPEND FROM ARRAY FIELDS EXCEPT AGE should leave AGE blank for row 2");

    fs::remove_all(temp_root, ignored);
}

void test_copy_append_dbf_like_and_except_field_filters() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_append_dbf_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{{"Alpha", "One", "30"}, {"Bravo", "Two", "25"}};
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "COPY/APPEND DBF LIKE/EXCEPT source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "COPY/APPEND DBF LIKE/EXCEPT destination fixture should be created");

    const fs::path selected_path = temp_root / "selected.dbf";
    const fs::path main_path = temp_root / "copy_append_dbf_like_except.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "COPY TO '" + selected_path.string() + "' FIELDS LIKE N*\n"
        "USE '" + dest_path.string() + "'\n"
        "APPEND FROM '" + selected_path.string() + "' FIELDS EXCEPT AGE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "cNote1 = NOTE\n"
        "nAge1 = AGE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "cNote2 = NOTE\n"
        "nAge2 = AGE\n"
        "nRows = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM DBF FIELDS LIKE/EXCEPT script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("nrows", "2", "APPEND FROM FIELDS EXCEPT AGE should append both rows");
    chk("cname1", "Alpha", "COPY TO FIELDS LIKE N* should preserve keyword-heavy NAME for row 1");
    chk("cnote1", "One", "COPY TO FIELDS LIKE N* should preserve NOTE for row 1");
    chk("nage1", "0", "APPEND FROM FIELDS EXCEPT AGE should leave AGE blank for row 1");
    chk("cname2", "Bravo", "COPY TO FIELDS LIKE N* should preserve NAME for row 2");
    chk("cnote2", "Two", "COPY TO FIELDS LIKE N* should preserve NOTE for row 2");
    chk("nage2", "0", "APPEND FROM FIELDS EXCEPT AGE should leave AGE blank for row 2");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_macro_source_preserves_date_and_datetime_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array_date_time";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "BIRTHDAY", .type = 'D', .length = 8U},
        {.name = "STAMP", .type = 'T', .length = 8U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"20240117", "julian:2459625 millis:37230000", "41"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "APPEND FROM ARRAY date/datetime source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "APPEND FROM ARRAY date/datetime destination fixture should be created");

    const fs::path main_path = temp_root / "append_from_array_date_time.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "cArrayName = 'aTemporal'\n"
        "cArrayNameHolder = 'cArrayName'\n"
        "cArrayNameDeepHolder = 'cArrayNameHolder'\n"
        "COPY TO ARRAY &cArrayNameDeepHolder FIELDS BIRTHDAY, STAMP\n"
        "USE '" + dest_path.string() + "'\n"
        "APPEND FROM ARRAY &cArrayNameDeepHolder FIELDS BIRTHDAY, STAMP\n"
        "GO 1\n"
        "cBirthday = DTOC(BIRTHDAY, 1)\n"
        "cStamp = TTOC(STAMP, 1)\n"
        "nAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM ARRAY macro date/datetime script should complete: " + state.message);

    const auto birthday = state.globals.find("cbirthday");
    const auto stamp = state.globals.find("cstamp");
    const auto age = state.globals.find("nage");
    expect(birthday != state.globals.end(), "APPEND FROM ARRAY date/datetime script should capture BIRTHDAY");
    expect(stamp != state.globals.end(), "APPEND FROM ARRAY date/datetime script should capture STAMP");
    expect(age != state.globals.end(), "APPEND FROM ARRAY date/datetime script should capture AGE");
    if (birthday != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(birthday->second);
        expect(actual == "20240117",
            "APPEND FROM ARRAY should serialize runtime date strings back into date fields (got '" + actual + "')");
    }
    if (stamp != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(stamp->second);
        expect(actual == "20240117102030",
            "APPEND FROM ARRAY should serialize runtime datetime strings back into datetime fields (got '" + actual + "')");
    }
    if (age != state.globals.end()) {
        expect(copperfin::runtime::format_value(age->second) == "0",
            "APPEND FROM ARRAY FIELDS BIRTHDAY, STAMP should leave omitted numeric fields blank");
    }

    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(persisted.ok, "APPEND FROM ARRAY date/datetime destination table should remain readable");
    expect(persisted.table.records.size() == 1U, "APPEND FROM ARRAY date/datetime destination should have one row");
    if (persisted.ok && persisted.table.records.size() == 1U) {
        expect(persisted.table.records[0].values[0].display_value == "2024-01-17",
            "APPEND FROM ARRAY should persist date storage strings through the DBF writer (got '" +
                persisted.table.records[0].values[0].display_value + "')");
        expect(persisted.table.records[0].values[1].display_value == "julian:2459625 millis:37230000",
            "APPEND FROM ARRAY should persist datetime storage strings through the DBF writer");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_data_io
