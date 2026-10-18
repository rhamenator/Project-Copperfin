// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Diagnostic only, retained unchanged-main runtime; no product modifications.
#include "copperfin/runtime/prg_engine.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
int main() {
    using namespace copperfin::runtime;
    const auto root=std::filesystem::absolute("build/nonwhole-expanded-original/query-owned-root");
    std::filesystem::create_directories(root);
    for (const auto mode : {"COPPERFIN","VFP9"}) for (const int setting : {0,14}) {
        RuntimeSessionOptions options;
        options.startup_path=(root/"query.prg").string();
        options.working_directory=root.string();options.temp_directory=(root/"temp").string();
        options.require_source_text_overrides=true;
        options.startup_source_text=std::string("SET NUMERICBEHAVIOR TO ")+mode+"\nSET DECIMALS TO "+std::to_string(setting)+
            "\nPUBLIC nPause\nnPause=0\nnPause=1\nRETURN\n";
        auto session=PrgRuntimeSession::create(options);
        session.add_breakpoint({options.startup_path,5});
        if(session.run(DebugResumeAction::continue_run).reason!=DebugPauseReason::breakpoint)return 1;
        for(const auto expression:{"SET('DECIMALS')","VARTYPE(SET('DECIMALS'))","VAL(TRANSFORM(SET('DECIMALS')))"}){
            const auto result=session.evaluate_watch_expression(expression);
            if(!result.ok)return 2;
            std::cout<<mode<<'|'<<setting<<'|'<<expression<<"|kind="<<static_cast<int>(result.value.kind)
                <<"|number="<<result.value.number_value<<"|string="<<std::quoted(result.value.string_value)<<'\n';
        }
    }
}
