// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-GO-RECORD-NUMERIC-001: conversion constants independent of
// implementation. #7081 record/type admission remains deliberately unchanged.
struct Case { const char* expression; std::optional<long long> normal, legacy; };
const std::vector<Case> cases{
    {"-1",-1,-1}, {"-0.9",0,0}, {"-0.5",0,0}, {"0",0,0},
    {"0.49",0,0}, {"0.5",0,0}, {"0.9",0,0},
    {"1",1,1}, {"1.49",1,1}, {"1.5",1,1}, {"1.9",1,1},
    {"2",2,2}, {"2.5",2,2}, {"2.9",2,2}, {"3",3,3}, {"3.9",3,3},
    {"4",4,4}, {"2147483647",2147483647,2147483647},
    {"2147483648",std::nullopt,std::nullopt},
    {"-2147483648",-2147483648LL,-2147483648LL},
    {"4294967295",std::nullopt,std::nullopt},
    {"4294967296",std::nullopt,std::nullopt},
    {"4294967297",std::nullopt,std::nullopt},
    {"-4294967295",std::nullopt,1},
    {"-4294967296",std::nullopt,std::nullopt},
    {"-4294967297",std::nullopt,std::nullopt},
    {"9007199254740992",std::nullopt,std::nullopt},
    {"1E20",std::nullopt,std::nullopt}, {"-1E20",std::nullopt,std::nullopt},
    {"1E300",std::nullopt,std::nullopt}, {"-1E300",std::nullopt,std::nullopt},
    {"1E300*1E300",std::nullopt,std::nullopt},
    {"-1E300*1E300",std::nullopt,std::nullopt},
    {"-1.49",-1,-1}, {"-1.5",-1,-1}, {"-1.9",-1,-1}, {"-2",-2,-2},
    {"-2.9",-2,-2}, {"-3",-3,-3}, {"-4294967298",std::nullopt,std::nullopt},
    {"$1.5",2,2}, {"$0.5",1,1}, {"-$1.5",-2,-2},
    {"$922337203685477.5807",std::nullopt,std::nullopt},
    {"'2'",2,2}, {"'2.5'",3,3}, {".T.",1,1}, {".F.",0,0}, {".NULL.",0,0}
};

void direct_boundaries() {
    struct Direct { PrgValue value; std::optional<std::int32_t> normal,legacy; };
    const double infinity=std::numeric_limits<double>::infinity();
    const std::vector<Direct> rows{
        {make_number_value(-1),-1,-1},
        {make_number_value(-0.5),0,0},
        {make_number_value(-0.0),0,0},
        {make_number_value(0),0,0},
        {make_number_value(0.5),0,0},
        {make_number_value(1.5),1,1},
        {make_number_value(2.9),2,2},
        {make_number_value(-1.9),-1,-1},
        {make_number_value(-2.9),-2,-2},
        {make_number_value(2147483647.0),INT32_MAX,INT32_MAX},
        {make_number_value(2147483648.0),std::nullopt,std::nullopt},
        {make_number_value(-2147483648.0),INT32_MIN,INT32_MIN},
        {make_number_value(std::nextafter(2147483648.0,0.0)),INT32_MAX,INT32_MAX},
        {make_number_value(std::nextafter(2147483648.0,infinity)),std::nullopt,std::nullopt},
        {make_number_value(std::nextafter(-2147483649.0,0.0)),INT32_MIN,INT32_MIN},
        {make_number_value(-2147483649.0),std::nullopt,INT32_MAX},
        {make_number_value(4294967297.0),std::nullopt,std::nullopt},
        {make_number_value(-4294967295.0),std::nullopt,1},
        {make_number_value(-4294967296.0),std::nullopt,std::nullopt},
        {make_number_value(-4294967297.0),std::nullopt,std::nullopt},
        {make_number_value(-4294967298.0),std::nullopt,std::nullopt},
        {make_number_value(9007199254740992.0),std::nullopt,std::nullopt},
        {make_number_value(1e20),std::nullopt,std::nullopt},
        {make_number_value(-1e20),std::nullopt,std::nullopt},
        {make_number_value(1e300),std::nullopt,std::nullopt},
        {make_number_value(-1e300),std::nullopt,std::nullopt},
        {make_number_value(infinity),std::nullopt,std::nullopt},
        {make_number_value(-infinity),std::nullopt,std::nullopt},
        {make_number_value(std::numeric_limits<double>::quiet_NaN()),std::nullopt,std::nullopt},
        {make_number_value(std::nextafter(9223372036854775808.0,0.0)),std::nullopt,std::nullopt},
        {make_number_value(9223372036854775808.0),std::nullopt,std::nullopt},
        {make_number_value(-9223372036854775808.0),std::nullopt,std::nullopt},
        {make_int64_value(0),0,0},
        {make_int64_value(-1),-1,-1},
        {make_int64_value(-2),-2,-2},
        {make_int64_value(INT32_MAX),INT32_MAX,INT32_MAX},
        {make_int64_value(INT32_MIN),INT32_MIN,INT32_MIN},
        {make_int64_value(INT64_C(2147483648)),std::nullopt,std::nullopt},
        {make_int64_value(-INT64_C(2147483649)),std::nullopt,INT32_MAX},
        {make_int64_value(INT64_MAX),std::nullopt,std::nullopt},
        {make_int64_value(INT64_MIN),std::nullopt,std::nullopt},
        {make_int64_value(INT64_MIN+1),std::nullopt,1},
        {make_int64_value(INT64_C(9007199254740993)),std::nullopt,std::nullopt},
        {make_int64_value(-INT64_C(4294967295)),std::nullopt,1},
        {make_int64_value(-INT64_C(4294967297)),std::nullopt,std::nullopt},
        {make_uint64_value(0),0,0},
        {make_uint64_value(1),1,1},
        {make_uint64_value(INT32_MAX),INT32_MAX,INT32_MAX},
        {make_uint64_value(UINT64_C(2147483648)),std::nullopt,std::nullopt},
        {make_uint64_value(UINT64_C(9007199254740993)),std::nullopt,std::nullopt},
        {make_uint64_value(UINT64_MAX),std::nullopt,std::nullopt},
        {make_currency_value(15000),2,2},
        {make_currency_value(5000),1,1},
        {make_currency_value(-15000),-2,-2},
        {make_currency_value(INT64_MAX),std::nullopt,std::nullopt},
        {make_currency_value(INT64_MIN),std::nullopt,std::nullopt},
        {make_string_value("2.5"),3,3},
        {make_string_value("1e300"),std::nullopt,std::nullopt},
        {make_boolean_value(true),1,1},
        {make_boolean_value(false),0,0},
        {make_null_value(),0,0},
    };
    for (const auto mode : {NumericBehavior::copperfin,NumericBehavior::vfp9})
        for (std::size_t i=0;i<rows.size();++i)
            expect(checked_go_record_argument(rows[i].value,mode)==
                (mode==NumericBehavior::vfp9?rows[i].legacy:rows[i].normal),
                "GO direct "+std::to_string(i));
    std::cout<<"DIRECTCOUNT "<<rows.size()*2<<'\n';
}
void localized_rejections() {
    ScopedEnvironmentValue locale_scope("COPPERFIN_LOCALE");
    for (const std::string locale : {"en-US","es-419","pt-BR","qps-ploc"})
    for (const std::string mode : {"COPPERFIN","VFP9"}) {
        set_env_value("COPPERFIN_LOCALE",locale.c_str(),true);
        const auto dir=fs::temp_directory_path()/("copperfin_go_locale_5611_"+locale+mode);
        fs::create_directories(dir);
        const auto path=dir/"locale.prg";
        write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR goguard (label C(8))\nINSERT INTO goguard VALUES ('one')\nINSERT INTO goguard VALUES ('two')\nGO 2\nnCode=0\ncMessage=''\nlMetadata=.F.\nTRY\nGO (1E300)\nCATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\n=AERROR(aError)\nlMetadata=ERROR()==5 AND aError[1,1]==5 AND aError[1,2]==cMessage\nENDTRY\ncState=TRANSFORM(RECNO())+'|'+ALLTRIM(label)+'|'+SET('DATASESSION')\nUSE IN goguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
        auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
        const auto state=session.run(DebugResumeAction::continue_run);
        expect(state.completed,"GO locale completes "+locale+mode+": "+state.message);
        const auto value=[&](const char* key){
            const auto found=state.globals.find(key);
            return found==state.globals.end()?std::string("<missing>"):format_value(found->second);
        };
        expect(value("ncode")=="5" && value("lmetadata")=="true","GO locale error metadata "+locale+mode);
        expect(value("cstate")=="2|two|1","GO locale unchanged position "+locale+mode);
        const auto message=value("cmessage");
        // Independent decimal expectation: the safe round-trip formatter
        // expands this exponent; do not reuse the production formatter here.
        const std::string operand = "1" + std::string(300, '0');
        const std::string expected=(locale=="en-US"?"Invalid GO record number: ":
            locale=="es-419"?"Número de registro de GO no válido: ":
            "Número de registro de GO inválido: ") + operand;
        expect(locale=="qps-ploc"?message.starts_with("[!! ")&&message.find(operand)!=std::string::npos:
            message==expected,"GO safe localized operand "+locale+mode+" got "+message);
        const auto events=std::count_if(state.events.begin(),state.events.end(),[](const auto& event){return event.category=="runtime.go";});
        expect(events==1,"GO locale no successful rejected event");
        std::error_code ignored; fs::remove_all(dir,ignored);
    }
}
void rejected_row_buffer_does_not_commit() {
    for (const std::string mode : {"COPPERFIN","VFP9"})
    for (const int buffering : {2,3}) {
        const auto dir=fs::temp_directory_path()/("copperfin_go_rowbuffer_5611_"+mode+std::to_string(buffering));
        fs::create_directories(dir);
        const auto table=dir/"goguard.dbf", path=dir/"rowguard.prg";
        write_simple_dbf(table,{"one","two","three"});
        write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nUSE '"+table.generic_string()+"' ALIAS goguard\n=CURSORSETPROP('Buffering',"+std::to_string(buffering)+",'goguard')\nGO 2\nREPLACE NAME WITH 'dirty'\nnCode=0\nlAfter=.F.\nTRY\nGO (1E300)\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncSnapshot=ALLTRIM(STR(nCode,20,0))+'|'+IIF(lAfter,'T','F')+'|'+TRANSFORM(RECNO())+'|'+IIF(BOF(),'T','F')+'|'+IIF(EOF(),'T','F')+'|'+ALLTRIM(NAME)\n=TABLEREVERT(.T.,'goguard')\nUSE IN goguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
        auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
        const auto state=session.run(DebugResumeAction::continue_run);
        expect(state.completed,"GO row buffer completion "+mode+std::to_string(buffering)+": "+state.message);
        const auto snapshot=state.globals.find("csnapshot");
        const std::string actual=snapshot==state.globals.end()?"<missing>":format_value(snapshot->second);
        std::cout<<"ROWBUFFER "<<mode<<' '<<buffering<<' '<<actual<<'\n';
        expect(actual=="5|F|2|F|F|dirty","GO unsafe target before dirty row navigation "+mode+std::to_string(buffering)+" got "+actual);
        const auto disk=copperfin::vfp::parse_dbf_table_from_file(table.string(),3U);
        expect(disk.ok && disk.table.records.size()==3U,"GO row buffer disk setup/count");
        const std::string saved=disk.ok && disk.table.records.size()==3U && !disk.table.records[1].values.empty()
            ? disk.table.records[1].values[0].display_value : "<missing>";
        std::cout<<"ROWBUFFERDISK "<<mode<<' '<<buffering<<' '<<saved<<'\n';
        expect(saved=="two","GO unsafe target does not persist buffered edit "+mode+std::to_string(buffering)+" got "+saved);
        const auto events=std::count_if(state.events.begin(),state.events.end(),[](const auto& event){return event.category=="runtime.go";});
        expect(events==1,"GO row buffer only setup navigation event");
        std::error_code ignored; fs::remove_all(dir,ignored);
    }
}
void guarded_cases() {
    for (const std::string mode : {"COPPERFIN","VFP9"})
    for (const std::string command : {"GO","GOTO"})
    for (const int buffering : {0,4,5}) {
        const auto dir=fs::temp_directory_path()/("copperfin_go_record_5611_"+mode+command+std::to_string(buffering));
        fs::create_directories(dir);
        const auto path=dir/"gorecord.prg";
        std::string source="SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR goguard (label C(8))\nINSERT INTO goguard VALUES ('one')\nINSERT INTO goguard VALUES ('two')\nINSERT INTO goguard VALUES ('three')\n";
        if (buffering) source+="=CURSORSETPROP('Buffering',"+std::to_string(buffering)+",'goguard')\nAPPEND BLANK\nREPLACE label WITH 'pending1'\nAPPEND BLANK\nREPLACE label WITH 'pending2'\nSELECT 0\nCREATE CURSOR observer (label C(8))\nINSERT INTO observer VALUES ('sentinel')\n";
        source+="cOut=''\n";
        std::vector<std::string> expected;
        std::size_t accepted=0;
        const std::string active=buffering?"OBSERVER":"GOGUARD";
        const auto count=buffering?5:3;
        for (const auto& row : cases) {
            const auto target=mode=="VFP9"?row.legacy:row.normal;
            if (target) ++accepted;
            long long rec=2; bool bof=false,eof=false; std::string payload="two";
            if (target) {
                if (buffering && (*target==-1 || *target==-2)) {
                    rec=*target; payload=*target==-1?"pending1":"pending2";
                } else if (*target<=0) { rec=1; bof=true; payload="one"; }
                else if (*target>count) {rec=count+1; eof=true; payload="";}
                else {
                    rec=*target; payload=rec==1?"one":rec==2?"two":rec==3?"three":rec==4?"pending1":"pending2";
                    if (buffering && rec>3) rec=-(rec-3);
                }
            }
            expected.push_back(std::string(target?"0|T|":"5|F|")+std::to_string(rec)+"|"+(bof?"T":"F")+"|"+(eof?"T":"F")+"|"+payload+"|"+std::to_string(count)+"|"+active+"|1|"+mode);
            source+="GO 2 IN goguard\nnCode=0\nlAfter=.F.\nTRY\n"+command+" ("+row.expression+")"+(buffering?" IN goguard":"")+"\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncOut=cOut+ALLTRIM(STR(nCode,20,0))+'|'+IIF(lAfter,'T','F')+'|'+ALLTRIM(STR(RECNO('goguard'),20,0))+'|'+IIF(BOF('goguard'),'T','F')+'|'+IIF(EOF('goguard'),'T','F')+'|'+ALLTRIM(goguard.label)+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+UPPER(ALIAS())+'|'+SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+CHR(10)\n";
        }
        if (buffering) source+="=TABLEREVERT(.T.,'goguard')\nUSE IN observer\n";
        source+="USE IN goguard\nSET NUMERICBEHAVIOR TO COPPERFIN\ncCleanup=IIF(USED('goguard'),'open','closed')+'|'+IIF(USED('observer'),'open','closed')+'|'+SET('NUMERICBEHAVIOR')\nRETURN\n";
        write_text(path,source);
        auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
        const auto state=session.run(DebugResumeAction::continue_run);
        expect(state.completed,"GO guarded completion "+mode+command+std::to_string(buffering)+": "+state.message);
        const auto output=state.globals.find("cout");
        std::istringstream stream(output==state.globals.end()?"<missing>":format_value(output->second));
        std::string line;
        for (std::size_t i=0;i<expected.size();++i) {
            std::getline(stream,line);
            if (i==0 || i==3 || i==16 || i==40 || i==44)
                std::cout<<"CONTROL "<<mode<<' '<<command<<' '<<buffering<<' '<<cases[i].expression<<' '<<line<<'\n';
            expect(line.ends_with("|"+std::to_string(count)+"|"+active+"|1|"+mode),"GO count/alias/session/mode "+std::to_string(i));
            expect(line==expected[i],"GO row "+mode+command+std::to_string(buffering)+" "+cases[i].expression+" expected "+expected[i]+" got "+line);
        }
        expect(!std::getline(stream,line),"GO no extra rows");
        const auto go_events=std::count_if(state.events.begin(),state.events.end(),[](const auto& event){return event.category=="runtime.go";});
        expect(static_cast<std::size_t>(go_events)==cases.size()+accepted,"GO success events "+mode+command+std::to_string(buffering)+" expected "+std::to_string(cases.size()+accepted)+" got "+std::to_string(go_events));
        const auto cleanup=state.globals.find("ccleanup");
        expect(cleanup!=state.globals.end()&&format_value(cleanup->second)=="closed|closed|COPPERFIN","GO cleanup/reset");
        std::error_code ignored; fs::remove_all(dir,ignored);
    }
}
}
int main() {
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    direct_boundaries();
    guarded_cases();
    rejected_row_buffer_does_not_commit();
    localized_rejections();
    return test_failures()==0?0:1;
}
