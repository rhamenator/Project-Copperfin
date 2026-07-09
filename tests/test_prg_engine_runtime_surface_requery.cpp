// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
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

}  // namespace

int main() {
    test_value_list_requery_refreshes_rows_and_preserves_selection();
    test_array_requery_refreshes_rows_and_growth();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
