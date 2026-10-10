// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-CAST-BYTE-NUMERIC-001 / VR-5611-CAST-BYTE-NUMERIC-001/002.
// Owner-derived extension policy/native truncation/independent integer math;
// literal expectations precede Copperfin execution, not sibling CAST outputs.
struct NumericRow { const char* text; double raw; std::optional<std::uint64_t> expected; };
const std::vector<NumericRow> numeric_rows{
    {"0",0,0}, {"-0",-0.,0},
    {"0.49",0.49,0}, {"0.5",0.5,0}, {"0.9",0.9,0},
    {"1.49",1.49,1}, {"1.5",1.5,1}, {"1.9",1.9,1},
    {"-0.49",-0.49,0}, {"-0.5",-0.5,0}, {"-0.9",-0.9,0},
    {"-0.9999999999999999",-0.9999999999999999,0},
    {"-1.0000000000000002",-1.0000000000000002,{}},
    {"-1",-1.,{}}, {"-1.49",-1.49,{}}, {"-1.5",-1.5,{}}, {"-1.9",-1.9,{}},
    {"127",127.,127}, {"128",128.,128}, {"254",254.,254}, {"255",255.,255},
    {"255.9",255.9,255}, {"255.99999999999997",255.99999999999997,255},
    {"256",256.,{}}, {"256.00000000000006",256.00000000000006,{}}, {"256.9",256.9,{}},
    {"257",257.,{}}, {"-255",-255.,{}}, {"-256",-256.,{}},
    {"65535",65535.,{}}, {"65536",65536.,{}},
    {"9007199254740992",9007199254740992.,{}}, {"9007199254740993",9007199254740992.,{}},
    {"1E20",1E20,{}}, {"-1E20",-1E20,{}}, {"1E300",1E300,{}}, {"-1E300",-1E300,{}},
    {"EXP(1000)",std::numeric_limits<double>::infinity(),{}},
    {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{}}
};
int direct_count=0, public_count=0;
void direct_cases() {
    for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
        // Adapter intentionally has no mode parameter: extension policy is identical.
        const auto check=[&](const PrgValue& input,std::optional<std::uint64_t> expected,const char* tag) {
            ++direct_count;
            expect(checked_cast_byte_numeric_argument(input)==expected,
                   std::string("BYTE direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+tag);
        };
        for(const auto& row:numeric_rows)check(make_number_value(row.raw),row.expected,row.text);
        check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},"NaN containment");
        check(make_number_value(std::nextafter(256.,0.)),255,"upper inner nextafter");
        check(make_number_value(std::nextafter(256.,std::numeric_limits<double>::infinity())),{},"upper outer nextafter");
        check(make_number_value(std::nextafter(-1.,0.)),0,"lower inner nextafter");
        check(make_number_value(std::nextafter(-1.,-std::numeric_limits<double>::infinity())),{},"lower outer nextafter");
        struct SignedRow { std::int64_t raw; std::optional<std::uint64_t> expected; };
        for(const auto& row:std::vector<SignedRow>{
            {0,0},{1,1},{254,254},{255,255},{256,{}},{-1,{}},{-255,{}},{-256,{}},
            {9007199254740993LL,{}},{-9007199254740993LL,{}},{INT64_MIN,{}},{INT64_MAX,{}},{INT64_MAX-1,{}}})
            check(make_int64_value(row.raw),row.expected,"exact signed64");
        struct UnsignedRow { std::uint64_t raw; std::optional<std::uint64_t> expected; };
        for(const auto& row:std::vector<UnsignedRow>{
            {0,0},{1,1},{254,254},{255,255},{256,{}},{257,{}},{4294967297ULL,{}},
            {9007199254740993ULL,{}},{9223372036854775808ULL,{}},{UINT64_MAX-1,{}},{UINT64_MAX,{}}})
            check(make_uint64_value(row.raw),row.expected,"exact unsigned64");
        check(make_boolean_value(true),{},"non-Numeric adapter boundary");
        check(make_string_value("1"),{},"Character adapter boundary");
    }
}
struct ExactRow { const char* expression; std::optional<std::uint64_t> expected; };
const std::vector<ExactRow> exact_rows{
    {"CAST(0 AS INT64)",0}, {"CAST(255 AS INT64)",255}, {"CAST(255 AS UINT64)",255},
    {"CAST(256 AS INT64)",{}}, {"CAST(256 AS UINT64)",{}}, {"CAST(-1 AS INT64)",{}},
    {"(CAST(9007199254740992 AS INT64)+CAST(1 AS INT64))",{}},
    {"(CAST(-9007199254740992 AS INT64)-CAST(1 AS INT64))",{}},
    {"(CAST(9007199254740992 AS UINT64)+CAST(1 AS UINT64))",{}},
    {"(CAST(9223372036854774784 AS INT64)+CAST(1023 AS INT64))",{}},
    {"CAST(-9223372036854775808 AS INT64)",{}},
    {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{}},
    {"(CAST(18446744073709549568 AS UINT64)+CAST(2046 AS UINT64))",{}},
    {"CAST(0 AS UINT64)",0}
};
std::string value(const RuntimePauseState& state,const char* key) {
    const auto found=state.globals.find(key);
    return found==state.globals.end()?"<missing>":format_value(found->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,const std::string& expression,
                 std::optional<std::uint64_t> expected,const std::string& expected_message={},int expected_error=11) {
    ++public_count;
    const auto other=mode=="VFP9"?"COPPERFIN":"VFP9";
    const auto tag=mode+" session"+std::to_string(selected)+" "+expression;
    write_text(path,(selected==2?std::string("SET NUMERICBEHAVIOR TO ")+other+"\nSET DATASESSION TO 2\n":"")+
        "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC nResult,nCode,lAfter,cMessage,cBefore,cAfter,cReset,nGuard\n"
        "nResult=12345\nnCode=0\nlAfter=.F.\ncMessage=''\n"
        "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
        "cBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
        "TRY\nnResult="+expression+"\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\n"
        "lAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\n"
        "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
        "USE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
        "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
    auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
    const auto state=session.run(DebugResumeAction::continue_run);
    expect(state.completed,"BYTE completion "+tag+": "+state.message);
    expect(value(state,"ncode")== (expected?"0":std::to_string(expected_error)),"BYTE selected error "+tag+" got "+value(state,"ncode"));
    expect(value(state,"nresult")== (expected?std::to_string(*expected):"12345"),"BYTE selected result "+tag+" got "+value(state,"nresult"));
    const auto result=state.globals.find("nresult");
    expect(result!=state.globals.end()&&result->second.kind==
           (expected?PrgValueKind::uint64:PrgValueKind::number),"BYTE selected kind/assignment "+tag);
    expect(value(state,"lafter")=="true"&&value(state,"nguard")=="1","BYTE continuation/control "+tag);
    const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
    expect(value(state,"cbefore")==context&&value(state,"cafter")==context,"BYTE mode/session/cursor "+tag);
    expect(value(state,"creset")=="1|COPPERFIN|","BYTE cleanup/reset "+tag);
    if(expected)expect(value(state,"cmessage").empty(),"BYTE successful message "+tag);
    if(!expected_message.empty())expect(value(state,"cmessage")==expected_message,"BYTE selected localized message "+tag);
}
void public_cases(const std::string& mode,int selected) {
    const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto dir=fs::temp_directory_path()/("copperfin_cast_byte_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
    fs::create_directories(dir);
    const auto path=dir/"query.prg";
    for(const char* alias:{"BYTE","UINT8"}) {
        const auto cast=[&](const std::string& input){return "CAST("+input+" AS "+alias+")";};
        for(const auto& row:numeric_rows)public_case(path,mode,selected,cast(row.text),row.expected);
        for(const auto& row:exact_rows)public_case(path,mode,selected,cast(row.expression),row.expected);
        // Untouched non-Numeric path: Character256 retains its existing low8 result.
        for(const auto& row:std::vector<std::pair<const char*,std::uint64_t>>{
                {".T.",1},{".F.",0},{"'1.9'",1},{"$1.5",1},{"'256'",0}})
            public_case(path,mode,selected,cast(row.first),row.second);
        public_case(path,mode,selected,cast("'x'"),{},std::string{},1);
        for(const auto& row:std::vector<std::pair<std::string,std::string>>{
                {"en-US","Function argument value, type, or count is invalid."},
                {"es-419","El valor, tipo o numero de un argumento de funcion no es valido."},
                {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido."},
                {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]"}}) {
            set_env_value("COPPERFIN_LOCALE",row.first,true);
            public_case(path,mode,selected,cast("1E300"),{},row.second);
        }
        set_env_value("COPPERFIN_LOCALE","en-US",true);
    }
    std::error_code ignored;
    fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv) {
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    if(argc==1) {
        direct_cases();
        expect(direct_count==140&&public_count==0,"BYTE complete direct shard count");
    } else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
              (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
        public_cases(argv[1],std::string(argv[2])=="1"?1:2);
        expect(direct_count==0&&public_count==126,"BYTE complete public shard count");
    } else {
        std::cerr<<"Expected no arguments (direct), or COPPERFIN|VFP9 1|2 (public shard)\n";
        return 2;
    }
    std::cout<<"BYTE frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
    return test_failures()==0?0:1;
}
