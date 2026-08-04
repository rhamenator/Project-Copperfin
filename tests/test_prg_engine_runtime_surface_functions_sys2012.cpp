// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_sys2012_reports_memo_field_block_size()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_sys2012";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path memo_table = temp_root / "memo.dbf";
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            memo_table.string(),
            {copperfin::vfp::DbfFieldDescriptor{"NOTES", 'M', 0U, 10U, 0U}},
            {{"memo text"}});
        expect(create_result.ok, "SYS(2012) memo fixture should be created");

        const fs::path plain_table = temp_root / "plain.dbf";
        write_simple_dbf(plain_table, {"ALPHA"});
        const fs::path main_path = temp_root / "sys2012.prg";
        write_text(
            main_path,
            "cNoTable = SYS(2012)\n"
            "USE 'memo.dbf' ALIAS Memo2012\n"
            "cMemo = SYS(2012)\n"
            "cMemoAlias = SYS(2012, 'Memo2012')\n"
            "USE 'plain.dbf' ALIAS Plain2012\n"
            "cPlain = SYS(2012)\n"
            "cUnknown = SYS(2012, 'Missing2012')\n"
            "USE IN Memo2012\n"
            "USE IN Plain2012\n"
            "cAfterClose = SYS(2012)\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path, temp_root))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "SYS(2012) script should complete: " + state.message);

        const auto value = [&](const std::string& name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            return found == state.globals.end()
                       ? std::string{}
                       : copperfin::runtime::format_value(found->second);
        };
        expect(value("cnotable") == "0", "SYS(2012) should return character zero without a table");
        expect(value("cmemo") == "512" && value("cmemoalias") == "512",
               "SYS(2012) should return the memo sidecar block size as character text");
        expect(value("cplain") == "0" && value("cunknown") == "0" && value("cafterclose") == "0",
               "SYS(2012) should return character zero for non-memo and unavailable tables");

        fs::remove_all(temp_root, ignored);
    }
}
