// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-INTEGER-ASSIGNMENT-001 / VR-5611-NUMERIC-INTEGER-ASSIGNMENT-001.
// Expected table transcribed from retained native rows; no production helper.
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
            auto candidate=std::filesystem::current_path()/("numeric-assignment-"+std::to_string(nonce)+"-"+std::to_string(attempt));
            if(std::filesystem::create_directory(candidate)){path=std::move(candidate);return;}
        }
        throw std::runtime_error("Cannot create exclusive Integer assignment test directory");
    }
    ~OwnedDirectory(){std::error_code ignored;std::filesystem::remove_all(path,ignored);}
};
struct Row{std::string source;std::uint32_t width;std::uint64_t bits;};
}
int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool condition,const std::string& message){if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    const std::vector<Row> rows={
        {"0",10,0},{"-0",10,0},{"1",10,0x3FF0000000000000ULL},{"-1",10,0xBFF0000000000000ULL},
        {"5",10,0x4014000000000000ULL},{"-5",10,0xC014000000000000ULL},
        {"2147483647",10,0x41DFFFFFFFC00000ULL},{"-2147483647",11,0xC1DFFFFFFFC00000ULL},
        {"00005",10,0x4014000000000000ULL},{"-999999999",10,0xC1CDCD64FF800000ULL},
        {"-1000000000",11,0xC1CDCD6500000000ULL},{"999999999",10,0x41CDCD64FF800000ULL},
        {"1000000000",10,0x41CDCD6500000000ULL},{"-2147483646",11,0xC1DFFFFFFF800000ULL},
        {"-0000000001",10,0xBFF0000000000000ULL},{"-0000000000",10,0},
        {"-(-2147483647)",10,0x41DFFFFFFFC00000ULL},{"-(-00005)",10,0x4014000000000000ULL}};
    try {
        OwnedDirectory directory;
        for(const auto mode:{"COPPERFIN","VFP9"})for(const auto decimals:{2,14})
        for(const auto fixed:{"OFF","ON"})for(const bool memoryPrefix:{false,true}) {
            const std::string prefix=memoryPrefix?"m.":"";
            std::vector<std::string> lines={std::string("SET NUMERICBEHAVIOR TO ")+mode,
                "SET DECIMALS TO "+std::to_string(decimals),std::string("SET FIXED ")+fixed};
            for(std::size_t i=0;i<rows.size();++i){
                const auto suffix=std::to_string(i);
                lines.push_back("LOCAL l"+suffix+",c"+suffix);
                lines.push_back("PRIVATE p"+suffix);
                lines.push_back("PUBLIC g"+suffix);
                for(const auto name:{"l","p","g"})lines.push_back(prefix+name+suffix+"="+rows[i].source);
                lines.push_back(prefix+"c"+suffix+"="+prefix+"l"+suffix);
            }
            lines.push_back("LOCAL nReal,nUnknown,nNull,nPause");
            lines.push_back("nReal=-0.0");lines.push_back("nUnknown=1E300");lines.push_back("nNull=.NULL.");
            lines.push_back("nPause=0");
            const auto pauseLine=static_cast<int>(lines.size());
            lines.push_back("nPause=1");lines.push_back("RETURN");
            std::string source;for(const auto& line:lines)source+=line+"\n";
            RuntimeSessionOptions options;
            options.startup_path=(directory.path/"assignment.prg").string();options.working_directory=directory.path.string();
            options.temp_directory=(directory.path/"runtime-temp").string();options.require_source_text_overrides=true;
            options.startup_source_text=source;
            auto session=PrgRuntimeSession::create(options);session.add_breakpoint({options.startup_path,pauseLine});
            const auto paused=session.run(DebugResumeAction::continue_run);
            const auto context=std::string(mode)+"/D"+std::to_string(decimals)+"/FIXED"+fixed+"/"+prefix;
            expect(paused.reason==DebugPauseReason::breakpoint,context+" paused "+paused.message);
            if(paused.reason!=DebugPauseReason::breakpoint)continue;
            for(std::size_t i=0;i<rows.size();++i)for(const auto name:{"l","p","g","c"}){
                const auto label=context+" "+name+std::to_string(i)+"="+rows[i].source;
                const auto value=session.evaluate_watch_expression(prefix+name+std::to_string(i));
                expect(value.ok,label+" evaluation "+value.message);if(!value.ok)continue;
                expect(value.value.kind==PrgValueKind::number&&!value.value.is_null,label+" number/non-NULL");
                expect(value.value.numeric_metadata==PrgNumericMetadata{rows[i].width,0U,PrgNumericStorage::integer},label+" native assignment descriptor");
                expect(std::bit_cast<std::uint64_t>(value.value.number_value)==rows[i].bits,label+" native payload bits");
            }
            const auto direct=session.evaluate_watch_expression("5");
            expect(direct.ok&&direct.value.numeric_metadata==PrgNumericMetadata{1,0,PrgNumericStorage::integer},context+" direct literal untouched");
            const auto real=session.evaluate_watch_expression("nReal");
            expect(real.ok&&real.value.numeric_metadata==PrgNumericMetadata{4,1}&&std::bit_cast<std::uint64_t>(real.value.number_value)==0x8000000000000000ULL,context+" real assignment not migrated");
            const auto unknown=session.evaluate_watch_expression("nUnknown");
            expect(unknown.ok&&!unknown.value.numeric_metadata,context+" unknown producer not inferred");
            expect(session.evaluate_watch_expression("nNull").value.is_null,context+" NULL unchanged");
            const auto& after=session.state();
            expect(after.reason==paused.reason&&after.statement_text==paused.statement_text&&after.executed_statement_count==paused.executed_statement_count&&after.globals.size()==paused.globals.size(),context+" watch state unchanged");
            expect(session.run(DebugResumeAction::continue_run).completed,context+" resumed");
        }
    }catch(const std::exception& error){expect(false,error.what());}
    if(failures){std::cerr<<failures<<" Integer assignment failures\n";return 1;}
    std::cout<<"Native Integer scalar assignments: modes/settings/scopes/copies/namespaces passed\n";
}
