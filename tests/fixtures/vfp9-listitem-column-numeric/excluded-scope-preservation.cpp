// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// NON-NATIVE baseline/fixed preservation, NEVER a compatibility oracle.
// RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR-5611-LISTITEM-COLUMN-NUMERIC-002.
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {
struct Case {
    std::string group;
    std::string identity;
    std::string configuration;
    std::string call;
};
}

int main(int argc, char** argv) {
    if (argc != 2) return 2;
    using namespace copperfin::runtime;
    using namespace copperfin::test_support;
    const std::filesystem::path root = argv[1];
    std::filesystem::create_directories(root);
    const auto script = root / "preservation.prg";
    std::vector<Case> cases{
        {"arity", "one", "", "o.ListItem(11)"},
        {"arity", "three", "", "o.ListItem(11,2,7)"},
        {"second-type", "null", "", "o.ListItem(11,.NULL.)"},
        {"second-type", "character", "", "o.ListItem(11,'2')"},
        {"second-type", "logical", "", "o.ListItem(11,.T.)"},
        {"second-type", "currency", "", "o.ListItem(11,$2)"},
        {"second-type", "date", "", "o.ListItem(11,{^2026-10-31})"},
        {"second-type", "datetime", "", "o.ListItem(11,{^2026-10-31 12:00:00})"},
        {"first-selector", "fractional", "", "o.ListItem(11.9,2)"},
        {"first-selector", "zero", "", "o.ListItem(0,2)"},
        {"first-selector", "negative", "", "o.ListItem(-1,2)"},
        {"first-selector", "missing", "", "o.ListItem(99,2)"},
        {"first-selector", "character", "", "o.ListItem('11',2)"},
        {"first-selector", "logical", "", "o.ListItem(.T.,2)"},
        {"first-selector", "null", "", "o.ListItem(.NULL.,2)"},
        {"first-selector", "exact64", "", "o.ListItem(CAST(11 AS INT64),2)"},
        {"binding", "rst2", "CREATE CURSOR source (caption C(8))\nINSERT INTO source VALUES ('one')\nINSERT INTO source VALUES ('two')\no.RowSourceType=2\no.RowSource='source'\no.Requery()\n", "o.ListItem(11,2)"},
        {"binding", "rst5", "PUBLIC ARRAY aRows[2]\naRows[1]='one'\naRows[2]='two'\no.RowSourceType=5\no.RowSource='aRows'\no.Requery()\n", "o.ListItem(11,2)"},
        {"binding", "controlsource", "PUBLIC cBound\ncBound=''\no.ControlSource='cBound'\n", "o.ListItem(11,2)"},
        {"nonexact-rst", "logical", "o.RowSourceType=.F.\n", "o.ListItem(11,2)"},
        {"nonexact-rst", "null", "o.RowSourceType=.NULL.\n", "o.ListItem(11,2)"},
        {"nonexact-rst", "currency", "o.RowSourceType=$0\n", "o.ListItem(11,2)"},
        {"nonexact-rst", "character", "o.RowSourceType='0'\n", "o.ListItem(11,2)"},
        {"nonexact-rst", "positive-fraction", "o.RowSourceType=0.4\n", "o.ListItem(11,2)"},
        {"nonexact-rst", "negative-fraction", "o.RowSourceType=-0.4\n", "o.ListItem(11,2)"},
        {"missing-column", "four", "", "o.ListItem(11,4)"},
        {"missing-column", "32768", "", "o.ListItem(11,32768)"},
        {"missing-column", "int32max", "", "o.ListItem(11,2147483647)"},
        {"ordinary-control", "column1", "", "o.ListItem(11,1)"},
        {"ordinary-control", "column2", "", "o.ListItem(11,2)"},
        {"ordinary-control", "column3", "", "o.ListItem(22,3)"},
        {"ordinary-control", "exact64-column2", "", "o.ListItem(22,CAST(2 AS INT64))"},
    };
    int calls = 0;
    for (const std::string cls : {"ListBox", "ComboBox"})
        for (const std::string mode : {"COPPERFIN", "VFP9"})
            for (const int selected : {1, 2})
                for (const auto& item : cases) {
                    ++calls;
                    const std::string other = mode == "VFP9" ? "COPPERFIN" : "VFP9";
                    const bool bound = item.group == "binding" && item.identity != "controlsource";
                    const std::string population =
                        "o.AddListItem('row1',11)\n"
                        "o.AddListItem('row2',22)\n"
                        "o.AddListItem('row3',33)\n"
                        "o.List(1,2)='cell12'\n"
                        "o.List(2,2)='cell22'\n"
                        "o.List(3,2)='cell32'\n"
                        "o.List(1,3)='cell13'\n"
                        "o.List(2,3)='cell23'\n"
                        "o.List(3,3)='cell33'\n"
                        "o.ItemData(1)=101\n"
                        "o.ItemData(2)=202\n"
                        "o.ItemData(3)=303\n";
                    write_text(script,
                        "SET NUMERICBEHAVIOR TO " + other + "\nSET DATASESSION TO " +
                        std::to_string(selected) + "\nSET NUMERICBEHAVIOR TO " + mode +
                        "\nPUBLIC o,cPhase,nCode,vResult,nCount,nFocus,nID,cValue,cDisplay,nTop,nTopID,cContext,cOther\n"
                        "cPhase='setup'\nnCode=0\nvResult='not-returned'\nTRY\n"
                        "o=CREATEOBJECT('" + cls + "')\no.ColumnCount=3\n" +
                        item.configuration + (bound ? "" : population) +
                        "o.ListIndex=2\n" + (cls == "ListBox" ? "o.TopIndex=2\n" : "") +
                        "cPhase='invoke'\nvResult=" + item.call +
                        "\nCATCH TO problem\nnCode=problem.ErrorNo\nENDTRY\n"
                        "nCount=o.ListCount\nnFocus=o.ListIndex\nnID=o.ListItemID\ncValue=o.Value\ncDisplay=o.DisplayValue\n" +
                        (cls == "ListBox" ? "nTop=o.TopIndex\nnTopID=o.TopItemID\n" : "") +
                        "cContext=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())\n"
                        "SET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nRETURN\n");
                    auto session = PrgRuntimeSession::create(make_runtime_session_options(script, root, false));
                    const auto state = session.run(DebugResumeAction::continue_run);
                    const auto get = [&](const char* key) {
                        const auto found = state.globals.find(key);
                        return found == state.globals.end() ? std::string("<missing>") : format_value(found->second);
                    };
                    std::cout << cls << '|' << mode << '|' << selected << '|' << item.group << '|'
                              << item.identity << "|completed=" << state.completed;
                    for (const auto* key : {"cphase", "ncode", "vresult", "ncount", "nfocus", "nid",
                                             "cvalue", "cdisplay", "ntop", "ntopid", "ccontext", "cother"})
                        std::cout << '|' << key << '=' << get(key);
                    for (const auto& object : state.ole_objects) if (object.base_class_name == cls) {
                        std::cout << "|ids=";
                        for (const auto& id : object.collection_item_keys) std::cout << id << ',';
                        std::cout << "|primary=";
                        for (const auto& value : object.collection_items) std::cout << format_value(value) << ',';
                        std::cout << "|selected=";
                        for (const bool bit : object.list_selected) std::cout << bit;
                        std::cout << "|rows=";
                        for (const auto& row : object.list_rows) {
                            std::cout << '[' << row.size() << ':';
                            for (const auto& cell : row) std::cout << format_value(cell) << ',';
                            std::cout << ']';
                        }
                        std::cout << "|data=";
                        for (const auto& value : object.list_item_data) std::cout << format_value(value) << ',';
                    }
                    std::cout << '\n';
                }
    std::cout << "DONE|cases=" << cases.size() << "|calls=" << calls << '\n';
    return cases.size() == 32U && calls == 256 ? 0 : 1;
}
