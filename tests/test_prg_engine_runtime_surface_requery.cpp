// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
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
        {.name = "REGION_NAME", .type = 'C', .length = 24U}};
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
            "SELECT employee.last_name, regions.region_name ;\n"
            " FROM employee " + join_keyword +
                " regions ON employee.region_id = regions.region_id ;\n"
            " ORDER BY employee.last_name INTO CURSOR temp2\n");
        row_source_assignment = "oList.RowSource = 'region_join.qpr'\n";
    } else {
        row_source_assignment =
            "oList.RowSource = \"SELECT employee.last_name, regions.region_name "
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
        "REPLACE REGION_NAME WITH 'South' IN regions\n"
        "APPEND BLANK IN regions\n"
        "REPLACE REGION_ID WITH 3 IN regions\n"
        "REPLACE REGION_NAME WITH 'North' IN regions\n"
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
        {.name = "REGION_NAME", .type = 'C', .length = 24U}};
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
            "SELECT e.last_name, r.region_name ;\n"
            " FROM employee AS e LEFT JOIN regions AS r ON e.region_id = r.region_id ;\n"
            " ORDER BY e.last_name INTO CURSOR temp2\n");
        row_source_assignment = "oList.RowSource = 'region_alias_join.qpr'\n";
    } else {
        row_source_assignment =
            "oList.RowSource = \"SELECT e.last_name, r.region_name "
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
        "REPLACE REGION_NAME WITH 'South' IN regions\n"
        "APPEND BLANK IN regions\n"
        "REPLACE REGION_ID WITH 3 IN regions\n"
        "REPLACE REGION_NAME WITH 'North' IN regions\n"
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

}  // namespace

int main() {
    test_value_list_requery_refreshes_rows_and_preserves_selection();
    test_array_requery_refreshes_rows_and_growth();
    test_alias_requery_refreshes_open_cursor_rows_and_preserves_cursor_position();
    test_fields_requery_refreshes_alias_qualified_field_list();
    test_sql_statement_requery_refreshes_single_cursor_query_rows();
    test_query_file_requery_refreshes_single_cursor_query_rows();
    test_sql_statement_requery_refreshes_single_cursor_query_rows_with_aliases();
    test_query_file_requery_refreshes_single_cursor_query_rows_with_aliases();
    test_sql_statement_requery_refreshes_joined_query_rows();
    test_sql_statement_requery_refreshes_explicit_inner_join_query_rows();
    test_sql_statement_requery_refreshes_left_join_query_rows();
    test_sql_statement_requery_refreshes_left_outer_join_query_rows();
    test_query_file_requery_refreshes_joined_query_rows();
    test_query_file_requery_refreshes_explicit_inner_join_query_rows();
    test_query_file_requery_refreshes_left_join_query_rows();
    test_query_file_requery_refreshes_left_outer_join_query_rows();
    test_sql_statement_requery_refreshes_joined_query_rows_with_aliases();
    test_query_file_requery_refreshes_joined_query_rows_with_aliases();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
