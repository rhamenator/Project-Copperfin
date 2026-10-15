// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-LITERAL-METADATA-001 / VR-5611-NUMERIC-LITERAL-METADATA-001.
// Expected descriptors/bits come from retained installed-VFP9 observations,
// not the production classifier. This tests direct evaluation, NOT assignment.
#include "copperfin/runtime/prg_engine.h"

#include <bit>
#include <chrono>
#include <cstdint>
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
            auto candidate=std::filesystem::current_path()/
                ("numeric-literal-test-"+std::to_string(nonce)+"-"+std::to_string(attempt));
            if(std::filesystem::create_directory(candidate)) { path=std::move(candidate); return; }
        }
        throw std::runtime_error("Cannot create an exclusive literal test directory");
    }
    ~OwnedDirectory() { std::error_code ignored; std::filesystem::remove_all(path,ignored); }
};
struct LiteralCase {
    std::string expression;
    copperfin::runtime::PrgNumericMetadata metadata;
    std::uint64_t bits;
};
}

int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool condition,const std::string& message) {
        if(!condition) { ++failures; std::cerr<<"FAIL: "<<message<<'\n'; }
    };
    try {
        OwnedDirectory directory;
        const std::vector<LiteralCase> cases={
            {"1.0",{3,1},0x3FF0000000000000ULL},
            {"1.00",{4,2},0x3FF0000000000000ULL},
            {"001.2300",{8,4},0x3FF3AE147AE147AEULL},
            {"0.0000",{6,4},0x0000000000000000ULL},
            {"0.0000000000000000001",{21,18},0x3BFD83C94FB6D2ACULL},
            {"1."+std::string(17,'0'),{19,17},0x3FF0000000000000ULL},
            {"1."+std::string(18,'0'),{20,18},0x3FF0000000000000ULL},
            {"1."+std::string(38,'0'),{40,18},0x3FF0000000000000ULL},
            {"1."+std::string(39,'0'),{40,18},0x3FF0000000000000ULL},
            {"1."+std::string(254,'0'),{40,18},0x3FF0000000000000ULL},
            {std::string(254,'0')+".1",{40,1},0x3FB999999999999AULL},
            {"(1.00)",{4,2},0x3FF0000000000000ULL},
            {"  1.00  ",{4,2},0x3FF0000000000000ULL},
            {"+12.3400",{7,4},0x4028AE147AE147AEULL}
        };
        for(const auto mode:{"COPPERFIN","VFP9"}) for(const auto decimals:{2,14}) {
            RuntimeSessionOptions options;
            options.startup_path=(directory.path/"literal.prg").string();
            options.working_directory=directory.path.string();
            options.temp_directory=(directory.path/"runtime-temp").string();
            options.require_source_text_overrides=true;
            options.startup_source_text=std::string("SET NUMERICBEHAVIOR TO ")+mode+
                "\nSET DECIMALS TO "+std::to_string(decimals)+"\nLOCAL nPause\nnPause = 0\nRETURN\n";
            auto session=PrgRuntimeSession::create(options);
            session.add_breakpoint({options.startup_path,4});
            const auto paused=session.run(DebugResumeAction::continue_run);
            const std::string context=std::string(mode)+"/DECIMALS"+std::to_string(decimals);
            expect(paused.reason==DebugPauseReason::breakpoint,context+" paused: "+paused.message);
            if(paused.reason!=DebugPauseReason::breakpoint) continue;
            for(const auto& test:cases) {
                const auto watch=session.evaluate_watch_expression(test.expression);
                const auto label=context+" "+test.expression;
                expect(watch.ok,label+" evaluation: "+watch.message);
                if(!watch.ok) continue;
                expect(watch.value.kind==PrgValueKind::number && !watch.value.is_null,label+" number/non-NULL");
                expect(watch.value.numeric_metadata==test.metadata,label+" native width/decimal metadata");
                expect(std::bit_cast<std::uint64_t>(watch.value.number_value)==test.bits,label+" native double bits");
            }
            // Existing explicit formatting/arithmetic are controls, not claims
            // that default display or arithmetic metadata has been migrated.
            const auto formatted=session.evaluate_watch_expression("STR(1.00,8,3)");
            expect(formatted.ok && formatted.value.string_value=="   1.000",context+" explicit STR unchanged");
            const auto arithmetic=session.evaluate_watch_expression("1.00 + 2.50");
            expect(arithmetic.ok && arithmetic.value.number_value==3.5,context+" arithmetic payload unchanged");
            const auto malformed=session.evaluate_watch_expression("1..00");
            expect(!malformed.ok,context+" malformed syntax still fails");
            const auto& after=session.state();
            expect(after.reason==paused.reason && after.statement_text==paused.statement_text &&
                after.executed_statement_count==paused.executed_statement_count &&
                after.call_stack.size()==paused.call_stack.size() && after.globals.size()==paused.globals.size(),
                context+" watches preserve paused execution state");
            expect(session.run(DebugResumeAction::continue_run).completed,context+" resumes to completion");
        }
    } catch(const std::exception& error) { expect(false,error.what()); }
    if(failures) { std::cerr<<failures<<" literal metadata failures\n"; return 1; }
    std::cout<<"Direct plain-decimal metadata: both modes/DECIMALS2,14 passed; payload/state controls passed\n";
    return 0;
}
