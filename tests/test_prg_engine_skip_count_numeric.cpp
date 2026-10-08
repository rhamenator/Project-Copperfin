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
// RQ-CF-PRG-SKIP-COUNT-NUMERIC-001: independently retained count constants.
// Saturated pointer expectations below are preservation controls, not a
// replacement requirement for shared navigation or native type admission.
struct Case { const char* expression; std::optional<long long> normal,legacy; };
const std::vector<Case> cases{
 {"-1.9",-1,-1},{"-1.5",-1,-1},{"-0.9",0,0},{"-0.5",0,0},{"0",0,0},
 {"0.5",0,0},{"0.9",0,0},{"1",1,1},{"1.5",1,1},{"1.9",1,1},{"2.5",2,2},
 {"2147483647",2147483647,2147483647},
 {"2147483648",std::nullopt,-2147483648LL},
 {"2147483649",std::nullopt,-2147483647LL},
 {"4294967295",std::nullopt,-1},{"4294967296",std::nullopt,0},
 {"4294967297",std::nullopt,1},{"4294967298",std::nullopt,2},
 {"-2147483648",-2147483648LL,-2147483648LL},
 {"-2147483649",std::nullopt,2147483647},
 {"-4294967295",std::nullopt,1},{"-4294967296",std::nullopt,0},
 {"-4294967297",std::nullopt,-1},{"-4294967298",std::nullopt,-2},
 {"9007199254740992",std::nullopt,0},{"1E20",std::nullopt,0},
 {"-1E20",std::nullopt,0},{"1E300",std::nullopt,0},{"-1E300",std::nullopt,0},
 {"1E300*1E300",std::nullopt,0},{"-1E300*1E300",std::nullopt,0},
 {"$1.5",2,2},{"-$1.5",-2,-2},{"$0.5",1,1},
 {"$922337203685477.5807",std::nullopt,std::nullopt},
 {"'1.5'",2,2},{"'1e300'",std::nullopt,std::nullopt},
 {".T.",1,1},{".F.",0,0},{".NULL.",0,0}
};
void direct_boundaries() {
 struct Direct { PrgValue value; std::optional<std::int32_t> normal,legacy; };
 const double inf=std::numeric_limits<double>::infinity();
 const std::vector<Direct> rows{
 {make_number_value(-0.0),0,0},{make_number_value(-0.5),0,0},
 {make_number_value(0.5),0,0},{make_number_value(1.9),1,1},
 {make_number_value(-1.9),-1,-1},{make_number_value(2147483647.0),INT32_MAX,INT32_MAX},
 {make_number_value(2147483648.0),std::nullopt,INT32_MIN},
 {make_number_value(std::nextafter(2147483648.0,0.0)),INT32_MAX,INT32_MAX},
 {make_number_value(std::nextafter(2147483648.0,inf)),std::nullopt,INT32_MIN},
 {make_number_value(-2147483648.0),INT32_MIN,INT32_MIN},
 {make_number_value(std::nextafter(-2147483649.0,0.0)),INT32_MIN,INT32_MIN},
 {make_number_value(-2147483649.0),std::nullopt,INT32_MAX},
 {make_number_value(4294967295.0),std::nullopt,-1},
 {make_number_value(4294967296.0),std::nullopt,0},
 {make_number_value(4294967297.0),std::nullopt,1},
 {make_number_value(-4294967295.0),std::nullopt,1},
 {make_number_value(-4294967297.0),std::nullopt,-1},
 {make_number_value(1e300),std::nullopt,0},{make_number_value(-1e300),std::nullopt,0},
 {make_number_value(inf),std::nullopt,0},{make_number_value(-inf),std::nullopt,0},
 {make_number_value(std::numeric_limits<double>::quiet_NaN()),std::nullopt,std::nullopt},
 {make_number_value(std::nextafter(9223372036854775808.0,0.0)),std::nullopt,-1024},
 {make_number_value(9223372036854775808.0),std::nullopt,0},
 {make_number_value(-9223372036854775808.0),std::nullopt,0},
 {make_number_value(std::nextafter(-9223372036854775808.0,-inf)),std::nullopt,0},
 {make_int64_value(0),0,0},{make_int64_value(INT32_MAX),INT32_MAX,INT32_MAX},
 {make_int64_value(INT32_MIN),INT32_MIN,INT32_MIN},
 {make_int64_value(INT64_C(2147483648)),std::nullopt,INT32_MIN},
 {make_int64_value(INT64_MAX),std::nullopt,-1},
 {make_int64_value(INT64_MIN),std::nullopt,0},
 {make_int64_value(INT64_MIN+1),std::nullopt,1},
 {make_int64_value(INT64_C(9007199254740993)),std::nullopt,1},
 {make_uint64_value(0),0,0},{make_uint64_value(1),1,1},
 {make_uint64_value(INT32_MAX),INT32_MAX,INT32_MAX},
 {make_uint64_value(UINT64_C(2147483648)),std::nullopt,INT32_MIN},
 {make_uint64_value(UINT64_C(9007199254740993)),std::nullopt,1},
 {make_uint64_value(UINT64_MAX),std::nullopt,-1},
 {make_currency_value(15000),2,2},{make_currency_value(-15000),-2,-2},
 {make_currency_value(INT64_MAX),std::nullopt,std::nullopt},
 {make_currency_value(INT64_MIN),std::nullopt,std::nullopt},
 {make_string_value("1.5"),2,2},{make_string_value("1e300"),std::nullopt,std::nullopt},
 {make_boolean_value(true),1,1},{make_boolean_value(false),0,0},
 {make_null_value(),0,0}
 };
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9})
 for(std::size_t i=0;i<rows.size();++i)
 expect(checked_skip_count_argument(rows[i].value,mode)==
 (mode==NumericBehavior::vfp9?rows[i].legacy:rows[i].normal),"SKIP direct "+std::to_string(i));
 std::cout<<"DIRECTCOUNT "<<rows.size()*2<<'\n';
}
void guarded_cases() {
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const bool qualified:{false,true})
 for(const int buffering:{0,4,5}) {
 const auto dir=fs::temp_directory_path()/("copperfin_skip_count_5611_"+mode+std::to_string(qualified)+std::to_string(buffering));
 fs::create_directories(dir);
 const auto path=dir/"skipcount.prg";
 std::string source="SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR skipguard (label C(8))\nINSERT INTO skipguard VALUES ('one')\nINSERT INTO skipguard VALUES ('two')\nINSERT INTO skipguard VALUES ('three')\n";
 if(buffering) source+="=CURSORSETPROP('Buffering',"+std::to_string(buffering)+",'skipguard')\nAPPEND BLANK\nREPLACE label WITH 'pending1'\nAPPEND BLANK\nREPLACE label WITH 'pending2'\n";
 if(qualified) source+="SELECT 0\nCREATE CURSOR observer (label C(8))\nINSERT INTO observer VALUES ('sentinel')\n";
 source+="cOut=''\n";
 std::vector<std::string> expected;std::size_t accepted=0;
 const std::string active=qualified?"OBSERVER":"SKIPGUARD";
 const long long count=buffering?5:3, start=buffering?5:2;
 for(const auto& row:cases) {
 const auto delta=mode=="VFP9"?row.legacy:row.normal;
 long long physical=start; bool bof=false,eof=false;
 if(delta) {
 ++accepted;
 const long long requested=start+*delta;
 if(requested<=0){physical=1;bof=true;}
 else if(requested>count){physical=count+1;eof=true;}
 else physical=requested;
 }
 std::string payload=eof?"":physical==1?"one":physical==2?"two":physical==3?"three":physical==4?"pending1":"pending2";
 const long long rec=buffering&&!eof&&physical>3?-(physical-3):physical;
 expected.push_back(std::string(delta?"0|T|":"11|F|")+std::to_string(rec)+"|"+(bof?"T":"F")+"|"+(eof?"T":"F")+"|"+payload+"|"+std::to_string(count)+"|"+active+"|1|"+mode);
 source+="SELECT skipguard\nGO "+std::string(buffering?"-2":"2")+"\n"+(qualified?"SELECT observer\n":"")+"nCode=0\nlAfter=.F.\nTRY\nSKIP ("+row.expression+")"+(qualified?" IN skipguard":"")+"\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncOut=cOut+ALLTRIM(STR(nCode,20,0))+'|'+IIF(lAfter,'T','F')+'|'+ALLTRIM(STR(RECNO('skipguard'),20,0))+'|'+IIF(BOF('skipguard'),'T','F')+'|'+IIF(EOF('skipguard'),'T','F')+'|'+ALLTRIM(skipguard.label)+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+UPPER(ALIAS())+'|'+SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+CHR(10)\n";
 }
 if(buffering)source+="=TABLEREVERT(.T.,'skipguard')\n";
 if(qualified)source+="USE IN observer\n";
 source+="USE IN skipguard\nSET NUMERICBEHAVIOR TO COPPERFIN\ncCleanup=IIF(USED('skipguard'),'open','closed')+'|'+IIF(USED('observer'),'open','closed')+'|'+SET('NUMERICBEHAVIOR')\nRETURN\n";
 write_text(path,source);
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto state=session.run(DebugResumeAction::continue_run);
 const std::string tag=mode+std::to_string(qualified)+std::to_string(buffering);
 expect(state.completed,"SKIP guarded completion "+tag+": "+state.message);
 const auto output=state.globals.find("cout");
 std::istringstream stream(output==state.globals.end()?"<missing>":format_value(output->second));std::string line;
 for(std::size_t i=0;i<expected.size();++i) {
 std::getline(stream,line);
 if(i==1||i==15||i==20||i==31||i==35)
 std::cout<<"CONTROL "<<tag<<' '<<cases[i].expression<<' '<<line<<'\n';
 expect(line.ends_with("|"+std::to_string(count)+"|"+active+"|1|"+mode),"SKIP state suffix "+tag+" "+std::to_string(i));
 expect(line==expected[i],"SKIP row "+tag+" "+cases[i].expression+" expected "+expected[i]+" got "+line);
 }
 expect(!std::getline(stream,line),"SKIP no extra rows");
 const auto events=std::count_if(state.events.begin(),state.events.end(),[](const auto& e){return e.category=="runtime.skip";});
 expect(static_cast<std::size_t>(events)==accepted,"SKIP success events "+tag);
 const auto cleanup=state.globals.find("ccleanup");
 expect(cleanup!=state.globals.end()&&format_value(cleanup->second)=="closed|closed|COPPERFIN","SKIP cleanup/reset");
 std::error_code ignored;fs::remove_all(dir,ignored);
 }
}
void rejected_row_buffer_does_not_commit() {
    for (const std::string mode : {"COPPERFIN","VFP9"})
    for (const int buffering : {2,3}) {
        const auto dir=fs::temp_directory_path()/("copperfin_skip_rowbuffer_5611_"+mode+std::to_string(buffering));
        fs::create_directories(dir);
        const auto table=dir/"goguard.dbf", path=dir/"rowguard.prg";
        write_simple_dbf(table,{"one","two","three"});
        write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nUSE '"+table.generic_string()+"' ALIAS goguard\n=CURSORSETPROP('Buffering',"+std::to_string(buffering)+",'goguard')\nGO 2\nREPLACE NAME WITH 'dirty'\nnCode=0\nlAfter=.F.\nTRY\nSKIP ('1e300')\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncSnapshot=ALLTRIM(STR(nCode,20,0))+'|'+IIF(lAfter,'T','F')+'|'+TRANSFORM(RECNO())+'|'+IIF(BOF(),'T','F')+'|'+IIF(EOF(),'T','F')+'|'+ALLTRIM(NAME)\n=TABLEREVERT(.T.,'goguard')\nUSE IN goguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
        auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
        const auto state=session.run(DebugResumeAction::continue_run);
        expect(state.completed,"SKIP row buffer completion "+mode+std::to_string(buffering)+": "+state.message);
        const auto snapshot=state.globals.find("csnapshot");
        const std::string actual=snapshot==state.globals.end()?"<missing>":format_value(snapshot->second);
        std::cout<<"ROWBUFFER "<<mode<<' '<<buffering<<' '<<actual<<'\n';
        expect(actual=="11|F|2|F|F|dirty","SKIP unsafe count before dirty row navigation "+mode+std::to_string(buffering)+" got "+actual);
        const auto disk=copperfin::vfp::parse_dbf_table_from_file(table.string(),3U);
        expect(disk.ok && disk.table.records.size()==3U,"SKIP row buffer disk setup/count");
        const std::string saved=disk.ok && disk.table.records.size()==3U && !disk.table.records[1].values.empty()
            ? disk.table.records[1].values[0].display_value : "<missing>";
        std::cout<<"ROWBUFFERDISK "<<mode<<' '<<buffering<<' '<<saved<<'\n';
        expect(saved=="two","SKIP unsafe count does not persist buffered edit "+mode+std::to_string(buffering)+" got "+saved);
        const auto events=std::count_if(state.events.begin(),state.events.end(),[](const auto& event){return event.category=="runtime.skip";});
        expect(events==0,"SKIP row buffer only setup navigation event");
        std::error_code ignored; fs::remove_all(dir,ignored);
    }
}

}
namespace {
void localized_rejections_and_defaults() {
 ScopedEnvironmentValue locale_scope("COPPERFIN_LOCALE");
 for(const std::string locale:{"en-US","es-419","pt-BR","qps-ploc"})
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const bool numeric:{false,true}) {
 if(numeric&&mode=="VFP9")continue;
 set_env_value("COPPERFIN_LOCALE",locale.c_str(),true);
 const auto dir=fs::temp_directory_path()/("copperfin_skip_locale_5611_"+locale+mode+std::to_string(numeric));
 fs::create_directories(dir);const auto path=dir/"locale.prg";
 write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR skipguard (label C(8))\nINSERT INTO skipguard VALUES ('one')\nINSERT INTO skipguard VALUES ('two')\nGO 2\nnCode=0\ncMessage=''\nlMetadata=.F.\nTRY\nSKIP ("+std::string(numeric?"1E300":"'1e300'")+")\nCATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\n=AERROR(aError)\nlMetadata=ERROR()==11 AND aError[1,1]==11 AND aError[1,2]==cMessage\nENDTRY\ncState=TRANSFORM(RECNO())+'|'+ALLTRIM(label)+'|'+SET('DATASESSION')\nUSE IN skipguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"SKIP locale completion "+locale+mode);
 const auto value=[&](const char* key){const auto i=state.globals.find(key);return i==state.globals.end()?std::string("<missing>"):format_value(i->second);};
 expect(value("ncode")=="11"&&value("lmetadata")=="true","SKIP locale metadata");
 expect(value("cstate")=="2|two|1","SKIP locale pointer/session");
 const std::string operand=numeric?"1"+std::string(300,'0'):"1e300";
 const std::string expected=(locale=="en-US"?"Invalid SKIP count: ":locale=="es-419"?"Cantidad de SKIP no válida: ":"Contagem de SKIP inválida: ")+operand;
 const auto message=value("cmessage");
 expect(locale=="qps-ploc"?message.starts_with("[!! ")&&message.find(operand)!=std::string::npos:message==expected,"SKIP safe localized original operand "+locale+mode+" got "+message);
 expect(std::none_of(state.events.begin(),state.events.end(),[](const auto& e){return e.category=="runtime.skip";}),"SKIP locale rejected event");
 std::error_code ignored;fs::remove_all(dir,ignored);
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 for(const std::string mode:{"COPPERFIN","VFP9"}) {
 const auto dir=fs::temp_directory_path()/("copperfin_skip_defaults_5611_"+mode);
 fs::create_directories(dir);const auto path=dir/"defaults.prg";
 write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR skipguard (label C(8))\nINSERT INTO skipguard VALUES ('one')\nINSERT INTO skipguard VALUES ('two')\nGO 1\nSKIP\nnDefault=RECNO()\nSKIP 0\nnZero=RECNO()\nUSE IN skipguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"SKIP defaults completion");
 for(const char* key:{"ndefault","nzero"}){const auto i=state.globals.find(key);expect(i!=state.globals.end()&&format_value(i->second)=="2","SKIP default/zero preserves legacy syntax");}
 std::error_code ignored;fs::remove_all(dir,ignored);
 }
}
}
int main(){ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);direct_boundaries();guarded_cases();rejected_row_buffer_does_not_commit();localized_rejections_and_defaults();return test_failures()==0?0:1;}
