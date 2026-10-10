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
// RQ-CF-PRG-CAST-INT16-NUMERIC-001 / VR-5611-CAST-INT16-NUMERIC-001/002.
// Owner-derived extension policy/native truncation/independent integer math;
// literal expectations precede Copperfin execution, not sibling CAST outputs.
struct NumericRow { const char* text; double raw; std::optional<std::int64_t> expected; };
const std::vector<NumericRow> numeric_rows{
    {"0",0,0},
    {"-0",-0.,0},
    {"0.49",0.49,0},
    {"0.5",0.5,0},
    {"0.9",0.9,0},
    {"1.49",1.49,1},
    {"1.5",1.5,1},
    {"1.9",1.9,1},
    {"-0.49",-0.49,0},
    {"-0.5",-0.5,0},
    {"-0.9",-0.9,0},
    {"-1.49",-1.49,-1},
    {"-1.5",-1.5,-1},
    {"-1.9",-1.9,-1},
    {"32766",32766.,32766},
    {"32767",32767.,32767},
    {"32767.49",32767.49,32767},
    {"32767.5",32767.5,32767},
    {"32767.9",32767.9,32767},
    {"32767.999999999996",32767.999999999996,32767},
    {"32768",32768.,{}},
    {"32768.00000000001",32768.00000000001,{}},
    {"32768.9",32768.9,{}},
    {"-32767",-32767.,-32767},
    {"-32768",-32768.,-32768},
    {"-32768.49",-32768.49,-32768},
    {"-32768.5",-32768.5,-32768},
    {"-32768.9",-32768.9,-32768},
    {"-32768.99999999999",-32768.99999999999,-32768},
    {"-32769",-32769.,{}},
    {"-32769.00000000001",-32769.00000000001,{}},
    {"-32769.9",-32769.9,{}},
    {"65535",65535.,{}},
    {"65536",65536.,{}},
    {"-65536",-65536.,{}},
    {"2147483647",2147483647.,{}},
    {"2147483648",2147483648.,{}},
    {"-2147483649",-2147483649.,{}},
    {"9007199254740992",9007199254740992.,{}},
    {"9007199254740993",9007199254740992.,{}},
    {"1E20",1E20,{}},
    {"-1E20",-1E20,{}},
    {"1E300",1E300,{}},
    {"-1E300",-1E300,{}},
    {"EXP(1000)",std::numeric_limits<double>::infinity(),{}},
    {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{}}
};
int direct_count=0, public_count=0;
void direct_cases() {
    for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
        // Adapter intentionally has no mode parameter: extension policy is identical.
        const auto check=[&](const PrgValue& input,std::optional<std::int64_t> expected,const char* tag) {
            ++direct_count;
            expect(checked_cast_int16_numeric_argument(input)==expected,
                   std::string("INT16 direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+tag);
        };
        for(const auto& row:numeric_rows)check(make_number_value(row.raw),row.expected,row.text);
        check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},"NaN containment");
        check(make_number_value(std::nextafter(32768.,0.)),32767,"upper inner nextafter");
        check(make_number_value(std::nextafter(32768.,std::numeric_limits<double>::infinity())),{},"upper outer nextafter");
        check(make_number_value(std::nextafter(-32769.,0.)),-32768,"lower inner nextafter");
        check(make_number_value(std::nextafter(-32769.,-std::numeric_limits<double>::infinity())),{},"lower outer nextafter");
        struct SignedRow { std::int64_t raw; std::optional<std::int64_t> expected; };
        for(const auto& row:std::vector<SignedRow>{
            {0,0},{1,1},{-1,-1},{32766,32766},{32767,32767},{32768,{}},
            {-32767,-32767},{-32768,-32768},{-32769,{}},{65536,{}},{-65536,{}},
            {9007199254740993LL,{}},{-9007199254740993LL,{}},{INT64_MIN,{}},{INT64_MAX,{}},{INT64_MAX-1,{}}})
            check(make_int64_value(row.raw),row.expected,"exact signed64");
        struct UnsignedRow { std::uint64_t raw; std::optional<std::int64_t> expected; };
        for(const auto& row:std::vector<UnsignedRow>{
            {0,0},{1,1},{32766,32766},{32767,32767},{32768,{}},{65535,{}},{65536,{}},
            {4294967297ULL,{}},{9007199254740993ULL,{}},{9223372036854775808ULL,{}},
            {UINT64_MAX-1,{}},{UINT64_MAX,{}}})
            check(make_uint64_value(row.raw),row.expected,"exact unsigned64");
        check(make_boolean_value(true),{},"non-Numeric adapter boundary");
        check(make_string_value("1"),{},"Character adapter boundary");
    }
}
struct ExactRow { const char* expression; std::optional<std::int64_t> expected; };
const std::vector<ExactRow> exact_rows{
    {"CAST(0 AS INT64)",0}, {"CAST(32767 AS INT64)",32767},
    {"CAST(-32768 AS INT64)",-32768}, {"CAST(-1 AS INT64)",-1},
    {"CAST(32767 AS UINT64)",32767}, {"CAST(32768 AS INT64)",{}},
    {"CAST(-32769 AS INT64)",{}}, {"CAST(32768 AS UINT64)",{}},
    {"CAST(65536 AS INT64)",{}}, {"CAST(-65536 AS INT64)",{}},
    {"CAST(65536 AS UINT64)",{}},
    {"(CAST(9007199254740992 AS INT64)+CAST(1 AS INT64))",{}},
    {"(CAST(-9007199254740992 AS INT64)-CAST(1 AS INT64))",{}},
    {"(CAST(9007199254740992 AS UINT64)+CAST(1 AS UINT64))",{}},
    {"(CAST(9223372036854774784 AS INT64)+CAST(1023 AS INT64))",{}},
    {"CAST(-9223372036854775808 AS INT64)",{}},
    {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{}},
    {"(CAST(18446744073709549568 AS UINT64)+CAST(2046 AS UINT64))",{}},
    {"CAST(0 AS UINT64)",0}, {"CAST(1 AS UINT64)",1}
};
std::string value(const RuntimePauseState& state,const char* key) {
    const auto found=state.globals.find(key);
    return found==state.globals.end()?"<missing>":format_value(found->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,const std::string& expression,
                 std::optional<std::int64_t> expected,const std::string& expected_message={},int expected_error=11) {
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
    expect(state.completed,"INT16 completion "+tag+": "+state.message);
    expect(value(state,"ncode")== (expected?"0":std::to_string(expected_error)),"INT16 selected error "+tag+" got "+value(state,"ncode"));
    expect(value(state,"nresult")== (expected?std::to_string(*expected):"12345"),"INT16 selected result "+tag+" got "+value(state,"nresult"));
    const auto result=state.globals.find("nresult");
    expect(result!=state.globals.end()&&result->second.kind==
           (expected?PrgValueKind::int64:PrgValueKind::number),"INT16 selected kind/assignment "+tag);
    expect(value(state,"lafter")=="true"&&value(state,"nguard")=="1","INT16 continuation/control "+tag);
    const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
    expect(value(state,"cbefore")==context&&value(state,"cafter")==context,"INT16 mode/session/cursor "+tag);
    expect(value(state,"creset")=="1|COPPERFIN|","INT16 cleanup/reset "+tag);
    if(expected)expect(value(state,"cmessage").empty(),"INT16 successful message "+tag);
    if(!expected_message.empty())expect(value(state,"cmessage")==expected_message,"INT16 selected localized message "+tag);
}
void public_cases(const std::string& mode,int selected) {
    const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto dir=fs::temp_directory_path()/("copperfin_cast_int16_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
    fs::create_directories(dir);
    const auto path=dir/"query.prg";
    for(const char* alias:{"INT16","SHORT"}) {
        const auto cast=[&](const std::string& input){return "CAST("+input+" AS "+alias+")";};
        for(const auto& row:numeric_rows)public_case(path,mode,selected,cast(row.text),row.expected);
        for(const auto& row:exact_rows)public_case(path,mode,selected,cast(row.expression),row.expected);
        // Untouched non-Numeric path: Character32768 retains its existing unbounded result.
        for(const auto& row:std::vector<std::pair<const char*,std::int64_t>>{
                {".T.",1},{".F.",0},{"'1.9'",1},{"$1.5",1},{"'32768'",32768}})
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
        expect(direct_count==162&&public_count==0,"INT16 complete direct shard count");
    } else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
              (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
        public_cases(argv[1],std::string(argv[2])=="1"?1:2);
        expect(direct_count==0&&public_count==152,"INT16 complete public shard count");
    } else {
        std::cerr<<"Expected no arguments (direct), or COPPERFIN|VFP9 1|2 (public shard)\n";
        return 2;
    }
    std::cout<<"INT16 frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
    return test_failures()==0?0:1;
}
