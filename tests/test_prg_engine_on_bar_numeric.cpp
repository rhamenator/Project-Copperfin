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
// RQ-CF-PRG-ON-BAR-NUMERIC-001: independent native constants and parent
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
 return !id?167:*id<0?1612:0;
}
void direct_boundaries() {
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  for(const auto& row:rows)
   expect(checked_on_bar_number_argument(row.raw,mode)==(mode==NumericBehavior::vfp9?row.legacy:row.ordinary),"ON BAR direct "+std::string(row.text));
  for(const auto number:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
   expect(!checked_on_bar_number_argument(number,mode),"ON BAR nonfinite both modes");
  expect(!checked_on_bar_number_argument(std::nextafter(1.,0.),mode),"ON BAR below1");
  expect(checked_on_bar_number_argument(std::nextafter(2147483648.,0.),mode)==INT32_MAX,"ON BAR below2^31");
  expect(checked_on_bar_number_argument(std::nextafter(4294967296.,0.),mode)==(mode==NumericBehavior::vfp9?std::optional<int>(-1):std::nullopt),"ON BAR below2^32 sentinel");
 }
}
std::string setup() {
 return "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
        "DEFINE POPUP parent RELATIVE\nDEFINE POPUP stay RELATIVE\nDEFINE POPUP changed RELATIVE\n"
        "DEFINE BAR 1 OF parent PROMPT 'one'\nDEFINE BAR 2 OF parent PROMPT 'two'\n"
        "DEFINE BAR 3 OF parent PROMPT 'finish'\nDEFINE BAR 2147483647 OF parent PROMPT 'cap'\n"
        "DEFINE BAR 1 OF stay PROMPT 'stay'\nDEFINE BAR 1 OF changed PROMPT 'changed'\n"
        "ON BAR 1 OF parent ACTIVATE POPUP stay\nON BAR 2 OF parent ACTIVATE POPUP stay\n"
        "ON BAR 2147483647 OF parent ACTIVATE POPUP stay\nON SELECTION BAR 3 OF parent DO Finish\n";
}
std::string attempt(const std::string& text) {
 return "PUBLIC nCleanup\nPUBLIC cReset\nnCleanup=-1\ncReset=''\nnCode=0\nlAfter=.F.\nTRY\nON BAR "+text+" OF parent ACTIVATE POPUP changed\nlAfter=.T.\n"
        "CATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\n"
        "nCount=CNTBAR('parent')\ncPrompts=PRMBAR('parent',1)+'|'+PRMBAR('parent',2)+'|'+PRMBAR('parent',2147483647)\n"
        "cState=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
        "ACTIVATE POPUP parent\nRETURN\nPROCEDURE Finish\nDEACTIVATE POPUP parent\n"
        "RELEASE POPUP parent\nRELEASE POPUP stay\nRELEASE POPUP changed\n"
        "nCleanup=CNTBAR('parent')\nUSE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\ncReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n"
        "ENDPROC\n";
}
void run_guard(const std::string& mode, const Row& row, const int selected_session) {
 const auto id=mode=="VFP9"?row.legacy:row.ordinary;
 const auto code=expected_error(id);
 const auto dir=fs::temp_directory_path()/"copperfin_on_bar_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"onbar.prg";
 // Session2's selected policy must win over the opposite session1 policy.
 const auto other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto prefix=selected_session==2?"SET NUMERICBEHAVIOR TO "+std::string(other)+"\nSET DATASESSION TO 2\n":"";
 write_text(path,prefix+"SET NUMERICBEHAVIOR TO "+mode+"\n"+setup()+attempt(row.text));
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 auto state=session.run(DebugResumeAction::continue_run);
 const auto tag=mode+" "+row.text+" session"+std::to_string(selected_session);
 expect(state.waiting_for_events,"ON BAR guarded wait "+tag+": "+state.message);
 expect(value(state,"ncode")==std::to_string(code),"ON BAR code "+tag+" got "+value(state,"ncode"));
 expect(value(state,"lafter")==std::string(code==0?"true":"false"),"ON BAR continuation "+tag);
 expect(value(state,"ncount")=="4"&&value(state,"cprompts")=="one|two|cap","ON BAR prompts/count unchanged "+tag);
 expect(value(state,"cstate")==std::to_string(selected_session)+"|"+mode+"|GUARD|1|guard","ON BAR session/mode/cursor unchanged "+tag);
 expect(std::count_if(state.events.begin(),state.events.end(),[](const RuntimeEvent& event){return event.category=="popup.activate";})==1,"ON BAR declaration must not activate a submenu "+tag);
 for(const int key:{1,2,INT32_MAX}) {
  expect(session.dispatch_popup_bar_selection("parent",key),"ON BAR guarded selection "+tag);
  state=session.run(DebugResumeAction::continue_run);
  const auto event=std::find_if(state.events.rbegin(),state.events.rend(),[](const RuntimeEvent& item){return item.category=="popup.activate";});
  const std::string target=code==0&&id&&*id==key?"changed":"stay";
  expect(state.waiting_for_events&&event!=state.events.rend()&&event->detail==target,"ON BAR independent route key"+std::to_string(key)+" "+tag+" expected "+target);
 }
 expect(!session.dispatch_popup_bar_selection("parent",-1)&&!session.dispatch_popup_bar_selection("parent",-2),"ON BAR no negative user-bar dispatch "+tag);
 expect(session.dispatch_popup_bar_selection("parent",3),"ON BAR cleanup callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(value(state,"ncleanup")=="0","ON BAR explicit callback release "+tag+" got "+value(state,"ncleanup")+": "+state.message);
 expect(value(state,"creset")=="1|COPPERFIN|","ON BAR reset state "+tag+" got "+value(state,"creset"));
 // Event-loop termination after callback is separate #4630 lifecycle work.
 // Explicitly reap this test-owned session after checking callback cleanup.
 session.request_cancel();(void)session.run(DebugResumeAction::continue_run);
 std::error_code ec;fs::remove_all(dir,ec);
}
void guarded_cases() {
 for(const std::string mode:{"COPPERFIN","VFP9"})
  for(const auto& row:rows)run_guard(mode,row,1);
 for(const std::string mode:{"COPPERFIN","VFP9"})
  for(const auto& row:{rows[22],rows[23]})run_guard(mode,row,2);
}
void localized_metadata() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
 for(const std::string language:{"en-US","es-419","pt-BR","qps-ploc"})
 for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const bool sentinel:{false,true}) {
  set_env_value("COPPERFIN_LOCALE",language.c_str(),true);
  const auto dir=fs::temp_directory_path()/"copperfin_on_bar_locale_5611";
  fs::create_directories(dir);const auto path=dir/"locale.prg";
  const bool lookup=sentinel&&mode=="VFP9";
  const int code=lookup?1612:167;
  write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\n"+setup()+"nCode=0\ncMessage=''\nlMetadata=.F.\nTRY\nON BAR "+(sentinel?"4294967295":"2147483648")+" OF parent ACTIVATE POPUP changed\nCATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\n=AERROR(aError)\nlMetadata=ERROR()=="+std::to_string(code)+" AND aError[1,1]=="+std::to_string(code)+" AND aError[1,2]==cMessage\nENDTRY\ncGuard=PRMBAR('parent',1)\nRELEASE POPUP parent\nRELEASE POPUP stay\nRELEASE POPUP changed\nUSE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n");
  const auto state=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false)).run(DebugResumeAction::continue_run);
  expect(state.completed&&value(state,"ncode")==std::to_string(code)&&value(state,"lmetadata")=="true","ON BAR localized metadata "+language+" code"+std::to_string(code));
  const std::string expected=language=="es-419"?(lookup?"ON BAR: no se encontro la barra":"ON BAR: identificador de barra no valido"):language=="pt-BR"?(lookup?"ON BAR: barra nao encontrada":"ON BAR: identificador de barra invalido"):(lookup?"ON BAR: bar not found":"ON BAR: invalid bar identifier");
  const auto message=value(state,"cmessage");
  expect(language=="qps-ploc"?message.starts_with("[!! ")&&message.ends_with(" !!]")&&message!=expected:message==expected,"ON BAR localized message "+language+" got "+message);
  expect(value(state,"cguard")=="one","ON BAR locale guard unchanged");
  std::error_code ec;fs::remove_all(dir,ec);
 }
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();guarded_cases();localized_metadata();return test_failures()==0?0:1;
}
