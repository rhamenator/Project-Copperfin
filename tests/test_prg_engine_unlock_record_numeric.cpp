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
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-UNLOCK-RECORD-NUMERIC-001: conversion constants independently
// recovered from the retained native shared-table output. Record existence
// is a preservation control, not an inferred native requirement.
struct Case { const char* expression; std::optional<std::uint64_t> normal,legacy; };
const std::vector<Case> cases{
 {"-1.9",std::nullopt,std::nullopt},{"-0.9",0,0},{"0",0,0},{"0.5",0,0},
 {"0.9",0,0},{"1",1,1},{"1.5",1,1},{"1.9",1,1},{"2.5",2,2},{"3.9",3,3},{"4",4,4},
 {"2147483647",INT32_MAX,INT32_MAX},{"2147483648",std::nullopt,std::nullopt},
 {"4294967295",std::nullopt,std::nullopt},{"4294967296",std::nullopt,UINT64_C(4294967296)},
 {"4294967297",std::nullopt,UINT64_C(4294967297)},
 {"-2147483648",std::nullopt,std::nullopt},{"-2147483649",std::nullopt,INT32_MAX},
 {"-4294967295",std::nullopt,1},{"-4294967296",std::nullopt,std::nullopt},
 {"-4294967297",std::nullopt,std::nullopt},
 {"1E300",std::nullopt,0},{"-1E300",std::nullopt,std::nullopt},
 {"1E300*1E300",std::nullopt,0},{"-1E300*1E300",std::nullopt,std::nullopt},
 {"$1.5",2,2},{"'1.5'",2,2},{"'1e300'",std::nullopt,std::nullopt},
 {".T.",1,1},{".F.",0,0},{".NULL.",0,0}
};
void direct_boundaries() {
 struct Direct { PrgValue value; std::optional<std::uint64_t> normal,legacy; };
 const double inf=std::numeric_limits<double>::infinity();
 const std::vector<Direct> rows{
 {make_number_value(-0.0),0,0},{make_number_value(-0.9),0,0},
 {make_number_value(0.5),0,0},{make_number_value(1.9),1,1},
 {make_number_value(-1.9),std::nullopt,std::nullopt},
 {make_number_value(std::nextafter(2147483648.0,0.0)),INT32_MAX,INT32_MAX},
 {make_number_value(2147483648.0),std::nullopt,std::nullopt},
 {make_number_value(std::nextafter(4294967296.0,0.0)),std::nullopt,std::nullopt},
 {make_number_value(4294967296.0),std::nullopt,UINT64_C(4294967296)},
 {make_number_value(4294967297.0),std::nullopt,UINT64_C(4294967297)},
 {make_number_value(-2147483649.0),std::nullopt,INT32_MAX},
 {make_number_value(-4294967295.0),std::nullopt,1},
 {make_number_value(-4294967296.0),std::nullopt,std::nullopt},
 {make_number_value(1e300),std::nullopt,0},{make_number_value(-1e300),std::nullopt,std::nullopt},
 {make_number_value(inf),std::nullopt,0},{make_number_value(-inf),std::nullopt,std::nullopt},
 {make_number_value(std::numeric_limits<double>::quiet_NaN()),std::nullopt,std::nullopt},
 {make_number_value(std::nextafter(9223372036854775808.0,0.0)),std::nullopt,UINT64_C(9223372036854774784)},
 {make_number_value(9223372036854775808.0),std::nullopt,0},
 {make_number_value(-9223372036854775808.0),std::nullopt,std::nullopt},
 {make_int64_value(0),0,0},{make_int64_value(INT32_MAX),INT32_MAX,INT32_MAX},
 {make_int64_value(INT32_MIN),std::nullopt,std::nullopt},
 {make_int64_value(INT64_MAX),std::nullopt,static_cast<std::uint64_t>(INT64_MAX)},
 {make_int64_value(INT64_MIN),std::nullopt,std::nullopt},
 {make_int64_value(INT64_MIN+1),std::nullopt,1},
 {make_int64_value(INT64_C(9007199254740993)),std::nullopt,UINT64_C(9007199254740993)},
 {make_uint64_value(0),0,0},{make_uint64_value(1),1,1},
 {make_uint64_value(INT32_MAX),INT32_MAX,INT32_MAX},
 {make_uint64_value(UINT64_C(2147483648)),std::nullopt,std::nullopt},
 {make_uint64_value(UINT64_C(4294967297)),std::nullopt,UINT64_C(4294967297)},
 {make_uint64_value(UINT64_MAX),std::nullopt,UINT64_MAX},
 {make_currency_value(15000),2,2},{make_currency_value(INT64_MAX),std::nullopt,std::nullopt},
 {make_string_value("1.5"),2,2},{make_string_value("1e300"),std::nullopt,std::nullopt},
 {make_boolean_value(true),1,1},{make_boolean_value(false),0,0},{make_null_value(),0,0}
 };
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9})
 for(std::size_t i=0;i<rows.size();++i)
 expect(checked_unlock_record_argument(rows[i].value,mode)==
 (mode==NumericBehavior::vfp9?rows[i].legacy:rows[i].normal),"UNLOCK direct "+std::to_string(i));
 std::cout<<"DIRECTCOUNT "<<rows.size()*2<<'\n';
}
std::string value(const RuntimePauseState& state,const char* key) {
 const auto it=state.globals.find(key);
 return it==state.globals.end()?"<missing>":format_value(it->second);
}
void guarded_cases() {
 // Fresh sessions isolate each operand from the existing stale explicit-error
 // metadata on generic missing-record failures (#7085); no lifecycle fix here.
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const bool qualified:{false,true})
 for(const auto& row:cases) {
 const auto dir=fs::temp_directory_path()/("copperfin_unlock_numeric_5611_"+mode+std::to_string(qualified));
 fs::create_directories(dir);const auto path=dir/"unlock.prg";
 std::string source="SET NUMERICBEHAVIOR TO "+mode+"\nSET MULTILOCKS ON\nCREATE CURSOR unlockguard (label C(8))\nINSERT INTO unlockguard VALUES ('one')\nINSERT INTO unlockguard VALUES ('two')\nINSERT INTO unlockguard VALUES ('three')\n";
 if(qualified)source+="SELECT 0\nCREATE CURSOR observer (label C(8))\nINSERT INTO observer VALUES ('sentinel')\n";
 source+="cOut=''\n";
 std::vector<std::string> expected;std::size_t accepted=0;
 const std::string active=qualified?"OBSERVER":"UNLOCKGUARD";
 const auto target=mode=="VFP9"?row.legacy:row.normal;
 const bool exists=target && *target>=1 && *target<=3;
 if(exists)++accepted;
 // Existing missing-record errors remain code1; native harmless no-op
 // differences are explicitly retained as a separate gap, not fixed here.
 const std::string prefix=!target?"10|F|":exists?"0|T|":"1|F|";
 expected.push_back(prefix+std::string(exists&&*target==1?"F":"T")+"|"+
 (exists&&*target==2?"F":"T")+"|"+(exists&&*target==3?"F":"T")+"|2|two|"+active+"|1|"+mode+"|T");
 source+="SELECT unlockguard\nUNLOCK\nGO 1\n=RLOCK()\nGO 2\n=RLOCK()\nGO 3\n=RLOCK()\nGO 2\nlSetup=ISRLOCKED(1,'unlockguard') AND ISRLOCKED(2,'unlockguard') AND ISRLOCKED(3,'unlockguard')\n";
 if(qualified)source+="SELECT observer\n";
 // Observe caller selection before selecting the target for one-argument
 // lock queries; existing two-argument ISRLOCKED alias handling is separate.
 source+="nCode=0\nlAfter=.F.\nTRY\nUNLOCK RECORD ("+std::string(row.expression)+")"+(qualified?" IN unlockguard":"")+"\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncCaller=UPPER(ALIAS())\nSELECT unlockguard\ncOut=cOut+TRANSFORM(nCode)+'|'+IIF(lAfter,'T','F')+'|'+IIF(ISRLOCKED(1),'T','F')+'|'+IIF(ISRLOCKED(2),'T','F')+'|'+IIF(ISRLOCKED(3),'T','F')+'|'+TRANSFORM(RECNO('unlockguard'))+'|'+ALLTRIM(unlockguard.label)+'|'+cCaller+'|'+SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+IIF(lSetup,'T','F')+CHR(10)\n";
 source+="SELECT unlockguard\nUNLOCK\nUSE IN unlockguard\n";
 if(qualified)source+="USE IN observer\n";
 source+="SET MULTILOCKS OFF\nSET NUMERICBEHAVIOR TO COPPERFIN\ncCleanup=IIF(USED('unlockguard'),'open','closed')+'|'+IIF(USED('observer'),'open','closed')+'|'+SET('MULTILOCKS')+'|'+SET('NUMERICBEHAVIOR')\nRETURN\n";
 write_text(path,source);
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto state=session.run(DebugResumeAction::continue_run);
 const std::string tag=mode+std::to_string(qualified);
 expect(state.completed,"UNLOCK completion "+tag+": "+state.message);
 std::istringstream stream(value(state,"cout"));std::string line;
 for(std::size_t i=0;i<expected.size();++i) {
 std::getline(stream,line);
 if(std::string(row.expression)=="1.5"||std::string(row.expression)=="4294967297"||std::string(row.expression)=="-4294967295"||std::string(row.expression)=="$1.5"||std::string(row.expression)=="'1e300'")
 std::cout<<"CONTROL "<<tag<<' '<<row.expression<<' '<<line<<'\n';
 expect(line.ends_with("|2|two|"+active+"|1|"+mode+"|T"),"UNLOCK setup/pointer/session suffix "+tag+" "+std::to_string(i));
 expect(line==expected[i],"UNLOCK row "+tag+" "+row.expression+" expected "+expected[i]+" got "+line);
 }
 expect(!std::getline(stream,line),"UNLOCK no extra rows");
 const auto events=std::count_if(state.events.begin(),state.events.end(),[](const auto& e){
 return e.category=="runtime.unlock" && e.detail.find(" RECORD ")!=std::string::npos;});
 expect(static_cast<std::size_t>(events)==accepted,"UNLOCK success events "+tag);
 expect(value(state,"ccleanup")=="closed|closed|OFF|COPPERFIN","UNLOCK cleanup/reset");
 std::error_code ignored;fs::remove_all(dir,ignored);
 }
}
}
namespace {
void localized_rejections_and_target_session() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
 for(const std::string language:{"en-US","es-419","pt-BR","qps-ploc"})
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const bool numeric:{false,true}) {
 if(numeric&&mode=="VFP9")continue;
 set_env_value("COPPERFIN_LOCALE",language.c_str(),true);
 const auto dir=fs::temp_directory_path()/("copperfin_unlock_locale_5611_"+language+mode+std::to_string(numeric));
 fs::create_directories(dir);const auto path=dir/"locale.prg";
 write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR unlockguard (label C(8))\nINSERT INTO unlockguard VALUES ('one')\n=RLOCK()\nnCode=0\ncMessage=''\nlMetadata=.F.\nTRY\nUNLOCK RECORD ("+std::string(numeric?"1E300":"'1e300'")+")\nCATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\n=AERROR(aError)\nlMetadata=ERROR()==10 AND aError[1,1]==10 AND aError[1,2]==cMessage\nENDTRY\ncState=TRANSFORM(RECNO())+'|'+IIF(ISRLOCKED(),'T','F')+'|'+SET('DATASESSION')\nUNLOCK\nUSE IN unlockguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"UNLOCK locale completion "+language+mode+": "+state.message);
 expect(value(state,"ncode")=="10"&&value(state,"lmetadata")=="true","UNLOCK locale metadata");
 expect(value(state,"cstate")=="1|T|1","UNLOCK locale lock/pointer/session");
 const std::string operand=numeric?"1"+std::string(300,'0'):"1e300";
 const std::string expected=(language=="en-US"?"Invalid UNLOCK RECORD number: ":language=="es-419"?"Número de UNLOCK RECORD no válido: ":"Número de UNLOCK RECORD inválido: ")+operand;
 const auto message=value(state,"cmessage");
 expect(language=="qps-ploc"?message.starts_with("[!! ")&&message.find(operand)!=std::string::npos:message==expected,"UNLOCK safe original localized operand "+language+mode+" got "+message);
 expect(std::none_of(state.events.begin(),state.events.end(),[](const auto& e){return e.category=="runtime.unlock"&&e.detail.find(" RECORD ")!=std::string::npos;}),"UNLOCK rejected locale event");
 std::error_code ignored;fs::remove_all(dir,ignored);
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 for(const std::string target_mode:{"COPPERFIN","VFP9"}) {
 const auto dir=fs::temp_directory_path()/("copperfin_unlock_session_5611_"+target_mode);
 fs::create_directories(dir);const auto path=dir/"session.prg";
 const std::string other=target_mode=="VFP9"?"COPPERFIN":"VFP9";
 write_text(path,"SET NUMERICBEHAVIOR TO "+target_mode+"\nCREATE CURSOR unlockguard (label C(8))\nINSERT INTO unlockguard VALUES ('one')\n=RLOCK()\nnCalls=0\nnCode=0\nlAfter=.F.\nTRY\nUNLOCK RECORD EVALUATE('SwitchSession()') IN unlockguard\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncCaller=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')\nSET DATASESSION TO 1\ncOrigin=TRANSFORM(nCode)+'|'+IIF(lAfter,'T','F')+'|'+IIF(ISRLOCKED(),'T','F')+'|'+TRANSFORM(nCalls)+'|'+SET('NUMERICBEHAVIOR')\nUNLOCK\nUSE IN unlockguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 2\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nRETURN\nFUNCTION SwitchSession\nnCalls=nCalls+1\nSET DATASESSION TO 2\nSET NUMERICBEHAVIOR TO "+other+"\nRETURN -4294967295\nENDFUNC\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"UNLOCK resumed target session completion "+target_mode+": "+state.message);
 expect(value(state,"ccaller")=="2|"+other,"UNLOCK restores callback-selected session");
 expect(value(state,"corigin")==(target_mode=="VFP9"?"0|T|F|1|VFP9":"10|F|T|1|COPPERFIN"),"UNLOCK conversion uses captured target mode exactly once");
 std::error_code ignored;fs::remove_all(dir,ignored);
 }
}
void dirty_row_and_disk_preservation() {
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const int buffering:{2,3}) {
 const auto dir=fs::temp_directory_path()/("copperfin_unlock_dirty_5611_"+mode+std::to_string(buffering));
 fs::create_directories(dir);const auto table=dir/"guard.dbf",path=dir/"dirty.prg";
 write_simple_dbf(table,{"one","two","three"});
 write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nUSE '"+table.generic_string()+"' ALIAS unlockguard\n=CURSORSETPROP('Buffering',"+std::to_string(buffering)+",'unlockguard')\nGO 2\n=RLOCK()\nREPLACE NAME WITH 'dirty'\nnCode=0\nTRY\nUNLOCK RECORD ('1e300')\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncState=TRANSFORM(nCode)+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(NAME)+'|'+IIF(ISRLOCKED(),'T','F')\n=TABLEREVERT(.T.,'unlockguard')\nUNLOCK\nUSE IN unlockguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"UNLOCK dirty completion "+mode+std::to_string(buffering)+": "+state.message);
 expect(value(state,"cstate")=="10|2|dirty|T","UNLOCK rejection retains dirty buffer and record lock");
 const auto disk=copperfin::vfp::parse_dbf_table_from_file(table.string(),3U);
 expect(disk.ok&&disk.table.records.size()==3U&&!disk.table.records[1].values.empty()&&disk.table.records[1].values[0].display_value=="two","UNLOCK rejection does not persist dirty row");
 std::error_code ignored;fs::remove_all(dir,ignored);
 }
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();guarded_cases();localized_rejections_and_target_session();dirty_row_and_disk_preservation();
 return test_failures()==0?0:1;
}
