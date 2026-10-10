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
namespace fs=std::filesystem;
// RQ-CF-PRG-GETCOLOR-DEFAULT-NUMERIC-001 / VR-5611-GETCOLOR-DEFAULT-NUMERIC-001/002.
// Independent native/help/parent-derived literals, not helper/current llround output.
struct NumericRow { const char* text; double raw; std::optional<std::uint32_t> modern,old; };
const std::vector<NumericRow> numeric_rows{
    {"0",0,0,0},{"-0",-0.,0,0},
    {"0.49",.49,0,0},{"0.5",.5,0,0},{"0.9",.9,0,0},
    {"255",255,255,255},{"255.49",255.49,255,255},{"255.5",255.5,255,255},
    {"255.9",255.9,255,255},{"256",256,256,256},{"256.9",256.9,256,256},
    {"65535",65535,65535,65535},{"65280",65280,65280,65280},{"16711680",16711680,16711680,16711680},
    {"16777215",16777215.,16777215,16777215},{"16777215.5",16777215.5,16777215,16777215},
    {"16777215.9",16777215.9,16777215,16777215},{"16777216",16777216.,{},0},
    {"-0.49",-.49,0,0},{"-0.5",-.5,0,0},{"-0.9",-.9,0,0},
    {"-1",-1.,{},16777215},{"-1.49",-1.49,{},16777215},
    {"-1.5",-1.5,{},16777215},{"-1.9",-1.9,{},16777215},
    {"2147483647",2147483647.,{},16777215},{"2147483647.9",2147483647.9,{},16777215},
    {"2147483648",2147483648.,{},0},{"-2147483648",-2147483648.,{},0},
    {"-2147483648.9",-2147483648.9,{},0},{"-2147483649",-2147483649.,{},16777215},
    {"4294967295",4294967295.,{},16777215},{"4294967295.1",4294967295.1,{},16777215},
    {"4294967295.5",4294967295.5,{},16777215},{"4294967295.9",4294967295.9,{},16777215},
    {"4294967296",4294967296.,{},0},{"-4294967296",-4294967296.,{},0},
    {"-4294967041",-4294967041.,{},255},{"-4294967040.5",-4294967040.5,{},256},
    {"9007199254740992",9007199254740992.,{},0},{"-9007199254740992",-9007199254740992.,{},0},
    {"1E20",1E20,{},0},{"-1E20",-1E20,{},0},{"1E300",1E300,{},0},{"-1E300",-1E300,{},0},
    {"EXP(1000)",std::numeric_limits<double>::infinity(),{},0},
    {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},0}
};
int direct_count=0,public_count=0;
void direct_cases() {
    expect(numeric_rows.size()==47,"GETCOLOR independent Numeric row count");
    for (const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
        const bool legacy=mode==NumericBehavior::vfp9;
        const auto check=[&](const PrgValue& input,std::optional<std::uint32_t> modern,
                             std::optional<std::uint32_t> old,const char* tag) {
            ++direct_count;
            expect(checked_getcolor_default_numeric_argument(input,mode)==(legacy?old:modern),
                   std::string("GETCOLOR direct ")+(legacy?"VFP9 ":"COPPERFIN ")+tag);
        };
        for (const auto& row:numeric_rows) check(make_number_value(row.raw),row.modern,row.old,row.text);
        check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},0,"derived NaN");
        check(make_number_value(std::nextafter(16777216.,0.)),16777215,16777215,"RGB inner");
        check(make_number_value(std::nextafter(16777216.,INFINITY)),{},0,"RGB outer");
        check(make_number_value(std::nextafter(-1.,0.)),0,0,"negative truncation inner");
        check(make_number_value(std::nextafter(-1.,-INFINITY)),{},16777215,"negative truncation outer");
        check(make_number_value(std::nextafter(0.,1.)),0,0,"positive subnormal");
        check(make_number_value(std::nextafter(0.,-1.)),0,0,"negative subnormal");
        struct SignedRow {std::int64_t raw; std::optional<std::uint32_t> modern,old;};
        for (const auto& row:std::vector<SignedRow>{
            {0,0,0},{1,1,1},{255,255,255},{16777215,16777215,16777215},
            {16777216,{},0},{-1,{},16777215},{-2147483648LL,{},0},
            {-2147483649LL,{},16777215},{4294967295LL,{},16777215},{4294967296LL,{},0},
            {-4294967041LL,{},255},{9007199254740993LL,{},1},{-9007199254740993LL,{},16777215},
            {9007199254741247LL,{},255},{INT64_MIN,{},0},{INT64_MIN+255,{},255},{INT64_MAX,{},16777215}})
            check(make_int64_value(row.raw),row.modern,row.old,"exact signed64");
        struct UnsignedRow {std::uint64_t raw; std::optional<std::uint32_t> modern,old;};
        for (const auto& row:std::vector<UnsignedRow>{
            {0,0,0},{255,255,255},{16777215,16777215,16777215},{16777216,{},0},
            {2147483647ULL,{},16777215},{2147483648ULL,{},0},{4294967295ULL,{},16777215},
            {4294967296ULL,{},0},{9007199254740993ULL,{},1},{9007199254741247ULL,{},255},
            {9223372036854775808ULL,{},0},{UINT64_MAX-1,{},16777214},{UINT64_MAX,{},16777215}})
            check(make_uint64_value(row.raw),row.modern,row.old,"exact unsigned64");
        check(make_boolean_value(true),{}, {},"Logical adapter boundary");
        check(make_string_value("255.9"),{}, {},"Character adapter boundary");
        check(make_null_value(),{}, {},"NULL adapter boundary");
    }
}
struct ExactRow {const char* expression; std::optional<std::uint32_t> modern,old;};
const std::vector<ExactRow> exact_rows{
    {"CAST(0 AS INT64)",0,0},{"CAST(1 AS INT64)",1,1},{"CAST(-1 AS INT64)",{},16777215},
    {"CAST(16777215 AS INT64)",16777215,16777215},{"CAST(16777216 AS INT64)",{},0},
    {"CAST(-2147483648 AS INT64)",{},0},
    {"(CAST(-2147483648 AS INT64)-CAST(1 AS INT64))",{},16777215},
    {"CAST(4294967295 AS INT64)",{},16777215},{"CAST(4294967296 AS INT64)",{},0},
    {"CAST(-4294967041 AS INT64)",{},255},
    {"CAST(16777215 AS UINT64)",16777215,16777215},{"CAST(16777216 AS UINT64)",{},0},
    {"CAST(4294967295 AS UINT64)",{},16777215},{"CAST(4294967296 AS UINT64)",{},0},
    {"(CAST(9007199254740992 AS INT64)+CAST(1 AS INT64))",{},1},
    {"(CAST(9007199254740992 AS INT64)+CAST(255 AS INT64))",{},255},
    {"(CAST(-9007199254740992 AS INT64)-CAST(1 AS INT64))",{},16777215},
    {"CAST(-9223372036854775808 AS INT64)",{},0},
    {"(CAST(-9223372036854775808 AS INT64)+CAST(255 AS INT64))",{},255},
    {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{},16777215}
};
std::string value(const RuntimePauseState& state,const char* key) {
    const auto found=state.globals.find(key);
    return found==state.globals.end()?"<missing>":format_value(found->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,
                 const std::string& expression,std::optional<std::uint32_t> color,
                 const std::string& literal_event={},const std::string& literal_result={},
                 const std::string& expected_message={}) {
    ++public_count;
    const bool admitted=color.has_value()||!literal_event.empty();
    const auto other=mode=="VFP9"?"COPPERFIN":"VFP9";
    const auto tag=mode+" session"+std::to_string(selected)+" "+expression;
    write_text(path,(selected==2?std::string("SET NUMERICBEHAVIOR TO ")+other+"\nSET DATASESSION TO 2\n":"")+
        "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC cColor,nCode,lAfter,cMessage,cBefore,cAfter,cReset,nGuard,cOther\n"
        "cColor='sentinel'\nnCode=0\nlAfter=.F.\ncMessage=''\n"
        "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
        "cBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
        "TRY\ncColor="+expression+"\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\n"
        "lAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\n"
        "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
        "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
        "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
    auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
    const auto state=session.run(DebugResumeAction::continue_run);
    expect(state.completed,"GETCOLOR completion "+tag+": "+state.message);
    expect(value(state,"ncode")== (admitted?"0":"11"),"GETCOLOR selected error "+tag+" got "+value(state,"ncode"));
    const auto result=!literal_result.empty()?literal_result:color?std::to_string(*color):std::string{};
    expect(value(state,"ccolor")== (admitted?result:"sentinel"),"GETCOLOR selected assignment "+tag);
    const auto actual=state.globals.find("ccolor");
    expect(actual!=state.globals.end()&&actual->second.kind==(admitted?PrgValueKind::number:PrgValueKind::string),"GETCOLOR kind "+tag);
    expect(value(state,"lafter")=="true"&&value(state,"nguard")=="1","GETCOLOR continuation/control "+tag);
    const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
    expect(value(state,"cbefore")==context&&value(state,"cafter")==context,"GETCOLOR mode/session/cursor "+tag);
    expect(value(state,"cother")== (selected==2?other:mode),"GETCOLOR neighboring policy "+tag);
    expect(value(state,"creset")=="1|COPPERFIN|","GETCOLOR cleanup/reset "+tag);
    if (admitted) expect(value(state,"cmessage").empty(),"GETCOLOR successful message "+tag);
    if (!expected_message.empty()) expect(value(state,"cmessage")==expected_message,"GETCOLOR localized11 "+tag);
    const auto detail=!literal_event.empty()?literal_event:color
        ?"mode=headless default="+std::to_string(*color)+" title=\"numeric\" result="+std::to_string(*color):std::string{};
    int event_count=0;
    for (const auto& event:state.events) if(event.category=="runtime.getcolor") {
        ++event_count;
        expect(admitted&&event.detail==detail,"GETCOLOR selected event "+tag+" got "+event.detail);
    }
    expect(event_count==(admitted?1:0),"GETCOLOR event atomicity/count "+tag);
}
void public_cases(const std::string& mode,int selected) {
    expect(numeric_rows.size()==47&&exact_rows.size()==20,"GETCOLOR independent public counts");
    const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto dir=fs::temp_directory_path()/("copperfin_getcolor_default_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
    fs::create_directories(dir);
    const auto path=dir/"query.prg";
    const auto call=[](const std::string& input){return "GETCOLOR("+input+",'numeric')";};
    for(const auto& row:numeric_rows) public_case(path,mode,selected,call(row.text),mode=="VFP9"?row.old:row.modern);
    for(const auto& row:exact_rows) public_case(path,mode,selected,call(row.expression),mode=="VFP9"?row.old:row.modern);
    // Preserved coercions, not native type-admission parity requirements.
    struct Other {const char* input; const char* result; const char* event;};
    for(const auto& row:std::vector<Other>{
        {".T.","1","1"},{".F.","0","0"},{".NULL.","0","0"},
        {"'255.9'","255.9","256"},{"$255.5","255.5","256"},{"'-1.9'","-1.9","-2"}})
        public_case(path,mode,selected,call(row.input),{},
            std::string("mode=headless default=")+row.event+" title=\"numeric\" result="+row.event,row.result);
    public_case(path,mode,selected,"GETCOLOR()",0,"mode=headless default=0 title=\"\" result=0");
    public_case(path,mode,selected,"GETCOLOR(255.9)",255,"mode=headless default=255 title=\"\" result=255");
    public_case(path,mode,selected,"GETCOLOR(255,'title')",255,"mode=headless default=255 title=\"title\" result=255");
    for(const auto& row:std::vector<std::pair<std::string,std::string>>{
        {"en-US","Function argument value, type, or count is invalid."},
        {"es-419","El valor, tipo o numero de un argumento de funcion no es valido."},
        {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido."},
        {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]"}}) {
        set_env_value("COPPERFIN_LOCALE",row.first,true);
        // RGB boundary errors apply only to default; VFP9 intentionally accepts all Numeric inputs.
        if(mode=="COPPERFIN") public_case(path,mode,selected,call("1E300"),{},std::string{},std::string{},row.second);
        else public_case(path,mode,selected,call("1E300"),0);
    }
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    std::error_code ignored;
    fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv) {
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    if(argc==1) {
        direct_cases();
        expect(direct_count==174&&public_count==0,"GETCOLOR complete direct count");
    } else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
              (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
        public_cases(argv[1],std::string(argv[2])=="1"?1:2);
        expect(direct_count==0&&public_count==80,"GETCOLOR complete public count");
    } else {std::cerr<<"Expected no arguments, or COPPERFIN|VFP9 1|2\n";return 2;}
    std::cout<<"GETCOLOR frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
    return test_failures()==0?0:1;
}
