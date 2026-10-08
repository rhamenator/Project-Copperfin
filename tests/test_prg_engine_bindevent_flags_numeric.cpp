// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-BINDEVENT-FLAGS-NUMERIC-001: independent native/derived
// constants, not predictions computed with the implementation under test.
struct Case { const char* expression; double value; std::optional<int> copperfin, vfp9; };
const std::vector<Case> cases{
    {"0", 0, 0, 0},
    {"1", 1, 1, 1},
    {"2", 2, 2, 2},
    {"3", 3, 3, 3},
    {"4", 4, 0, 0},
    {"5", 5, 1, 1},
    {"7", 7, 3, 3},
    {"-1", -1, std::nullopt, std::nullopt},
    {"-2", -2, std::nullopt, std::nullopt},
    {"-3", -3, std::nullopt, std::nullopt},
    {"-4", -4, std::nullopt, std::nullopt},
    {"0.49", 0.49, 0, 0},
    {"0.5", 0.5, 0, 0},
    {"0.9", 0.9, 0, 0},
    {"1.49", 1.49, 1, 1},
    {"1.5", 1.5, 1, 1},
    {"1.9", 1.9, 1, 1},
    {"2.9", 2.9, 2, 2},
    {"3.9", 3.9, 3, 3},
    {"-0.5", -0.5, 0, 0},
    {"-1.5", -1.5, std::nullopt, std::nullopt},
    {"2147483647", 2147483647, 3, 3},
    {"2147483648", 2147483648, std::nullopt, std::nullopt},
    {"-2147483648", -2147483648, std::nullopt, std::nullopt},
    {"-2147483649", -2147483649, std::nullopt, 3},
    {"4294967295", 4294967295, std::nullopt, std::nullopt},
    {"4294967296", 4294967296, std::nullopt, 0},
    {"4294967297", 4294967297, std::nullopt, 1},
    {"4294967298", 4294967298, std::nullopt, 2},
    {"4294967299", 4294967299, std::nullopt, 3},
    {"-4294967295", -4294967295, std::nullopt, 1},
    {"9007199254740991", 9007199254740991, std::nullopt, std::nullopt},
    {"9007199254740992", 9007199254740992, std::nullopt, 0},
    {"1E20", 1e20, std::nullopt, 0},
    {"-1E20", -1e20, std::nullopt, 0},
    {"1E300", 1e300, std::nullopt, 0},
    {"-1E300", -1e300, std::nullopt, 0},
    {"1E300*1E300", std::numeric_limits<double>::infinity(), std::nullopt, 0},
    {"-1E300*1E300", -std::numeric_limits<double>::infinity(), std::nullopt, 0},
};
void direct_boundaries() {
    struct Extra { PrgValue value; std::optional<int> copperfin, vfp9; };
    const double inf=std::numeric_limits<double>::infinity();
    const std::vector<Extra> extra{
        {make_number_value(std::numeric_limits<double>::quiet_NaN()),std::nullopt,0},
        {make_number_value(std::nextafter(2147483648.0,0.0)),3,3},
        {make_number_value(std::nextafter(2147483648.0,inf)),std::nullopt,std::nullopt},
        {make_number_value(std::nextafter(-1.0,0.0)),0,0},
        {make_number_value(std::nextafter(-1.0,-inf)),std::nullopt,std::nullopt},
        {make_number_value(std::nextafter(9223372036854775808.0,0.0)),std::nullopt,std::nullopt},
        {make_number_value(9223372036854775808.0),std::nullopt,0},
        {make_number_value(-9223372036854775808.0),std::nullopt,0},
        {make_number_value(std::nextafter(-9223372036854775808.0,-inf)),std::nullopt,0},
        {make_int64_value(INT32_MAX),3,3},
        {make_int64_value(INT64_MAX),std::nullopt,std::nullopt},
        {make_int64_value(INT64_MIN),std::nullopt,0},
        {make_int64_value(INT64_MIN+1),std::nullopt,1},
        {make_int64_value(INT64_MIN+3),std::nullopt,3},
        {make_int64_value(INT64_C(9007199254740993)),std::nullopt,1},
        {make_uint64_value(UINT64_C(9007199254740993)),std::nullopt,1},
        {make_uint64_value(UINT64_MAX),std::nullopt,std::nullopt},
        {make_uint64_value(UINT64_MAX-2),std::nullopt,std::nullopt},
        {make_uint64_value(UINT64_C(9223372036854775809)),std::nullopt,1},
        {make_uint64_value(3),3,3},
        {make_null_value(),0,0},
        {make_boolean_value(true),1,1},
        {make_string_value("1"),1,1},
        {make_currency_value(15000),2,2},
        {make_currency_value(-15000),2,2},
        {make_currency_value(INT64_MAX),std::nullopt,std::nullopt},
        {make_currency_value(INT64_MIN),std::nullopt,std::nullopt},
    };
    for (const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
        for (const auto& row:cases) {
            expect(checked_bindevent_flags_argument(make_number_value(row.value),mode)==
                (mode==NumericBehavior::copperfin?row.copperfin:row.vfp9),
                std::string("BINDEVENT direct ")+row.expression+
                (mode==NumericBehavior::copperfin?" COPPERFIN":" VFP9"));
        }
        for (std::size_t i=0;i<extra.size();++i) {
            expect(checked_bindevent_flags_argument(extra[i].value,mode)==
                (mode==NumericBehavior::copperfin?extra[i].copperfin:extra[i].vfp9),
                "BINDEVENT extra boundary "+std::to_string(i));
        }
    }
}
std::string classes=R"prg(
RETURN
PROCEDURE FlagRoutine
    cLog=cLog+'H'
ENDPROC
DEFINE CLASS FlagSource AS Custom
    PROCEDURE Ping
        cLog=cLog+'B'
    ENDPROC
ENDDEFINE
DEFINE CLASS FlagSink AS Custom
    PROCEDURE Handle
        cLog=cLog+'H'
    ENDPROC
ENDDEFINE
)prg";
void script_rows() {
    for (const bool legacy:{false,true}) for (const bool routine:{false,true}) {
        const std::string mode=legacy?"VFP9":"COPPERFIN";
        const auto dir=fs::temp_directory_path()/("copperfin_bindevent_flags_5611_"+mode+(routine?"_routine":"_method"));
        fs::create_directories(dir);
        const auto path=dir/"flags.prg";
        const auto bind=[&](const std::string& flag) {
            const std::string prefix=routine?"BINDEVENT(oSource,'Ping','FlagRoutine'":
                "BINDEVENT(oSource,'Ping',oHandler,'Handle'";
            return prefix+(flag.empty()?"":","+flag)+")";
        };
        std::string source="PUBLIC cLog\ncOut=''\nSET NUMERICBEHAVIOR TO "+mode+
            "\nCREATE CURSOR flagguard (label C(8))\nINSERT INTO flagguard VALUES ('guard')\n"+
            "oHandler=CREATEOBJECT('FlagSink')\n";
        std::vector<std::string> expected;
        std::size_t bind_events=0,delegate_events=0,raised_events=0;
        const auto add=[&](const std::string& expression,const std::optional<int>& flag) {
            source+="oSource=CREATEOBJECT('FlagSource')\nx="+bind("1")+"\ncStatus='OK'\ncMessage=''\nx=-99\nTRY\nx="+bind(expression)+
                "\nCATCH TO oError\ncStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))\ncMessage=oError.Message\nENDTRY\n"+
                "cLog=''\noSource.Ping()\ncSimple=cLog\ncLog=''\nRAISEEVENT(oSource,'Ping')\ncRaised=cLog\n"+
                "nRemoved=UNBINDEVENTS(oSource)\ncOut=cOut+cStatus+'|'+TRANSFORM(x)+'|'+cMessage+'|'+cSimple+'|'+cRaised+'|'+"+
                "TRANSFORM(nRemoved)+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+"+
                "SET('NUMERICBEHAVIOR')+'|'+SET('DATASESSION')+CHR(10)\n";
            const int active=flag.value_or(1);
            const std::string simple=(active&2)?"B":((active&1)?"BH":"HB");
            const std::string raised=(active&1)?"BH":"HB";
            expected.push_back((flag?"OK|1||":"ERR11|-99|Function argument value, type, or count is invalid.|")+
                simple+"|"+raised+"|1|1:1:guard|"+mode+"|1");
            bind_events+=flag?2U:1U;
            delegate_events+=(active&2)?1U:2U;
            ++raised_events;
        };
        for(const auto& row:cases) add(row.expression,legacy?row.vfp9:row.copperfin);
        // Omitted flags and preserved other coercions are controls, not native
        // type-parity claims; native NULL/Boolean/Character/Currency gaps separate.
        add("",0); add(".NULL.",0); add(".T.",1); add("'1'",1); add("$1.5",2);
        source+="USE IN flagguard\nSET NUMERICBEHAVIOR TO COPPERFIN\ncCleanup=SET('NUMERICBEHAVIOR')\n"+classes;
        write_text(path,source);
        auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
        const auto state=session.run(DebugResumeAction::continue_run);
        expect(state.completed,"BINDEVENT script completes "+mode+(routine?" routine: ":" method: ")+state.message);
        const auto output=state.globals.find("cout");
        const std::string actual=output==state.globals.end()?"<missing>":format_value(output->second);
        std::istringstream stream(actual);
        std::string line;
        for(std::size_t i=0;i<expected.size();++i) {
            std::getline(stream,line);
            expect(line==expected[i],"BINDEVENT row "+mode+(routine?" routine ":" method ")+
                std::to_string(i)+" expected "+expected[i]+" got "+line);
        }
        expect(!std::getline(stream,line),"BINDEVENT output has no extra rows");
        const auto cleanup=state.globals.find("ccleanup");
        expect(cleanup!=state.globals.end()&&format_value(cleanup->second)=="COPPERFIN","BINDEVENT mode reset");
        std::size_t binds=0,delegates=0,raises=0,unbinds=0;
        for(const auto& event:state.events) {
            if(event.category=="prg.event.bind") ++binds;
            if(event.category=="prg.event.delegate") ++delegates;
            if(event.category=="prg.event.raise") ++raises;
            if(event.category=="prg.event.unbind") ++unbinds;
        }
        expect(binds==bind_events,"BINDEVENT success events "+mode+(routine?" routine":" method")+
            " expected "+std::to_string(bind_events)+" got "+std::to_string(binds));
        expect(delegates==delegate_events,"BINDEVENT delegate events match admitted flags");
        expect(raises==raised_events&&unbinds==expected.size(),"BINDEVENT raise/cleanup events");
        std::error_code ignored;
        fs::remove_all(dir,ignored);
    }
}
}
int main() {
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    direct_boundaries();
    script_rows();
    return test_failures()==0?0:1;
}
