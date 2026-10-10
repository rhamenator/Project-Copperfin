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
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-CAST-INT64-NUMERIC-001 / VR-5611-CAST-INT64-NUMERIC-001/002.
// Literal extension expectations come from owner policy and integer mathematics,
// not Copperfin output. Native probe controls document absence, not native64 parity.
struct NumericRow { const char* text; double raw; std::optional<std::int64_t> expected; };
const std::vector<NumericRow> numeric_rows{
    {"0",0,0}, {"-0",-0.,0},
    {"0.49",0.49,0}, {"0.5",0.5,0}, {"0.9",0.9,0},
    {"1.49",1.49,1}, {"1.5",1.5,1}, {"1.9",1.9,1},
    {"-0.49",-0.49,0}, {"-0.5",-0.5,0}, {"-0.9",-0.9,0},
    {"-1.49",-1.49,-1}, {"-1.5",-1.5,-1}, {"-1.9",-1.9,-1},
    {"2147483648",2147483648.,2147483648LL},
    {"-2147483648",-2147483648.,-2147483648LL},
    {"9007199254740992",9007199254740992.,9007199254740992LL},
    {"-9007199254740992",-9007199254740992.,-9007199254740992LL},
    {"9223372036854774784",9223372036854774784.,9223372036854774784LL},
    {"9223372036854775807",9223372036854775808.,{}},
    {"9223372036854775808",9223372036854775808.,{}},
    {"-9223372036854774784",-9223372036854774784.,-9223372036854774784LL},
    {"-9223372036854775808",-9223372036854775808.,INT64_MIN},
    {"-9223372036854777856",-9223372036854777856.,{}},
    {"1E20",1E20,{}}, {"-1E20",-1E20,{}},
    {"1E300",1E300,{}}, {"-1E300",-1E300,{}},
    {"EXP(1000)",std::numeric_limits<double>::infinity(),{}},
    {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{}}
};
int direct_count=0, public_count=0;
void direct_cases() {
    const auto check=[](const PrgValue& input,std::optional<std::int64_t> expected,const char* tag) {
        ++direct_count;
        const auto actual=checked_cast_int64_argument(input);
        expect(actual==expected,std::string("CAST direct ")+tag);
    };
    for(const auto& row:numeric_rows)check(make_number_value(row.raw),row.expected,row.text);
    check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},"NaN");
    check(make_number_value(std::nextafter(9223372036854775808.,0.)),9223372036854774784LL,"nextafter below2^63");
    check(make_number_value(std::nextafter(-9223372036854775808.,0.)),-9223372036854774784LL,"nextafter above-minus2^63");
    check(make_number_value(std::nextafter(-9223372036854775808.,-std::numeric_limits<double>::infinity())),{},"nextafter below-minus2^63");
    for(const auto n:std::vector<std::int64_t>{0,1,-1,9007199254740993LL,-9007199254740993LL,
            INT64_MIN,INT64_MIN+1,INT64_MAX-1,INT64_MAX}) {
        check(make_int64_value(n),n,"exact signed64 identity");
    }
    struct UnsignedRow { std::uint64_t raw; std::optional<std::int64_t> expected; };
    for(const auto& row:std::vector<UnsignedRow>{
            {0,0},{1,1},{9007199254740993ULL,9007199254740993LL},
            {9223372036854775806ULL,9223372036854775806LL},
            {9223372036854775807ULL,9223372036854775807LL},
            {9223372036854775808ULL,{}},{UINT64_MAX,{}}}) {
        check(make_uint64_value(row.raw),row.expected,"exact unsigned64 boundary");
    }
    // Safe old coercion preservation, not a new native type/Currency policy.
    check(make_boolean_value(true),1,"Logical true");
    check(make_boolean_value(false),0,"Logical false");
    check(make_string_value("1.9"),1,"Character fraction");
    // Preserve the existing coercion exception; this is not a native type rule.
    ++direct_count;
    bool nonnumeric_rejected=false;
    try {
        (void)checked_cast_int64_argument(make_string_value("x"));
    } catch(const std::invalid_argument&) {
        nonnumeric_rejected=true;
    }
    expect(nonnumeric_rejected,"CAST direct nonnumeric Character coercion preservation");
    check(make_currency_value(15000),1,"Currency fraction");
    check(make_empty_value(),0,"empty");
}
struct PublicRow { const char* expression; std::optional<std::int64_t> expected; };
const std::vector<PublicRow> exact_rows{
    {"(CAST(9007199254740992 AS INT64)+CAST(1 AS INT64))",9007199254740993LL},
    {"(CAST(-9007199254740992 AS INT64)-CAST(1 AS INT64))",-9007199254740993LL},
    {"(CAST(9223372036854774784 AS INT64)+CAST(1023 AS INT64))",9223372036854775807LL},
    {"(CAST(9223372036854774784 AS INT64)+CAST(1022 AS INT64))",9223372036854775806LL},
    {"(CAST(-9223372036854775808 AS INT64)+CAST(1 AS INT64))",-9223372036854775807LL},
    {"(CAST(9007199254740992 AS UINT64)+CAST(1 AS UINT64))",9007199254740993LL},
    {"(CAST(9223372036854774784 AS UINT64)+CAST(1023 AS UINT64))",9223372036854775807LL},
    {"CAST(9223372036854775808 AS UINT64)",{}},
    {"(CAST(9223372036854775808 AS UINT64)+CAST(9223372036854774784 AS UINT64)+CAST(1023 AS UINT64))",{}}
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
        "lAfter=.T.\nnGuard=CAST(1.9 AS INT64)\n"
        "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
        "USE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
        "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
    auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
    const auto state=session.run(DebugResumeAction::continue_run);
    expect(state.completed,"CAST completion "+tag+": "+state.message);
    expect(value(state,"ncode")== (expected?"0":std::to_string(expected_error)),"CAST selected error "+tag+" got "+value(state,"ncode"));
    expect(value(state,"nresult")== (expected?std::to_string(*expected):"12345"),
           "CAST selected result "+tag+" got "+value(state,"nresult"));
    const auto result=state.globals.find("nresult");
    expect(result!=state.globals.end()&&result->second.kind==
           (expected?PrgValueKind::int64:PrgValueKind::number),"CAST selected kind/assignment "+tag);
    expect(value(state,"lafter")=="true"&&value(state,"nguard")=="1","CAST continuation/control "+tag);
    const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
    expect(value(state,"cbefore")==context&&value(state,"cafter")==context,"CAST mode/session/cursor "+tag);
    expect(value(state,"creset")=="1|COPPERFIN|","CAST cleanup/reset "+tag);
    if(!expected_message.empty())expect(value(state,"cmessage")==expected_message,"CAST selected localized message "+tag);
}
void public_cases(const std::string& mode,int selected) {
    const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto dir=fs::temp_directory_path()/("copperfin_cast_int64_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
    fs::create_directories(dir);
    const auto path=dir/"query.prg";
    for(const char* alias:{"INT64","LONGLONG","BIGINT"}) {
        const auto cast=[&](const std::string& input){return "CAST("+input+" AS "+alias+")";};
        for(const auto& row:numeric_rows)public_case(path,mode,selected,cast(row.text),row.expected);
        for(const auto& row:exact_rows)public_case(path,mode,selected,cast(row.expression),row.expected);
        for(const auto& row:std::vector<PublicRow>{{".T.",1},{"'1.9'",1},{"$1.5",1}})
            public_case(path,mode,selected,cast(row.expression),row.expected);
        // Existing Character coercion error1/assignment retention, not a new
        // Numeric-domain rejection rule or native type-policy requirement.
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
        expect(direct_count==56&&public_count==0,"CAST complete direct shard count");
    } else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
              (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
        public_cases(argv[1],std::string(argv[2])=="1"?1:2);
        expect(direct_count==0&&public_count==141,"CAST complete public shard count");
    } else {
        std::cerr<<"Expected no arguments (direct), or COPPERFIN|VFP9 1|2 (public shard)\n";
        return 2;
    }
    std::cout<<"CAST frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
    return test_failures()==0?0:1;
}
