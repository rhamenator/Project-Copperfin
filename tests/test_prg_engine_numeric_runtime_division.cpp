// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-RUNTIME-DIVISION-001: independent native constants, no helper.
#include "copperfin/runtime/prg_engine.h"
#include "fixtures/vfp9-numeric-division-context/runtime-public-expected.h"
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
    OwnedDirectory(){const auto n=std::chrono::steady_clock::now().time_since_epoch().count();for(unsigned i=0;i<100;++i){auto p=std::filesystem::current_path()/("numeric-runtime-division-"+std::to_string(n)+"-"+std::to_string(i));if(std::filesystem::create_directory(p)){path=std::move(p);return;}}throw std::runtime_error("Cannot create exclusive division test directory");}
    ~OwnedDirectory(){std::error_code ignored;std::filesystem::remove_all(path,ignored);}
};
}
int main(){
    using namespace copperfin::runtime;
    int failures=0;const auto expect=[&](bool ok,const std::string& label){if(!ok){++failures;std::cerr<<"FAIL: "<<label<<'\n';}};
    try{
        OwnedDirectory directory;
        const int settings[]={0,2,6,14,18};
        for(const auto mode:{"COPPERFIN","VFP9"})for(std::size_t d=0;d<5;++d)for(const auto fixed:{"OFF","ON"}){
            std::vector<std::string> lines={std::string("SET NUMERICBEHAVIOR TO ")+mode,"SET DECIMALS TO "+std::to_string(settings[d]),std::string("SET FIXED ")+fixed,
                "LOCAL iFive,iTwo,nFive,nTwo,iZero,nZero,nNegZero,iBoundary,nBoundary,nTiny,nBig,nNegFive,nNegTwo,nPause",
                "iFive=5","iTwo=2","nFive=5.00","nTwo=2.00","iZero=0","nZero=0.0","nNegZero=-0.0","iBoundary=2147483647","nBoundary=2147483647.00","nTiny=0.000000000000000001","nBig=9999999999.0","nNegFive=-5.00","nNegTwo=-2.00"};
            for(unsigned i=1;i<=18;++i){lines.push_back("LOCAL nScale"+std::to_string(i));lines.push_back("nScale"+std::to_string(i)+"=5."+std::string(i,'0'));}
            lines.push_back("nPause=0");
            const auto pauseLine=lines.size();lines.push_back("nPause=1");lines.push_back("RETURN");std::string source;for(const auto& line:lines)source+=line+"\n";
            RuntimeSessionOptions options;options.startup_path=(directory.path/"division.prg").string();options.working_directory=directory.path.string();options.temp_directory=(directory.path/"runtime-temp").string();options.require_source_text_overrides=true;options.startup_source_text=source;
            auto session=PrgRuntimeSession::create(options);session.add_breakpoint({options.startup_path,pauseLine});const auto paused=session.run(DebugResumeAction::continue_run);
            const auto context=std::string(mode)+"/D"+std::to_string(settings[d])+"/FIXED"+fixed;expect(paused.reason==DebugPauseReason::breakpoint,context+" paused "+paused.message);if(paused.reason!=DebugPauseReason::breakpoint)continue;
            for(const auto& row:copperfin::test::kNativeDivisionRows){const auto value=session.evaluate_watch_expression(row.expression);const auto label=context+" "+row.expression;expect(value.ok,label+" evaluates "+value.message);if(!value.ok)continue;expect(value.value.kind==PrgValueKind::number&&!value.value.is_null,label+" ordinary Number");expect(value.value.numeric_metadata==row.metadata[d],label+" native descriptor");expect(std::bit_cast<std::uint64_t>(value.value.number_value)==row.bits,label+" native quotient bits");}
            const auto prefixed=session.evaluate_watch_expression("m.iFive/m.iTwo");expect(prefixed.ok&&prefixed.value.numeric_metadata==copperfin::test::kNativeDivisionRows[0].metadata[d],context+" memory prefix descriptor");
            // Explicitly retained gaps/unchanged routes, not VFP parity claims.
            for(const auto expression:{"5/2","-(5/2)","IIF(.T.,5/2,iFive)","EVALUATE('5/2')","(5/2)/iTwo","iFive/(5/2)","1E300/iTwo","iFive/(1E300)","(iFive*2)/iTwo"}){const auto value=session.evaluate_watch_expression(expression);expect(value.ok&&!value.value.numeric_metadata,context+" unsupported stays unknown: "+expression);}
            expect(session.evaluate_watch_expression(".NULL./iTwo").value.is_null,context+" NULL unchanged");expect(!session.evaluate_watch_expression("iFive/.T.").ok,context+" invalid type unchanged");expect(!session.evaluate_watch_expression("iFive/0").ok,context+" zero divisor unchanged");
            const auto currency=session.evaluate_watch_expression("$5/iTwo");expect(currency.ok&&currency.value.kind==PrgValueKind::currency&&currency.value.currency_value==25000,context+" Currency unchanged");
            const auto suppressed=session.evaluate_watch_expression("IIF(.T.,5,iFive/0)");expect(suppressed.ok&&suppressed.value.number_value==5,context+" suppressed zero divisor unchanged");
            const auto& after=session.state();expect(after.reason==paused.reason&&after.executed_statement_count==paused.executed_statement_count&&after.statement_text==paused.statement_text&&after.globals.size()==paused.globals.size(),context+" watch state unchanged");expect(session.run(DebugResumeAction::continue_run).completed,context+" resumed");
            options.startup_path=(directory.path/"division-cache.prg").string();options.startup_source_text=std::string("SET NUMERICBEHAVIOR TO ")+mode+"\nSET DECIMALS TO "+std::to_string(settings[d])+"\nPUBLIC callCount\ncallCount=0\niFive=5\nanswer=(iFive/2)/LaterTwo()\nRETURN\nFUNCTION LaterTwo\ncallCount=callCount+1\nRETURN 2\nENDFUNC\n";
            auto replay=PrgRuntimeSession::create(options);const auto done=replay.run(DebugResumeAction::continue_run);expect(done.completed,context+" continuation completed "+done.message);const auto calls=done.globals.find("callcount"),answer=done.globals.find("answer");expect(calls!=done.globals.end()&&calls->second.number_value==1,context+" callback once");expect(answer!=done.globals.end()&&answer->second.number_value==1.25,context+" continued quotient");
        }
    }catch(const std::exception& error){expect(false,error.what());}
    if(failures){std::cerr<<failures<<" runtime division failures\n";return 1;}std::cout<<"Native runtime division metadata and unchanged-route controls passed\n";
}
