// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
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

}  // namespace

int main()
{
    test_active_order_drives_locate_scan_report_and_copy();
    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return 1;
    }
    return 0;
}
