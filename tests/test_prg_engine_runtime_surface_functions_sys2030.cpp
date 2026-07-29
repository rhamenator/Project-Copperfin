// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_sys2030_debug_feature_state_is_session_local()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_sys2030";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path enabled_path = temp_root / "sys2030_enabled.prg";
        write_text(
            enabled_path,
            "nInitial = SYS(2030)\n"
            "cInitialType = VARTYPE(SYS(2030))\n"
            "nEnable = SYS(2030, 1)\n"
            "nEnabled = SYS(2030)\n"
            "nInvalid = SYS(2030, 2)\n"
            "nAfterInvalid = SYS(2030)\n"
            "nDisable = SYS(2030, 0)\n"
            "nDisabled = SYS(2030)\n"
            "RETURN\n");

        const auto enabled_state = copperfin::runtime::PrgRuntimeSession::create(
                                         make_runtime_session_options(enabled_path, temp_root))
                                         .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(enabled_state.completed,
               "SYS(2030) state script should complete: " + enabled_state.message);

        const auto value = [&](const std::string& name)
        {
            const auto found = enabled_state.globals.find(name);
            expect(found != enabled_state.globals.end(), name + " should be captured");
            return found == enabled_state.globals.end()
                       ? std::string{}
                       : copperfin::runtime::format_value(found->second);
        };
        const auto kind = [&](const std::string& name)
        {
            const auto found = enabled_state.globals.find(name);
            expect(found != enabled_state.globals.end(), name + " should be captured");
            return found == enabled_state.globals.end()
                       ? copperfin::runtime::PrgValueKind::empty
                       : found->second.kind;
        };

        expect(value("ninitial") == "0", "SYS(2030) should default to numeric zero");
        expect(value("cinitialtype") == "N", "SYS(2030) should have numeric VFP type");
        expect(value("nenable") == "1" && value("nenabled") == "1",
               "SYS(2030, 1) should enable and report the session state");
        expect(kind("nenable") == copperfin::runtime::PrgValueKind::number,
               "SYS(2030, 1) should return a numeric value");
        expect(value("ninvalid") == "1" && value("nafterinvalid") == "1",
               "invalid SYS(2030) setter values should preserve the current state");
        expect(value("ndisable") == "0" && value("ndisabled") == "0",
               "SYS(2030, 0) should disable and report the session state");

        const fs::path fresh_path = temp_root / "sys2030_fresh.prg";
        write_text(fresh_path, "nFresh = SYS(2030)\nRETURN\n");
        const auto fresh_state = copperfin::runtime::PrgRuntimeSession::create(
                                      make_runtime_session_options(fresh_path, temp_root))
                                      .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(fresh_state.completed, "a fresh SYS(2030) session should complete");
        const auto fresh = fresh_state.globals.find("nfresh");
        expect(fresh != fresh_state.globals.end(), "fresh SYS(2030) value should be captured");
        if (fresh != fresh_state.globals.end())
        {
            expect(fresh->second.kind == copperfin::runtime::PrgValueKind::number &&
                       copperfin::runtime::format_value(fresh->second) == "0",
                   "fresh SYS(2030) sessions should start at numeric zero");
        }

        fs::remove_all(temp_root, ignored);
    }
}
