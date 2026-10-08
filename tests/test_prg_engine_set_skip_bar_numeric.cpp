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
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-SET-SKIP-BAR-NUMERIC-001: independent native constants and parent
// safety policy, not output from the converter or correlated menu queries.
struct Row { const char* text; double raw; std::optional<int> ordinary, legacy; };
const std::vector<Row> rows{
 {"-3",-3,{},{}},{"-2.9",-2.9,{},-2},{"-2",-2,{},-2},
 {"-1",-1,{},-1},{"-0.9",-0.9,{},{}},{"0",0,{},{}},
 {"0.5",0.5,{},{}},{"0.9",0.9,{},{}},{"1",1,1,1},
 {"1.4",1.4,1,1},{"1.5",1.5,1,1},{"1.9",1.9,1,1},
 {"2",2,2,2},{"2.9",2.9,2,2},
 {"2147483647",2147483647.,INT32_MAX,INT32_MAX},
 {"2147483647.9",2147483647.9,INT32_MAX,INT32_MAX},
 {"2147483648",2147483648.,{},{}},{"2147483649",2147483649.,{},{}},
 {"4294967293",4294967293.,{},{}},{"4294967294",4294967294.,{},-2},
 {"4294967295",4294967295.,{},-1},{"4294967296",4294967296.,{},INT32_MAX},
 {"4294967297",4294967297.,{},INT32_MAX},
 {"-4294967295",-4294967295.,{},1},{"-4294967294",-4294967294.,{},2},
 {"-4294967296",-4294967296.,{},{}},
 {"1E20",1e20,{},INT32_MAX},{"1E300",1e300,{},INT32_MAX},
 {"-1E300",-1e300,{},{}}
};
std::string value(const RuntimePauseState& state, const char* name) {
 const auto found=state.globals.find(name);
 return found==state.globals.end()?"<missing>":format_value(found->second);
}
int expected_error(const std::optional<int> id) {
 return !id?167:0;
}
void direct_boundaries() {
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  for(const auto& row:rows)
   expect(checked_set_skip_bar_number_argument(row.raw,mode)==(mode==NumericBehavior::vfp9?row.legacy:row.ordinary),"SET SKIP OF BAR direct "+std::string(row.text));
  for(const auto number:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
   expect(!checked_set_skip_bar_number_argument(number,mode),"SET SKIP OF BAR nonfinite both modes");
  expect(!checked_set_skip_bar_number_argument(std::nextafter(1.,0.),mode),"SET SKIP OF BAR below1");
  expect(checked_set_skip_bar_number_argument(std::nextafter(2147483648.,0.),mode)==INT32_MAX,"SET SKIP OF BAR below2^31");
  expect(checked_set_skip_bar_number_argument(std::nextafter(4294967296.,0.),mode)==(mode==NumericBehavior::vfp9?std::optional<int>(-1):std::nullopt),"SET SKIP OF BAR below2^32 sentinel");
 }
}


std::string setup(bool target) {
 std::string text="PUBLIC nChoice,nCode,lAfter,nCount,cPrompts,cState,cSkip,cMark,nCleanup,cReset\n"
 "nChoice=0\nnCode=0\nlAfter=.F.\nnCleanup=-1\ncReset=''\n"
 "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
 "DEFINE POPUP parent RELATIVE\nDEFINE BAR 1 OF parent PROMPT 'one'\n"
 "DEFINE BAR 2 OF parent PROMPT 'two'\nDEFINE BAR 100 OF parent PROMPT 'finish'\n"
 "DEFINE BAR 4 OF parent PROMPT 'apply'\nDEFINE BAR 2147483647 OF parent PROMPT 'cap'\n"
 "ON SELECTION BAR 100 OF parent DO Finish\nON SELECTION BAR 4 OF parent DO Apply\n"
 "ON SELECTION BAR 1 OF parent nChoice=11\nON SELECTION BAR 2 OF parent nChoice=22\n"
 "ON SELECTION BAR 2147483647 OF parent nChoice=33\n"
 "SET MARK OF BAR 1 OF parent TO .T.\nSET MARK OF BAR 2147483647 OF parent TO .T.\n";
 for(const char* key:{"1","2","2147483647"})
  text+="SET SKIP OF BAR "+std::string(key)+" OF parent "+(target?".F.":".T.")+"\n";
 return text;
}
std::string command(const std::string& number,bool target) {
 return "SET SKIP OF BAR "+number+" OF parent "+(target?".T.":".F.")+"\n";
}
std::string callbacks(const std::string& number,bool target) {
 return "RETURN\nPROCEDURE Apply\nTRY\n"+command(number,target)+
 "lAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\n"
 "nCount=CNTBAR('parent')\ncPrompts=PRMBAR('parent',1)+'|'+PRMBAR('parent',2)+'|'+PRMBAR('parent',2147483647)\n"
 "cSkip=IIF(SKPBAR('parent',1),'.T.','.F.')+'|'+IIF(SKPBAR('parent',2),'.T.','.F.')+'|'+IIF(SKPBAR('parent',2147483647),'.T.','.F.')\n"
 "cMark=IIF(MRKBAR('parent',1),'.T.','.F.')+'|'+IIF(MRKBAR('parent',2),'.T.','.F.')+'|'+IIF(MRKBAR('parent',2147483647),'.T.','.F.')\n"
 "cState=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\nRETURN\nENDPROC\n"
 "PROCEDURE Finish\nDEACTIVATE POPUP parent\nRELEASE POPUP parent\n"
 "nCleanup=CNTBAR('parent')\nUSE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
 "SET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
 "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\nENDPROC\n";
}
void run_guard(const std::string& mode,const Row& row,int selected_session,bool target) {
 const auto id=mode=="VFP9"?row.legacy:row.ordinary; const int code=expected_error(id);
 const auto dir=fs::temp_directory_path()/"copperfin_set_skip_bar_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"skip.prg";
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto prefix=selected_session==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"";
 write_text(path,prefix+"SET NUMERICBEHAVIOR TO "+mode+"\n"+setup(target)+"ACTIVATE POPUP parent\n"+callbacks(row.text,target));
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 auto state=session.run(DebugResumeAction::continue_run);
 const auto tag=mode+" "+row.text+" session"+std::to_string(selected_session)+(target?" TRUE":" FALSE");
 expect(state.waiting_for_events,"skip guarded wait "+tag+": "+state.message);
 const auto count_events=[](const RuntimePauseState& s){return std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.set_skip";});};
 const auto before=count_events(state);
 expect(before==3,"skip seeded events "+tag);
 expect(session.dispatch_popup_bar_selection("parent",4),"skip apply callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(state.waiting_for_events,"skip apply returns to event loop "+tag);
 expect(value(state,"ncode")==std::to_string(code),"skip code "+tag+" got "+value(state,"ncode"));
 expect(value(state,"lafter")==std::string(code==0?"true":"false"),"skip continuation "+tag);
 expect(count_events(state)==before+(code==0?1:0),"skip failure-atomic event count "+tag);
 if(code==0) {
  const auto event=std::find_if(state.events.rbegin(),state.events.rend(),[](const auto& e){return e.category=="runtime.set_skip";});
  expect(event!=state.events.rend()&&event->detail=="popup=parent bar="+std::to_string(*id)+" disabled="+(target?"true":"false"),"skip converted telemetry "+tag);
 }
 expect(value(state,"ncount")=="5"&&value(state,"cprompts")=="one|two|cap","skip prompts/count guard "+tag);
 expect(value(state,"cstate")==std::to_string(selected_session)+"|"+mode+"|GUARD|1|guard","skip session/cursor guard "+tag);
 expect(value(state,"cmark")==".T.|.F.|.T.","skip mark guard "+tag+" got "+value(state,"cmark"));
 const std::vector<std::pair<int,int>> keys{{1,11},{2,22},{INT32_MAX,33}};
 std::string expected_flags;
 for(const auto& [key,marker]:keys) {
  const bool disabled=id&&*id==key?target:!target;
  if(!expected_flags.empty())expected_flags+="|";expected_flags+=disabled?".T.":".F.";
  for(int repeat=0;repeat<2;++repeat) {
   const bool dispatched=session.dispatch_popup_bar_selection("parent",key);
   expect(dispatched==!disabled,"skip independent selection key"+std::to_string(key)+" "+tag);
   if(dispatched) {state=session.run(DebugResumeAction::continue_run);
    expect(state.waiting_for_events&&value(state,"nchoice")==std::to_string(marker),"skip callback identity "+tag);}
  }
 }
 expect(value(state,"cskip")==expected_flags,"skip seeded query flags "+tag+" expected "+expected_flags+" got "+value(state,"cskip"));
 expect(!session.dispatch_popup_bar_selection("parent",-1)&&!session.dispatch_popup_bar_selection("parent",-2),"skip no negative user keys "+tag);
 expect(session.dispatch_popup_bar_selection("parent",100),"skip cleanup callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(value(state,"ncleanup")=="0"&&value(state,"creset")=="1|COPPERFIN|","skip explicit cleanup/reset "+tag);
 // #4630 callback-return lifecycle remains separate; reap only test-owned wait.
 session.request_cancel();(void)session.run(DebugResumeAction::continue_run);
 std::error_code ec;fs::remove_all(dir,ec);
}
void guarded_cases() {
 for(bool target:{false,true})for(const std::string mode:{"COPPERFIN","VFP9"}) {
  for(const auto& row:rows)run_guard(mode,row,1,target);
  for(const auto& row:{rows[22],rows[23]})run_guard(mode,row,2,target);
 }
}
void localized_metadata() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
 for(const std::string language:{"en-US","es-419","pt-BR","qps-ploc"})
 for(const std::string mode:{"COPPERFIN","VFP9"})for(bool zero:{false,true}) {
  set_env_value("COPPERFIN_LOCALE",language.c_str(),true);
  const auto dir=fs::temp_directory_path()/"copperfin_set_skip_bar_locale_5611";
  fs::create_directories(dir);const auto path=dir/"locale.prg";
  write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\n"+setup(true)+
   "cMessage=''\nlMetadata=.F.\nTRY\n"+command(zero?"0.5":"2147483648",true)+
   "CATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\n=AERROR(aError)\n"
   "lMetadata=ERROR()==167 AND aError[1,1]==167 AND aError[1,2]==cMessage\nENDTRY\n"
   "cGuard=PRMBAR('parent',1)+'|'+IIF(SKPBAR('parent',1),'.T.','.F.')\nRELEASE POPUP parent\nUSE IN guard\n"
   "SET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
  const auto state=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false)).run(DebugResumeAction::continue_run);
  expect(state.completed&&value(state,"ncode")=="167"&&value(state,"lmetadata")=="true","skip localized metadata "+language);
  const std::string expected=language=="es-419"?"SET SKIP OF BAR: identificador de barra no valido":
   language=="pt-BR"?"SET SKIP OF BAR: identificador de barra invalido":"SET SKIP OF BAR: invalid bar identifier";
  const auto message=value(state,"cmessage");
  expect(language=="qps-ploc"?message.starts_with("[!! ")&&message.ends_with(" !!]")&&message!=expected:message==expected,"skip localized message "+language+" got "+message);
  expect(value(state,"cguard")=="one|.F.","skip localized state guard");
  std::error_code ec;fs::remove_all(dir,ec);
 }
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();guarded_cases();localized_metadata();return test_failures()==0?0:1;
}
