// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-UNARY-ZERO-CONTEXT-001 / VR-5611-NUMERIC-UNARY-ZERO-CONTEXT-001.
// Native-backed zero BITS, not assignment-width/default-display acceptance.
#include "copperfin/runtime/prg_engine.h"
#include <bit>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct OwnedDirectory {
    std::filesystem::path path;
    OwnedDirectory() {
        const auto nonce=std::chrono::steady_clock::now().time_since_epoch().count();
        for(unsigned attempt=0;attempt<100;++attempt) {
            auto candidate=std::filesystem::current_path()/("numeric-zero-"+std::to_string(nonce)+"-"+std::to_string(attempt));
            if(std::filesystem::create_directory(candidate)) { path=std::move(candidate); return; }
        }
        throw std::runtime_error("Cannot create exclusive zero-context test directory");
    }
    ~OwnedDirectory() { std::error_code ignored; std::filesystem::remove_all(path,ignored); }
};
}
int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool condition,const std::string& message) {if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    constexpr std::uint64_t minusZero=0x8000000000000000ULL;
    const std::vector<std::pair<std::string,std::uint64_t>> cases={
        {"-0.00",minusZero},{"-(0.00)",minusZero},{"-(-0.00)",minusZero},{"-(-(-0.00))",minusZero},
        {"-nZero",0},{"-(-nZero)",0},{"-nMinusZero",minusZero},{"-(-nMinusZero)",minusZero},
        {"-ABS(0.00)",0},{"-VAL('0.00')",0},{"-(-VAL('0.00'))",0},
        {"-IIF(.T.,0.00,1.00)",0},{"-ICASE(.T.,0.00,1.00)",0},
        {"-EVALUATE('0.00')",0},{"-EVALUATE('-0.00')",minusZero},
        {"-MAX(0.00,0.00)",0},{"-MIN(0.00,0.00)",0},
        {"-1.00",0xBFF0000000000000ULL},{"-(-1.00)",0x3FF0000000000000ULL}};
    try {
        OwnedDirectory directory;
        for(const auto mode:{"COPPERFIN","VFP9"}) for(const auto decimals:{2,14}) {
            RuntimeSessionOptions options;
            options.startup_path=(directory.path/"zero.prg").string();
            options.working_directory=directory.path.string();
            options.temp_directory=(directory.path/"runtime-temp").string();
            options.require_source_text_overrides=true;
            options.startup_source_text=std::string("SET NUMERICBEHAVIOR TO ")+mode+"\nSET DECIMALS TO "+std::to_string(decimals)+
                "\nLOCAL nZero,nMinusZero,nPause\nnZero=0.00\nnMinusZero=-0.00\nnPause=0\nRETURN\n";
            auto session=PrgRuntimeSession::create(options);
            session.add_breakpoint({options.startup_path,6});
            const auto paused=session.run(DebugResumeAction::continue_run);
            const auto context=std::string(mode)+"/DECIMALS"+std::to_string(decimals);
            expect(paused.reason==DebugPauseReason::breakpoint,context+" paused "+paused.message);
            if(paused.reason!=DebugPauseReason::breakpoint)continue;
            for(const auto& [expression,bits]:cases) {
                const auto watch=session.evaluate_watch_expression(expression);const auto label=context+" "+expression;
                expect(watch.ok,label+" evaluation "+watch.message);
                if(!watch.ok)continue;
                expect(watch.value.kind==PrgValueKind::number&&!watch.value.is_null,label+" kind/non-NULL");
                expect(std::bit_cast<std::uint64_t>(watch.value.number_value)==bits,label+" native payload bits");
            }
            const auto nullValue=session.evaluate_watch_expression("-.NULL.");
            expect(nullValue.ok&&nullValue.value.is_null,context+" NULL unchanged");
            expect(!session.evaluate_watch_expression("-.T.").ok,context+" invalid type unchanged");
            const auto suppressed=session.evaluate_watch_expression("IIF(.T.,0.00,-.T.)");
            expect(suppressed.ok&&suppressed.value.number_value==0.0,context+" suppression unchanged");
            const auto& after=session.state();
            expect(after.reason==paused.reason&&after.executed_statement_count==paused.executed_statement_count&&
                after.statement_text==paused.statement_text&&after.globals.size()==paused.globals.size(),context+" paused state unchanged");
            expect(session.run(DebugResumeAction::continue_run).completed,context+" resumed");

            // A later routine suspends evaluation: the earlier primary cache
            // must retain runtime/constant classification without extra calls.
            options.startup_path=(directory.path/"zero-cache.prg").string();
            options.startup_source_text=std::string("SET NUMERICBEHAVIOR TO ")+mode+"\nSET DECIMALS TO "+std::to_string(decimals)+"\nPUBLIC callCount\ncallCount=0\n"
                "runtimeZero=-(IIF(.T.,0.00,1.00))*LaterOne()\n"
                "constantZero=-(0.00)*LaterOne()\n"
                "negativeZero=-(-0.00)*LaterOne()\nRETURN\n"
                "FUNCTION LaterOne\ncallCount=callCount+1\nRETURN 1.00\nENDFUNC\n";
            auto replay=PrgRuntimeSession::create(options);
            const auto done=replay.run(DebugResumeAction::continue_run);
            expect(done.completed,context+" continuation completed "+done.message);
            for(const auto& [name,bits]:std::vector<std::pair<std::string,std::uint64_t>>{{"runtimezero",0},{"constantzero",minusZero},{"negativezero",minusZero}}) {
                const auto found=done.globals.find(name);expect(found!=done.globals.end(),context+" cached result exists "+name);
                if(found!=done.globals.end())expect(std::bit_cast<std::uint64_t>(found->second.number_value)==bits,context+" cached native payload "+name);
            }
            const auto calls=done.globals.find("callcount");
            expect(calls!=done.globals.end()&&calls->second.number_value==3.0,context+" callbacks exactly once each");
        }
    } catch(const std::exception& error){expect(false,error.what());}
    if(failures){std::cerr<<failures<<" zero-context failures\n";return 1;}
    std::cout<<"Native zero bits and expression cache context: both modes/settings and callback counts passed\n";
    return 0;
}
