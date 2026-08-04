// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_native_list_control_itemid_selector_reads_stay_coherent() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_native_list_control_itemid_selectors";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_itemid_selectors.prg";
    write_text(
        main_path,
        "oPlain = CREATEOBJECT('ComboBox')\n"
        "oPlain.AddListItem('Zulu', 10)\n"
        "oPlain.AddListItem('Alpha', 20)\n"
        "oPlain.AddListItem('Echo', 30)\n"
        "nBracketIndex1IdBeforeSort = oPlain.IndexToItemID[1]\n"
        "nSelectorIndex = 2\n"
        "nBracketIndex2IdBeforeSort = oPlain.IndexToItemID[m.nSelectorIndex]\n"
        "nBracketItem20IndexBeforeSort = oPlain.ItemIDToIndex[20]\n"
        "oPlain.Sorted = .T.\n"
        "nBracketIndex3IdAfterSort = oPlain.IndexToItemID[oPlain.ListCount]\n"
        "nSelectorItemId = 10\n"
        "nBracketItem10IndexAfterSort = oPlain.ItemIDToIndex[m.nSelectorItemId]\n"
        "oPlain.RemoveListItem(30)\n"
        "nBracketMissingIndexAfterRemove = oPlain.ItemIDToIndex[30]\n"
        "oSeed = CREATEOBJECT('SeededList')\n"
        "nSeedBracketFirstId = oSeed.IndexToItemID[1]\n"
        "nSeedBracketLastId = oSeed.IndexToItemID[oSeed.ListCount]\n"
        "nSeedBracketItem200Index = oSeed.ItemIDToIndex[200]\n"
        "RETURN\n"
        "DEFINE CLASS SeededList AS ListBox\n"
        "    PROCEDURE Init\n"
        "        THIS.AddListItem('North', 200)\n"
        "        THIS.AddListItem('East', 100)\n"
        "        THIS.Sorted = .T.\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native item-id selector script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }

        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" +
                   copperfin::runtime::format_value(it->second) + "'");
    };

    check("nbracketindex1idbeforesort", "10");
    check("nbracketindex2idbeforesort", "20");
    check("nbracketitem20indexbeforesort", "2");
    check("nbracketindex3idaftersort", "10");
    check("nbracketitem10indexaftersort", "3");
    check("nbracketmissingindexafterremove", "0");
    check("nseedbracketfirstid", "100");
    check("nseedbracketlastid", "200");
    check("nseedbracketitem200index", "2");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_native_list_control_itemid_selector_reads_stay_coherent();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
