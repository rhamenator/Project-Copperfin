// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_sys2021_returns_filtered_index_expressions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_sys2021";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / "people.dbf";
        const fs::path index_path = temp_root / "people.dbf.idx";
        const fs::path filtered_index_path = temp_root / "people.idx";
        write_simple_dbf(table_path, {{"ALPHA", 20}, {"BRAVO", 30}});
        write_synthetic_idx(index_path, "NAME");
        write_synthetic_idx_with_for(filtered_index_path, "UPPER(NAME)", "DELETED() = .F.");

        const fs::path main_path = temp_root / "sys2021.prg";
        write_text(
            main_path,
            "USE '" + table_path.string() + "' ALIAS People IN 0\n"
            "nBeforeRecno = RECNO()\n"
            "cCurrent = SYS(2021, 1)\n"
            "cAlias = SYS(2021, 2, 'People')\n"
            "cOutOfRange = SYS(2021, 3)\n"
            "cInvalid = SYS(2021, 0)\n"
            "nAfterRecno = RECNO()\n"
            "cNoCursor = SYS(2021, 1, 'MissingAlias')\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path, temp_root))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "SYS(2021) script should complete: " + state.message);

        const auto value = [&](const std::string& name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            return found == state.globals.end()
                       ? std::string{}
                       : copperfin::runtime::format_value(found->second);
        };
        const auto current = state.globals.find("ccurrent");
        expect(current != state.globals.end(), "SYS(2021) current expression should be captured");
        if (current != state.globals.end())
        {
            expect(current->second.kind == copperfin::runtime::PrgValueKind::string &&
                       copperfin::runtime::format_value(current->second).empty(),
                   "SYS(2021) should preserve an empty slot for an unfiltered first order");
        }
        expect(value("calias") == "DELETED() = .F.",
               "SYS(2021) should resolve an explicit alias through the cursor seam");
        expect(value("coutofrange").empty() && value("cinvalid").empty() && value("cnocursor").empty(),
               "SYS(2021) should return empty text for invalid, missing, and out-of-range requests");
        expect(value("nbeforerecno") == value("nafterrecno"),
               "SYS(2021) should not mutate cursor position");

        fs::remove_all(temp_root, ignored);
    }
}
