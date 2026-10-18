// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001: native expectations BEFORE hooks.
#include "copperfin/runtime/prg_engine.h"
#include "fixtures/vfp9-numeric-literal-payload/native-expected.h"

#include <bit>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
struct OwnedDirectory {
    std::filesystem::path path;
    OwnedDirectory() {
        const auto stamp=std::chrono::steady_clock::now().time_since_epoch().count();
        for(unsigned attempt=0;attempt<100;++attempt) {
            auto candidate=std::filesystem::current_path()/
                ("numeric-literal-payload-"+std::to_string(stamp)+"-"+std::to_string(attempt));
            if(std::filesystem::create_directory(candidate)){path=std::move(candidate);return;}
        }
        throw std::runtime_error("Cannot create exclusive literal payload test directory");
    }
    ~OwnedDirectory(){std::error_code ignored;std::filesystem::remove_all(path,ignored);}
};
std::string hex(std::uint64_t bits) {
    std::ostringstream out;
    out<<std::hex<<std::uppercase<<std::setfill('0')<<std::setw(16)<<bits;
    return out.str();
}
}

int main() {
    using namespace copperfin::runtime;
    unsigned failures=0,descriptorFailures=0,bitFailures=0,evaluationFailures=0,controlFailures=0;
    const auto expect=[&](bool condition,const std::string& label,unsigned& category) {
        if(!condition){++failures;++category;std::cerr<<"FAIL: "<<label<<'\n';}
    };
    try {
        OwnedDirectory directory;
        const int settings[]={0,18};
        for(const auto mode:{"COPPERFIN","VFP9"})for(std::size_t setting=0;setting<2;++setting)
            for(const auto fixed:{"OFF","ON"}) {
                RuntimeSessionOptions options;
                options.startup_path=(directory.path/"literal-payload.prg").string();
                options.working_directory=directory.path.string();
                options.temp_directory=(directory.path/"runtime-temp").string();
                options.require_source_text_overrides=true;
                options.startup_source_text=std::string("SET NUMERICBEHAVIOR TO ")+mode+
                    "\nSET DECIMALS TO "+std::to_string(settings[setting])+"\nSET FIXED "+fixed+
                    "\nLOCAL nPause\nnPause=0\nnPause=1\nRETURN\n";
                const auto context=std::string(mode)+"/D"+std::to_string(settings[setting])+"/FIXED"+fixed;
                auto session=PrgRuntimeSession::create(options);
                session.add_breakpoint({options.startup_path,6});
                const auto paused=session.run(DebugResumeAction::continue_run);
                expect(paused.reason==DebugPauseReason::breakpoint,context+" paused "+paused.message,controlFailures);
                if(paused.reason!=DebugPauseReason::breakpoint)continue;
                unsigned selected=0;
                for(const auto& row:copperfin::test::native_literal_payload_rows) {
                    ++selected; // All native rows are assertions, including calibration overlaps.
                    const auto result=session.evaluate_watch_expression(row.expression);
                    const auto label=context+" "+row.label+" "+row.expression;
                    expect(result.ok,label+" evaluates "+result.message,evaluationFailures);
                    if(!result.ok)continue;
                    expect(result.value.kind==PrgValueKind::number&&!result.value.is_null,
                           label+" ordinary Number",evaluationFailures);
                    if(result.value.kind!=PrgValueKind::number||result.value.is_null)continue;
                    expect(result.value.numeric_metadata==row.metadata[setting],
                           label+" native descriptor",descriptorFailures);
                    const auto actual=std::bit_cast<std::uint64_t>(result.value.number_value);
                    expect(actual==row.bits,label+" bits actual="+hex(actual)+" native="+hex(row.bits),bitFailures);
                }
                expect(selected==1193,context+" exact native inventory",controlFailures);
                const auto scientific=session.evaluate_watch_expression("1E-14");
                expect(scientific.ok&&scientific.value.number_value==1E-14&&!scientific.value.numeric_metadata,
                       context+" unmigrated scientific producer unchanged",controlFailures);
                const auto integer=session.evaluate_watch_expression("5");
                expect(integer.ok&&integer.value.number_value==5&&integer.value.numeric_metadata==
                           PrgNumericMetadata{1,0,PrgNumericStorage::integer},
                       context+" Integer producer unchanged",controlFailures);
                const auto currency=session.evaluate_watch_expression("$1.2500");
                expect(currency.ok&&currency.value.kind==PrgValueKind::currency&&
                           currency.value.currency_value==12500,context+" Currency unchanged",controlFailures);
                expect(session.evaluate_watch_expression(".NULL.").value.is_null,
                       context+" NULL unchanged",controlFailures);
                expect(!session.evaluate_watch_expression("1..0").ok,
                       context+" malformed grammar unchanged",controlFailures);
                const auto overlong=session.evaluate_watch_expression(std::string(255,'0')+".1");
                expect(overlong.ok&&!overlong.value.numeric_metadata,
                       context+" retained overlong metadata boundary (not native parity)",controlFailures);
                const auto suppressed=session.evaluate_watch_expression("IIF(.T.,1.25,1/0)");
                expect(suppressed.ok&&suppressed.value.number_value==1.25,
                       context+" suppressed branch unchanged",controlFailures);
                const auto& after=session.state();
                expect(after.reason==paused.reason&&after.statement_text==paused.statement_text&&
                           after.executed_statement_count==paused.executed_statement_count&&
                           after.globals.size()==paused.globals.size()&&after.call_stack.size()==paused.call_stack.size(),
                       context+" watch state unchanged",controlFailures);
                expect(session.run(DebugResumeAction::continue_run).completed,
                       context+" resumes",controlFailures);
            }
    }catch(const std::exception& error){expect(false,error.what(),controlFailures);}
    std::cout<<"literal payload: descriptor="<<descriptorFailures<<" bits="<<bitFailures
             <<" evaluation="<<evaluationFailures<<" controls="<<controlFailures<<" total="<<failures<<'\n';
    return failures?1:0;
}
