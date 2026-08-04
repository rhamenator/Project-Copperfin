// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_value_list_requery_refreshes_rows_and_preserves_selection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_value_list";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "requery_value_list.prg";
    write_text(
        main_path,
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = 1\n"
        "oList.RowSource = 'Alpha,One,Beta,Two'\n"
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "oList.RowSource = 'Gamma,Three,Delta,Four,Epsilon,Five'\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond32 = oList.List(3, 2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "value-list Requery() script should complete");

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst11", "Alpha");
    check("cfirst12", "One");
    check("cfirst21", "Beta");
    check("cfirst22", "Two");
    check("cdisplaybeforesecond", "Beta");
    check("naftersecond", "3");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "Delta");
    check("csecond11", "Gamma");
    check("csecond12", "Three");
    check("csecond21", "Delta");
    check("csecond22", "Four");
    check("csecond31", "Epsilon");
    check("csecond32", "Five");

    const bool has_value_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_value_requery_event,
           "value-list Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_array_requery_refreshes_rows_and_growth() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "requery_array.prg";
    write_text(
        main_path,
        "DIMENSION gaMonths[2,2]\n"
        "gaMonths[1,1] = 'Jan'\n"
        "gaMonths[1,2] = '01'\n"
        "gaMonths[2,1] = 'Feb'\n"
        "gaMonths[2,2] = '02'\n"
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.RowSourceType = 5\n"
        "oCombo.RowSource = 'gaMonths'\n"
        "nBefore = oCombo.ListCount\n"
        "oCombo.Requery()\n"
        "nAfterFirst = oCombo.ListCount\n"
        "cFirst11 = oCombo.List(1, 1)\n"
        "cFirst12 = oCombo.List(1, 2)\n"
        "cFirst21 = oCombo.List(2, 1)\n"
        "cFirst22 = oCombo.List(2, 2)\n"
        "oCombo.ListIndex = 1\n"
        "cDisplayBeforeSecond = oCombo.DisplayValue\n"
        "DIMENSION gaMonths[3,2]\n"
        "gaMonths[1,1] = 'Mar'\n"
        "gaMonths[1,2] = '03'\n"
        "gaMonths[2,1] = 'Apr'\n"
        "gaMonths[2,2] = '04'\n"
        "gaMonths[3,1] = 'May'\n"
        "gaMonths[3,2] = '05'\n"
        "oCombo.Requery()\n"
        "nAfterSecond = oCombo.ListCount\n"
        "nListIndexAfterSecond = oCombo.ListIndex\n"
        "cDisplayAfterSecond = oCombo.DisplayValue\n"
        "cSecond11 = oCombo.List(1, 1)\n"
        "cSecond12 = oCombo.List(1, 2)\n"
        "cSecond31 = oCombo.List(3, 1)\n"
        "cSecond32 = oCombo.List(3, 2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "array-backed Requery() script should complete");

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst11", "Jan");
    check("cfirst12", "01");
    check("cfirst21", "Feb");
    check("cfirst22", "02");
    check("cdisplaybeforesecond", "Jan");
    check("naftersecond", "3");
    check("nlistindexaftersecond", "1");
    check("cdisplayaftersecond", "Mar");
    check("csecond11", "Mar");
    check("csecond12", "03");
    check("csecond31", "May");
    check("csecond32", "05");

    const bool has_array_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_array_requery_event,
           "array-backed Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_alias_requery_refreshes_open_cursor_rows_and_preserves_cursor_position() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "CITY", .type = 'C', .length = 20U}};
    const std::vector<std::vector<std::string>> records{
        {"Ada", "London"},
        {"Babbage", "Paris"}};
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "alias Requery() fixture should create a DBF table");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "requery_alias.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS customers IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = 2\n"
        "oList.RowSource = 'customers'\n"
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "GO 1 IN customers\n"
        "REPLACE CITY WITH 'Oxford' IN customers\n"
        "APPEND BLANK IN customers\n"
        "REPLACE NAME WITH 'Grace' IN customers\n"
        "REPLACE CITY WITH 'Arlington' IN customers\n"
        "nRecnoBeforeSecond = RECNO('customers')\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond32 = oList.List(3, 2)\n"
        "nRecnoAfterSecond = RECNO('customers')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("alias-backed Requery() script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst11", "Ada");
    check("cfirst12", "London");
    check("cfirst21", "Babbage");
    check("cfirst22", "Paris");
    check("cdisplaybeforesecond", "Babbage");
    check("nrecnobeforesecond", "3");
    check("naftersecond", "3");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "Babbage");
    check("csecond12", "Oxford");
    check("csecond31", "Grace");
    check("csecond32", "Arlington");
    check("nrecnoaftersecond", "3");

    const bool has_alias_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_alias_requery_event,
           "alias-backed Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_fields_requery_refreshes_alias_qualified_field_list() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_fields";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "products.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "PRODUCT", .type = 'C', .length = 24U},
        {.name = "SKU", .type = 'C', .length = 12U}};
    const std::vector<std::vector<std::string>> records{
        {"Desk Lamp", "A100"},
        {"Keyboard", "B200"}};
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "fields Requery() fixture should create a DBF table");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path other_path = temp_root / "shadow.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> other_fields{
        {.name = "FLAG", .type = 'C', .length = 8U}};
    const std::vector<std::vector<std::string>> other_records{{"shadow"}};
    const auto other_create_result =
        copperfin::vfp::create_dbf_table_file(other_path.string(), other_fields, other_records);
    expect(other_create_result.ok, "fields Requery() fixture should create a shadow DBF table");
    if (!other_create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "requery_fields.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS inventory IN 0\n"
        "USE '" + other_path.string() + "' ALIAS shadow IN 0\n"
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.ColumnCount = 2\n"
        "oCombo.RowSourceType = 6\n"
        "oCombo.RowSource = 'inventory.sku, product'\n"
        "nBefore = oCombo.ListCount\n"
        "SELECT shadow\n"
        "oCombo.Requery()\n"
        "nAfterFirst = oCombo.ListCount\n"
        "cFirst11 = oCombo.List(1, 1)\n"
        "cFirst12 = oCombo.List(1, 2)\n"
        "cFirst21 = oCombo.List(2, 1)\n"
        "cFirst22 = oCombo.List(2, 2)\n"
        "oCombo.ListIndex = 1\n"
        "cDisplayBeforeSecond = oCombo.DisplayValue\n"
        "GO 2 IN inventory\n"
        "REPLACE PRODUCT WITH 'Keyboard Pro' IN inventory\n"
        "APPEND BLANK IN inventory\n"
        "REPLACE PRODUCT WITH 'Monitor Arm' IN inventory\n"
        "REPLACE SKU WITH 'C300' IN inventory\n"
        "nRecnoBeforeSecond = RECNO('inventory')\n"
        "SELECT shadow\n"
        "oCombo.Requery()\n"
        "nAfterSecond = oCombo.ListCount\n"
        "nListIndexAfterSecond = oCombo.ListIndex\n"
        "cDisplayAfterSecond = oCombo.DisplayValue\n"
        "cSecond22 = oCombo.List(2, 2)\n"
        "cSecond31 = oCombo.List(3, 1)\n"
        "cSecond32 = oCombo.List(3, 2)\n"
        "nRecnoAfterSecond = RECNO('inventory')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("fields-backed Requery() script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst11", "A100");
    check("cfirst12", "Desk Lamp");
    check("cfirst21", "B200");
    check("cfirst22", "Keyboard");
    check("cdisplaybeforesecond", "A100");
    check("nrecnobeforesecond", "3");
    check("naftersecond", "3");
    check("nlistindexaftersecond", "1");
    check("cdisplayaftersecond", "A100");
    check("csecond22", "Keyboard Pro");
    check("csecond31", "C300");
    check("csecond32", "Monitor Arm");
    check("nrecnoaftersecond", "3");

    const bool has_fields_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_fields_requery_event,
           "fields-backed Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_refreshes_single_cursor_query_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_statement";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION", .type = 'C', .length = 12U}};
    const std::vector<std::vector<std::string>> records{
        {"Zulu", "Anne", ""},
        {"Baker", "Ben", "NE"},
        {"Alpha", "Ada", ""}};
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "sql-statement Requery() fixture should create a DBF table");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "requery_sql_statement.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS employee IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT last_name, first_name FROM employee WHERE EMPTY(region) ORDER BY last_name INTO CURSOR temp2\"\n"
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "GO 2 IN employee\n"
        "REPLACE REGION WITH '' IN employee\n"
        "REPLACE LAST_NAME WITH 'Marlow' IN employee\n"
        "APPEND BLANK IN employee\n"
        "REPLACE LAST_NAME WITH 'Delta' IN employee\n"
        "REPLACE FIRST_NAME WITH 'Dia' IN employee\n"
        "REPLACE REGION WITH '' IN employee\n"
        "nRecnoBeforeSecond = RECNO('employee')\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond41 = oList.List(4, 1)\n"
        "nRecnoAfterSecond = RECNO('employee')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("sql-statement Requery() script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst11", "Alpha");
    check("cfirst12", "Ada");
    check("cfirst21", "Zulu");
    check("cfirst22", "Anne");
    check("cdisplaybeforesecond", "Zulu");
    check("nrecnobeforesecond", "4");
    check("naftersecond", "4");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "Delta");
    check("csecond11", "Alpha");
    check("csecond12", "Ada");
    check("csecond21", "Delta");
    check("csecond22", "Dia");
    check("csecond31", "Marlow");
    check("csecond41", "Zulu");
    check("nrecnoaftersecond", "4");

    const bool has_sql_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_sql_requery_event,
           "sql-statement Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_query_file_requery_refreshes_single_cursor_query_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_query_file";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION", .type = 'C', .length = 12U}};
    const std::vector<std::vector<std::string>> records{
        {"Zulu", "Anne", ""},
        {"Baker", "Ben", "NE"},
        {"Alpha", "Ada", ""}};
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "query-file Requery() fixture should create a DBF table");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path query_path = temp_root / "region.qpr";
    write_text(
        query_path,
        "SELECT last_name, first_name ;\n"
        " FROM employee ;\n"
        " WHERE EMPTY(region) ;\n"
        " ORDER BY last_name INTO CURSOR temp2\n");

    const fs::path main_path = temp_root / "requery_query_file.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS employee IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = 4\n"
        "oList.RowSource = 'region.qpr'\n"
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "GO 2 IN employee\n"
        "REPLACE REGION WITH '' IN employee\n"
        "REPLACE LAST_NAME WITH 'Marlow' IN employee\n"
        "APPEND BLANK IN employee\n"
        "REPLACE LAST_NAME WITH 'Delta' IN employee\n"
        "REPLACE FIRST_NAME WITH 'Dia' IN employee\n"
        "REPLACE REGION WITH '' IN employee\n"
        "nRecnoBeforeSecond = RECNO('employee')\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond41 = oList.List(4, 1)\n"
        "nRecnoAfterSecond = RECNO('employee')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("query-file Requery() script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst11", "Alpha");
    check("cfirst12", "Ada");
    check("cfirst21", "Zulu");
    check("cfirst22", "Anne");
    check("cdisplaybeforesecond", "Zulu");
    check("nrecnobeforesecond", "4");
    check("naftersecond", "4");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "Delta");
    check("csecond11", "Alpha");
    check("csecond12", "Ada");
    check("csecond21", "Delta");
    check("csecond22", "Dia");
    check("csecond31", "Marlow");
    check("csecond41", "Zulu");
    check("nrecnoaftersecond", "4");

    const bool has_query_file_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_query_file_requery_event,
           "query-file Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void run_distinct_order_ordinal_requery_test(int row_source_type) {
    namespace fs = std::filesystem;
    const std::string row_source_label =
        row_source_type == 3 ? "sql-statement" : "query-file";
    const fs::path temp_root =
        fs::temp_directory_path() / ("copperfin_native_requery_distinct_order_ordinal_" +
                                     std::string(row_source_type == 3 ? "sql" : "query"));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION", .type = 'C', .length = 12U}};
    const std::vector<std::vector<std::string>> records{
        {"Zulu", "Anne", "West"},
        {"Baker", "Ben", "East"},
        {"Alpha", "Ada", "East"},
        {"Marlow", "Mia", ""}};
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok,
           "distinct ORDER BY ordinal " + row_source_label +
               " Requery() fixture should create a DBF table");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    std::string row_source_assignment;
    if (row_source_type == 4) {
        const fs::path query_path = temp_root / "distinct_regions.qpr";
        write_text(
            query_path,
            "SELECT DISTINCT region ;\n"
            " FROM employee ;\n"
            " WHERE NOT EMPTY(region) ;\n"
            " ORDER BY 1 INTO CURSOR temp2\n");
        row_source_assignment = "oList.RowSource = 'distinct_regions.qpr'\n";
    } else {
        row_source_assignment =
            "oList.RowSource = \"SELECT DISTINCT region FROM employee "
            "WHERE NOT EMPTY(region) ORDER BY 1 INTO CURSOR temp2\"\n";
    }

    const fs::path main_path =
        temp_root / ("requery_distinct_order_ordinal_" +
                      std::string(row_source_type == 3 ? "sql" : "query") + ".prg");
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS employee IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.RowSourceType = " + std::to_string(row_source_type) + "\n" +
            row_source_assignment +
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst1 = oList.List(1)\n"
        "cFirst2 = oList.List(2)\n"
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "GO 3 IN employee\n"
        "REPLACE REGION WITH 'Central' IN employee\n"
        "APPEND BLANK IN employee\n"
        "REPLACE LAST_NAME WITH 'North' IN employee\n"
        "REPLACE FIRST_NAME WITH 'Nia' IN employee\n"
        "REPLACE REGION WITH 'East' IN employee\n"
        "nRecnoBeforeSecond = RECNO('employee')\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond1 = oList.List(1)\n"
        "cSecond2 = oList.List(2)\n"
        "cSecond3 = oList.List(3)\n"
        "nRecnoAfterSecond = RECNO('employee')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "distinct ORDER BY ordinal " + row_source_label +
               " Requery() script should complete: " + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst1", "East");
    check("cfirst2", "West");
    check("cdisplaybeforesecond", "West");
    check("nrecnobeforesecond", "5");
    check("naftersecond", "3");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "East");
    check("csecond1", "Central");
    check("csecond2", "East");
    check("csecond3", "West");
    check("nrecnoaftersecond", "5");

    const bool has_distinct_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_distinct_requery_event,
           "distinct ORDER BY ordinal " + row_source_label +
               " Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_collapses_aggregate_query_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_aggregates";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "requery_sql_aggregates.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 5\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT COUNT(*), SUM(age), AVG(age), MIN(age), MAX(age) FROM people WHERE age >= 20\"\n"
        "oList.Requery()\n"
        "nRows = oList.ListCount\n"
        "nCount = oList.List(1, 1)\n"
        "nSum = oList.List(1, 2)\n"
        "nAverage = oList.List(1, 3)\n"
        "nMinimum = oList.List(1, 4)\n"
        "nMaximum = oList.List(1, 5)\n"
        "oList.RowSource = \"SELECT COUNT(*), SUM(age) FROM people WHERE age > 100\"\n"
        "oList.ColumnCount = 2\n"
        "oList.Requery()\n"
        "nEmptyRows = oList.ListCount\n"
        "nEmptyCount = oList.List(1, 1)\n"
        "nEmptySum = oList.List(1, 2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("aggregate SQL Requery() script should complete: ") + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nrows", "1");
    check("ncount", "3");
    check("nsum", "90");
    check("naverage", "30");
    check("nminimum", "20");
    check("nmaximum", "40");
    check("nemptyrows", "1");
    check("nemptycount", "0");
    check("nemptysum", "0");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_refreshes_distinct_ordered_single_cursor_query_rows() {
    run_distinct_order_ordinal_requery_test(3);
}

void test_query_file_requery_refreshes_distinct_ordered_single_cursor_query_rows() {
    run_distinct_order_ordinal_requery_test(4);
}

void test_sql_statement_requery_refreshes_single_cursor_query_rows_with_aliases() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_statement_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION", .type = 'C', .length = 12U}};
    const std::vector<std::vector<std::string>> records{
        {"Zulu", "Anne", ""},
        {"Baker", "Ben", "NE"},
        {"Alpha", "Ada", ""}};
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "aliased sql-statement Requery() fixture should create a DBF table");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "requery_sql_statement_alias.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS employee IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT e.last_name, e.first_name FROM employee e WHERE EMPTY(e.region) ORDER BY e.last_name INTO CURSOR temp2\"\n"
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "GO 2 IN employee\n"
        "REPLACE REGION WITH '' IN employee\n"
        "REPLACE LAST_NAME WITH 'Marlow' IN employee\n"
        "APPEND BLANK IN employee\n"
        "REPLACE LAST_NAME WITH 'Delta' IN employee\n"
        "REPLACE FIRST_NAME WITH 'Dia' IN employee\n"
        "REPLACE REGION WITH '' IN employee\n"
        "nRecnoBeforeSecond = RECNO('employee')\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond41 = oList.List(4, 1)\n"
        "nRecnoAfterSecond = RECNO('employee')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("aliased sql-statement Requery() script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst11", "Alpha");
    check("cfirst12", "Ada");
    check("cfirst21", "Zulu");
    check("cfirst22", "Anne");
    check("cdisplaybeforesecond", "Zulu");
    check("nrecnobeforesecond", "4");
    check("naftersecond", "4");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "Delta");
    check("csecond11", "Alpha");
    check("csecond12", "Ada");
    check("csecond21", "Delta");
    check("csecond22", "Dia");
    check("csecond31", "Marlow");
    check("csecond41", "Zulu");
    check("nrecnoaftersecond", "4");

    const bool has_sql_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_sql_requery_event,
           "aliased sql-statement Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_query_file_requery_refreshes_single_cursor_query_rows_with_aliases() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_query_file_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION", .type = 'C', .length = 12U}};
    const std::vector<std::vector<std::string>> records{
        {"Zulu", "Anne", ""},
        {"Baker", "Ben", "NE"},
        {"Alpha", "Ada", ""}};
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "aliased query-file Requery() fixture should create a DBF table");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path query_path = temp_root / "region_alias.qpr";
    write_text(
        query_path,
        "SELECT e.last_name, e.first_name ;\n"
        " FROM employee AS e ;\n"
        " WHERE EMPTY(e.region) ;\n"
        " ORDER BY e.last_name INTO CURSOR temp2\n");

    const fs::path main_path = temp_root / "requery_query_file_alias.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS employee IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = 4\n"
        "oList.RowSource = 'region_alias.qpr'\n"
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "GO 2 IN employee\n"
        "REPLACE REGION WITH '' IN employee\n"
        "REPLACE LAST_NAME WITH 'Marlow' IN employee\n"
        "APPEND BLANK IN employee\n"
        "REPLACE LAST_NAME WITH 'Delta' IN employee\n"
        "REPLACE FIRST_NAME WITH 'Dia' IN employee\n"
        "REPLACE REGION WITH '' IN employee\n"
        "nRecnoBeforeSecond = RECNO('employee')\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond41 = oList.List(4, 1)\n"
        "nRecnoAfterSecond = RECNO('employee')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("aliased query-file Requery() script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "2");
    check("cfirst11", "Alpha");
    check("cfirst12", "Ada");
    check("cfirst21", "Zulu");
    check("cfirst22", "Anne");
    check("cdisplaybeforesecond", "Zulu");
    check("nrecnobeforesecond", "4");
    check("naftersecond", "4");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "Delta");
    check("csecond11", "Alpha");
    check("csecond12", "Ada");
    check("csecond21", "Delta");
    check("csecond22", "Dia");
    check("csecond31", "Marlow");
    check("csecond41", "Zulu");
    check("nrecnoaftersecond", "4");

    const bool has_query_file_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_query_file_requery_event,
           "aliased query-file Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

struct JoinedQueryVariant {
    const char *keyword;
    const char *label;
    const char *suffix;
    bool expect_unmatched_left_row;
};

void run_joined_query_requery_test(int row_source_type, const JoinedQueryVariant &variant) {
    namespace fs = std::filesystem;
    const std::string row_source_label =
        row_source_type == 3 ? "sql-statement" : "query-file";
    const std::string join_label = variant.label;
    const std::string fixture_suffix =
        std::string(row_source_type == 3 ? "sql" : "query") +
        variant.suffix;
    const std::string join_keyword = variant.keyword;

    const fs::path temp_root =
        fs::temp_directory_path() / ("copperfin_native_requery_" + fixture_suffix);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path employee_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> employee_fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION_ID", .type = 'N', .length = 3U}};
    const std::vector<std::vector<std::string>> employee_records{
        {"Zulu", "Anne", "2"},
        {"Alpha", "Ada", "1"},
        {"Marlow", "Mia", "3"}};
    const auto create_employee_result =
        copperfin::vfp::create_dbf_table_file(
            employee_path.string(),
            employee_fields,
            employee_records);
    expect(create_employee_result.ok,
           join_label + " " + row_source_label +
               " Requery() fixture should create the employee DBF");
    if (!create_employee_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path regions_path = temp_root / "regions.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> region_fields{
        {.name = "REGION_ID", .type = 'N', .length = 3U},
        {.name = "REGION_NAM", .type = 'C', .length = 24U}};
    const std::vector<std::vector<std::string>> region_records{
        {"1", "West"},
        {"2", "East"}};
    const auto create_region_result =
        copperfin::vfp::create_dbf_table_file(
            regions_path.string(),
            region_fields,
            region_records);
    expect(create_region_result.ok,
           join_label + " " + row_source_label +
               " Requery() fixture should create the regions DBF");
    if (!create_region_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    std::string row_source_assignment;
    if (row_source_type == 4) {
        const fs::path query_path = temp_root / "region_join.qpr";
        write_text(
            query_path,
            "SELECT employee.last_name, regions.region_nam ;\n"
            " FROM employee " + join_keyword +
                " regions ON employee.region_id = regions.region_id ;\n"
            " ORDER BY employee.last_name INTO CURSOR temp2\n");
        row_source_assignment = "oList.RowSource = 'region_join.qpr'\n";
    } else {
        row_source_assignment =
            "oList.RowSource = \"SELECT employee.last_name, regions.region_nam "
            "FROM employee " +
            join_keyword +
            " regions ON employee.region_id = regions.region_id ORDER BY "
            "employee.last_name INTO CURSOR temp2\"\n";
    }

    const fs::path main_path = temp_root / ("requery_" + fixture_suffix + ".prg");
    write_text(
        main_path,
        "USE '" + employee_path.string() + "' ALIAS employee IN 0\n"
        "USE '" + regions_path.string() + "' ALIAS regions IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = " + std::to_string(row_source_type) + "\n" +
            row_source_assignment +
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        + (variant.expect_unmatched_left_row
               ? std::string(
                     "cFirst31 = oList.List(3, 1)\n"
                     "cFirst32 = oList.List(3, 2)\n")
               : std::string()) +
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "GO 1 IN employee\n"
        "REPLACE LAST_NAME WITH 'Delta' IN employee\n"
        "GO 2 IN regions\n"
        "REPLACE REGION_NAM WITH 'South' IN regions\n"
        "APPEND BLANK IN regions\n"
        "REPLACE REGION_ID WITH 3 IN regions\n"
        "REPLACE REGION_NAM WITH 'North' IN regions\n"
        "GO 3 IN employee\n"
        "GO 3 IN regions\n"
        "nEmployeeRecnoBeforeSecond = RECNO('employee')\n"
        "nRegionRecnoBeforeSecond = RECNO('regions')\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond32 = oList.List(3, 2)\n"
        "nEmployeeRecnoAfterSecond = RECNO('employee')\n"
        "nRegionRecnoAfterSecond = RECNO('regions')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           join_label + " " + row_source_label +
               " Requery() script should complete: " + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    if (variant.expect_unmatched_left_row) {
        check("nafterfirst", "3");
        check("cfirst11", "Alpha");
        check("cfirst12", "West");
        check("cfirst21", "Marlow");
        check("cfirst22", "");
        check("cfirst31", "Zulu");
        check("cfirst32", "East");
        check("cdisplaybeforesecond", "Marlow");
    } else {
        check("nafterfirst", "2");
        check("cfirst11", "Alpha");
        check("cfirst12", "West");
        check("cfirst21", "Zulu");
        check("cfirst22", "East");
        check("cdisplaybeforesecond", "Zulu");
    }
    check("nemployeerecnobeforesecond", "3");
    check("nregionrecnobeforesecond", "3");
    check("naftersecond", "3");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "Delta");
    check("csecond11", "Alpha");
    check("csecond12", "West");
    check("csecond21", "Delta");
    check("csecond22", "South");
    check("csecond31", "Marlow");
    check("csecond32", "North");
    check("nemployeerecnoaftersecond", "3");
    check("nregionrecnoaftersecond", "3");

    const bool has_joined_query_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_joined_query_requery_event,
           join_label + " " + row_source_label +
               " Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_refreshes_joined_query_rows() {
    static constexpr JoinedQueryVariant kJoined{
        .keyword = "JOIN",
        .label = "joined",
        .suffix = "_join",
        .expect_unmatched_left_row = false};
    run_joined_query_requery_test(3, kJoined);
}

void test_sql_statement_requery_refreshes_explicit_inner_join_query_rows() {
    static constexpr JoinedQueryVariant kInnerJoined{
        .keyword = "INNER JOIN",
        .label = "inner-joined",
        .suffix = "_inner_join",
        .expect_unmatched_left_row = false};
    run_joined_query_requery_test(3, kInnerJoined);
}

void test_sql_statement_requery_refreshes_left_join_query_rows() {
    static constexpr JoinedQueryVariant kLeftJoined{
        .keyword = "LEFT JOIN",
        .label = "left-joined",
        .suffix = "_left_join",
        .expect_unmatched_left_row = true};
    run_joined_query_requery_test(3, kLeftJoined);
}

void test_sql_statement_requery_refreshes_left_outer_join_query_rows() {
    static constexpr JoinedQueryVariant kLeftOuterJoined{
        .keyword = "LEFT OUTER JOIN",
        .label = "left-outer-joined",
        .suffix = "_left_outer_join",
        .expect_unmatched_left_row = true};
    run_joined_query_requery_test(3, kLeftOuterJoined);
}

void test_query_file_requery_refreshes_joined_query_rows() {
    static constexpr JoinedQueryVariant kJoined{
        .keyword = "JOIN",
        .label = "joined",
        .suffix = "_join",
        .expect_unmatched_left_row = false};
    run_joined_query_requery_test(4, kJoined);
}

void test_query_file_requery_refreshes_explicit_inner_join_query_rows() {
    static constexpr JoinedQueryVariant kInnerJoined{
        .keyword = "INNER JOIN",
        .label = "inner-joined",
        .suffix = "_inner_join",
        .expect_unmatched_left_row = false};
    run_joined_query_requery_test(4, kInnerJoined);
}

void test_query_file_requery_refreshes_left_join_query_rows() {
    static constexpr JoinedQueryVariant kLeftJoined{
        .keyword = "LEFT JOIN",
        .label = "left-joined",
        .suffix = "_left_join",
        .expect_unmatched_left_row = true};
    run_joined_query_requery_test(4, kLeftJoined);
}

void test_query_file_requery_refreshes_left_outer_join_query_rows() {
    static constexpr JoinedQueryVariant kLeftOuterJoined{
        .keyword = "LEFT OUTER JOIN",
        .label = "left-outer-joined",
        .suffix = "_left_outer_join",
        .expect_unmatched_left_row = true};
    run_joined_query_requery_test(4, kLeftOuterJoined);
}

void run_right_joined_query_requery_test(int row_source_type) {
    namespace fs = std::filesystem;
    const std::string row_source_label =
        row_source_type == 3 ? "sql-statement" : "query-file";
    const fs::path temp_root =
        fs::temp_directory_path() / ("copperfin_native_requery_" +
                                     std::string(row_source_type == 3 ? "sql" : "query") +
                                     "_right_join");
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path employee_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> employee_fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION_ID", .type = 'N', .length = 3U}};
    const std::vector<std::vector<std::string>> employee_records{
        {"Zulu", "Anne", "2"},
        {"Alpha", "Ada", "1"},
        {"Marlow", "Mia", "3"}};
    const auto create_employee_result =
        copperfin::vfp::create_dbf_table_file(
            employee_path.string(),
            employee_fields,
            employee_records);
    expect(create_employee_result.ok,
           "right-joined " + row_source_label +
               " Requery() fixture should create the employee DBF");
    if (!create_employee_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path regions_path = temp_root / "regions.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> region_fields{
        {.name = "REGION_ID", .type = 'N', .length = 3U},
        {.name = "REGION_NAM", .type = 'C', .length = 24U}};
    const std::vector<std::vector<std::string>> region_records{
        {"1", "West"},
        {"2", "East"},
        {"4", "North"}};
    const auto create_region_result =
        copperfin::vfp::create_dbf_table_file(
            regions_path.string(),
            region_fields,
            region_records);
    expect(create_region_result.ok,
           "right-joined " + row_source_label +
               " Requery() fixture should create the regions DBF");
    if (!create_region_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    std::string row_source_assignment;
    if (row_source_type == 4) {
        const fs::path query_path = temp_root / "region_right_join.qpr";
        write_text(
            query_path,
            "SELECT employee.last_name, regions.region_nam ;\n"
            " FROM employee RIGHT JOIN regions ON employee.region_id = regions.region_id ;\n"
            " ORDER BY employee.last_name INTO CURSOR temp2\n");
        row_source_assignment = "oList.RowSource = 'region_right_join.qpr'\n";
    } else {
        row_source_assignment =
            "oList.RowSource = \"SELECT employee.last_name, regions.region_nam "
            "FROM employee RIGHT JOIN regions ON employee.region_id = regions.region_id ORDER BY "
            "employee.last_name INTO CURSOR temp2\"\n";
    }

    const fs::path main_path =
        temp_root / ("requery_" + std::string(row_source_type == 3 ? "sql" : "query") + "_right_join.prg");
    write_text(
        main_path,
        "USE '" + employee_path.string() + "' ALIAS employee IN 0\n"
        "USE '" + regions_path.string() + "' ALIAS regions IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = " + std::to_string(row_source_type) + "\n" +
            row_source_assignment +
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "cFirst31 = oList.List(3, 1)\n"
        "cFirst32 = oList.List(3, 2)\n"
        "GO 1 IN employee\n"
        "REPLACE LAST_NAME WITH 'Delta' IN employee\n"
        "GO 2 IN regions\n"
        "REPLACE REGION_NAM WITH 'South' IN regions\n"
        "APPEND BLANK IN regions\n"
        "REPLACE REGION_ID WITH 3 IN regions\n"
        "REPLACE REGION_NAM WITH 'Central' IN regions\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond32 = oList.List(3, 2)\n"
        "cSecond41 = oList.List(4, 1)\n"
        "cSecond42 = oList.List(4, 2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "right-joined " + row_source_label +
               " Requery() script should complete: " + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nafterfirst", "3");
    check("cfirst11", "");
    check("cfirst12", "North");
    check("cfirst21", "Alpha");
    check("cfirst22", "West");
    check("cfirst31", "Zulu");
    check("cfirst32", "East");
    check("naftersecond", "4");
    check("csecond11", "");
    check("csecond12", "North");
    check("csecond21", "Alpha");
    check("csecond22", "West");
    check("csecond31", "Delta");
    check("csecond32", "South");
    check("csecond41", "Marlow");
    check("csecond42", "Central");

    const bool has_joined_query_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_joined_query_requery_event,
           "right-joined " + row_source_label +
               " Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_refreshes_right_join_query_rows() {
    run_right_joined_query_requery_test(3);
}

void test_query_file_requery_refreshes_right_join_query_rows() {
    run_right_joined_query_requery_test(4);
}

void run_right_joined_query_requery_test_with_aliases(int row_source_type) {
    namespace fs = std::filesystem;
    const std::string row_source_label =
        row_source_type == 3 ? "sql-statement" : "query-file";
    const fs::path temp_root =
        fs::temp_directory_path() / ("copperfin_native_requery_" +
                                     std::string(row_source_type == 3 ? "sql" : "query") +
                                     "_alias_right_join");
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path employee_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> employee_fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION_ID", .type = 'N', .length = 3U}};
    const std::vector<std::vector<std::string>> employee_records{
        {"Zulu", "Anne", "2"},
        {"Alpha", "Ada", "1"},
        {"Marlow", "Mia", "3"}};
    const auto create_employee_result =
        copperfin::vfp::create_dbf_table_file(
            employee_path.string(),
            employee_fields,
            employee_records);
    expect(create_employee_result.ok,
           "aliased right-joined " + row_source_label +
               " Requery() fixture should create the employee DBF");
    if (!create_employee_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path regions_path = temp_root / "regions.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> region_fields{
        {.name = "REGION_ID", .type = 'N', .length = 3U},
        {.name = "REGION_NAM", .type = 'C', .length = 24U}};
    const std::vector<std::vector<std::string>> region_records{
        {"1", "West"},
        {"2", "East"},
        {"4", "North"}};
    const auto create_region_result =
        copperfin::vfp::create_dbf_table_file(
            regions_path.string(),
            region_fields,
            region_records);
    expect(create_region_result.ok,
           "aliased right-joined " + row_source_label +
               " Requery() fixture should create the regions DBF");
    if (!create_region_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    std::string row_source_assignment;
    if (row_source_type == 4) {
        const fs::path query_path = temp_root / "region_alias_right_join.qpr";
        write_text(
            query_path,
            "SELECT e.last_name, r.region_nam ;\n"
            " FROM employee e RIGHT JOIN regions r ON e.region_id = r.region_id ;\n"
            " ORDER BY e.last_name INTO CURSOR temp2\n");
        row_source_assignment = "oList.RowSource = 'region_alias_right_join.qpr'\n";
    } else {
        row_source_assignment =
            "oList.RowSource = \"SELECT e.last_name, r.region_nam "
            "FROM employee e RIGHT JOIN regions r ON e.region_id = r.region_id ORDER BY "
            "e.last_name INTO CURSOR temp2\"\n";
    }

    const fs::path main_path =
        temp_root / ("requery_" + std::string(row_source_type == 3 ? "sql" : "query") + "_alias_right_join.prg");
    write_text(
        main_path,
        "USE '" + employee_path.string() + "' ALIAS employee IN 0\n"
        "USE '" + regions_path.string() + "' ALIAS regions IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = " + std::to_string(row_source_type) + "\n" +
            row_source_assignment +
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "cFirst31 = oList.List(3, 1)\n"
        "cFirst32 = oList.List(3, 2)\n"
        "GO 1 IN employee\n"
        "REPLACE LAST_NAME WITH 'Delta' IN employee\n"
        "GO 2 IN regions\n"
        "REPLACE REGION_NAM WITH 'South' IN regions\n"
        "APPEND BLANK IN regions\n"
        "REPLACE REGION_ID WITH 3 IN regions\n"
        "REPLACE REGION_NAM WITH 'Central' IN regions\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond32 = oList.List(3, 2)\n"
        "cSecond41 = oList.List(4, 1)\n"
        "cSecond42 = oList.List(4, 2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "aliased right-joined " + row_source_label +
               " Requery() script should complete: " + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nafterfirst", "3");
    check("cfirst11", "");
    check("cfirst12", "North");
    check("cfirst21", "Alpha");
    check("cfirst22", "West");
    check("cfirst31", "Zulu");
    check("cfirst32", "East");
    check("naftersecond", "4");
    check("csecond11", "");
    check("csecond12", "North");
    check("csecond21", "Alpha");
    check("csecond22", "West");
    check("csecond31", "Delta");
    check("csecond32", "South");
    check("csecond41", "Marlow");
    check("csecond42", "Central");

    const bool has_joined_query_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_joined_query_requery_event,
           "aliased right-joined " + row_source_label +
               " Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_refreshes_right_join_query_rows_with_aliases() {
    run_right_joined_query_requery_test_with_aliases(3);
}

void test_query_file_requery_refreshes_right_join_query_rows_with_aliases() {
    run_right_joined_query_requery_test_with_aliases(4);
}

void run_joined_query_requery_test_with_aliases(int row_source_type) {
    namespace fs = std::filesystem;
    const std::string row_source_label =
        row_source_type == 3 ? "sql-statement" : "query-file";
    const fs::path temp_root =
        fs::temp_directory_path() / ("copperfin_native_requery_" +
                                     std::string(row_source_type == 3 ? "sql" : "query") +
                                     "_alias_left_join");
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path employee_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> employee_fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION_ID", .type = 'N', .length = 3U}};
    const std::vector<std::vector<std::string>> employee_records{
        {"Zulu", "Anne", "2"},
        {"Alpha", "Ada", "1"},
        {"Marlow", "Mia", "3"}};
    const auto create_employee_result =
        copperfin::vfp::create_dbf_table_file(
            employee_path.string(),
            employee_fields,
            employee_records);
    expect(create_employee_result.ok,
           "aliased left-joined " + row_source_label +
               " Requery() fixture should create the employee DBF");
    if (!create_employee_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path regions_path = temp_root / "regions.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> region_fields{
        {.name = "REGION_ID", .type = 'N', .length = 3U},
        {.name = "REGION_NAM", .type = 'C', .length = 24U}};
    const std::vector<std::vector<std::string>> region_records{
        {"1", "West"},
        {"2", "East"}};
    const auto create_region_result =
        copperfin::vfp::create_dbf_table_file(
            regions_path.string(),
            region_fields,
            region_records);
    expect(create_region_result.ok,
           "aliased left-joined " + row_source_label +
               " Requery() fixture should create the regions DBF");
    if (!create_region_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    std::string row_source_assignment;
    if (row_source_type == 4) {
        const fs::path query_path = temp_root / "region_alias_join.qpr";
        write_text(
            query_path,
            "SELECT e.last_name, r.region_nam ;\n"
            " FROM employee AS e LEFT JOIN regions AS r ON e.region_id = r.region_id ;\n"
            " ORDER BY e.last_name INTO CURSOR temp2\n");
        row_source_assignment = "oList.RowSource = 'region_alias_join.qpr'\n";
    } else {
        row_source_assignment =
            "oList.RowSource = \"SELECT e.last_name, r.region_nam "
            "FROM employee e LEFT JOIN regions r ON e.region_id = r.region_id ORDER BY "
            "e.last_name INTO CURSOR temp2\"\n";
    }

    const fs::path main_path = temp_root / ("requery_alias_left_join_" +
                                            std::string(row_source_type == 3 ? "sql" : "query") +
                                            ".prg");
    write_text(
        main_path,
        "USE '" + employee_path.string() + "' ALIAS employee IN 0\n"
        "USE '" + regions_path.string() + "' ALIAS regions IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = " + std::to_string(row_source_type) + "\n" +
            row_source_assignment +
        "nBefore = oList.ListCount\n"
        "oList.Requery()\n"
        "nAfterFirst = oList.ListCount\n"
        "cFirst11 = oList.List(1, 1)\n"
        "cFirst12 = oList.List(1, 2)\n"
        "cFirst21 = oList.List(2, 1)\n"
        "cFirst22 = oList.List(2, 2)\n"
        "cFirst31 = oList.List(3, 1)\n"
        "cFirst32 = oList.List(3, 2)\n"
        "oList.ListIndex = 2\n"
        "cDisplayBeforeSecond = oList.DisplayValue\n"
        "GO 1 IN employee\n"
        "REPLACE LAST_NAME WITH 'Delta' IN employee\n"
        "GO 2 IN regions\n"
        "REPLACE REGION_NAM WITH 'South' IN regions\n"
        "APPEND BLANK IN regions\n"
        "REPLACE REGION_ID WITH 3 IN regions\n"
        "REPLACE REGION_NAM WITH 'North' IN regions\n"
        "GO 3 IN employee\n"
        "GO 3 IN regions\n"
        "nEmployeeRecnoBeforeSecond = RECNO('employee')\n"
        "nRegionRecnoBeforeSecond = RECNO('regions')\n"
        "oList.Requery()\n"
        "nAfterSecond = oList.ListCount\n"
        "nListIndexAfterSecond = oList.ListIndex\n"
        "cDisplayAfterSecond = oList.DisplayValue\n"
        "cSecond11 = oList.List(1, 1)\n"
        "cSecond12 = oList.List(1, 2)\n"
        "cSecond21 = oList.List(2, 1)\n"
        "cSecond22 = oList.List(2, 2)\n"
        "cSecond31 = oList.List(3, 1)\n"
        "cSecond32 = oList.List(3, 2)\n"
        "nEmployeeRecnoAfterSecond = RECNO('employee')\n"
        "nRegionRecnoAfterSecond = RECNO('regions')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "aliased left-joined " + row_source_label +
               " Requery() script should complete: " + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nbefore", "0");
    check("nafterfirst", "3");
    check("cfirst11", "Alpha");
    check("cfirst12", "West");
    check("cfirst21", "Marlow");
    check("cfirst22", "");
    check("cfirst31", "Zulu");
    check("cfirst32", "East");
    check("cdisplaybeforesecond", "Marlow");
    check("nemployeerecnobeforesecond", "3");
    check("nregionrecnobeforesecond", "3");
    check("naftersecond", "3");
    check("nlistindexaftersecond", "2");
    check("cdisplayaftersecond", "Delta");
    check("csecond11", "Alpha");
    check("csecond12", "West");
    check("csecond21", "Delta");
    check("csecond22", "South");
    check("csecond31", "Marlow");
    check("csecond32", "North");
    check("nemployeerecnoaftersecond", "3");
    check("nregionrecnoaftersecond", "3");

    const bool has_joined_query_requery_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "prg.object.requery";
        });
    expect(has_joined_query_requery_event,
           "aliased left-joined " + row_source_label +
               " Requery() should emit a native requery event");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_refreshes_joined_query_rows_with_aliases() {
    run_joined_query_requery_test_with_aliases(3);
}

void test_query_file_requery_refreshes_joined_query_rows_with_aliases() {
    run_joined_query_requery_test_with_aliases(4);
}

void test_sql_statement_requery_select_star_includes_joined_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_join_star";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path employee_path = temp_root / "employee.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> employee_fields{
        {.name = "LAST_NAME", .type = 'C', .length = 24U},
        {.name = "FIRST_NAME", .type = 'C', .length = 24U},
        {.name = "REGION_ID", .type = 'N', .length = 3U}};
    const auto create_employee_result = copperfin::vfp::create_dbf_table_file(
        employee_path.string(),
        employee_fields,
        {{"Zulu", "Anne", "2"}, {"Alpha", "Ada", "1"}, {"Marlow", "Mia", "3"}});
    expect(create_employee_result.ok, "SELECT * JOIN fixture should create the employee DBF");

    const fs::path regions_path = temp_root / "regions.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> region_fields{
        {.name = "REGION_ID", .type = 'N', .length = 3U},
        {.name = "REGION_NAM", .type = 'C', .length = 24U}};
    const auto create_region_result = copperfin::vfp::create_dbf_table_file(
        regions_path.string(), region_fields, {{"1", "West"}, {"2", "East"}});
    expect(create_region_result.ok, "SELECT * JOIN fixture should create the regions DBF");
    if (!create_employee_result.ok || !create_region_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "requery_sql_join_star.prg";
    write_text(
        main_path,
        "USE '" + employee_path.string() + "' ALIAS employee IN 0\n"
        "USE '" + regions_path.string() + "' ALIAS regions IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 5\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT * FROM employee JOIN regions ON employee.region_id = regions.region_id ORDER BY employee.last_name\"\n"
        "oList.Requery()\n"
        "nInnerRows = oList.ListCount\n"
        "cInnerFirstName = oList.List(1, 2)\n"
        "cInnerFirstRegion = oList.List(1, 5)\n"
        "cInnerSecondRegion = oList.List(2, 5)\n"
        "oList.RowSource = \"SELECT * FROM employee LEFT JOIN regions ON employee.region_id = regions.region_id ORDER BY employee.last_name\"\n"
        "oList.Requery()\n"
        "nLeftRows = oList.ListCount\n"
        "cLeftUnmatchedName = oList.List(2, 1)\n"
        "cLeftUnmatchedRegion = oList.List(2, 5)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("SELECT * JOIN Requery() script should complete: ") + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("ninnerrows", "2");
    check("cinnerfirstname", "Ada");
    check("cinnerfirstregion", "West");
    check("cinnersecondregion", "East");
    check("nleftrows", "3");
    check("cleftunmatchedname", "Marlow");
    check("cleftunmatchedregion", "");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_rejects_three_table_join_chain() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_three_way_join";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "ID", .type = 'N', .length = 3U},
        {.name = "NAME", .type = 'C', .length = 16U}};
    const auto create_table = [&](const fs::path &path, const std::string &name) {
        const auto result = copperfin::vfp::create_dbf_table_file(
            path.string(), fields, {{"1", name}});
        expect(result.ok, "three-table JOIN fixture should create " + name + " DBF");
        return result.ok;
    };
    const bool created =
        create_table(temp_root / "first.dbf", "first") &&
        create_table(temp_root / "second.dbf", "second") &&
        create_table(temp_root / "third.dbf", "third");
    if (!created) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "requery_sql_three_way_join.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "first.dbf").string() + "' ALIAS first IN 0\n"
        "USE '" + (temp_root / "second.dbf").string() + "' ALIAS second IN 0\n"
        "USE '" + (temp_root / "third.dbf").string() + "' ALIAS third IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT * FROM first JOIN second ON first.id = second.id JOIN third ON second.id = third.id\"\n"
        "oList.Requery()\n"
        "nRows = oList.ListCount\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("three-table JOIN Requery() script should complete: ") + state.message);
    const auto rows = state.globals.find("nrows");
    expect(rows != state.globals.end(), "three-table JOIN row count should be captured");
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "0",
               "unsupported three-table JOIN should fail closed without fabricating rows");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_applies_top_after_ordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_top";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "requery_sql_top.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT TOP 2 * FROM people ORDER BY age DESC\"\n"
        "oList.Requery()\n"
        "nTopRows = oList.ListCount\n"
        "cTopFirst = oList.List(1, 1)\n"
        "cTopSecond = oList.List(2, 1)\n"
        "oList.RowSource = \"SELECT TOP 0 * FROM people ORDER BY age DESC\"\n"
        "oList.Requery()\n"
        "nZeroRows = oList.ListCount\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("TOP query Requery() script should complete: ") + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("ntoprows", "2");
    check("ctopfirst", "DELTA");
    check("ctopsecond", "CHARLIE");
    check("nzerorows", "0");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_rejects_where_subquery_predicate() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_subquery";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "requery_sql_subquery.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 2\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT * FROM people WHERE age IN (SELECT age FROM people WHERE age >= 30)\"\n"
        "oList.Requery()\n"
        "nRows = oList.ListCount\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("subquery predicate Requery() script should complete: ") + state.message);
    const auto rows = state.globals.find("nrows");
    expect(rows != state.globals.end(), "subquery predicate row count should be captured");
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "0",
               "unsupported SQL subquery predicate should fail closed without returning all rows");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_groups_and_filters_aggregate_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_group_by";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "sales.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "CATEGORY", .type = 'C', .length = 12U},
        {.name = "REGION", .type = 'C', .length = 12U},
        {.name = "AMOUNT", .type = 'N', .length = 8U}};
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"A", "X", "10"}, {"A", "X", "20"}, {"A", "Y", "30"}, {"B", "X", "5"}, {"B", "X", "15"}});
    expect(create_result.ok, "GROUP BY fixture should create the sales DBF");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "requery_sql_group_by.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS sales IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 3\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT category, COUNT(*) AS cnt, SUM(amount) AS total FROM sales WHERE amount >= 10 GROUP BY category HAVING cnt > 1 AND COUNT(*) > 2 ORDER BY category\"\n"
        "oList.Requery()\n"
        "nHavingRows = oList.ListCount\n"
        "cHavingCategory = oList.List(1, 1)\n"
        "nHavingCount = oList.List(1, 2)\n"
        "nHavingTotal = oList.List(1, 3)\n"
        "oList.RowSource = \"SELECT category, region, COUNT(*) AS cnt FROM sales GROUP BY category, region ORDER BY category, region\"\n"
        "oList.Requery()\n"
        "nCompositeRows = oList.ListCount\n"
        "cCompositeFirstCategory = oList.List(1, 1)\n"
        "cCompositeFirstRegion = oList.List(1, 2)\n"
        "nCompositeFirstCount = oList.List(1, 3)\n"
        "cCompositeSecondRegion = oList.List(2, 2)\n"
        "nCompositeSecondCount = oList.List(2, 3)\n"
        "cCompositeThirdCategory = oList.List(3, 1)\n"
        "nCompositeThirdCount = oList.List(3, 3)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("GROUP BY Requery() script should complete: ") + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nhavingrows", "1");
    check("chavingcategory", "A");
    check("nhavingcount", "3");
    check("nhavingtotal", "60");
    check("ncompositerows", "3");
    check("ccompositefirstcategory", "A");
    check("ccompositefirstregion", "X");
    check("ncompositefirstcount", "2");
    check("ccompositesecondregion", "Y");
    check("ncompositesecondcount", "1");
    check("ccompositethirdcategory", "B");
    check("ncompositethirdcount", "2");

    fs::remove_all(temp_root, ignored);
}

void test_sql_statement_requery_orders_date_and_datetime_values_chronologically() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_requery_sql_date_order";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto timestamp = [](int year, int month, int day, int hour, int minute, int second) {
        const long long millis =
            ((static_cast<long long>(hour) * 60LL * 60LL) +
             (static_cast<long long>(minute) * 60LL) +
             static_cast<long long>(second)) * 1000LL;
        return std::string("julian:") +
               std::to_string(copperfin::runtime::date_to_julian(year, month, day)) +
               " millis:" + std::to_string(millis);
    };

    const fs::path table_path = temp_root / "events.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "EVENT_ID", .type = 'C', .length = 12U},
        {.name = "EDATE", .type = 'D', .length = 8U},
        {.name = "STAMP", .type = 'T', .length = 8U}};
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"early", "20201201", timestamp(2020, 12, 31, 23, 59, 59)},
         {"late", "20210115", timestamp(2021, 1, 1, 0, 0, 0)},
         {"tie", "20210115", timestamp(2021, 1, 1, 0, 0, 0)}});
    expect(create_result.ok,
           "DATE/DATETIME ORDER BY fixture should create the events DBF: " + create_result.error);
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "requery_sql_date_order.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS events IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ColumnCount = 1\n"
        "oList.RowSourceType = 3\n"
        "SET DATE TO DMY\n"
        "SET MARK TO '.'\n"
        "oList.RowSource = \"SELECT event_id FROM events ORDER BY edate\"\n"
        "oList.Requery()\n"
        "cDateFirst = oList.List(1, 1)\n"
        "cDateSecond = oList.List(2, 1)\n"
        "cDateThird = oList.List(3, 1)\n"
        "SET DATE TO YMD\n"
        "SET MARK TO '-'\n"
        "SET HOURS TO 12\n"
        "SET SECONDS OFF\n"
        "oList.RowSource = \"SELECT event_id FROM events ORDER BY stamp DESC\"\n"
        "oList.Requery()\n"
        "cDateTimeFirst = oList.List(1, 1)\n"
        "cDateTimeSecond = oList.List(2, 1)\n"
        "cDateTimeThird = oList.List(3, 1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("DATE/DATETIME ORDER BY Requery() script should complete: ") + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("cdatefirst", "early");
    check("cdatesecond", "late");
    check("cdatethird", "tie");
    check("cdatetimefirst", "late");
    check("cdatetimesecond", "tie");
    check("cdatetimethird", "early");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_value_list_requery_refreshes_rows_and_preserves_selection();
    test_array_requery_refreshes_rows_and_growth();
    test_alias_requery_refreshes_open_cursor_rows_and_preserves_cursor_position();
    test_fields_requery_refreshes_alias_qualified_field_list();
    test_sql_statement_requery_refreshes_single_cursor_query_rows();
    test_query_file_requery_refreshes_single_cursor_query_rows();
    test_sql_statement_requery_collapses_aggregate_query_rows();
    test_sql_statement_requery_refreshes_distinct_ordered_single_cursor_query_rows();
    test_query_file_requery_refreshes_distinct_ordered_single_cursor_query_rows();
    test_sql_statement_requery_refreshes_single_cursor_query_rows_with_aliases();
    test_query_file_requery_refreshes_single_cursor_query_rows_with_aliases();
    test_sql_statement_requery_refreshes_joined_query_rows();
    test_sql_statement_requery_refreshes_explicit_inner_join_query_rows();
    test_sql_statement_requery_refreshes_left_join_query_rows();
    test_sql_statement_requery_refreshes_left_outer_join_query_rows();
    test_sql_statement_requery_refreshes_right_join_query_rows();
    test_query_file_requery_refreshes_joined_query_rows();
    test_query_file_requery_refreshes_explicit_inner_join_query_rows();
    test_query_file_requery_refreshes_left_join_query_rows();
    test_query_file_requery_refreshes_left_outer_join_query_rows();
    test_query_file_requery_refreshes_right_join_query_rows();
    test_sql_statement_requery_refreshes_joined_query_rows_with_aliases();
    test_query_file_requery_refreshes_joined_query_rows_with_aliases();
    test_sql_statement_requery_refreshes_right_join_query_rows_with_aliases();
    test_query_file_requery_refreshes_right_join_query_rows_with_aliases();
    test_sql_statement_requery_select_star_includes_joined_fields();
    test_sql_statement_requery_rejects_three_table_join_chain();
    test_sql_statement_requery_applies_top_after_ordering();
    test_sql_statement_requery_orders_date_and_datetime_values_chronologically();
    test_sql_statement_requery_rejects_where_subquery_predicate();
    test_sql_statement_requery_groups_and_filters_aggregate_rows();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
