// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <cmath>
#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-DEFINE-BAR-NUMERIC-001: constants from retained native observations
// or explicitly derived finite/int32 admission, never implementation output.
struct Row {const char* text; double raw; std::optional<int> ordinary, legacy;};
const std::vector<Row> rows{
 {"-1",-1,{},{}},{"-0.1",-0.1,{},{}},{"0",0,{},{}},
 {"0.5",0.5,{},{}},{"0.9",0.9,{},{}},{"1",1,1,1},
 {"1.1",1.1,1,1},{"1.5",1.5,1,1},{"1.9",1.9,1,1},
 {"2",2,2,2},{"32767",32767,32767,32767},{"32768",32768,32768,32768},
 {"65535",65535,65535,65535},{"65536",65536,65536,65536},
 {"2147483647",2147483647.,INT32_MAX,INT32_MAX},
 {"2147483647.9",2147483647.9,INT32_MAX,INT32_MAX},
 {"2147483648",2147483648.,{},{}},{"4294967295",4294967295.,{},{}},
 {"4294967295.9",4294967295.9,{},{}},
 {"4294967296",4294967296.,{},INT32_MAX},
 {"4294967297",4294967297.,{},INT32_MAX},
 {"4294967296.1",4294967296.1,{},INT32_MAX},
 {"-4294967295",-4294967295.,{},1},{"-4294967294",-4294967294.,{},2},
 {"-4294967296",-4294967296.,{},{}},
 {"1E20",1e20,{},INT32_MAX},{"1E300",1e300,{},INT32_MAX},
 {"-1E300",-1e300,{},{}}
};
std::string value(const RuntimePauseState& state, const char* key) {
 const auto found=state.globals.find(key);
 return found==state.globals.end()?"<missing>":format_value(found->second);
}
void direct_boundaries() {
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  for(const auto& row:rows)
   expect(checked_define_bar_number_argument(row.raw,mode)==(mode==NumericBehavior::vfp9?row.legacy:row.ordinary),"DEFINE BAR direct "+std::string(row.text));
  for(const auto number:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
   expect(!checked_define_bar_number_argument(number,mode),"DEFINE BAR nonfinite both modes");
  expect(checked_define_bar_number_argument(std::nextafter(1.,0.),mode)==std::nullopt,"DEFINE BAR just below1");
  expect(checked_define_bar_number_argument(std::nextafter(2147483648.,0.),mode)==INT32_MAX,"DEFINE BAR immediately below2^31");
  expect(!checked_define_bar_number_argument(std::nextafter(4294967296.,0.),mode),"DEFINE BAR immediately below2^32");
 }
}
void guarded_cases() {
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const auto& row:rows) {
  const auto id=mode=="VFP9"?row.legacy:row.ordinary;
  const bool existing=id&&(*id==1||*id==2||*id==INT32_MAX);
  const auto dir=fs::temp_directory_path()/"copperfin_define_bar_numeric_5611";
  fs::create_directories(dir);const auto path=dir/"bar.prg";
  const std::string source="SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\nDEFINE POPUP numguard RELATIVE\nDEFINE BAR 1 OF numguard PROMPT 'one'\nDEFINE BAR 2 OF numguard PROMPT 'two'\nDEFINE BAR 2147483647 OF numguard PROMPT 'cap'\nnCode=0\nnCalls=0\nlAfter=.F.\nTRY\nDEFINE BAR "+row.text+" OF numguard PROMPT EVALUATE('GetPrompt()')\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\nnCount=CNTBAR('numguard')\ncOne=PRMBAR('numguard',1)\ncTwo=PRMBAR('numguard',2)\ncCap=PRMBAR('numguard',2147483647)\ncState=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\nRELEASE POPUP numguard\nnCleanup=CNTBAR('numguard')\nUSE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\nFUNCTION GetPrompt\nnCalls=nCalls+1\nRETURN 'new'\nENDFUNC\n";
  write_text(path,source);auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
  const auto state=session.run(DebugResumeAction::continue_run);
  const auto tag=mode+" "+row.text;
  expect(state.completed,"DEFINE BAR guarded completion "+tag+": "+state.message);
  expect(value(state,"ncode")==std::string(id?"0":"167"),"DEFINE BAR code "+tag+" got "+value(state,"ncode"));
  expect(value(state,"lafter")==std::string(id?"true":"false"),"DEFINE BAR continuation "+tag);
  expect(value(state,"ncalls")==std::string(id?"1":"0"),"DEFINE BAR admission before one-time PROMPT "+tag);
  expect(value(state,"ncount")==std::string(!id||existing?"3":"4"),"DEFINE BAR sparse count "+tag);
  expect(value(state,"cone")==std::string(id&&*id==1?"new":"one"),"DEFINE BAR small alias1 "+tag);
  expect(value(state,"ctwo")==std::string(id&&*id==2?"new":"two"),"DEFINE BAR small alias2 "+tag);
  expect(value(state,"ccap")==std::string(id&&*id==INT32_MAX?"new":"cap"),"DEFINE BAR max replacement "+tag);
  expect(value(state,"cstate")=="1|"+mode+"|GUARD|1|guard","DEFINE BAR session/mode/cursor unchanged "+tag);
  expect(value(state,"ncleanup")=="0","DEFINE BAR release/reset "+tag);
  std::error_code ec;fs::remove_all(dir,ec);
 }
}
void session_policy_and_prompt_selection() {
 // Added AFTER original-dispatch baseline; mode is selected before the
 // ordinary inline PROMPT callback, whose selected data session is retained.
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const bool wide:{false,true}) {
  const bool valid=!wide||mode=="VFP9";
  const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
  const auto dir=fs::temp_directory_path()/"copperfin_define_bar_sessions_5611";
  fs::create_directories(dir);const auto path=dir/"sessions.prg";
  write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nDEFINE POPUP numguard\nDEFINE BAR 1 OF numguard PROMPT 'origin'\nDEFINE BAR 2147483647 OF numguard PROMPT 'cap'\nSET DATASESSION TO 2\nSET NUMERICBEHAVIOR TO "+other+"\nDEFINE POPUP numguard\nDEFINE BAR 1 OF numguard PROMPT 'other'\nSET DATASESSION TO 1\nnCode=0\nnCalls=0\nTRY\nDEFINE BAR "+(wide?"4294967297":"1.9")+" OF numguard PROMPT EVALUATE('SwitchPrompt()')\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\ncSelected=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')\nSET DATASESSION TO 1\ncOrigin=PRMBAR('numguard',1)+'|'+PRMBAR('numguard',2147483647)\nRELEASE POPUP numguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 2\ncTarget=PRMBAR('numguard',1)+'|'+PRMBAR('numguard',2147483647)\nRELEASE POPUP numguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nRETURN\nFUNCTION SwitchPrompt\nnCalls=nCalls+1\nSET DATASESSION TO 2\nRETURN 'new'\nENDFUNC\n");
  const auto state=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false)).run(DebugResumeAction::continue_run);
  expect(state.completed,"DEFINE BAR session callback completion "+mode);
  expect(value(state,"ncode")==std::string(valid?"0":"167")&&value(state,"ncalls")==std::string(valid?"1":"0"),"DEFINE BAR captured admission before callback "+mode);
  expect(value(state,"cselected")==std::string(valid?"2|":"1|")+(valid?other:mode),"DEFINE BAR preserves callback-selected session/mode");
  expect(value(state,"corigin")=="origin|cap","DEFINE BAR origin popup intact");
  expect(value(state,"ctarget")==std::string(valid?(wide?"other|new":"new|"):"other|"),"DEFINE BAR admitted ID stable across inline PROMPT policy change");
  std::error_code ec;fs::remove_all(dir,ec);
 }
}
void localized_metadata() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
 for(const std::string language:{"en-US","es-419","pt-BR","qps-ploc"})
 for(const std::string mode:{"COPPERFIN","VFP9"}) {
  set_env_value("COPPERFIN_LOCALE",language.c_str(),true);
  const auto dir=fs::temp_directory_path()/"copperfin_define_bar_locale_5611";
  fs::create_directories(dir);const auto path=dir/"locale.prg";
  write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nDEFINE POPUP numguard\nDEFINE BAR 1 OF numguard PROMPT 'guard'\nnCode=0\ncMessage=''\nlMetadata=.F.\nTRY\nDEFINE BAR 2147483648 OF numguard PROMPT 'new'\nCATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\n=AERROR(aError)\nlMetadata=ERROR()==167 AND aError[1,1]==167 AND aError[1,2]==cMessage\nENDTRY\ncGuard=PRMBAR('numguard',1)\nRELEASE POPUP numguard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
  const auto state=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false)).run(DebugResumeAction::continue_run);
  expect(state.completed&&value(state,"ncode")=="167"&&value(state,"lmetadata")=="true","DEFINE BAR localized167 metadata "+language);
  const auto message=value(state,"cmessage");
  const std::string expected=language=="es-419"?"DEFINE BAR: identificador de barra no valido":language=="pt-BR"?"DEFINE BAR: identificador de barra invalido":"DEFINE BAR: invalid bar identifier";
  expect(language=="qps-ploc"?message.starts_with("[!! ")&&message.ends_with(" !!]")&&message!=expected:message==expected,"DEFINE BAR localized message "+language+" got "+message);
  expect(value(state,"cguard")=="guard","DEFINE BAR locale failure atomicity");
  std::error_code ec;fs::remove_all(dir,ec);
 }
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();guarded_cases();localized_metadata();session_policy_and_prompt_selection();return test_failures()==0?0:1;
}
