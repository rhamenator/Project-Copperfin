// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-SLEEP-DURATION-NUMERIC-001: owner-derived extension constants,
// not recovered from rejected native SLEEP spellings or implementation output.
std::optional<std::size_t> admitted(std::optional<std::uint64_t> value) {
 if(!value || *value>std::numeric_limits<std::size_t>::max())return std::nullopt;
 return static_cast<std::size_t>(*value);
}
void direct_boundaries() {
 struct Row {PrgValue value; std::optional<std::uint64_t> expected;};
 const double inf=std::numeric_limits<double>::infinity();
 const std::vector<Row> rows{
 {make_number_value(-0.0),0},{make_number_value(-0.1),std::nullopt},
 {make_number_value(0.49),0},{make_number_value(0.5),1},
 {make_number_value(1.5),2},{make_number_value(1.9),2},
 {make_number_value(2147483648.0),UINT64_C(2147483648)},
 {make_number_value(4294967295.0),UINT64_C(4294967295)},
 {make_number_value(4294967296.0),UINT64_C(4294967296)},
 {make_number_value(std::nextafter(9223372036854775808.0,0.0)),UINT64_C(9223372036854774784)},
 {make_number_value(9223372036854775808.0),std::nullopt},
 {make_number_value(-9223372036854775808.0),std::nullopt},
 {make_number_value(1e300),std::nullopt},{make_number_value(-1e300),std::nullopt},
 {make_number_value(inf),std::nullopt},{make_number_value(-inf),std::nullopt},
 {make_number_value(std::numeric_limits<double>::quiet_NaN()),std::nullopt},
 {make_int64_value(0),0},{make_int64_value(-1),std::nullopt},
 {make_int64_value(INT64_MAX),static_cast<std::uint64_t>(INT64_MAX)},
 {make_int64_value(INT64_MIN),std::nullopt},
 {make_int64_value(INT64_C(9007199254740993)),UINT64_C(9007199254740993)},
 {make_uint64_value(0),0},{make_uint64_value(1),1},
 {make_uint64_value(static_cast<std::uint64_t>(INT64_MAX)),static_cast<std::uint64_t>(INT64_MAX)},
 {make_uint64_value(UINT64_C(9223372036854775808)),std::nullopt},
 {make_uint64_value(UINT64_MAX),std::nullopt},
 {make_currency_value(5000),1},{make_currency_value(-1000),std::nullopt},
 {make_string_value("0.5"),1},{make_string_value("1e300"),std::nullopt},
 {make_boolean_value(true),1},{make_boolean_value(false),0},{make_null_value(),0}
 };
 for(std::size_t i=0;i<rows.size();++i)
 expect(checked_sleep_duration_argument(rows[i].value)==admitted(rows[i].expected),"SLEEP direct "+std::to_string(i));
 std::cout<<"DIRECTCOUNT "<<rows.size()<<'\n';
}
std::string value(const RuntimePauseState& s,const char* key) {
 const auto i=s.globals.find(key);return i==s.globals.end()?"<missing>":format_value(i->second);
}
void guarded_cases() {
 struct Row {const char* expression; std::optional<std::uint64_t> duration;};
 const std::vector<Row> rows{
 {"-0.1",std::nullopt},{"-1",std::nullopt},{"0",0},{"0.49",0},
 {"0.5",1},{"1",1},{"1.5",2},{"2147483648",UINT64_C(2147483648)},
 {"4294967295",UINT64_C(4294967295)},{"4294967296",UINT64_C(4294967296)},
 {"9223372036854774784",UINT64_C(9223372036854774784)},
 {"9223372036854775808",std::nullopt},{"1E300",std::nullopt},
 {"-1E300",std::nullopt},{"1E300*1E300",std::nullopt},
 {"-1E300*1E300",std::nullopt},{"$0.5",1},{"'0.5'",1},
 {"'1e300'",std::nullopt},{".T.",1},{".F.",0},{".NULL.",0}
 };
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const auto& row:rows)
 for(const bool critical:{false,true}) {
 // No unsafe or large admitted duration may physically wait in this test.
 // Critical-section containment catches every positive original conversion.
 if(!critical && row.duration && *row.duration>2)continue;
 if(!critical && !row.duration && std::string(row.expression)!="-0.1" && std::string(row.expression)!="-1")continue;
 const auto duration=admitted(row.duration);
 const bool blocked=critical&&duration&&*duration>0;
 const std::string expected=(!duration?"11|F|":blocked?"1|F|":"0|T|")+mode+"|1|GUARD|1|guard";
 const auto dir=fs::temp_directory_path()/"copperfin_sleep_numeric_5611";fs::create_directories(dir);
 const auto path=dir/"sleep.prg";
 std::string source="SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\nnCode=0\nlAfter=.F.\n";
 if(critical)source+="ENTER CRITICAL sleepguard\n";
 source+="TRY\nSLEEP ("+std::string(row.expression)+")\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncState=TRANSFORM(nCode)+'|'+IIF(lAfter,'T','F')+'|'+SET('NUMERICBEHAVIOR')+'|'+SET('DATASESSION')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n";
 if(critical)source+="EXIT CRITICAL sleepguard\n";
 source+="USE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\ncCleanup=IIF(USED('guard'),'open','closed')+'|'+SET('NUMERICBEHAVIOR')\nRETURN\n";
 write_text(path,source);auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto s=session.run(DebugResumeAction::continue_run);
 const std::string tag=mode+" "+row.expression+" critical="+std::to_string(critical);
 expect(s.completed,"SLEEP guarded completion "+tag+": "+s.message);
 expect(value(s,"cstate")==expected,"SLEEP guarded "+tag+" expected "+expected+" got "+value(s,"cstate"));
 const auto sleeps=std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.sleep";});
 const auto blocks=std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.critical.blocking_violation";});
 expect(sleeps==(!duration||blocked?0:1),"SLEEP success-event count "+tag);
 expect(blocks==(blocked?1:0),"SLEEP rejection before blocking-policy event "+tag);
 if(duration&&!blocked)expect(std::any_of(s.events.begin(),s.events.end(),[&](const auto& e){return e.category=="runtime.sleep"&&e.detail.starts_with("duration="+std::to_string(*duration)+"ms");}),"SLEEP resolved duration "+tag);
 expect(value(s,"ccleanup")=="closed|COPPERFIN","SLEEP cleanup "+tag);
 std::error_code ec;fs::remove_all(dir,ec);
 }
}
void localized_metadata_and_defaults() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
 for(const std::string language:{"en-US","es-419","pt-BR","qps-ploc"})
 for(const std::string mode:{"COPPERFIN","VFP9"}) {
 set_env_value("COPPERFIN_LOCALE",language.c_str(),true);
 const auto dir=fs::temp_directory_path()/"copperfin_sleep_locale_5611";fs::create_directories(dir);const auto path=dir/"locale.prg";
 write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nENTER CRITICAL guard\nnCode=0\nlMetadata=.F.\ncMessage=''\nTRY\nSLEEP 1E300\nCATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\n=AERROR(aError)\nlMetadata=ERROR()==11 AND aError[1,1]==11 AND aError[1,2]==cMessage\nENDTRY\nEXIT CRITICAL guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));const auto s=session.run(DebugResumeAction::continue_run);
 expect(s.completed,"SLEEP localized completion");expect(value(s,"ncode")=="11"&&value(s,"lmetadata")=="true","SLEEP ERROR/AERROR metadata");
 const auto message=value(s,"cmessage");const std::string expected=language=="es-419"?"SLEEP: duracion no valida":language=="pt-BR"?"SLEEP: duracao invalida":"SLEEP: invalid duration";
 expect(language=="qps-ploc"?message.starts_with("[!! ")&&message.ends_with(" !!]")&&message!=expected:message==expected,"SLEEP localized invalid duration "+language+" got "+message);
 expect(std::none_of(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.sleep"||e.category=="runtime.critical.blocking_violation";}),"SLEEP locale rejects before waiting/block event");
 std::error_code ec;fs::remove_all(dir,ec);
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 for(const std::string mode:{"COPPERFIN","VFP9"}) {
 const auto dir=fs::temp_directory_path()/"copperfin_sleep_default_5611";fs::create_directories(dir);const auto path=dir/"default.prg";
 write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nENTER CRITICAL guard\nSLEEP\nEXIT CRITICAL guard\nnCalls=0\nSLEEP EVALUATE('Delay()')\nnAfter=nCalls\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\nFUNCTION Delay\nnCalls=nCalls+1\nRETURN 0\nENDFUNC\n");
 auto options=make_runtime_session_options(path,dir,false);options.scheduler_yield_sleep_ms=0;
 auto session=PrgRuntimeSession::create(options);const auto s=session.run(DebugResumeAction::continue_run);
 expect(s.completed&&value(s,"nafter")=="1","SLEEP default/one-time resumed expression");
 expect(std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.sleep"&&e.detail.starts_with("duration=0ms");})==2,"SLEEP default and resumed event exactly once each");
 std::error_code ec;fs::remove_all(dir,ec);
}
}
void resumed_session_boundaries() {
 // Added after the original-dispatch baseline; not baseline evidence.
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const bool invalid:{false,true}) {
 const auto dir=fs::temp_directory_path()/"copperfin_sleep_resumed_5611";
 fs::create_directories(dir);const auto path=dir/"resumed.prg";
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 std::string source="SET NUMERICBEHAVIOR TO "+mode+"\nnCalls=0\nnCode=0\nlAfter=.F.\n";
 if(invalid)source+="ENTER CRITICAL guard\n";
 source+="TRY\nSLEEP EVALUATE('SwitchSession()')\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncCaller=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')\n";
 if(invalid)source+="EXIT CRITICAL guard\n";
 source+="SET DATASESSION TO 1\ncOrigin=TRANSFORM(nCode)+'|'+IIF(lAfter,'T','F')+'|'+TRANSFORM(nCalls)+'|'+SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 2\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nRETURN\nFUNCTION SwitchSession\nnCalls=nCalls+1\nSET DATASESSION TO 2\nSET NUMERICBEHAVIOR TO "+other+"\nRETURN "+(invalid?"1E300":"0.5")+"\nENDFUNC\n";
 write_text(path,source);auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto s=session.run(DebugResumeAction::continue_run);
 expect(s.completed,"SLEEP resumed boundary completion "+mode);
 expect(value(s,"ccaller")=="2|"+other,"SLEEP preserves callback-selected session/mode");
 expect(value(s,"corigin")==std::string(invalid?"11|F|1|":"0|T|1|")+mode,"SLEEP resumed duration exactly once/origin state");
 expect(std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.sleep"&&e.detail.starts_with("duration=1ms");})==(invalid?0:1),"SLEEP resumed half-away success event");
 expect(std::none_of(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.critical.blocking_violation";}),"SLEEP resumed invalid conversion precedes critical event");
 std::error_code ec;fs::remove_all(dir,ec);
 }
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();guarded_cases();localized_metadata_and_defaults();resumed_session_boundaries();return test_failures()==0?0:1;
}
