// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace copperfin::test_support;
namespace fs = std::filesystem;

void write_named_age_dbf(const fs::path &path, const std::vector<std::pair<std::string, int>> &records)
{
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "AGE", .type = 'N', .length = 3U}};
    std::vector<std::vector<std::string>> values;
    for (const auto &[name, age] : records)
    {
        values.push_back({name, std::to_string(age)});
    }
    expect(copperfin::vfp::create_dbf_table_file(path.string(), fields, values).ok,
           "ordered iteration DBF fixture should be created");
}

void write_report_surface(const fs::path &path)
{
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}};
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "200", "detail-header-guid"},
        {"8", "", "NAME", "100", "20", "700", "100", "name-field-guid"}};
    expect(copperfin::vfp::create_dbf_table_file(path.string(), fields, records).ok,
           "ordered iteration report fixture should be created");
}

void test_active_order_drives_locate_scan_report_and_copy()
{
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ordered_iteration";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_named_age_dbf(table_path, {{"ZEBRA", 40}, {"MANGO", 30}, {"APPLE", 20}, {"BERRY", 25}});
    write_synthetic_idx(temp_root / "people.idx", "NAME");
    const fs::path report_path = temp_root / "people.frx";
    write_report_surface(report_path);
    const fs::path report_output = temp_root / "ordered_report.txt";
    const fs::path copy_output = temp_root / "ordered_copy.dbf";
    const fs::path main_path = temp_root / "ordered_iteration.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People\n"
        "SET ORDER TO 1\n"
        "LOCATE FOR AGE >= 20\n"
        "cLocate = NAME\n"
        "CONTINUE\n"
        "cContinue = NAME\n"
        "GO TOP\n"
        "cScan = ''\n"
        "SCAN FOR AGE >= 20\n"
        "cScan = cScan + NAME + ','\n"
        "ENDSCAN\n"
        "REPORT FORM '" + report_path.string() + "' TO FILE '" + report_output.string() + "'\n"
        "COPY TO ARRAY aRows\n"
        "nArrayRows = ALEN(aRows, 1)\n"
        "cArrayFirst = aRows[1, 1]\n"
        "COPY TO '" + copy_output.string() + "'\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "active-order iteration script should complete: " + state.message);

    const auto value = [&](const std::string &name) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured");
        return it == state.globals.end() ? std::string{} : copperfin::runtime::format_value(it->second);
    };
    expect(value("clocate") == "APPLE", "LOCATE should begin at the first active-order record");
    expect(value("ccontinue") == "BERRY", "CONTINUE should advance to the next active-order record");
    expect(value("cscan") == "APPLE,BERRY,MANGO,ZEBRA,", "SCAN should iterate in active-order sequence");
    expect(value("narrayrows") == "4", "COPY TO ARRAY should include all active-order rows");
    expect(value("carrayfirst") == "APPLE", "COPY TO ARRAY should preserve active-order row order");

    const std::string report_text = read_text(report_output);
    expect(report_text.find("row[3]=NAME=APPLE|AGE=20") != std::string::npos,
           "REPORT FORM should render the first active-order row");
    expect(report_text.find("row[4]=NAME=BERRY|AGE=25") != std::string::npos,
           "REPORT FORM should render the second active-order row");
    expect(report_text.find("row[2]=NAME=MANGO|AGE=30") != std::string::npos,
           "REPORT FORM should render the third active-order row");

    const auto copied = copperfin::vfp::parse_dbf_table_from_file(copy_output.string(), 10U);
    expect(copied.ok, "COPY TO output should remain readable");
    expect(copied.ok && copied.table.records.size() == 4U, "COPY TO should include all ordered rows");
    if (copied.ok && copied.table.records.size() == 4U)
    {
        expect(copied.table.records[0].values[0].display_value == "APPLE", "COPY TO row 1 should be ordered");
        expect(copied.table.records[1].values[0].display_value == "BERRY", "COPY TO row 2 should be ordered");
        expect(copied.table.records[2].values[0].display_value == "MANGO", "COPY TO row 3 should be ordered");
        expect(copied.table.records[3].values[0].display_value == "ZEBRA", "COPY TO row 4 should be ordered");
    }

    fs::remove_all(temp_root, ignored);
}

// Builds a synthetic two-tag CDX matching what collect_directory_leaf_tags()
// (src/vfp/cdx_header.cpp) expects: a single leaf+directory node listing
// both tags' 10-byte name slots in its tail region and a 4-byte page-hint
// offset per entry, each pointing to that tag's own dedicated header page
// where its expression text and persisted descending byte (offset 502)
// live. Mirrors write_synthetic_cdx()'s single-tag layout, extended to two
// entries -- not a general-purpose helper, so kept local to this test.
void write_synthetic_two_tag_cdx(
    const std::filesystem::path &path,
    const std::string &first_tag_name,
    const std::string &first_expression,
    bool first_descending,
    const std::string &second_tag_name,
    const std::string &second_expression,
    bool second_descending)
{
    std::vector<std::uint8_t> bytes(4096U, 0U);
    write_le_u16(bytes, 0U, 1024U);
    write_le_u16(bytes, 12U, 10U);
    write_le_u16(bytes, 14U, 480U);
    write_le_u16(bytes, 1024U, 0x0003U);
    write_le_u16(bytes, 1026U, 2U);
    write_le_u32(bytes, 1028U, 2048U);
    write_le_u32(bytes, 1032U, 2560U);

    if (first_expression.size() > 512U || second_expression.size() > 512U)
    {
        throw std::length_error("write_synthetic_two_tag_cdx: expression too long for synthetic fixture buffer");
    }
    for (std::size_t index = 0; index < first_expression.size(); ++index)
    {
        bytes[2048U + index] = static_cast<std::uint8_t>(first_expression[index]);
    }
    if (first_descending)
    {
        bytes[2048U + 502U] = 0x01U;
    }
    for (std::size_t index = 0; index < second_expression.size(); ++index)
    {
        bytes[2560U + index] = static_cast<std::uint8_t>(second_expression[index]);
    }
    if (second_descending)
    {
        bytes[2560U + 502U] = 0x01U;
    }

    const std::size_t tail_start = 1024U + 512U - (2U * 10U);
    if (first_tag_name.size() > 10U || second_tag_name.size() > 10U)
    {
        throw std::length_error("write_synthetic_two_tag_cdx: tag name too long for its 10-byte slot");
    }
    for (std::size_t index = 0; index < first_tag_name.size(); ++index)
    {
        bytes[tail_start + index] = static_cast<std::uint8_t>(first_tag_name[index]);
    }
    for (std::size_t index = 0; index < second_tag_name.size(); ++index)
    {
        bytes[tail_start + 10U + index] = static_cast<std::uint8_t>(second_tag_name[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

// #5358 two-argument form: DESCENDING(cCDXFileName, nTagNumber) reports the
// persisted direction of the nTagNumber-th tag *within that CDX file*,
// without changing the active order, and returns .F. (not an error) for an
// out-of-range tag number. Expected values transcribed from the same
// controlled real-VFP9 session (see
// docs/32-recovered-requirements-traceability.md, RQ-CF-PRG-019).
void test_descending_two_argument_form_reports_specific_tag_direction()
{
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_descending_two_arg";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "twotag.dbf";
    write_named_age_dbf(table_path, {{"ALPHA", 1}});
    const fs::path cdx_path = temp_root / "twotag.cdx";
    write_synthetic_two_tag_cdx(cdx_path, "ASCTAG", "NAME", false, "DESCTAG", "NAME", true);

    const fs::path main_path = temp_root / "descending_two_arg.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS TwoTag IN 0\n"
        "SET ORDER TO 1 IN TwoTag\n"
        "lTag1 = DESCENDING('" + cdx_path.string() + "', 1, 'TwoTag')\n"
        "lTag2 = DESCENDING('" + cdx_path.string() + "', 2, 'TwoTag')\n"
        "lTag3OutOfRange = DESCENDING('" + cdx_path.string() + "', 3, 'TwoTag')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DESCENDING(file, n) script should complete: " + state.message);

    const auto value = [&](const std::string &name) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured");
        return it == state.globals.end() ? std::string{} : copperfin::runtime::format_value(it->second);
    };
    expect(value("ltag1") == "false", "DESCENDING(file, 1) should report the CDX's own first tag (ascending)");
    expect(value("ltag2") == "true", "DESCENDING(file, 2) should report the CDX's own second tag (descending)");
    expect(value("ltag3outofrange") == "false", "DESCENDING(file, n) out of range should be .F., not an error");

    fs::remove_all(temp_root, ignored);
}

// #5358: DESCENDING() must report a tag's persisted creation direction,
// and a runtime SET ORDER TO ... ASCENDING|DESCENDING override must win
// over that persisted direction. Expected values below are transcribed
// from a controlled session against a real, fully patched VFP9 install
// (09.00.0000.7423): no active order -> .F.; a persisted ascending tag
// -> .F.; a persisted descending tag -> .T.; either tag with an explicit
// runtime override -> the override's own direction, regardless of the
// persisted bit. See docs/32-recovered-requirements-traceability.md for
// the recorded evidence (exact commands and observed output).
void test_descending_reports_persisted_direction_and_runtime_override()
{
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_descending_function";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path asc_table_path = temp_root / "ascorder.dbf";
    write_named_age_dbf(asc_table_path, {{"ALPHA", 1}});
    write_synthetic_cdx(temp_root / "ascorder.cdx", "ASCTAG", "NAME", /*descending=*/false);

    const fs::path desc_table_path = temp_root / "descorder.dbf";
    write_named_age_dbf(desc_table_path, {{"ALPHA", 1}});
    write_synthetic_cdx(temp_root / "descorder.cdx", "DESCTAG", "NAME", /*descending=*/true);

    const fs::path main_path = temp_root / "descending_function.prg";
    write_text(
        main_path,
        "USE '" + asc_table_path.string() + "' ALIAS AscOrder IN 0\n"
        "lNoOrder = DESCENDING('AscOrder')\n"
        "SET ORDER TO 1 IN AscOrder\n"
        "lAscTag = DESCENDING('AscOrder')\n"
        "SET ORDER TO 1 DESCENDING IN AscOrder\n"
        "lAscTagOverrideDesc = DESCENDING('AscOrder')\n"
        "USE '" + desc_table_path.string() + "' ALIAS DescOrder IN 0\n"
        "SET ORDER TO 1 IN DescOrder\n"
        "lDescTag = DESCENDING('DescOrder')\n"
        "SET ORDER TO 1 ASCENDING IN DescOrder\n"
        "lDescTagOverrideAsc = DESCENDING('DescOrder')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DESCENDING() script should complete: " + state.message);

    const auto value = [&](const std::string &name) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured");
        return it == state.globals.end() ? std::string{} : copperfin::runtime::format_value(it->second);
    };
    expect(value("lnoorder") == "false", "DESCENDING() with no active order should be .F., matching real VFP9");
    expect(value("lasctag") == "false", "DESCENDING() for a persisted ascending tag should be .F.");
    expect(value("lasctagoverridedesc") == "true",
           "a runtime DESCENDING override should win over the tag's persisted ascending direction");
    expect(value("ldesctag") == "true", "DESCENDING() for a persisted descending tag should be .T.");
    expect(value("ldesctagoverrideasc") == "false",
           "a runtime ASCENDING override should win over the tag's persisted descending direction");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main()
{
    test_active_order_drives_locate_scan_report_and_copy();
    test_descending_reports_persisted_direction_and_runtime_override();
    test_descending_two_argument_form_reports_specific_tag_direction();
    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return 1;
    }
    return 0;
}
