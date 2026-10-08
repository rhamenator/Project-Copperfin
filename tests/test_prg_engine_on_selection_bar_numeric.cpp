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
// RQ-CF-PRG-ON-SELECTION-BAR-NUMERIC-001: independent native constants and parent
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
 return !id?167:*id<0?1604:0;
}
void direct_boundaries() {
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  for(const auto& row:rows)
   expect(checked_on_selection_bar_number_argument(row.raw,mode)==(mode==NumericBehavior::vfp9?row.legacy:row.ordinary),"ON SELECTION BAR direct "+std::string(row.text));
  for(const auto number:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
   expect(!checked_on_selection_bar_number_argument(number,mode),"ON SELECTION BAR nonfinite both modes");
  expect(!checked_on_selection_bar_number_argument(std::nextafter(1.,0.),mode),"ON SELECTION BAR below1");
  expect(checked_on_selection_bar_number_argument(std::nextafter(2147483648.,0.),mode)==INT32_MAX,"ON SELECTION BAR below2^31");
  expect(checked_on_selection_bar_number_argument(std::nextafter(4294967296.,0.),mode)==(mode==NumericBehavior::vfp9?std::optional<int>(-1):std::nullopt),"ON SELECTION BAR below2^32 sentinel");
 }
}

std::string setup(bool action) {
 std::string text =
 "PUBLIC nChoice,nCode,lAfter,nCount,cPrompts,cState,nCleanup,cReset\n"
 "nChoice=0\nnCode=0\nlAfter=.F.\nnCleanup=-1\ncReset=''\n"
 "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
 "DEFINE POPUP parent RELATIVE\nDEFINE BAR 1 OF parent PROMPT 'one'\n"
 "DEFINE BAR 2 OF parent PROMPT 'two'\nDEFINE BAR 100 OF parent PROMPT 'finish'\n"
 "DEFINE BAR 4 OF parent PROMPT 'apply'\nDEFINE BAR 2147483647 OF parent PROMPT 'cap'\n"
 "ON SELECTION BAR 100 OF parent DO Finish\nON SELECTION BAR 4 OF parent DO Apply\n";
 for(const auto& [key,marker]:std::vector<std::pair<std::string,std::string>>{{"1","11"},{"2","22"},{"2147483647","33"}})
  text+="ON SELECTION BAR "+key+" OF parent "+(action?"nChoice="+marker:"DO Stay"+marker)+"\n";
 return text;
}
std::string binding(const std::string& number,bool action) {
 return "ON SELECTION BAR "+number+" OF parent "+(action?"nChoice=99":"DO Changed")+"\n";
}
std::string callbacks(const std::string& number,bool action) {
 return "RETURN\nPROCEDURE Apply\nTRY\n"+binding(number,action)+
 "lAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\n"
 "nCount=CNTBAR('parent')\ncPrompts=PRMBAR('parent',1)+'|'+PRMBAR('parent',2)+'|'+PRMBAR('parent',2147483647)\n"
 "cState=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\nRETURN\nENDPROC\n"
 "PROCEDURE Changed\nnChoice=99\nRETURN\nENDPROC\n"
 "PROCEDURE Stay11\nnChoice=11\nRETURN\nENDPROC\n"
 "PROCEDURE Stay22\nnChoice=22\nRETURN\nENDPROC\n"
 "PROCEDURE Stay33\nnChoice=33\nRETURN\nENDPROC\n"
 "PROCEDURE Finish\nDEACTIVATE POPUP parent\nRELEASE POPUP parent\n"
 "nCleanup=CNTBAR('parent')\nUSE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
 "SET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
 "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\nENDPROC\n";
}
void run_guard(const std::string& mode,const Row& row,int selected_session,bool action) {
 const auto id=mode=="VFP9"?row.legacy:row.ordinary;
 const int code=expected_error(id);
 const auto dir=fs::temp_directory_path()/"copperfin_selection_bar_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"selection.prg";
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto prefix=selected_session==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"";
 write_text(path,prefix+"SET NUMERICBEHAVIOR TO "+mode+"\n"+setup(action)+"ACTIVATE POPUP parent\n"+callbacks(row.text,action));
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 auto state=session.run(DebugResumeAction::continue_run);
 const auto tag=mode+" "+row.text+" session"+std::to_string(selected_session)+(action?" ACTION":" DO");
 expect(state.waiting_for_events,"selection guarded wait "+tag+": "+state.message);
 expect(value(state,"nchoice")=="0","selection registration must not execute "+tag);
 const std::vector<std::pair<int,int>> keys{{1,11},{2,22},{INT32_MAX,33}};
 // Prime compiled action routines before reentrant replacement/rejection.
 for(const auto& [key,marker]:keys) {
  expect(session.dispatch_popup_bar_selection("parent",key),"selection prime dispatch "+tag);
  state=session.run(DebugResumeAction::continue_run);
  expect(state.waiting_for_events&&value(state,"nchoice")==std::to_string(marker),"selection prime identity "+tag);
 }
 expect(session.dispatch_popup_bar_selection("parent",4),"selection apply callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(state.waiting_for_events,"selection apply returns to existing event loop "+tag);
 expect(value(state,"ncode")==std::to_string(code),"selection code "+tag+" got "+value(state,"ncode"));
 expect(value(state,"lafter")==std::string(code==0?"true":"false"),"selection continuation "+tag);
 expect(value(state,"nchoice")=="33","selection replacement must not execute "+tag);
 expect(value(state,"ncount")=="5"&&value(state,"cprompts")=="one|two|cap","selection prompts/count guard "+tag);
 expect(value(state,"cstate")==std::to_string(selected_session)+"|"+mode+"|GUARD|1|guard","selection session/mode/cursor guard "+tag);
 // New action must replace a primed cache; rejection keeps prior routes.
 for(int repeat=0;repeat<2;++repeat)
 for(const auto& [key,marker]:keys) {
  const int expected=code==0&&id&&*id==key?99:marker;
  expect(session.dispatch_popup_bar_selection("parent",key),"selection guarded dispatch "+tag);
  state=session.run(DebugResumeAction::continue_run);
  expect(state.waiting_for_events&&value(state,"nchoice")==std::to_string(expected),"selection independent route key"+std::to_string(key)+" "+tag+" expected "+std::to_string(expected));
 }
 expect(!session.dispatch_popup_bar_selection("parent",-1)&&!session.dispatch_popup_bar_selection("parent",-2),"selection no negative keys "+tag);
 expect(session.dispatch_popup_bar_selection("parent",100),"selection cleanup callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(value(state,"ncleanup")=="0","selection explicit release "+tag);
 expect(value(state,"creset")=="1|COPPERFIN|","selection policy/cursor reset "+tag);
 // Callback-return/event-loop termination remains separate #4630 work.
 session.request_cancel();(void)session.run(DebugResumeAction::continue_run);
 std::error_code ec;fs::remove_all(dir,ec);
}
void guarded_cases() {
 for(bool action:{false,true})
 for(const std::string mode:{"COPPERFIN","VFP9"}) {
  for(const auto& row:rows)run_guard(mode,row,1,action);
  for(const auto& row:{rows[22],rows[23]})run_guard(mode,row,2,action);
 }
}
void localized_metadata() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
 for(const std::string language:{"en-US","es-419","pt-BR","qps-ploc"})
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(bool action:{false,true})
 for(bool sentinel:{false,true}) {
  set_env_value("COPPERFIN_LOCALE",language.c_str(),true);
  const bool lookup=sentinel&&mode=="VFP9";const int code=lookup?1604:167;
  const auto dir=fs::temp_directory_path()/"copperfin_selection_bar_locale_5611";
  fs::create_directories(dir);const auto path=dir/"locale.prg";
  write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\n"+setup(action)+
   "cMessage=''\nlMetadata=.F.\nTRY\n"+binding(sentinel?"4294967295":"2147483648",action)+
   "CATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\n=AERROR(aError)\n"
   "lMetadata=ERROR()=="+std::to_string(code)+" AND aError[1,1]=="+std::to_string(code)+" AND aError[1,2]==cMessage\nENDTRY\n"
   "cGuard=PRMBAR('parent',1)\nRELEASE POPUP parent\nUSE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
  const auto state=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false)).run(DebugResumeAction::continue_run);
  expect(state.completed&&value(state,"ncode")==std::to_string(code)&&value(state,"lmetadata")=="true","selection localized metadata "+language+(action?" ACTION":" DO"));
  const std::string expected=language=="es-419"?(lookup?"ON SELECTION BAR: no se encontro la barra":"ON SELECTION BAR: identificador de barra no valido"):
   language=="pt-BR"?(lookup?"ON SELECTION BAR: barra nao encontrada":"ON SELECTION BAR: identificador de barra invalido"):
   (lookup?"ON SELECTION BAR: bar not found":"ON SELECTION BAR: invalid bar identifier");
  const auto message=value(state,"cmessage");
  expect(language=="qps-ploc"?message.starts_with("[!! ")&&message.ends_with(" !!]")&&message!=expected:message==expected,"selection localized message "+language+" got "+message);
  expect(value(state,"cguard")=="one","selection localized prompt guard");
  std::error_code ec;fs::remove_all(dir,ec);
 }
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();guarded_cases();localized_metadata();return test_failures()==0?0:1;
}
