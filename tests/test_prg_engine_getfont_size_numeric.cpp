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
// RQ-CF-PRG-GETFONT-SIZE-NUMERIC-001 / VR-5611-GETFONT-SIZE-NUMERIC-001/002.
// Independent native/help/parent-derived literals, not current llround output.
struct NumericRow {
    const char* text;
    double raw;
    std::optional<std::int32_t> copperfin, vfp9;
};
const std::vector<NumericRow> numeric_rows{
    {"0",0,10,10}, {"-0",-0.,10,10},
    {"0.49",0.49,10,10}, {"0.5",0.5,10,10}, {"0.9",0.9,10,10},
    {"1.49",1.49,1,1}, {"1.5",1.5,1,1}, {"1.9",1.9,1,1},
    {"-0.49",-0.49,10,10}, {"-0.5",-0.5,10,10}, {"-0.9",-0.9,10,10},
    {"-1.49",-1.49,10,10}, {"-1.5",-1.5,10,10}, {"-1.9",-1.9,10,10},
    {"4",4,4,4}, {"12",12,12,12}, {"12.49",12.49,12,12},
    {"12.5",12.5,12,12}, {"12.9",12.9,12,12},
    {"127",127,127,127}, {"128",128,128,128},
    {"2147483647",2147483647.,2147483647,2147483647},
    {"2147483647.49",2147483647.49,2147483647,2147483647},
    {"2147483647.5",2147483647.5,2147483647,2147483647},
    {"2147483647.9",2147483647.9,2147483647,2147483647},
    {"2147483647.9999998",2147483647.9999998,2147483647,2147483647},
    {"2147483648",2147483648.,{},10},
    {"-2147483648",-2147483648.,10,10},
    {"-2147483648.9",-2147483648.9,10,10},
    {"-2147483649",-2147483649.,10,2147483647},
    {"4294967295",4294967295.,{},10},
    {"4294967295.1",4294967295.1,{},{}},
    {"4294967295.5",4294967295.5,{},{}},
    {"4294967295.9",4294967295.9,{},{}},
    {"4294967296",4294967296.,{},{}},
    {"-4294967296",-4294967296.,10,10},
    {"-4294967284",-4294967284.,10,12},
    {"-4294967283.5",-4294967283.5,10,13},
    {"9007199254740992",9007199254740992.,{},{}},
    {"-9007199254740992",-9007199254740992.,10,10},
    {"1E20",1E20,{},{}}, {"-1E20",-1E20,10,10},
    {"1E300",1E300,{},{}}, {"-1E300",-1E300,10,10},
    {"EXP(1000)",std::numeric_limits<double>::infinity(),{},{}},
    {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},10}
};
int direct_count=0, public_count=0;
void direct_cases() {
    expect(numeric_rows.size()==46,"GETFONT independent Numeric row count");
    for (const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
        const bool legacy=mode==NumericBehavior::vfp9;
        const auto check=[&](const PrgValue& input,std::optional<std::int32_t> modern,
                             std::optional<std::int32_t> old,const char* tag) {
            ++direct_count;
            expect(checked_getfont_size_numeric_argument(input,mode)==(legacy?old:modern),
                   std::string("GETFONT direct ")+(legacy?"VFP9 ":"COPPERFIN ")+tag);
        };
        for (const auto& row:numeric_rows)
            check(make_number_value(row.raw),row.copperfin,row.vfp9,row.text);
        check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},10,"derived NaN");
        check(make_number_value(std::nextafter(2147483648.,0.)),2147483647,2147483647,"signed32 inner");
        check(make_number_value(std::nextafter(2147483648.,INFINITY)),{},10,"signed32 outer");
        check(make_number_value(std::nextafter(4294967295.,0.)),{},10,"uint32 ceiling inner");
        check(make_number_value(std::nextafter(4294967295.,INFINITY)),{}, {},"uint32 ceiling outer");
        check(make_number_value(std::nextafter(0.,1.)),10,10,"positive subnormal");
        check(make_number_value(std::nextafter(0.,-1.)),10,10,"negative subnormal");
        struct SignedRow { std::int64_t raw; std::optional<std::int32_t> modern,old; };
        for (const auto& row:std::vector<SignedRow>{
            {0,10,10},{1,1,1},{-1,10,10},{2147483647,2147483647,2147483647},
            {2147483648LL,{},10},{-2147483648LL,10,10},{-2147483649LL,10,2147483647},
            {4294967295LL,{},10},{4294967296LL,{},{}},{-4294967296LL,10,10},
            {-4294967284LL,10,12},{-4294967283LL,10,13},
            {9007199254740993LL,{},{}},{-9007199254740993LL,10,10},
            {-9007203549708275LL,10,13},{INT64_MIN,10,10},{INT64_MIN+13,10,13},{INT64_MAX,{},{}}})
            check(make_int64_value(row.raw),row.modern,row.old,"exact signed64");
        struct UnsignedRow { std::uint64_t raw; std::optional<std::int32_t> modern,old; };
        for (const auto& row:std::vector<UnsignedRow>{
            {0,10,10},{1,1,1},{2147483647ULL,2147483647,2147483647},{2147483648ULL,{},10},
            {4294967294ULL,{},10},{4294967295ULL,{},10},{4294967296ULL,{},{}},
            {9007199254740993ULL,{},{}},{9223372036854775808ULL,{},{}},
            {UINT64_MAX-1,{},{}},{UINT64_MAX,{},{}},{12,12,12}})
            check(make_uint64_value(row.raw),row.modern,row.old,"exact unsigned64");
        check(make_boolean_value(true),{}, {},"non-Numeric Logical adapter boundary");
        check(make_string_value("12.9"),{}, {},"non-Numeric Character adapter boundary");
        check(make_null_value(),{}, {},"NULL adapter boundary");
    }
}
struct ExactRow { const char* expression; std::optional<std::int32_t> modern,old; };
const std::vector<ExactRow> exact_rows{
    {"CAST(0 AS INT64)",10,10},{"CAST(1 AS INT64)",1,1},{"CAST(-1 AS INT64)",10,10},
    {"CAST(2147483647 AS INT64)",2147483647,2147483647},
    {"(CAST(2147483647 AS INT64)+CAST(1 AS INT64))",{},10},
    {"CAST(-2147483648 AS INT64)",10,10},
    {"(CAST(-2147483648 AS INT64)-CAST(1 AS INT64))",10,2147483647},
    {"CAST(4294967295 AS INT64)",{},10},
    {"(CAST(4294967295 AS INT64)+CAST(1 AS INT64))",{},{}},
    {"CAST(-4294967284 AS INT64)",10,12},{"CAST(-4294967283 AS INT64)",10,13},
    {"CAST(4294967295 AS UINT64)",{},10},
    {"CAST(2147483647 AS UINT64)",2147483647,2147483647},
    {"CAST(2147483648 AS UINT64)",{},10},{"CAST(4294967296 AS UINT64)",{},{}},
    {"(CAST(9007199254740992 AS INT64)+CAST(1 AS INT64))",{},{}},
    {"(CAST(-9007203549708288 AS INT64)+CAST(13 AS INT64))",10,13},
    {"CAST(-9223372036854775808 AS INT64)",10,10},
    {"(CAST(-9223372036854775808 AS INT64)+CAST(13 AS INT64))",10,13},
    {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{}, {}}
};
std::string value(const RuntimePauseState& state,const char* key) {
    const auto found=state.globals.find(key);
    return found==state.globals.end()?"<missing>":format_value(found->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,
                 const std::string& expression,std::optional<std::int32_t> expected_size,
                 const std::string& literal_event={},const std::string& result="Arial",
                 const std::string& expected_message={}) {
    ++public_count;
    const bool admitted=expected_size.has_value()||!literal_event.empty();
    const auto other=mode=="VFP9"?"COPPERFIN":"VFP9";
    const auto tag=mode+" session"+std::to_string(selected)+" "+expression;
    write_text(path,(selected==2?std::string("SET NUMERICBEHAVIOR TO ")+other+"\nSET DATASESSION TO 2\n":"")+
        "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC cFont,nCode,lAfter,cMessage,cBefore,cAfter,cReset,nGuard,cOther\n"
        "cFont='sentinel'\nnCode=0\nlAfter=.F.\ncMessage=''\n"
        "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
        "cBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
        "TRY\ncFont="+expression+"\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\n"
        "lAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\n"
        "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
        "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
        "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
    auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
    const auto state=session.run(DebugResumeAction::continue_run);
    expect(state.completed,"GETFONT completion "+tag+": "+state.message);
    expect(value(state,"ncode")== (admitted?"0":"11"),"GETFONT selected error "+tag+" got "+value(state,"ncode"));
    expect(value(state,"cfont")== (admitted?result:"sentinel"),"GETFONT selected assignment "+tag);
    const auto font=state.globals.find("cfont");
    expect(font!=state.globals.end()&&font->second.kind==PrgValueKind::string,"GETFONT Character kind "+tag);
    expect(value(state,"lafter")=="true"&&value(state,"nguard")=="1","GETFONT continuation/control "+tag);
    const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
    expect(value(state,"cbefore")==context&&value(state,"cafter")==context,"GETFONT mode/session/cursor "+tag);
    expect(value(state,"cother")== (selected==2?other:mode),"GETFONT neighboring area policy "+tag);
    expect(value(state,"creset")=="1|COPPERFIN|","GETFONT cleanup/reset "+tag);
    if (admitted) expect(value(state,"cmessage").empty(),"GETFONT successful message "+tag);
    if (!expected_message.empty()) expect(value(state,"cmessage")==expected_message,"GETFONT localized11 "+tag);
    int event_count=0;
    const auto detail=!literal_event.empty()?literal_event:expected_size
        ? "mode=headless name=\"Arial\" size="+std::to_string(*expected_size)+" style=\"B\" result=\"Arial\""
        : std::string{};
    for (const auto& event:state.events) if (event.category=="runtime.getfont") {
        ++event_count;
        expect(admitted&&event.detail==detail,"GETFONT selected initial-request event "+tag+" got "+event.detail);
    }
    expect(event_count==(admitted?1:0),"GETFONT event atomicity/count "+tag);
}
void public_cases(const std::string& mode,int selected) {
    expect(numeric_rows.size()==46&&exact_rows.size()==20,"GETFONT independent public row counts");
    const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto dir=fs::temp_directory_path()/("copperfin_getfont_size_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
    fs::create_directories(dir);
    const auto path=dir/"query.prg";
    const auto call=[](const std::string& input){return "GETFONT('Arial',"+input+",'B')";};
    for (const auto& row:numeric_rows)
        public_case(path,mode,selected,call(row.text),mode=="VFP9"?row.vfp9:row.copperfin);
    for (const auto& row:exact_rows)
        public_case(path,mode,selected,call(row.expression),mode=="VFP9"?row.old:row.modern);
    // Untouched non-Numeric controls, not recovered native type-admission parity.
    for (const auto& row:std::vector<std::pair<const char*,std::int32_t>>{
        {".T.",1},{".F.",0},{".NULL.",0},{"'12.9'",13},{"$12.5",13},{"'-1.9'",-2}})
        public_case(path,mode,selected,call(row.first),row.second);
    public_case(path,mode,selected,"GETFONT()",{},"mode=headless name=\"\" result=\"\"","");
    public_case(path,mode,selected,"GETFONT('Arial')",{},"mode=headless name=\"Arial\" result=\"Arial\"");
    public_case(path,mode,selected,"GETFONT('Arial',12)",12,"mode=headless name=\"Arial\" size=12 result=\"Arial\"");
    for (const auto& row:std::vector<std::pair<std::string,std::string>>{
        {"en-US","Function argument value, type, or count is invalid."},
        {"es-419","El valor, tipo o numero de un argumento de funcion no es valido."},
        {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido."},
        {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]"}}) {
        set_env_value("COPPERFIN_LOCALE",row.first,true);
        public_case(path,mode,selected,call("1E300"),{},std::string{},"Arial",row.second);
    }
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    std::error_code ignored;
    fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv) {
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    if (argc==1) {
        direct_cases();
        expect(direct_count==172&&public_count==0,"GETFONT complete direct shard count");
    } else if (argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
               (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
        public_cases(argv[1],std::string(argv[2])=="1"?1:2);
        expect(direct_count==0&&public_count==79,"GETFONT complete public shard count");
    } else {
        std::cerr<<"Expected no arguments (direct), or COPPERFIN|VFP9 1|2 (public shard)\n";
        return 2;
    }
    std::cout<<"GETFONT frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
    return test_failures()==0?0:1;
}
