// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-INTEGER-DESCRIPTOR-001 / VR-5611-NUMERIC-INTEGER-DESCRIPTOR-001.
// Independent retained native direct descriptors/bits, NOT assignment width/display acceptance.
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
            auto candidate=std::filesystem::current_path()/("numeric-integer-"+std::to_string(nonce)+"-"+std::to_string(attempt));
            if(std::filesystem::create_directory(candidate)){path=std::move(candidate);return;}
        }
        throw std::runtime_error("Cannot create exclusive Integer metadata test directory");
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
        {"0",1,0},{"5",1,0x4014000000000000ULL},{"0005",4,0x4014000000000000ULL},
        {"0000000005",10,0x4014000000000000ULL},{"2147483646",10,0x41DFFFFFFF800000ULL},
        {"2147483647",10,0x41DFFFFFFFC00000ULL},{"-0",2,0},{"-(-0)",3,0},
        {"-(-5)",3,0x4014000000000000ULL},{"-2147483647",11,0xC1DFFFFFFFC00000ULL},
        {"-(-2147483647)",12,0x41DFFFFFFFC00000ULL},{"+0",1,0},{"+(5)",1,0x4014000000000000ULL},
        {"(2147483647)",10,0x41DFFFFFFFC00000ULL},{"-0005",5,0xC014000000000000ULL},
        {"-0000000005",11,0xC014000000000000ULL},{"-(-0005)",6,0x4014000000000000ULL},
        {"+0005",4,0x4014000000000000ULL}};
    try {
        OwnedDirectory directory;
        for(const auto mode:{"COPPERFIN","VFP9"})for(const auto decimals:{2,14}) {
            RuntimeSessionOptions options;
            options.startup_path=(directory.path/"integer.prg").string();options.working_directory=directory.path.string();
            options.temp_directory=(directory.path/"runtime-temp").string();options.require_source_text_overrides=true;
            options.startup_source_text=std::string("SET NUMERICBEHAVIOR TO ")+mode+"\nSET DECIMALS TO "+std::to_string(decimals)+"\nLOCAL nZero,nPause\nnZero=0\nnPause=0\nRETURN\n";
            auto session=PrgRuntimeSession::create(options);session.add_breakpoint({options.startup_path,5});
            const auto paused=session.run(DebugResumeAction::continue_run);const auto context=std::string(mode)+"/DECIMALS"+std::to_string(decimals);
            expect(paused.reason==DebugPauseReason::breakpoint,context+" paused "+paused.message);if(paused.reason!=DebugPauseReason::breakpoint)continue;
            for(const auto& row:rows){const auto value=session.evaluate_watch_expression(row.source);const auto label=context+" "+row.source;
                expect(value.ok,label+" evaluation "+value.message);if(!value.ok)continue;
                expect(value.value.kind==PrgValueKind::number&&!value.value.is_null,label+" number/non-NULL");
                expect(value.value.numeric_metadata==PrgNumericMetadata{row.width,0,PrgNumericStorage::integer},label+" native typed descriptor");
                expect(std::bit_cast<std::uint64_t>(value.value.number_value)==row.bits,label+" native payload bits");}
            const auto real=session.evaluate_watch_expression("-0.0");
            expect(real.ok&&real.value.numeric_metadata==PrgNumericMetadata{4,1}&&std::bit_cast<std::uint64_t>(real.value.number_value)==0x8000000000000000ULL,context+" real zero rule unchanged");
            const auto runtime=session.evaluate_watch_expression("-nZero");
            expect(runtime.ok&&runtime.value.kind==PrgValueKind::number&&std::bit_cast<std::uint64_t>(runtime.value.number_value)==0,context+" native runtime Integer zero bits");
            // Width normalization on assignment is NOT accepted here.
            for(const auto source:{"00000000005","2147483648"}){const auto value=session.evaluate_watch_expression(source);expect(value.ok&&!value.value.numeric_metadata,context+" unsupported producer stays unknown "+source);}
            const auto nullValue=session.evaluate_watch_expression("-.NULL.");expect(nullValue.ok&&nullValue.value.is_null,context+" NULL unchanged");
            expect(!session.evaluate_watch_expression("-.T.").ok,context+" invalid type unchanged");
            const auto suppressed=session.evaluate_watch_expression("IIF(.T.,0,-.T.)");expect(suppressed.ok&&suppressed.value.number_value==0.0,context+" suppression unchanged");
            const auto arithmetic=session.evaluate_watch_expression("5/2");expect(arithmetic.ok&&arithmetic.value.number_value==2.5,context+" arithmetic payload unchanged, precision not asserted");
            const auto& after=session.state();expect(after.reason==paused.reason&&after.statement_text==paused.statement_text&&after.executed_statement_count==paused.executed_statement_count&&after.globals.size()==paused.globals.size(),context+" paused state unchanged");
            expect(session.run(DebugResumeAction::continue_run).completed,context+" resumed");
        }
    }catch(const std::exception& error){expect(false,error.what());}
    if(failures){std::cerr<<failures<<" Integer descriptor failures\n";return 1;}
    std::cout<<"Native staged Integer descriptors/bits: both modes/settings passed; controls unchanged\n";
}
