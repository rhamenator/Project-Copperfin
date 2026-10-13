// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-REMOVEITEM-ROW-NUMERIC-001 / VR-5611-REMOVEITEM-ROW-NUMERIC-002.
// Review4253248168: explicit native RST0; exact zero derives named #6776 parent.
// Supplemental literals; initial five frozen numeric inputs stay unchanged.
namespace copperfin::runtime_surface_tests {
void test_native_removeitem_exactzero_rowsourcetype_admission() {
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / ("copperfin_removeitem_exactzero_" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(root);
    struct Case { const char* input; int error; int removed; bool alias; };
    const Case cases[] = {
        {"2.9", 0, 2, false}, {"0", 11, 0, false},
        {"4", 1234, 0, false}, {"32769", 11, 0, false},
        {"4294967297", 0, 1, true}, {"-4294967294.5", 0, 2, true}};
    int completed = 0;
    for (const auto* mode : {"COPPERFIN", "VFP9"}) {
        for (const auto* klass : {"ListBox", "ComboBox"}) {
            for (const auto* source : {"0", "CAST(0 AS INT64)", "CAST(0 AS UINT64)"}) {
                for (const auto& row : cases) {
                    const bool default_alias = row.alias && std::string(mode) == "COPPERFIN";
                    const int expected_error = default_alias ? 11 : row.error;
                    const int removed = default_alias ? 0 : row.removed;
                    const auto path = root / ("case" + std::to_string(completed) + ".prg");
                    const std::string script = std::string("SET NUMERICBEHAVIOR TO ") + mode + "\n" +
                        "o = CREATEOBJECT('" + klass + "')\n" +
                        "o.ColumnCount = 2\n"
                        "o.AddListItem('first', 11)\n"
                        "o.AddListItem('second', 22)\n"
                        "o.AddListItem('third', 33)\n"
                        "o.List(1, 2) = 'cell1'\n"
                        "o.List(2, 2) = 'cell2'\n"
                        "o.List(3, 2) = 'cell3'\n"
                        "o.ItemData(1) = 101\n"
                        "o.ItemData(2) = 202\n"
                        "o.ItemData(3) = 303\n"
                        "o.RowSourceType = " + source + "\n" +
                        "nError = 0\nTRY\no.RemoveItem(" + row.input + ")\n" +
                        "CATCH TO e\nnError = e.ErrorNo\nENDTRY\n"
                        "nCount = o.ListCount\n"
                        "nID1 = o.IndexToItemID(1)\nnID2 = o.IndexToItemID(2)\n"
                        "cFirst = o.List(1, 1)\ncSecond = o.List(2, 1)\n"
                        "cCell1 = o.List(1, 2)\ncCell2 = o.List(2, 2)\n"
                        "nData1 = o.ItemData(1)\nnData2 = o.ItemData(2)\nRETURN\n";
                    write_text(path, script);
                    const auto state = copperfin::runtime::PrgRuntimeSession::create(
                        make_runtime_session_options(path.string(), root.string()))
                        .run(copperfin::runtime::DebugResumeAction::continue_run);
                    const std::string label = std::string("RemoveItem exact-zero RowSourceType ") +
                        mode + "/" + klass + "/" + source + "/" + row.input + ": ";
                    expect(state.completed, label + "script completes: " + state.message);
                    // Verify exact configuration kinds, not a coerced Number-only oracle.
                    // CATCH also publishes an error object: count only the configured control.
                    int native_controls = 0;
                    for (const auto& object : state.ole_objects) {
                        if (object.base_class_name != klass) continue;
                        ++native_controls;
                        const auto found = object.properties.find("rowsourcetype");
                        expect(found != object.properties.end(), label + "RowSourceType captured");
                        if (found != object.properties.end()) {
                            using Kind = copperfin::runtime::PrgValueKind;
                            const auto expected_kind = std::string(source) == "0" ? Kind::number :
                                std::string(source) == "CAST(0 AS INT64)" ? Kind::int64 : Kind::uint64;
                            expect(found->second.kind == expected_kind, label + "configuration kind preserved");
                            expect(copperfin::runtime::format_value(found->second) == "0", label + "configuration value zero");
                        }
                    }
                    expect(native_controls == 1, label + "one native control captured");
                    const auto check = [&](const char* name, const std::string& expected) {
                        const auto found = state.globals.find(name);
                        expect(found != state.globals.end(), label + name + " captured");
                        if (found != state.globals.end()) {
                            expect(copperfin::runtime::format_value(found->second) == expected,
                                   label + name + " expected " + expected + " got " +
                                   copperfin::runtime::format_value(found->second));
                        }
                    };
                    check("nerror", std::to_string(expected_error));
                    check("ncount", removed == 0 ? "3" : "2");
                    check("nid1", removed == 1 ? "22" : "11");
                    check("nid2", removed == 0 ? "22" : "33");
                    check("cfirst", removed == 1 ? "second" : "first");
                    check("csecond", removed == 0 ? "second" : "third");
                    check("ccell1", removed == 1 ? "cell2" : "cell1");
                    check("ccell2", removed == 0 ? "cell2" : "cell3");
                    check("ndata1", removed == 1 ? "202" : "101");
                    check("ndata2", removed == 0 ? "202" : "303");
                    ++completed;
                }
            }
        }
    }
    expect(completed == 72, "RemoveItem exact-zero RowSourceType supplemental count72");
    std::cout << "RemoveItem exact-zero RowSourceType supplemental cases: " << completed << '\n';
    std::error_code ignored;
    fs::remove_all(root, ignored);
}
} // namespace copperfin::runtime_surface_tests
