// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-REAL-ASSIGNMENT-001 / VR-5611-NUMERIC-REAL-ASSIGNMENT-001.
// Expected constants transcribed from native rows1..46; never use production helper.
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
            auto candidate=std::filesystem::current_path()/("numeric-real-assignment-"+std::to_string(nonce)+"-"+std::to_string(attempt));
            if(std::filesystem::create_directory(candidate)){path=std::move(candidate);return;}
        }
        throw std::runtime_error("Cannot create exclusive real assignment test directory");
    }
    ~OwnedDirectory(){std::error_code ignored;std::filesystem::remove_all(path,ignored);}
};
struct Row{std::string source;std::uint32_t width,decimals;std::uint64_t bits;};
}
int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool condition,const std::string& message){if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    const std::vector<Row> rows={
        {"1.0",12,1,0x3FF0000000000000ULL},
        {"-1.0",13,1,0xBFF0000000000000ULL},
        {"1.00",13,2,0x3FF0000000000000ULL},
        {"-1.00",14,2,0xBFF0000000000000ULL},
        {"1.000",14,3,0x3FF0000000000000ULL},
        {"-1.000",15,3,0xBFF0000000000000ULL},
        {"1.0000",15,4,0x3FF0000000000000ULL},
        {"-1.0000",16,4,0xBFF0000000000000ULL},
        {"1.00000",16,5,0x3FF0000000000000ULL},
        {"-1.00000",17,5,0xBFF0000000000000ULL},
        {"1.000000",17,6,0x3FF0000000000000ULL},
        {"-1.000000",18,6,0xBFF0000000000000ULL},
        {"1.0000000",18,7,0x3FF0000000000000ULL},
        {"-1.0000000",19,7,0xBFF0000000000000ULL},
        {"1.00000000",19,8,0x3FF0000000000000ULL},
        {"-1.00000000",20,8,0xBFF0000000000000ULL},
        {"1.000000000",20,9,0x3FF0000000000000ULL},
        {"-1.000000000",21,9,0xBFF0000000000000ULL},
        {"1.0000000000",21,10,0x3FF0000000000000ULL},
        {"-1.0000000000",22,10,0xBFF0000000000000ULL},
        {"1.00000000000",22,11,0x3FF0000000000000ULL},
        {"-1.00000000000",23,11,0xBFF0000000000000ULL},
        {"1.000000000000",23,12,0x3FF0000000000000ULL},
        {"-1.000000000000",24,12,0xBFF0000000000000ULL},
        {"1.0000000000000",24,13,0x3FF0000000000000ULL},
        {"-1.0000000000000",25,13,0xBFF0000000000000ULL},
        {"1.00000000000000",25,14,0x3FF0000000000000ULL},
        {"-1.00000000000000",26,14,0xBFF0000000000000ULL},
        {"1.000000000000000",26,15,0x3FF0000000000000ULL},
        {"-1.000000000000000",27,15,0xBFF0000000000000ULL},
        {"1.0000000000000000",27,16,0x3FF0000000000000ULL},
        {"-1.0000000000000000",28,16,0xBFF0000000000000ULL},
        {"1.00000000000000000",28,17,0x3FF0000000000000ULL},
        {"-1.00000000000000000",29,17,0xBFF0000000000000ULL},
        {"1.000000000000000000",29,18,0x3FF0000000000000ULL},
        {"-1.000000000000000000",30,18,0xBFF0000000000000ULL},
        {"0.0",12,1,0x0000000000000000ULL},
        {"-0.0",12,1,0x8000000000000000ULL},
        {"0.000000000000000000",29,18,0x0000000000000000ULL},
        {"-0.000000000000000000",29,18,0x8000000000000000ULL},
        {"0.000000000000000001",29,18,0x3C32725DD1D243ACULL},
        {"-0.000000000000000001",30,18,0xBC32725DD1D243ACULL},
        {"9999999999.0",12,1,0x4202A05F1FF80000ULL},
        {"-9999999999.0",13,1,0xC202A05F1FF80000ULL},
        {"000000000000000000000000000000000005.00",13,2,0x4014000000000000ULL},
        {"-000000000000000000000000000000000005.00",14,2,0xC014000000000000ULL}};
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
                lines.push_back("PRIVATE p"+suffix);lines.push_back("PUBLIC g"+suffix);
                for(const auto name:{"l","p","g"})lines.push_back(prefix+name+suffix+"="+rows[i].source);
                lines.push_back(prefix+"c"+suffix+"="+prefix+"l"+suffix);
            }
            lines.push_back("LOCAL nInteger,nUnknown,nNull,nLarge,nPause");
            lines.push_back("nInteger=-5");lines.push_back("nUnknown=1E300");lines.push_back("nNull=.NULL.");
            lines.push_back("nLarge=9999999999.999999999");lines.push_back("nPause=0");
            const auto pauseLine=lines.size();
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
                expect(value.value.numeric_metadata==PrgNumericMetadata{rows[i].width,rows[i].decimals},label+" native assignment descriptor");
                expect(std::bit_cast<std::uint64_t>(value.value.number_value)==rows[i].bits,label+" native payload bits");
            }
            const auto direct=session.evaluate_watch_expression("-0.0");
            expect(direct.ok&&direct.value.numeric_metadata==PrgNumericMetadata{4,1}&&
                std::bit_cast<std::uint64_t>(direct.value.number_value)==0x8000000000000000ULL,context+" direct literal untouched");
            const auto integer=session.evaluate_watch_expression("nInteger");
            expect(integer.ok&&integer.value.numeric_metadata==PrgNumericMetadata{10,0,PrgNumericStorage::integer}&&
                integer.value.number_value==-5.0,context+" Integer assignment unchanged");
            const auto unknown=session.evaluate_watch_expression("nUnknown");
            expect(unknown.ok&&!unknown.value.numeric_metadata,context+" unknown producer not inferred");
            expect(session.evaluate_watch_expression("nNull").value.is_null,context+" NULL unchanged");
            const auto large=session.evaluate_watch_expression("nLarge");
            expect(large.ok&&large.value.numeric_metadata==PrgNumericMetadata{20,9}&&
                std::bit_cast<std::uint64_t>(large.value.number_value)==0x4202A05F20000000ULL,context+" excluded boundary unchanged, not parity");
            const auto& after=session.state();
            expect(after.reason==paused.reason&&after.statement_text==paused.statement_text&&
                after.executed_statement_count==paused.executed_statement_count&&after.globals.size()==paused.globals.size(),context+" watch state unchanged");
            expect(session.run(DebugResumeAction::continue_run).completed,context+" resumed");
        }
    }catch(const std::exception& error){expect(false,error.what());}
    if(failures){std::cerr<<failures<<" real assignment failures\n";return 1;}
    std::cout<<"Native bounded real assignments: modes/settings/scopes/copies/namespaces passed\n";
}
